#include "stream.h"
#include "video_decompress.h"
#include "video_decompress/libavcodec.h"
#include "video_codec.h"
#include "debug.h"

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
            decoder->data = malloc(1920*1080*4*sizeof(char)); //TODO this is the worst case in raw data, should be the worst case for specific codec
            if (decoder->data == NULL) {
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

encoder_thread_t *init_encoder(stream_data_t *stream)
{
    pthread_rwlock_rdlock(&stream->lock);
    // TODO ...
    pthread_rwlock_unlock(&stream->lock);
    return NULL;
}

void destroy_decoder(decoder_thread_t *decoder)
{
    assert(decoder->run == FALSE);

    pthread_mutex_destroy(&decoder->lock);
    pthread_cond_destroy(&decoder->notify_frame);

    decompress_done(decoder->sd);

    free(decoder->data);
    free(decoder);
}

void destroy_encoder(encoder_thread_t *encoder)
{
    // TODO
    UNUSED(encoder);
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
        free(stream->video.frame);
    } else if (stream->type == AUDIO){
        //Free audio structures
    }

    if (stream->io_type == INPUT && stream->decoder != NULL){
        destroy_decoder_thread(stream->decoder);
    } else if (stream->io_type == OUTPUT && stream->encoder != NULL){
        destroy_encoder_thread(stream->encoder);
    }
    
    pthread_mutex_destroy(&stream->lock);
    free(stream);
    return TRUE;
}

int set_stream_video_data(stream_data_t *stream, codec_t codec, uint32_t width, uint32_t height)
{
    pthread_rwlock_wrlock(&stream->lock);

    if (stream->type == AUDIO){
        //init audio structures
    } else if (stream->type == VIDEO){
        stream->video.codec = codec;
        stream->video.width = width;
        stream->video.height = height;
        stream->video.frame_len = vc_get_linesize(width, RGB)*height;
        stream->video.frame = malloc(stream->video.frame_len);
    } else {
        debug_msg("type not contemplated\n");
    }

    if (stream->io_type == INPUT && stream->decoder == NULL){
        stream->decoder = init_decoder(stream);
    } else if (stream->io_type == OUTPUT && stream->encoder == NULL){
        stream->encoder = init_encoder(stream);
    }

    pthread_rwlock_unlock(&stream->lock);

    return 0;

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

    pthread_rwlock_rwlock(&stream->lock);

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
