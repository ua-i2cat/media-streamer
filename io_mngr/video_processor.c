/*
 *  video_processor.c - Video stream processor
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of io_mngr.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:  Jordi "Txor" Casas Ríos <jordi.casas@i2cat.net>,
 *            David Cassany <david.cassany@i2cat.net>,
 *            Ignacio Contreras <ignacio.contreras@i2cat.net>,
 *            Marc Palau <marc.palau@i2cat.net>
 */

#include <stdlib.h>
#include "video_processor.h"
#include "video_decompress.h"
#include "video_decompress/libavcodec.h"
#include "video_codec.h"
#include "video_compress.h"
#include "video_frame.h"
#include "video_config.h"
#include "tv.h"
#include "module.h"
#include "debug.h"

// private functions
void *decoder_thread(void* data);
void *encoder_thread(void *arg);
static void *bag_init(void *init);
static void bag_destroy(void *bag);
static int *get_media_time_ptr(void *bag);

void *decoder_thread(void* data)
{
    video_processor_t *v_data = (video_processor_t *) data;

    video_data_frame_t* coded_frame;
    video_data_frame_t* decoded_frame;

    while(v_data->decoder->run){
        usleep(100);

        if (!v_data->decoder->run){
            break;
        }

        coded_frame = curr_out_frame(v_data->coded_frames);        
        if (coded_frame == NULL){
            continue;
        }

        decoded_frame = curr_in_frame(v_data->decoded_frames);
        while (decoded_frame == NULL){
            flush_frames(v_data->decoded_frames);
            decoded_frame = curr_in_frame(v_data->decoded_frames);
        }

        decompress_frame(v_data->decoder->sd, decoded_frame->buffer, 
                (unsigned char *)coded_frame->buffer, coded_frame->buffer_len, 0);

        decoded_frame->seqno = coded_frame->seqno; 

        remove_frame(v_data->coded_frames);
        decoded_frame->media_time = get_local_mediatime_us();
        put_frame(v_data->decoded_frames);
    }

    pthread_exit((void *)NULL);    
}

void *encoder_thread(void *arg)
{
    video_processor_t *video = (video_processor_t *)arg;
    encoder_thread_t *encoder = video->encoder;
    video_data_frame_t* decoded_frame;
    video_data_frame_t* coded_frame;
    struct video_frame *enc_frame;

    // decoded_frame len and memory already initialized

    struct module cmod;
    module_init_default(&cmod);

    assert(encoder != NULL);
    compress_init(&cmod, "libavcodec:codec=H.264", &encoder->cs);

    enc_frame = vf_alloc(1);
    if (enc_frame == NULL) {
        error_msg("encoder_routine: vf_alloc error");
        //compress_done(&encoder->cs);
        pthread_exit((void *)NULL);
    }

    encoder->run = TRUE; 
    encoder->index = 0;

    while (encoder->run) {

        if (!encoder->run) {
            break;
        }

        //TODO: set a non magic number in this usleep (maybe related to framerate)
        usleep(100);

        decoded_frame = curr_out_frame(video->decoded_frames);
        if (decoded_frame == NULL){
            continue;
        }

        coded_frame = curr_in_frame(video->coded_frames);
        while (coded_frame == NULL){
            flush_frames(video->coded_frames);
            error_msg("Warning! Discarting coded frame in transmission\n");
            video->lost_coded_frames++;
            coded_frame = curr_in_frame(video->coded_frames);
        }

        reconf_video_frame(decoded_frame, enc_frame, video->fps);

        enc_frame->tiles[0].data = (char *)decoded_frame->buffer;
        enc_frame->tiles[0].data_len = decoded_frame->buffer_len;

        struct video_frame *tx_frame;

        tx_frame = compress_frame(encoder->cs, enc_frame, encoder->index);

        encoder->frame = tx_frame;
        coded_frame->buffer = (uint8_t *)vf_get_tile(tx_frame, 0)->data;
        coded_frame->buffer_len = vf_get_tile(tx_frame, 0)->data_len;

        coded_frame->seqno = decoded_frame->seqno;

        remove_frame(video->decoded_frames);
        coded_frame->media_time = get_local_mediatime_us();
        put_frame(video->coded_frames);

        encoder->index = (encoder->index + 1) % 2;
    }

    module_done(CAST_MODULE(&cmod));
    free(enc_frame);
    pthread_exit((void *)NULL);
}

// int set_video_data_frame(video_data_frame_t *frame, codec_t codec, uint32_t width, uint32_t height)
static void *bag_init(void *init)
{
    frame->codec = codec;
    frame->width = width;
    frame->height = height;

    if (width == 0 || height == 0){
        width = MAX_WIDTH;
        height = MAX_HEIGHT;
    }

    frame->buffer_len = width*height*3; 
    frame->buffer = realloc(frame->buffer, frame->buffer_len);

    if (frame->buffer == NULL) {
        error_msg("set_stream_video_data: malloc error");
        return FALSE;
    }

    return TRUE;
}

//int destroy_video_data_frame(video_data_frame_t *frame)
static void bag_destroy(void *bag)
{
    free(frame->buffer);
    free(frame);
    return TRUE;
}

static int *get_media_time_ptr(void *bag)
{
    return &bag->media_time;
}

//video_processor_t *init_video_data(role_t type, float fps)
video_processor_t *vp_init(role_t type)
{
    video_processor_t *vp = malloc(sizeof(video_processor_t));

    //TODO: get rid of magic numbers
    vp->decoded_cq = cq_init(
            VIDEO_CIRCULAR_QUEUE_SIZE,
            bag_init,
            &init_data,
            bag_destroy,
            get_media_time_ptr);
    vp->coded_cq = cq_init(
            VIDEO_CIRCULAR_QUEUE_SIZE,
            bag_init,
            &init_data,
            bag_destroy,
            get_media_time_ptr);
    vp->type = type;
    vp->fps = fps;
    vp->bitrate = 0;
    vp->decoder = NULL; //As decoder and encoder are union, this is valid for both
    vp->seqno = 0; 
    vp->lost_coded_frames = 0;


    return vp;
}

decoder_thread_t *init_decoder(video_processor_t *data)
{
    decoder_thread_t *decoder;
    struct video_desc des;
    video_data_frame_t *coded_frame;

    initialize_video_decompress();

    decoder = malloc(sizeof(decoder_thread_t));
    if (decoder == NULL) {
        error_msg("decoder malloc failed");
        return NULL;
    }

    decoder->run = FALSE;

    if (decompress_is_available(LIBAVCODEC_MAGIC)) {
        //TODO: add some magic to determine codec
        decoder->sd = decompress_init(LIBAVCODEC_MAGIC);
        if (decoder->sd == NULL) {
            error_msg("decoder state decompress init failed");
            decompress_done(decoder->sd);
            free(decoder);
            return NULL;
        }

        coded_frame = curr_in_frame(data->coded_frames);
        if (coded_frame == NULL){
            return NULL;
        }

        des.width = coded_frame->width;
        des.height = coded_frame->height;
        des.color_spec  = coded_frame->codec;
        des.tile_count = 0;
        des.interlacing = data->interlacing;
        des.fps = data->fps; // TODO XXX

        if (!decompress_reconfigure(decoder->sd, des, 16, 8, 0, vc_get_linesize(des.width, RGB), RGB)) {
            error_msg("decoder decompress reconfigure failed");
            decompress_done(decoder->sd);
            free(decoder->sd);
            free(decoder);
            return NULL;
        }

    } else {
        error_msg("decompress not available");
        free(decoder);
        return NULL;
    }

    return decoder;
}



encoder_thread_t *init_encoder(video_processor_t *data)
{
    if (data->encoder != NULL) {
        debug_msg("init_encoder: encoder already initialized");
        return NULL;
    }

    encoder_thread_t *encoder = malloc(sizeof(encoder_thread_t));
    if (encoder == NULL) {
        error_msg("init_encoder: malloc error");
        return NULL;
    }

    encoder->run = FALSE;

    // TODO assign the encoder here?
    data->encoder = encoder;

    int ret = 0;
    ret = pthread_create(&encoder->thread, NULL, encoder_routine, data);
    if (ret < 0) {
        error_msg("init_encoder: pthread_create error");
        free(encoder);
        return NULL;
    }

    return encoder;
}


//int destroy_video_data(video_processor_t *data)
int vp_destroy(video_processor_t *vp)
{
    if (vp->type == DECODER && vp->decoder != NULL){
        stop_decoder(vp);
    } else if (vp->type == ENCODER && vp->encoder != NULL){
        stop_encoder(vp);
    }

    if (destroy_video_frame_cq(vp->decoded_frames) != TRUE){
        return FALSE;
    }

    if (destroy_video_frame_cq(vp->coded_frames) != TRUE){
        return FALSE;
    }

    free(vp);

    return TRUE;
}

//int reconf_video_frame(video_data_frame_t *frame, struct video_frame *enc_frame, uint32_t fps)
void vp_config(video_data_frame_t *frame, struct video_frame *enc_frame, uint32_t fps)
{
    if (frame->width != vf_get_tile(enc_frame, 0)->width
            || frame->height != vf_get_tile(enc_frame, 0)->height) {
        vf_get_tile(enc_frame, 0)->width = frame->width;
        vf_get_tile(enc_frame, 0)->height = frame->height;
        enc_frame->color_spec = PIXEL_FORMAT;
        enc_frame->interlacing = PROGRESSIVE;
        // TODO: set default fps value. If it's not set -> core dump
        enc_frame->fps = fps;
        return TRUE;
    }
    return FALSE;
}

void vp_destroy(audio_processor_t *vp)
{
    vp->run = FALSE;
    pthread_join(vp->thread, NULL);
    cq_destroy(vp->decoded_cq);
    cq_destroy(vp->coded_cq);
    free(vp->external_config);
    free(vp->internal_config);
    free(vp);
}

void vp_worker_start(audio_processor_t *vp)
{
    vp->run = TRUE;
    if (pthread_create(&vp->thread, NULL, vp->worker, (void *)vp) != 0)
        vp->run = FALSE;
}

