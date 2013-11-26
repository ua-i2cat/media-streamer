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

void *encoder_routine(void *arg)
{
    video_data_t *video = (video_data_t *)arg;
    encoder_thread_t *encoder = video->encoder;

    int width = video->decoded_frame->width;
    int height = video->decoded_frame->height;

    // decoded_frame len and memory already initialized

    struct module cmod;
    module_init_default(&cmod);

    assert(encoder != NULL);
    compress_init(&cmod, "libavcodec:codec=H.264", &encoder->cs);

    struct video_frame *frame = vf_alloc(1);
    if (frame == NULL) {
        error_msg("encoder_routine: vf_alloc error");
        //compress_done(&encoder->cs);
        pthread_exit((void *)NULL);
    }

    vf_get_tile(frame, 0)->width = width;
    vf_get_tile(frame, 0)->height = height;
    frame->color_spec = PIXEL_FORMAT;
    // TODO: set default fps value. If it's not set -> core dump
    frame->fps = video->fps; 
    frame->interlacing = PROGRESSIVE;

    encoder->run = TRUE; 
    encoder->index = 0;

    sem_wait(&encoder->input_sem);
   
    while (encoder->run) {
        
        //TODO: set a non magic number in this usleep (maybe related to framerate)
        usleep(100);
                
        if (video->new_decoded_frame == FALSE){
            continue;
        }
        
        if (!encoder->run) {
            break;
        }

        video->new_decoded_frame = FALSE;

        pthread_rwlock_rdlock(&video->decoded_frame->lock);
        frame->tiles[0].data = (char *)video->decoded_frame->buffer;
        frame->tiles[0].data_len = video->decoded_frame->buffer_len;
        pthread_rwlock_unlock(&video->decoded_frame->lock);
        
        struct video_frame *tx_frame;
        
        video->decoded_frame->media_time = get_local_mediatime();
        tx_frame = compress_frame(encoder->cs, frame, encoder->index);
        video->coded_frame->media_time = get_local_mediatime();
        
        pthread_rwlock_wrlock(&video->coded_frame->lock);
        encoder->frame = tx_frame;
        video->coded_frame->buffer = (uint8_t *)vf_get_tile(tx_frame, 0)->data;
        video->coded_frame->buffer_len = vf_get_tile(tx_frame, 0)->data_len;
        video->coded_frame->seqno++;
        pthread_rwlock_unlock(&video->coded_frame->lock);
       
        pthread_mutex_lock(&encoder->output_lock);
        pthread_cond_broadcast(&encoder->output_cond);
        pthread_mutex_unlock(&encoder->output_lock);
        encoder->index = (encoder->index + 1) % 2;
        
    }

    module_done(CAST_MODULE(&cmod));
    free(frame);
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

    if (pthread_mutex_init(&encoder->lock, NULL) < 0) {
        error_msg("init_encoder: pthread_mutex_init error");
        free(encoder);
        return NULL;
    }

    if (sem_init(&encoder->input_sem, 1, 0) < 0) {
        error_msg("init_encoder: sem_init error");
        pthread_mutex_destroy(&encoder->lock);
        free(encoder);
        return NULL;
    }
    
    if (pthread_mutex_init(&encoder->output_lock, NULL) < 0) {
        error_msg("init_encoder: pthread_mutex_init error");
        pthread_mutex_destroy(&encoder->lock);
        sem_destroy(&encoder->input_sem);
        free(encoder);
        return NULL;
    }

    if (pthread_cond_init(&encoder->output_cond, NULL) < 0) {
        error_msg("init_encoder: pthread_cond_init error");
        pthread_mutex_destroy(&encoder->lock);
        sem_destroy(&encoder->input_sem);
        pthread_mutex_destroy(&encoder->output_lock);
        free(encoder);
        return NULL;
    }

    encoder->run = FALSE;
    
    // TODO assign the encoder here?
    data->encoder = encoder;

    int ret = 0;
    ret = pthread_create(&encoder->thread, NULL, encoder_routine, data);
    if (ret < 0) {
        error_msg("init_encoder: pthread_create error");
        pthread_mutex_destroy(&encoder->lock);
        sem_destroy(&encoder->input_sem);
        pthread_mutex_destroy(&encoder->output_lock);
        pthread_cond_destroy(&encoder->output_cond);
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
    pthread_mutex_lock(&data->encoder->lock);
    data->encoder->run = FALSE;
    pthread_mutex_unlock(&data->encoder->lock);
    destroy_encoder(data);
}


decoder_thread_t *init_decoder(video_data_t *data){
	decoder_thread_t *decoder;
	struct video_desc des;

	initialize_video_decompress();
	
	decoder = malloc(sizeof(decoder_thread_t));
    if (decoder == NULL) {
        error_msg("decoder malloc failed");
        return NULL;
    }

	decoder->run = FALSE;
    decoder->last_seqno = 0;
    
	if (decompress_is_available(LIBAVCODEC_MAGIC)) {
        //TODO: add some magic to determine codec
	    decoder->sd = decompress_init(LIBAVCODEC_MAGIC);
        if (decoder->sd == NULL) {
            error_msg("decoder state decompress init failed");
            decompress_done(decoder->sd);
            free(decoder);
            return NULL;
        }

        des.width = data->coded_frame->width;
        des.height = data->coded_frame->height;
        des.color_spec  = data->coded_frame->codec;
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

    while(v_data->decoder->run){
        
        usleep(100);
        
        if (v_data->new_coded_frame == FALSE){
            continue;
        }
        
        if (!v_data->decoder->run){
            break;
        }

        v_data->new_coded_frame = FALSE;
        
        pthread_rwlock_rdlock(&v_data->coded_frame->lock);
        pthread_rwlock_wrlock(&v_data->decoded_frame->lock);
            
        v_data->coded_frame->media_time = get_local_mediatime();
        decompress_frame(v_data->decoder->sd,(unsigned char *)v_data->decoded_frame->buffer, 
            (unsigned char *)v_data->coded_frame->buffer, v_data->coded_frame->buffer_len, 0);
        v_data->decoded_frame->media_time = get_local_mediatime();

        v_data->decoded_frame->seqno ++;

        pthread_rwlock_unlock(&v_data->decoded_frame->lock);
        pthread_rwlock_unlock(&v_data->coded_frame->lock);

        v_data->new_decoded_frame = TRUE;
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

    data->new_coded_frame = TRUE;

    pthread_join(data->decoder->thread, NULL);

    destroy_decoder(data->decoder);
}

video_data_t *init_video_data(video_type_t type){
    video_data_t *data = malloc(sizeof(video_data_t));

    data->decoded_frame = init_video_data_frame();
    data->coded_frame = init_video_data_frame();
    data->type = type;
    data->fps = 25;
    data->new_coded_frame = FALSE;
    data->new_decoded_frame = FALSE;
    data->decoder = NULL; //As decoder and encoder are union, this is valid for both

    // locks initialization
    if (pthread_rwlock_init(&data->lock, NULL) < 0) {
        error_msg("set_stream_video_data: pthread_rwlock_init error");
        free(data->coded_frame);
        return NULL;
    }

    return data;
}

int destroy_video_data(video_data_t *data){
    pthread_rwlock_wrlock(&data->lock);

    if (data->type == DECODER && data->decoder != NULL){
        stop_decoder(data);
    } else if (data->type == ENCODER && data->encoder != NULL){
        stop_encoder(data);
    }

    if (destroy_video_data_frame(data->decoded_frame) != TRUE){
        return FALSE;
    }

    if (destroy_video_data_frame(data->coded_frame) != TRUE){
        return FALSE;
    }

    pthread_rwlock_destroy(&data->decoded_frame->lock);
    pthread_rwlock_destroy(&data->coded_frame->lock);

    pthread_rwlock_unlock(&data->lock);
    pthread_rwlock_destroy(&data->lock);
    free(data);

    return TRUE;
}
