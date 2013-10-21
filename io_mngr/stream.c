#include "stream.h"

decoder_thread_t *init_decoder(stream_data_t *stream)
{
    pthread_rwlock_rdlock(stream->lock);

	decoder_thread_t *decoder;
	struct video_desc des;

	initialize_video_decompress();
	
	decoder = malloc(sizeof(decoder_thread_thread_t));
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

        des.width = stream->width;
        des.height = stream->height;
        des.color_spec  = stream->codec;
        des.tile_count = 0;
        des.interlacing = PROGRESSIVE;
        des.fps = 24; // TODO XXX

        if (decompress_reconfigure(decoder->sd, des, 16, 8, 0, vc_get_linesize(des.width, RGB), RGB)) {
            decoder->data = malloc(1920*1080*4*sizeof(char)); //TODO this is the worst case in raw data, should be the worst case for specific codec
            if (decoder->data = NULL) {
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
    
}

void destroy_encoder(encoder_thread_t *encoder)
{

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
    pthread_rwlock_unlock(&list->lock);
}

stream_data_t *init_video_stream(stream_type_t type, uint32_t id, uint8_t active)
{

}

int destroy_stream(stream_data_t *stream)
{

}

int set_stream_video_data(stream_data_t *stream, codec_t codec, uint32_t width, uint32_t height)
{
}

int add_stream(stream_list_t *list, stream_data_t *stream)
{

}

int remove_stream(stream_list *list, uint32_t id)
{

}
