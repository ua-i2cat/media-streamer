#include "debug.h"
#include "video_data_frame.h"

video_data_frame_t *init_video_data_frame();
int destroy_video_data_frame(video_data_frame_t *frame);
int set_video_data_frame(video_data_frame_t *frame, codec_t codec, uint32_t width, uint32_t height);

video_data_frame_t *init_video_data_frame(){
    video_data_frame_t *frame = malloc(sizeof(video_data_frame_t));

    frame->buffer = NULL;
    frame->seqno = 0;
    frame->frame_type = BFRAME;

    return frame;
}

int destroy_video_data_frame(video_data_frame_t *frame){
    free(frame->buffer);
    free(frame);
    return TRUE;
}

int set_video_data_frame(video_data_frame_t *frame, codec_t codec, uint32_t width, uint32_t height){
    frame->codec = codec;
    frame->width = width;
    frame->height = height;
    
    if (width == 0 || height == 0){
        width = MAX_WIDTH;
        height = MAX_HEIGHT;
    }
    
    frame->buffer_len = width*height*3*sizeof(uint8_t); 

    if (frame->buffer == NULL){
        frame->buffer = malloc(frame->buffer_len);
    } else {
        free(frame->buffer);
        frame->buffer = malloc(frame->buffer_len);
    }

    if (frame->buffer == NULL) {
        error_msg("set_stream_video_data: malloc error");
        return FALSE;
    }

    return TRUE;
}

video_frame_cq_t *init_video_frame_cq(uint8_t max, uint32_t timeout){
    video_frame_cq_t* frame_cq = malloc(sizeof(video_frame_cq_t));
    
    if (max <= 1){
        error_msg("video frame queue must have at least 2 positions");
        return NULL;
    }
    
    frame_cq->rear = 0;
    frame_cq->front = 0;
    frame_cq->max = max;
    frame_cq->timeout = timeout;
    frame_cq->state = CQ_EMPTY;
    frame_cq->frames = malloc(sizeof(video_data_frame_t*)*max);
    
    for(uint8_t i; i < max; i++){
        frame_cq->frames[i] = init_video_data_frame();
    }
    
    return frame_cq;
}

int destroy_video_frame_cq(video_frame_cq_t* frame_cq){
    for(uint8_t i = 0; i < frame_cq->max; i++){
       destroy_video_data_frame(frame_cq->frames[i]);
    }
    free(frame_cq);
    return TRUE;
}

int set_video_frame_cq(video_frame_cq_t *frame_cq, codec_t codec, uint32_t width, uint32_t height){ 
    for(uint8_t i = 0; i < frame_cq->max; i++){
        if(! set_video_data_frame(frame_cq->frames[i], codec, width, height)){
            return FALSE;
        }
    }
    
    return TRUE;
}

video_data_frame_t* curr_in_frame(video_frame_cq_t *frame_cq){
    
    while (frame_cq->state == CQ_FULL){
        usleep(frame_cq->timeout);
        if (frame_cq->state == CQ_FULL) {
            return NULL;
        }
    }
    
    return frame_cq->frames[frame_cq->rear];
}

int put_frame(video_frame_cq_t *frame_cq){
    uint8_t r;
    
    if (frame_cq->state == CQ_FULL){
        return FALSE;
    }
    
    r =  (frame_cq->rear + 1) % frame_cq->max;
    if (r == frame_cq->front) {
        frame_cq->state = CQ_FULL;
    } else {
        frame_cq->state = CQ_OK;
    }
    frame_cq->rear = r;
    
    return TRUE;
}

video_data_frame_t* curr_out_frame(video_frame_cq_t *frame_cq){
    
    while (frame_cq->state == CQ_EMPTY){
        usleep(frame_cq->timeout);
        if (frame_cq->state == CQ_EMPTY) {
            return NULL;
        }
    }
    
    return frame_cq->frames[frame_cq->front];
}

int remove_frame(video_frame_cq_t *frame_cq){
    uint8_t f;
    
    if (frame_cq->state == CQ_EMPTY){
        return FALSE;
    }
    
    f =  (frame_cq->front + 1) % frame_cq->max;
    if (f == frame_cq->rear) {
        frame_cq->state = CQ_EMPTY;
    } else {
        frame_cq->state = CQ_OK;
    }
    frame_cq->front = f;
    
    return TRUE;
}


int flush_frames(video_frame_cq_t *frame_cq){
    uint8_t r;
    
    if (frame_cq->state == CQ_FULL){
        r = (frame_cq->rear + 1) % frame_cq->max;
        if (r != frame_cq->rear){
            frame_cq->state = CQ_OK;
            frame_cq->rear = r;
        }
    }
    
    return TRUE;
}

