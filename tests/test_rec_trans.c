#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"
#include "io_mngr/c_basicRTSPOnlyServer.h"

int main(){
    
    struct timeval a, b;
    uint8_t run = TRUE;
    
    stream_list_t *in_str_list;
    stream_data_t *in_str;
    receiver_t *receiver;
    
    stream_list_t *out_str_list;
    stream_data_t *out_str;
    transmitter_t *transmitter;
    
    stream_data_t *out_str1;
    stream_data_t *out_str2;
    
    rtsp_serv_t *server = (rtsp_serv_t*) malloc(sizeof(rtsp_serv_t));
    
    participant_data_t *in_p1;
    participant_data_t *in_p2;
    
    in_str_list     = init_stream_list();
    out_str_list    = init_stream_list();
    out_str1        = init_stream(VIDEO, OUTPUT, 1, ACTIVE, "i2cat_rocks");
    out_str2        = init_stream(VIDEO, OUTPUT, 2, ACTIVE, "i2cat_rocks_2nd");
    transmitter     = init_transmitter(out_str_list, 20.0);
    server          = init_rtsp_server(8554, out_str_list, transmitter);
    receiver        = init_receiver(in_str_list, 5004);
    in_p1           = init_participant(1, INPUT, NULL, 0);
    in_p2           = init_participant(2, INPUT, NULL, 0);
    
    gettimeofday(&a, NULL);
    gettimeofday(&b, NULL);
    
    add_stream(out_str_list, out_str1);
    c_start_server(server);
    
    add_participant(receiver->participant_list, in_p1);
    
    if (start_receiver(receiver)) {

        printf("This test stops normally after 120 seconds\n");
        
        while(run){
            pthread_rwlock_rdlock(&in_str_list->lock);
            in_str = in_str_list->first;
            pthread_rwlock_unlock(&in_str_list->lock);
            
            pthread_rwlock_rdlock(&out_str_list->lock);
            out_str = out_str_list->first;
            pthread_rwlock_unlock(&out_str_list->lock);
            
            while(in_str != NULL && out_str != NULL){
                
                if (in_str->video->decoder != NULL && out_str->video->encoder == NULL) {
                    pthread_rwlock_wrlock(&out_str_list->lock);
                    set_video_data(out_str->video, in_str->video->codec, in_str->video->width, in_str->video->height);
                    init_encoder(out_str->video);
                    pthread_rwlock_unlock(&out_str_list->lock);
                }
                
                pthread_mutex_lock(&in_str->video->new_decoded_frame_lock);
                if (in_str->video->new_decoded_frame){
                    pthread_rwlock_rdlock(&in_str->video->decoded_frame_lock);
                    pthread_rwlock_wrlock(&out_str->video->decoded_frame_lock);
                
                    memcpy(out_str->video->decoded_frame, in_str->video->decoded_frame, in_str->video->decoded_frame_len);
                    out_str->video->decoded_frame_len = in_str->video->decoded_frame_len;
                 
                    pthread_rwlock_unlock(&out_str->video->decoded_frame_lock);
                    pthread_rwlock_unlock(&in_str->video->decoded_frame_lock);
                    in_str->video->new_decoded_frame = FALSE;
                    sem_post(&out_str->video->encoder->input_sem); 
                }
                pthread_mutex_unlock(&in_str->video->new_decoded_frame_lock);
                pthread_rwlock_unlock(&in_str_list->lock);
                
                in_str = in_str->next;
                out_str = out_str->next;
            }
            
            gettimeofday(&b, NULL);
            if (b.tv_sec - a.tv_sec >= 120){
                run = FALSE;
            }  if (b.tv_sec - a.tv_sec >= 50){
                add_participant(receiver->participant_list, in_p2);
                add_stream(out_str_list, out_str2);
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
