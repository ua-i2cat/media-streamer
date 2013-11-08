#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"
#include "io_mngr/c_basicRTSPOnlyServer.h"

int main(){
    
    stream_list_t *in_str_list;
    stream_data_t *in_str;
    receiver_t *receiver;
    
    stream_list_t *out_str_list;
    stream_data_t *out_str;
    transmitter_t *transmitter;
    
    rtsp_serv_t *server = (rtsp_serv_t*) malloc(sizeof(rtsp_serv_t));
    
    participant_data_t *in_part;
    
    in_str_list     = init_stream_list();
    out_str_list    = init_stream_list();
    out_str         = init_stream(VIDEO, OUTPUT, 0, ACTIVE, "i2catrocks");
    transmitter     = init_transmitter(out_str_list, 20.0);
    server          = init_rtsp_server(8554, out_str_list, transmitter);
    receiver        = init_receiver(in_str_list, 5004);
    in_part         = init_participant(1, INPUT, NULL, 0);
    
    add_stream(out_str_list, out_str);
    c_start_server(server);
    
    add_participant(receiver->participant_list, in_part);

    
    if (start_receiver(receiver)) {

        while(1){
            pthread_rwlock_rdlock(&in_str_list->lock);
            in_str = in_str_list->first;
            if (in_str == NULL){
                pthread_rwlock_unlock(&in_str_list->lock);
                continue;   
            } else if (in_str->video->decoder != NULL && out_str->video->encoder == NULL) {
                pthread_rwlock_wrlock(&out_str_list->lock);
                //set_video_data(out_str->video, in_str->video->codec, in_str->video->width, in_str->video->height); NOTE: set_video_data doesn't exist
                init_encoder(out_str->video);
                pthread_rwlock_unlock(&out_str_list->lock);
            }
            pthread_mutex_lock(&in_str->video->new_decoded_frame_lock);
            if (in_str->video->new_decoded_frame){
                pthread_rwlock_rdlock(&in_str->video->decoded_frame->lock);
                pthread_rwlock_wrlock(&out_str->video->decoded_frame->lock);
                
                memcpy(out_str->video->decoded_frame->buffer, in_str->video->decoded_frame->buffer, in_str->video->decoded_frame->buffer_len);
                out_str->video->decoded_frame->buffer_len = in_str->video->decoded_frame->buffer_len;
                 
                pthread_rwlock_unlock(&out_str->video->decoded_frame->lock);
                pthread_rwlock_unlock(&in_str->video->decoded_frame->lock);
                in_str->video->new_decoded_frame = FALSE;
                sem_post(&out_str->video->encoder->input_sem); 
            }
            pthread_mutex_unlock(&in_str->video->new_decoded_frame_lock);
            pthread_rwlock_unlock(&in_str_list->lock);
        }       
        
    }
    
    stop_transmitter(transmitter);
    destroy_stream_list(out_str_list);
    stop_receiver(receiver);
    destroy_stream_list(in_str_list);
}
