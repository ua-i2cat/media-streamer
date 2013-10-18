#include "stream.h"

decoder_t *init_decoder_thread(stream_t *stream)
{
	decoder_t *decoder;
	struct video_desc des;

	initialize_video_decompress();
	
	decoder = malloc(sizeof(decoder_thread_t));
    if (decoder == NULL) {
        error_msg("decoder malloc failed");
        return NULL;
    }
	decoder->new_frame = FALSE;
	decoder->run = FALSE;
	
    /* TODO: calloc may be unnecessary?
	decoder->sd = calloc(2, sizeof(struct state_decompress *));
    if (decoder->sd == NULL) {
        error_msg("decoder state decompress calloc failed");
        free(decoder);
        return NULL;
    }
    */
	
	pthread_mutex_init(&decoder->lock, NULL);
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
                return NULL;
            }
        } else {
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

decoder_t *init_encoder_thread(stream_t *stream)
{
    return NULL;
}
