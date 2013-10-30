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

    int width = video->width;
    int height = video->height;

    // decoded_frame len and memory already initialized

    struct module cmod;
    module_init_default(&cmod);

    //encoder->cs = malloc(sizeof(struct compress_state));
    //assert(encoder->cs != NULL);

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
    frame->fps = DEFAULT_FPS; 
    frame->interlacing = PROGRESSIVE;

    encoder->run = TRUE; 
    encoder->index = 0;

    while (encoder->run) {
        sem_wait(&encoder->input_sem);
        if (!encoder->run) {
            break;
        }
       
        //pthread_rwlock_rdlock(&video->decoded_frame_lock);
        pthread_rwlock_wrlock(&video->decoded_frame_lock);
        
        frame->tiles[0].data = (char *)video->decoded_frame;
        frame->tiles[0].data_len = video->decoded_frame_len;
        struct video_frame *tx_frame;
        tx_frame = compress_frame(encoder->cs, frame, encoder->index);
        
        pthread_rwlock_unlock(&video->decoded_frame_lock);

        pthread_rwlock_wrlock(&video->coded_frame_lock);
        
        encoder->frame = tx_frame;
        video->coded_frame = (uint8_t *)vf_get_tile(tx_frame, 0)->data;
        video->coded_frame_len = vf_get_tile(tx_frame, 0)->data_len;

        pthread_rwlock_unlock(&video->coded_frame_lock);
        
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

    encoder->run = FALSE;
    
    // TODO assign the encoder here?
    data->encoder = encoder;

    int ret = 0;
    ret = pthread_create(&encoder->thread, NULL, encoder_routine, data);
    if (ret < 0) {
        error_msg("init_encoder: pthread_create error");
        pthread_mutex_destroy(&encoder->lock);
        sem_destroy(&encoder->input_sem);
        free(encoder);
        return NULL;
    }

    return encoder;
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
    
    pthread_cond_init(&decoder->notify_frame, NULL);
	
	if (decompress_is_available(LIBAVCODEC_MAGIC)) {
        //TODO: add some magic to determine codec
	    decoder->sd = decompress_init(LIBAVCODEC_MAGIC);
        if (decoder->sd == NULL) {
            error_msg("decoder state decompress init failed");
            decompress_done(decoder->sd);
            free(decoder);
            return NULL;
        }

        pthread_rwlock_rdlock(&data->lock);
        des.width = data->width;
        des.height = data->height;
        des.color_spec  = data->codec;
        des.tile_count = 0;
        des.interlacing = data->interlacing;
        des.fps = data->fps; // TODO XXX
        pthread_rwlock_unlock(&data->lock);

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
        
        pthread_mutex_lock(&v_data->new_coded_frame_lock);
        while(!v_data->new_coded_frame && v_data->decoder->run){
            pthread_cond_wait(&v_data->decoder->notify_frame, &v_data->new_coded_frame_lock); //TODO: some timeout
        }
        v_data->new_coded_frame = FALSE;
        pthread_mutex_unlock(&v_data->new_coded_frame_lock);

        if (v_data->decoder->run){
            pthread_rwlock_rdlock(&v_data->coded_frame_lock);
            pthread_rwlock_wrlock(&v_data->decoded_frame_lock);
            decompress_frame(v_data->decoder->sd,(unsigned char *)v_data->decoded_frame, 
                (unsigned char *)v_data->coded_frame, v_data->coded_frame_len, 0);
            v_data->decoder->last_seqno = v_data->coded_frame_seqno; 
            v_data->decoded_frame_seqno ++;
            pthread_rwlock_unlock(&v_data->decoded_frame_lock);
            pthread_rwlock_unlock(&v_data->coded_frame_lock);

            pthread_mutex_lock(&v_data->new_decoded_frame_lock);
            v_data->new_decoded_frame = TRUE;
            pthread_mutex_unlock(&v_data->new_decoded_frame_lock);
        }
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

    pthread_cond_destroy(&decoder->notify_frame);
    decompress_done(decoder->sd);
    free(decoder);
}

void stop_decoder(video_data_t *data){
    if (data->decoder == NULL)
        return;

    data->decoder->run = FALSE;

    pthread_mutex_lock(&data->new_coded_frame_lock);
    data->new_coded_frame = TRUE;
    pthread_cond_signal(&data->decoder->notify_frame);
    pthread_mutex_unlock(&data->new_coded_frame_lock);

    pthread_join(data->decoder->thread, NULL);

    destroy_decoder(data->decoder);
}

video_data_t *init_video_data(video_type_t type){
    video_data_t *data = malloc(sizeof(video_data_t));

    data->coded_frame = malloc(1920*1080*4*sizeof(char)); //TODO this is the worst case in raw data, should be the worst case for specific codec
    data->width = 0;
    data->height = 0;
    data->decoded_frame_len = 0;
    data->decoded_frame = NULL;
    data->decoded_frame_seqno = 0;
    data->coded_frame_seqno = 0;
    data->type = type;

    // locks initialization
    if (pthread_rwlock_init(&data->lock, NULL) < 0) {
        error_msg("set_stream_video_data: pthread_rwlock_init error");
        free(data->coded_frame);
        return NULL;
    }
    if (pthread_mutex_init(&data->new_decoded_frame_lock, NULL) < 0) {
        error_msg("set_stream_video_data: pthread_mutex_init error");
        pthread_rwlock_destroy(&data->lock);
        free(data->coded_frame);
        return NULL;
    }
    if (pthread_mutex_init(&data->new_coded_frame_lock, NULL) < 0) {
        error_msg("set_stream_video_data: pthread_mutex_init error");
        pthread_mutex_destroy(&data->new_decoded_frame_lock);
        pthread_rwlock_destroy(&data->lock);
        free(data->coded_frame);
        return NULL;
    }
    if (pthread_rwlock_init(&data->decoded_frame_lock, NULL) < 0) {
        error_msg("set_stream_video_data: pthread_rwlock_init error");
        pthread_mutex_destroy(&data->new_decoded_frame_lock);
        pthread_mutex_destroy(&data->new_coded_frame_lock);
        pthread_rwlock_destroy(&data->lock);
        free(data->coded_frame);
        return NULL;
    }
    if (pthread_rwlock_init(&data->coded_frame_lock, NULL) < 0) {
        error_msg("set_stream_video_data: pthread_rwlock_init error");
        pthread_mutex_destroy(&data->new_decoded_frame_lock);
        pthread_mutex_destroy(&data->new_coded_frame_lock);
        pthread_rwlock_destroy(&data->decoded_frame_lock);
        pthread_rwlock_destroy(&data->lock);
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
        //destroy_encoder(stream->encoder);
    }

    free(data->decoded_frame);
    free(data->coded_frame);
    pthread_mutex_destroy(&data->new_coded_frame_lock);
    pthread_mutex_destroy(&data->new_decoded_frame_lock);
    pthread_rwlock_unlock(&data->lock);
    pthread_rwlock_destroy(&data->lock);
    free(data);

    return TRUE;
}

int set_video_data(video_data_t *data, codec_t codec, uint32_t width, uint32_t height){
    pthread_rwlock_wrlock(&data->lock);
    data->codec = codec;
    data->width = width;
    data->height = height;
    data->decoded_frame_len = vc_get_linesize(width, RGB)*height;

    if (data->decoded_frame == NULL){
        data->decoded_frame = malloc(data->decoded_frame_len);
    } else {
        free(data->decoded_frame);
        data->decoded_frame = malloc(data->decoded_frame_len);
    }

    if (data->decoded_frame == NULL) {
        error_msg("set_stream_video_data: malloc error");
        pthread_rwlock_unlock(&data->lock);
        return FALSE;
    }

    if (data->type == DECODER){
        if (data->decoder == NULL){
            start_decoder(data);
        } else {
            //TODO: reconfigure decoder
        }
    }else if (data->type == ENCODER){

    }

    pthread_rwlock_unlock(&data->lock);
    return TRUE;
}
