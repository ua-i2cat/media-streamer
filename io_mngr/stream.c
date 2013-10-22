#include "stream.h"
#include "video_decompress.h"
#include "video_decompress/libavcodec.h"
#include "video_codec.h"
#include "video_compress.h"
#include "video_frame.h"
#include "module.h"
#include "debug.h"
#include <stdlib.h>

#define DEFAULT_FPS 24
#define PIXEL_FORMAT RGB

void *decoder_th(void* stream);

// private functions
void *encoder_routine(void *arg);


// Private functions
void *encoder_routine(void *arg);


decoder_thread_t *init_decoder(stream_data_t *stream)
{
    pthread_rwlock_rdlock(&stream->lock);

	decoder_thread_t *decoder;
	struct video_desc des;

	initialize_video_decompress();
	
	decoder = malloc(sizeof(decoder_thread_t));
    if (decoder == NULL) {
        error_msg("decoder malloc failed");
        pthread_rwlock_unlock(&stream->lock);
        return NULL;
    }
	decoder->new_frame = FALSE;
	decoder->run = FALSE;
	
	pthread_mutex_init(&decoder->lock, NULL);
	pthread_cond_init(&decoder->notify_frame, NULL);
	
	if (decompress_is_available(LIBAVCODEC_MAGIC)) {
        //TODO: add some magic to determine codec
	    decoder->sd = decompress_init(LIBAVCODEC_MAGIC);
        if (decoder->sd == NULL) {
            error_msg("decoder state decompress init failed");
            decompress_done(decoder->sd);
            free(decoder);
            pthread_rwlock_unlock(&stream->lock);
            return NULL;
        }

        des.width = stream->video.width;
        des.height = stream->video.height;
        des.color_spec  = stream->video.codec;
        des.tile_count = 0;
        des.interlacing = PROGRESSIVE;
        des.fps = 24; // TODO XXX

        if (decompress_reconfigure(decoder->sd, des, 16, 8, 0, vc_get_linesize(des.width, RGB), RGB)) {
            decoder->coded_frame = malloc(1920*1080*4*sizeof(char)); //TODO this is the worst case in raw data, should be the worst case for specific codec
            if (decoder->coded_frame == NULL) {
                error_msg("decoder data malloc failed");
                decompress_done(decoder->sd);
                free(decoder);
                pthread_rwlock_unlock(&stream->lock);
                return NULL;
            }
        } else {
            error_msg("decoder decompress reconfigure failed");
            decompress_done(decoder->sd);
            free(decoder->sd);
            free(decoder);
            pthread_rwlock_unlock(&stream->lock);
            return NULL;
        }
    } else {
	    error_msg("decompress not available");
        free(decoder);
        pthread_rwlock_unlock(&stream->lock);
        return NULL;
    }

    pthread_rwlock_unlock(&stream->lock);
    return decoder;
}

void *decoder_th(void* stream){
    stream_data_t *str = (stream_data_t *) stream;
      
    pthread_mutex_lock(&str->decoder->lock);

    while(str->decoder->run){
    
        while(!str->decoder->new_frame && str->decoder->run)
            pthread_cond_wait(&str->decoder->notify_frame, &str->decoder->lock); //TODO: some timeout

        if (str->decoder->run){
            str->decoder->new_frame = FALSE;
            pthread_rwlock_wrlock(&str->lock);
            decompress_frame(str->decoder->sd,(unsigned char *)str->video.decoded_frame, 
                (unsigned char *)str->decoder->coded_frame, str->decoder->coded_frame_len, 0);
            str->new_frame = TRUE;
            pthread_rwlock_unlock(&str->lock);
        } 
    }

    pthread_mutex_unlock(&str->decoder->lock);
    pthread_exit((void *)NULL);    
}

void start_decoder(stream_data_t *stream){
    assert(stream->decoder == NULL);
  
    pthread_rwlock_wrlock(&stream->lock);
    stream->decoder = init_decoder(stream);
    stream->decoder->run = TRUE;
    pthread_rwlock_unlock(&stream->lock);
     
    if (pthread_create(&stream->decoder->thread, NULL, (void *) decoder_th, stream) != 0)
        stream->decoder->run = FALSE;
}

void *encoder_routine(void *arg)
{
    stream_data_t *stream = (stream_data_t *)arg;
    encoder_thread_t *encoder = stream->encoder;

    int width = stream->video.width;
    int height = stream->video.height;

    encoder->input_frame_len = vc_get_linesize(width, PIXEL_FORMAT) * height;
    encoder->input_frame = malloc(encoder->input_frame_len);
    if (encoder->input_frame == NULL) {
        error_msg("encoder_routine: malloc error");
        pthread_exit((void *)NULL);
    }

    struct module cmod;
    module_init_default(&cmod);

    compress_init(&cmod, "libavcodec:codec=H.264", &encoder->cs);

    struct video_frame *frame = vf_alloc(1);
    if (frame == NULL) {
        error_msg("encoder_routine: vf_alloc error");
        compress_done(&encoder->cs);
        pthread_exit((void *)NULL);
    }

    vf_get_tile(frame, 0)->width = width;
    vf_get_tile(frame, 0)->height = height;
    frame->color_spec = PIXEL_FORMAT;
    // TODO: set default fps value. If it's not set -> core dump
    frame->fps = DEFAULT_FPS; 
    frame->interlacing = PROGRESSIVE;

    encoder->run = TRUE; // TODO: set to TRUE also in init_encoder!?
    encoder->index = 0;

    while (encoder->run) {
        sem_wait(&encoder->input_sem);
        if (!encoder->run) {
            break;
        }
    
        // TODO: create encoder input buffer
        pthread_mutex_lock(&encoder->lock);
        
        frame->tiles[0].data = encoder->input_frame;
        frame->tiles[0].data_len = encoder->input_frame_len;
    
        struct video_frame *tx_frame;
        tx_frame = compress_frame(encoder->cs, frame, encoder->index);

        encoder->frame = tx_frame;

        pthread_rwlock_unlock(&encoder->lock);

        encoder->index = (encoder->index + 1) % 2;

        /* NOTE: now the encoder knows nothing about the RTP
           thread, so the notify and pause strategy is implemented
           only trough the input and ouput semaphores
         */
        sem_post(&encoder->output_sem);
        sem_wait(&encoder->input_sem);
    }

    module_done(CAST_MODULE(&cmod));
    free(encoder->input_frame);
    pthread_exit((void *)NULL);

void *encoder_routine(void *arg)
{
    stream_data_t *stream = (stream_data_t *)arg;

    // TODO

    return (void *)NULL;
}

encoder_thread_t *init_encoder(stream_data_t *stream)
{
    
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

    if (sem_init(&encoder->input_sem, 1, 0)) {
        error_msg("init_encoder: sem_init error");
        pthread_mutex_destroy(&encoder->lock);
        free(encoder);
        return NULL;
    }
    if (sem_init(&encoder->output_sem, 1, 0)) {
        error_msg("init_encoder: sem_init error");
        pthread_mutex_destroy(&encoder->lock);
        sem_destroy(&encoder->input_sem);
        free(encoder);
        return NULL;
    }

    encoder->run = FALSE;

    int ret = 0;
    ret = pthread_create(&encoder->thread, NULL, encoder_routine, stream);
    if (ret < 0) {
        error_msg("init_encoder: pthread_create error");
        pthread_mutex_destroy(&encoder->lock);
        sem_destroy(&encoder->input_sem);
        sem_destroy(&encoder->output_sem);
        free(encoder);
        return NULL;
    }
    
    encoder->run = TRUE;

    // TODO assign the encoder here?
    pthread_rwlock_wrlock(&stream->lock);
    stream->encoder = encoder;
    pthread_rwlock_unlock(&stream->lock);
    return encoder;
}

void destroy_decoder(decoder_thread_t *decoder)
{
    assert(decoder->run == FALSE);

    pthread_mutex_destroy(&decoder->lock);
    pthread_cond_destroy(&decoder->notify_frame);

    decompress_done(decoder->sd);

    free(decoder->coded_frame);
    free(decoder);
}

void destroy_encoder(encoder_thread_t *encoder)
{
    if (encoder == NULL) {
        return;
    }

    // TODO: necessary?
    //if (encoder->run != TRUE) {
    //    return;
    //}

    // TODO: locks?


    encoder->run = FALSE;

    sem_post(&encoder->input_sem);
    sem_post(&encoder->output_sem);

    sem_destroy(&encoder->input_sem);
    sem_destroy(&encoder->output_sem);

    pthread_join(encoder->thread, NULL);
    
    pthread_mutex_destroy(&encoder->lock);

    free(encoder);
}

stream_list_t *init_stream_list(void)
{
    stream_list_t *list = malloc(sizeof(stream_list_t));
    if (list == NULL) {
        error_msg("init_stream_list malloc error");
        return NULL;
    }
    pthread_rwlock_init(&list->lock, NULL);
    list->count = 0;
    list->first = NULL;
    list->last = NULL;
    return list;
}

void destroy_stream_list(stream_list_t *list)
{
    pthread_rwlock_wrlock(&list->lock);
    stream_data_t *current = list->first;
    while (current != NULL) {
        stream_data_t *next = current->next;
        destroy_stream(current);
        current = next;
    }
    //pthread_rwlock_unlock(&list->lock);
    pthread_rwlock_destroy(&list->lock);
    free(list);
}

stream_data_t *init_video_stream(stream_type_t type, uint32_t id, uint8_t active)
{
    stream_data_t *stream = malloc(sizeof(stream_data_t));
    if (stream == NULL) {
        error_msg("init_video_stream malloc error");
        return NULL;
    }
    pthread_rwlock_init(&stream->lock, NULL);
    stream->type = type;
    // TODO
}

int destroy_stream(stream_data_t *stream)
{
    pthread_rwlock_wrlock(&stream->lock);

    if (stream->type == VIDEO){
        free(stream->video.decoded_frame);
    } else if (stream->type == AUDIO){
        //Free audio structures
    }

    if (stream->io_type == INPUT && stream->decoder != NULL){
        destroy_decoder(stream->decoder);
    } else if (stream->io_type == OUTPUT && stream->encoder != NULL){
        destroy_encoder(stream->encoder);
    }
    pthread_rwlock_destroy(&stream->lock);
    free(stream);
    return TRUE;
}

int set_stream_video_data(stream_data_t *stream, codec_t codec, uint32_t width, uint32_t height)
{
    pthread_rwlock_wrlock(&stream->lock);

    if (stream->type == AUDIO){
        //init audio structures
        error_msg("set_stream_video_data: stream type is AUDIO");
        pthread_rwlock_unlock(&stream->lock);
        return FALSE;
    } else if (stream->type == VIDEO){
        stream->video.codec = codec;
        stream->video.width = width;
        stream->video.height = height;
        stream->video.decoded_frame_len = vc_get_linesize(width, RGB)*height;
        stream->video.decoded_frame = malloc(stream->video.decoded_frame_len);
    } else {
        error_msg("set_stream_video_data: type not contemplated\n");
        pthread_rwlock_unlock(&stream->lock);
        return FALSE;
    }

    if (stream->io_type == INPUT && stream->decoder == NULL){
        stream->decoder = init_decoder(stream);
    } else if (stream->io_type == OUTPUT && stream->encoder == NULL){
        stream->encoder = init_encoder(stream);
    }

    pthread_rwlock_unlock(&stream->lock);

    return TRUE;
}

int add_stream(stream_list_t *list, stream_data_t *stream)
{
    pthread_rwlock_wrlock(&list->lock);
    pthread_rwlock_wrlock(&stream->lock);
    int ret = TRUE;

    if (list->count == 0) {
        assert(list->first == NULL && list->last == NULL);
        list->count++;
        list->first = list->last = stream;
    } else if (list->count > 0) {
        assert(list->first != NULL && list->last != NULL);
        stream->next = NULL;
        stream->prev = list->last;
        list->last->next = stream;
        list->last = stream;
        list->count++;
    } else {
        error_msg("add_stream list->count < 0");
        ret = FALSE;
    }
    pthread_rwlock_unlock(&list->lock);
    pthread_rwlock_unlock(&stream->lock);
    return ret;
}

stream_data_t *get_stream_id(stream_list_t *list, uint32_t id)
{
    pthread_rwlock_rdlock(&list->lock);

    int ret = TRUE;

    if (list->count == 0) {
        assert(list->first == NULL && list->last == NULL);
        list->count++;
        list->first = list->last = stream;
    } else if (list->count > 0) {
        assert(list->first != NULL && list->last != NULL);
        stream->next = NULL;
        stream->prev = list->last;
        list->last->next = stream;
        list->last = stream;
        list->count++;
    } else {
        error_msg("add_stream list->count < 0");
        ret = FALSE;
    }
    pthread_rwlock_unlock(&list->lock);
    pthread_rwlock_unlock(&stream->lock);
    return ret;
}

stream_data_t *get_stream_id(stream_list_t *list, uint32_t id)
{
    pthread_rwlock_rdlock(&list->lock);

    stream_data_t *stream = list->first;
    while (stream != NULL) {
        if (stream->id == id) {
            break;
        }
        stream = stream->next;
    }
    pthread_rwlock_unlock(&list->lock);
    return stream;
}

int remove_stream(stream_list_t *list, uint32_t id)
{
    pthread_rwlock_wrlock(&list->lock);
    
    if (list->count == 0) {
        pthread_rwlock_unlock(&list->lock);
        return FALSE;
    }
    stream_data_t *stream = get_stream_id(list, id);
    if (stream == NULL) {
        pthread_rwlock_unlock(&list->lock);
        return FALSE;
    }

    pthread_rwlock_wrlock(&stream->lock);

    if (stream->prev == NULL) {
        assert(list->first == stream);
        list->first = stream->next;
    } else {
        stream->prev->next = stream->next;
    }

    if (stream->next == NULL) {
        assert(list->last == stream);
        list->last = stream->prev;
    } else {
        stream->next->prev = stream->prev;
    }

    list->count--;

    pthread_rwlock_unlock(&list->lock);
    
    destroy_stream(stream);

    pthread_rwlock_unlock(&stream->lock);
    
    return TRUE;
}
