#include <stdlib.h>
#include "video_data.h"
#include "video_decompress.h"
#include "video_decompress/libavcodec.h"
#include "video_codec.h"
#include "video_compress.h"
#include "video_frame.h"
#include "module.h"
#include "debug.h"

#define PIXEL_FORMAT RGB
#define DEFAULT_FPS 25

// private functions
void *decoder_th(void* data);
void *encoder_routine(void *arg);

int reconf_video_frame(video_data_frame_t *frame, struct video_frame *enc_frame, uint32_t fps){
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

void *encoder_routine(void *arg)
{
    video_data_t *video = (video_data_t *)arg;
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

encoder_thread_t *init_encoder(video_data_t *data)
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

void destroy_encoder(video_data_t *data)
{
    pthread_join(data->encoder->thread, NULL);
    free(data->encoder);
}

void stop_encoder(video_data_t *data)
{
    data->encoder->run = FALSE;
    destroy_encoder(data);
}


decoder_thread_t *init_decoder(video_data_t *data){
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

void *decoder_th(void* data){
    video_data_t *v_data = (video_data_t *) data;
    
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

void start_decoder(video_data_t *v_data){
    assert(v_data->decoder == NULL);
  
    v_data->decoder = init_decoder(v_data);
    v_data->decoder->run = TRUE;
     
    if (pthread_create(&v_data->decoder->thread, NULL, (void *) decoder_th, v_data) != 0)
        v_data->decoder->run = FALSE;
}

void destroy_decoder(decoder_thread_t *decoder){
    if (decoder == NULL) {
        return;
    }

    decompress_done(decoder->sd);
    free(decoder);
}

void stop_decoder(video_data_t *data){
    if (data->decoder == NULL)
        return;

    data->decoder->run = FALSE;

    pthread_join(data->decoder->thread, NULL);

    destroy_decoder(data->decoder);
}

video_data_t *init_video_data(video_type_t type, float fps){
    video_data_t *data = malloc(sizeof(video_data_t));

    //TODO: get rid of magic numbers
    data->decoded_frames = init_video_frame_cq(2);
    data->coded_frames = init_video_frame_cq(2);
    data->type = type;
    data->fps = fps;
    data->bitrate = 0;
    data->decoder = NULL; //As decoder and encoder are union, this is valid for both
    data->seqno = 0; 
    data->lost_coded_frames = 0;


    return data;
}

int destroy_video_data(video_data_t *data){

    if (data->type == DECODER && data->decoder != NULL){
        stop_decoder(data);
    } else if (data->type == ENCODER && data->encoder != NULL){
        stop_encoder(data);
    }

    if (destroy_video_frame_cq(data->decoded_frames) != TRUE){
        return FALSE;
    }

    if (destroy_video_frame_cq(data->coded_frames) != TRUE){
        return FALSE;
    }

    free(data);

    return TRUE;
}
