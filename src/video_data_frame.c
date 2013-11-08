#include "debug.h"
#include "video_data_frame.h"

video_data_frame_t *init_video_data_frame();

video_data_frame_t *init_video_data_frame(){
	video_data_frame_t *frame = malloc(sizeof(video_data_frame_t));

	if (pthread_rwlock_init(&frame->lock, NULL) < 0) {
        error_msg("init_video_data_frame_t: pthread_rwlock_init error");
        return NULL;
    }

    frame->buffer = NULL;
    frame->seqno = 0;
    frame->frame_type = BFRAME;

    return frame;
}

int destroy_video_data_frame(video_data_frame_t *frame){
	pthread_rwlock_wrlock(&frame->lock);
    free(frame->buffer);
    pthread_rwlock_unlock(&frame->lock);

    pthread_rwlock_destroy(&frame->lock);
	free(frame);

	return TRUE;
}

int set_video_data_frame(video_data_frame_t *frame, codec_t codec, uint32_t width, uint32_t height){
	pthread_rwlock_wrlock(&frame->lock);
    frame->codec = codec;
    frame->width = width;
    frame->height = height;
    frame->buffer_len = width*height*3*sizeof(uint8_t); 

    if (frame->buffer == NULL){
    	frame->buffer = malloc(frame->buffer_len);
    } else {
    	free(frame->buffer);
    	frame->buffer = malloc(frame->buffer_len);
    }

    if (frame->buffer == NULL) {
        error_msg("set_stream_video_data: malloc error");
        pthread_rwlock_unlock(&frame->lock);
        return FALSE;
    }

    pthread_rwlock_unlock(&frame->lock);
    return TRUE;
}