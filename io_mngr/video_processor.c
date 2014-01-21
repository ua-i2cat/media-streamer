/*
 *  video_processor.c - Implementation of the video stream processor.
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of media-streamer.
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
 *  Authors:  Jordi "Txor" Casas Ríos <txorlings@gmail.com>,
 *            David Cassany <david.cassany@i2cat.net>,
 *            Ignacio Contreras <ignacio.contreras@i2cat.net>,
 *            Marc Palau <marc.palau@i2cat.net>
 */

#include "video_processor.h"
#include "video_frame.h"
#include "video_codec.h"
#include "video_compress.h"
#include "video_decompress.h"
#include "video_decompress/libavcodec.h"
#include "video_config.h"
#include "types.h"
#include "module.h"
#include "tv.h"
#include "debug.h"

// Used to emphasize that the state is actually a proxy (from video_compress.c).
typedef struct compress_state compress_state_proxy;

// private functions
void *decoder_thread(void* data);
void *encoder_thread(void *arg);
static void *video_frame_init(void *init);
static void video_frame_destroy(void *destroy);
static unsigned int *get_media_time_ptr(void *frame);

void *decoder_thread(void* arg)
{
    video_processor_t *vp = (video_processor_t *) arg;

    video_frame2* coded_frame;
    video_frame2* decoded_frame;

    while(vp->run) {
        usleep(THREAD_SLEEP_TIMEOUT);

        if ((coded_frame = cq_get_front(vp->coded_cq)) != NULL) {

            while((decoded_frame = cq_get_rear(vp->decoded_cq)) == NULL) {
                cq_flush(vp->decoded_cq);
            }

            decompress_frame(vp->decompressor, decoded_frame->buffer, 
                    (unsigned char *)coded_frame->buffer, coded_frame->buffer_len, 0);

            decoded_frame->type = coded_frame->type; 
            decoded_frame->seqno = coded_frame->seqno; 

            cq_remove_bag(vp->coded_cq);
            decoded_frame->media_time = get_local_mediatime_us();
            cq_add_bag(vp->decoded_cq);
        }
    }

    pthread_exit((void *)NULL);    
}

void *encoder_thread(void *arg)
{
    video_processor_t *vp = (video_processor_t *)arg;

    video_frame2* decoded_frame;
    video_frame2* coded_frame;

    int index = 0;

    struct video_frame *enc_frame;

    module_init_default(&vp->module);

    compress_init(&vp->module, "libavcodec:codec=H.264", &vp->compressor);

    if ((enc_frame = vf_alloc(1)) == NULL) {
        error_msg("encoder_routine: vf_alloc error");
        //compress_done(&encoder->cs);
        vp->run = false; 
    }

    while (vp->run) {
        usleep(THREAD_SLEEP_TIMEOUT);

        if ((decoded_frame = cq_get_front(vp->decoded_cq)) != NULL) {

            while ((coded_frame = cq_get_rear(vp->coded_cq)) == NULL) {
                cq_flush(vp->coded_cq);
                debug_msg("Dropped coded frame in transmission");
                vp->lost_coded_frames++;
            }

            if (decoded_frame->width != vf_get_tile(enc_frame, 0)->width ||
                    decoded_frame->height != vf_get_tile(enc_frame, 0)->height) {
                vf_get_tile(enc_frame, 0)->width = decoded_frame->width;
                vf_get_tile(enc_frame, 0)->height = decoded_frame->height;
                enc_frame->color_spec = VIDEO_DEFAULT_PIXEL_FORMAT;
                enc_frame->interlacing = VIDEO_DEFAULT_INTERLACING;
                enc_frame->fps = VIDEO_DEFAULT_FPS;
            }

            enc_frame->tiles[0].data = (char *)decoded_frame->buffer;
            enc_frame->tiles[0].data_len = decoded_frame->buffer_len;

            struct video_frame *tx_frame;

            tx_frame = compress_frame((compress_state_proxy *)vp->compressor, enc_frame, index);

            coded_frame->buffer = (unsigned char *)vf_get_tile(tx_frame, 0)->data;
            coded_frame->buffer_len = vf_get_tile(tx_frame, 0)->data_len;

            coded_frame->seqno = decoded_frame->seqno;

            cq_remove_bag(vp->decoded_cq);
            coded_frame->media_time = get_local_mediatime_us();
            cq_add_bag(vp->coded_cq);

            index = (index + 1) % 2;
        }
    }

    module_done(CAST_MODULE(&vp->module));
    free(enc_frame);
    pthread_exit((void *)NULL);
}

static void *video_frame_init(void *init)
{
    struct video_desc *init_object = (struct video_desc *)init;
    video_frame2 *frame;

    if ((frame = rtp_video_frame2_init()) == NULL) {
        error_msg("video_frame_init: out of memory");
        return NULL;
    }

    rtp_video_frame2_allocate(frame,
            init_object->width,
            init_object->height,
            init_object->color_spec);

    // Stats
    frame->media_time = VIDEO_DEFAULT_MEDIA_TIME;
    frame->seqno = VIDEO_DEFAULT_SEQNO;

    // Control
    frame->type = VIDEO_DEFAULT_TYPE;

    return frame;
}

static void video_frame_destroy(void *destroy)
{
    video_frame2 * frame = (video_frame2 *)destroy; 
    rtp_video_frame2_free(frame);
}

static unsigned int *get_media_time_ptr(void *frame)
{
    video_frame2 *f = (video_frame2 *)frame;
    unsigned int *time = &f->media_time;
    return time;
}

video_processor_t *vp_init(role_t role)
{
    video_processor_t *vp;

    if ((vp = malloc(sizeof(video_processor_t))) == NULL) {
        error_msg("vp_init: malloc: out of memory!");
        return NULL;
    }

    if ((vp->external_config = malloc(sizeof(struct video_desc))) == NULL) {
        error_msg("vp_init: malloc: out of memory!");
        return NULL;
    }
    if ((vp->internal_config = malloc(sizeof(struct video_desc))) == NULL) {
        error_msg("vp_init: malloc: out of memory!");
        return NULL;
    }

    // Common values
    vp->role = role;
    vp->bitrate = 0;
    vp->lost_coded_frames = 0;
    vp->run = false;
    vp->thread = 0;

    // Default values
    vp->internal_config->width = VIDEO_DEFAULT_INTERNAL_WIDTH;
    vp->internal_config->height = VIDEO_DEFAULT_INTERNAL_HEIGHT;
    vp->internal_config->color_spec = VIDEO_DEFAULT_INTERNAL_CODEC;
    vp->internal_config->fps = VIDEO_DEFAULT_FPS;
    vp->internal_config->interlacing = VIDEO_DEFAULT_INTERLACING;
    vp->internal_config->tile_count = VIDEO_DEFAULT_TILE_COUNT;
    vp->external_config->width = VIDEO_DEFAULT_EXTERNAL_WIDTH;
    vp->external_config->height = VIDEO_DEFAULT_EXTERNAL_HEIGHT;
    vp->external_config->color_spec = VIDEO_DEFAULT_EXTERNAL_CODEC;
    vp->external_config->fps = VIDEO_DEFAULT_FPS;
    vp->external_config->interlacing = VIDEO_DEFAULT_INTERLACING;
    vp->external_config->tile_count = VIDEO_DEFAULT_TILE_COUNT;

    // Decoded circular queue
    vp->decoded_cq = cq_init(
            VIDEO_DECODED_CIRCULAR_QUEUE_SIZE,
            video_frame_init,
            vp->internal_config,
            video_frame_destroy,
            get_media_time_ptr);

    // Coded circular queue
    vp->coded_cq = cq_init(
            VIDEO_CODED_CIRCULAR_QUEUE_SIZE,
            video_frame_init,
            vp->external_config,
            video_frame_destroy,
            get_media_time_ptr);

    switch(vp->role) {
        case DECODER:
            vp->worker = decoder_thread;
            break;
        case ENCODER:
            vp->worker = encoder_thread;
            break;
        default:
            break;
    }

    return vp;
}

void vp_destroy(video_processor_t *vp)
{
    if (vp->run == true) {
        vp->run = false;
        pthread_join(vp->thread, NULL);
    }
    cq_destroy(vp->decoded_cq);
    cq_destroy(vp->coded_cq);
    free(vp->external_config);
    free(vp->internal_config);
    free(vp);
}

void vp_reconfig_internal(video_processor_t *vp,
        unsigned int width,
        unsigned int height,
        codec_t codec)
{
    // TODO protect access with locks
    if (width != vp->internal_config->width ||
            height != vp->internal_config->height) {
        vp->internal_config->width = width;
        vp->internal_config->height = height;
        vp->internal_config->color_spec = codec;
        cq_reconfig(vp->decoded_cq, vp->internal_config);
    }
}

void vp_reconfig_external(video_processor_t *vp,
        unsigned int width,
        unsigned int height,
        codec_t codec)
{
    // TODO protect access with locks
    if (vp->external_config->width != width ||
            vp->external_config->height != height) {
        vp->external_config->width = width;
        vp->external_config->height = height;
        vp->external_config->color_spec = codec;
    }
}

void vp_worker_start(video_processor_t *vp)
{
    video_frame2 *coded_frame;

    if (vp->run == false) {

        switch(vp->role) {
            case DECODER:

                initialize_video_decompress();

                if (decompress_is_available(LIBAVCODEC_MAGIC)) {
                    if((vp->decompressor = decompress_init(LIBAVCODEC_MAGIC)) == NULL) {
                        error_msg("decoder state decompress init failed");
                        decompress_done(vp->decompressor);
                        return;
                    }

                    if ((coded_frame = cq_get_rear(vp->coded_cq)) == NULL) {
                        return;
                    }

                    vp_reconfig_external(vp, coded_frame->width,
                            coded_frame->height,
                            coded_frame->codec);

                    if (!decompress_reconfigure(vp->decompressor, *vp->external_config,
                                16, 8, 0,
                                vc_get_linesize(vp->external_config->width, RGB),
                                RGB)) {
                        error_msg("decoder decompress reconfigure failed");
                        decompress_done(vp->decompressor);
                        vp_destroy(vp);
                        return;
                    }

                } else {
                    error_msg("decompress not available");
                    return;
                }
                break;

            case ENCODER:
                break;

            default:
                break;
        }

        vp->run = true;
        if (pthread_create(&vp->thread, NULL, vp->worker, (void *)vp) != 0)
            vp->run = false;
    }
}

