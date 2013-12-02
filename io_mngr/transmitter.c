#include "config_unix.h"
#include "transmitter.h"
#include "rtp/rtp.h"
#include "pdb.h"
#include "video_codec.h"
#include "video_compress.h"
#include "debug.h"
#include "tv.h"
#include <stdlib.h>

int send_coded_frame(stream_data_t *stream, video_data_frame_t *coded_frame, struct timeval start_time);
void *transmitter_thread(void *arg);

transmitter_t *init_transmitter(stream_list_t *stream_list, float fps) {

    transmitter_t *transmitter = malloc(sizeof(transmitter_t));
    if (transmitter == NULL) {
        error_msg("init_transmitter: malloc error");
        return NULL;
    }

    transmitter->run = FALSE;
    
    if (fps <= 0.0) {
        transmitter->fps = DEFAULT_FPS;
    } else {
        transmitter->fps = fps;
    }
    transmitter->wait_time = (1000000.0/transmitter->fps);

    transmitter->ttl = DEFAULT_TTL;
    transmitter->send_buffer_size = DEFAULT_SEND_BUFFER_SIZE;
    transmitter->mtu = MTU;

    transmitter->stream_list = stream_list;

    return transmitter;
}

int send_coded_frame(stream_data_t *stream, video_data_frame_t *coded_frame, struct timeval start_time) {

    participant_data_t *participant;
    struct timeval curr_time;
    double timestamp;
    int ret = FALSE;
    
    pthread_rwlock_rdlock(&stream->plist->lock);
    
    participant = stream->plist->first;
    while (participant != NULL && participant->rtp != NULL){
        
        gettimeofday(&curr_time, NULL);
        rtp_update(participant->rtp->rtp, curr_time);
        timestamp = tv_diff(curr_time, start_time)*90000;
        rtp_send_ctrl(participant->rtp->rtp, timestamp, 0, curr_time);            

        tx_send_h264(participant->rtp->tx_session, stream->video->encoder->frame, 
                     participant->rtp->rtp, coded_frame->media_time);
        ret = TRUE;
        
        participant = participant->next;
    }
    
    pthread_rwlock_unlock(&stream->plist->lock);
    
    return ret;
}

void *transmitter_thread(void *arg) {

    transmitter_t *transmitter = (transmitter_t *)arg;
    stream_data_t *stream;
    video_data_frame_t *coded_frame;
    

    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    
    while(transmitter->run){
        usleep(100);
        
        pthread_rwlock_rdlock(&transmitter->stream_list->lock);
        
        stream = transmitter->stream_list->first;
        while(stream != NULL && transmitter->run){
            coded_frame = curr_out_frame(stream->video->coded_frames);
            if (coded_frame == NULL){
                continue;
            }
            send_coded_frame(stream, coded_frame, start_time);
            remove_frame(stream->video->coded_frames);
            stream = stream->next;
        }
        
        pthread_rwlock_unlock(&transmitter->stream_list->lock);
    }
    
    pthread_exit(NULL);
}

int start_transmitter(transmitter_t *transmitter)
{  
    transmitter->run = TRUE;
  
    if (pthread_create(&transmitter->thread, NULL, transmitter_thread, transmitter) != 0) {
        transmitter->run = FALSE;
    }
  
    return transmitter->run;
}

int stop_transmitter(transmitter_t *transmitter)
{
    transmitter->run = FALSE;
  
    pthread_join(transmitter->thread, NULL); 
  
    return TRUE;
}

int destroy_transmitter(transmitter_t *transmitter)
{
    if (transmitter->run){
        return FALSE;
    }

    free(transmitter);
    return TRUE;
}
