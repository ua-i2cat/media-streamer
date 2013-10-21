#include "stream.h"

decoder_thread_t *init_decoder(stream_data_t *stream)
{
    pthread_rwlock_rlock(stream->lock);

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
    pthread_rwlock_rlock(&stream->lock);
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
    stream_list_t  *list;
    
    list = (stream_list_t *) malloc(sizeof(stream_list_t));
  
    pthread_rwlock_init(&list->lock, NULL);
  
    list->count = 0;
    list->first = NULL;
    list->last = NULL;
  
    return list;
}

void destroy_stream_list(stream_list_t *list)
{
    stream_data_t *stream;
  
    stream = list->first;

    while(stream != NULL){
        pthread_rwlock_wrlock(&list->lock);
        remove_stream(list, stream->id);
        pthread_rwlock_unlock(&list->lock);
        stream = stream->next;
    }
  
    assert(list->count == 0);
 
    pthread_rwlock_destroy(&list->lock);
  
    free(list);

}

stream_data_t *init_stream(uint32_t id, uint8_t active, io_type_t io_type, stream_type_t type)
{
    stream_data_t *stream;

    stream = (stream_data_t*)malloc(sizeof(stream_data_t));

    pthread_mutex_init(&stream->lock, NULL);
    stream->type = type;
    stream->io_type = io_type;
    stream->id = id;
    stream->active = active;
    stream->next = stream->previous = NULL;

    return stream;
}

int destroy_stream(stream_data_t *stream)
{
    if (stream->type == VIDEO){
        free(stream->video.frame);
    } else if (stream->type == AUDIO){
        //Free audio structures
    }

    if (stream->io_type == INPUT && stream->decoder != NULL){
        destroy_decoder_thread(stream->decoder);
    } else if (stream->io_type == OUTPUT && stream->encoder != NULL){
        destroy_encoder_thread(encoder);
    }
  
    pthread_mutex_destroy(&stream->lock);
  
    free(stream);

    return 0;
}

int set_stream_data(stream_data_t *stream, codec_t codec, uint32_t width, uint32_t height)
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
        debug_msg("type not contemplated\n")
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

}

int remove_stream(stream_list *list, uint32_t id)
{

}
