#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"
#include "io_mngr/c_basicRTSPOnlyServer.h"

int main(){
    
    struct timeval a, b;
    uint8_t run = TRUE;
    
    //Receiver structures
    stream_list_t *in_str_list;
    stream_data_t *in_str;
    receiver_t *receiver;
    
    //Transmitter structures
    stream_list_t *out_str_list;
    stream_data_t *out_str;
    transmitter_t *transmitter;
    
    //Outgoing streams
    stream_data_t *out_str1;
    stream_data_t *out_str2;
    
    //RTSP server
    rtsp_serv_t *server = (rtsp_serv_t*) malloc(sizeof(rtsp_serv_t));
    
    //Incoming participants
    participant_data_t *in_p1;
    participant_data_t *in_p2;
    
    //Initialization of all data
    in_str_list     = init_stream_list();
    out_str_list    = init_stream_list();
    out_str1        = init_stream(VIDEO, OUTPUT, 1, ACTIVE, "i2cat_rocks");
    out_str2        = init_stream(VIDEO, OUTPUT, 2, ACTIVE, "i2cat_rocks_2nd");
    transmitter     = init_transmitter(out_str_list, 20.0);
    server          = init_rtsp_server(8554, out_str_list, transmitter);
    receiver        = init_receiver(in_str_list, 5004);
    in_p1           = init_participant(1, INPUT, NULL, 0);
    in_p2           = init_participant(2, INPUT, NULL, 0);
    
    //Timestamp of start time
    gettimeofday(&a, NULL);
    gettimeofday(&b, NULL);
    
    //Adding 1st outgoing stream
    add_stream(out_str_list, out_str1);
    //Starting RTSP server
    c_start_server(server);
    //Adding 1st incoming participant
    add_participant(receiver->participant_list, in_p1);
    
    //Start receiving
    if (start_receiver(receiver)) {

        printf("This test stops normally after 120 seconds\n");
        
        while(run){
            pthread_rwlock_rdlock(&in_str_list->lock);
            pthread_rwlock_rdlock(&out_str_list->lock);
            out_str = out_str_list->first;
            in_str = in_str_list->first;
            
            while(in_str != NULL && out_str != NULL){
                
                if (in_str->video->decoder != NULL && out_str->video->encoder == NULL) {
                    pthread_rwlock_wrlock(&out_str->video->lock);
                    set_video_data_frame(out_str->video->decoded_frame, RAW, 
                                         in_str->video->decoded_frame->width, 
                                         in_str->video->decoded_frame->height);
                    set_video_data_frame(out_str->video->coded_frame, H264, 
                                         in_str->video->decoded_frame->width, 
                                         in_str->video->decoded_frame->height);
                    init_encoder(out_str->video);
                    pthread_rwlock_unlock(&out_str->video->lock);
                }
                
                pthread_mutex_lock(&in_str->video->new_decoded_frame_lock);
                if (in_str->video->new_decoded_frame){
                    pthread_rwlock_rdlock(&in_str->video->decoded_frame->lock);
                    pthread_rwlock_wrlock(&out_str->video->decoded_frame->lock);
                
                    memcpy(out_str->video->decoded_frame->buffer, 
                           in_str->video->decoded_frame->buffer, 
                           in_str->video->decoded_frame->buffer_len);
                    out_str->video->decoded_frame->buffer_len 
                        = in_str->video->decoded_frame->buffer_len;
                 
                    pthread_rwlock_unlock(&out_str->video->decoded_frame->lock);
                    pthread_rwlock_unlock(&in_str->video->decoded_frame->lock);
                    in_str->video->new_decoded_frame = FALSE;
                    sem_post(&out_str->video->encoder->input_sem);
                }
                pthread_mutex_unlock(&in_str->video->new_decoded_frame_lock);
                
                in_str = in_str->next;
                out_str = out_str->next;
            }
            
            pthread_rwlock_unlock(&out_str_list->lock);
            pthread_rwlock_unlock(&in_str_list->lock);
            
            gettimeofday(&b, NULL);
            if (b.tv_sec - a.tv_sec >= 120){
                run = FALSE;
            }  if (b.tv_sec - a.tv_sec >= 50){
                //Adding 2nd incoming participant
                pthread_rwlock_wrlock(&receiver->participant_list->lock);
                add_participant(receiver->participant_list, in_p2);
                pthread_rwlock_unlock(&receiver->participant_list->lock);
                //Adding 2nd outgoing stream
                pthread_rwlock_wrlock(&out_str_list->lock);
                add_stream(out_str_list, out_str2);
                pthread_rwlock_unlock(&out_str_list->lock);
                //Updating server for new stream
                c_update_server(server);
            }  else {
                usleep(5000);
            }
        }       
        
    }
    
    printf("Stopping all managers\n");
    
    c_stop_server(server);
    stop_transmitter(transmitter);
    destroy_stream_list(out_str_list);
    stop_receiver(receiver);
    destroy_stream_list(in_str_list);
}
