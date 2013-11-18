#include <signal.h>
#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"
#include "io_mngr/c_basicRTSPOnlyServer.h"

static volatile bool stop = false;

static void signal_handler(int signal)
{
    if (signal) { // Avoid annoying warnings.
        stop = true;
    }
    return;
}

int main(){
    
    struct timeval a, b;
    
    //video_frames structures
    video_data_frame_t *in_frame;
    video_data_frame_t *out_frame;
    
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
    
    //Attach signal handler
    //signal(SIGINT, signal_handler);
    
    //Initialization of all data
    in_str_list     = init_stream_list();
    out_str_list    = init_stream_list();
    out_str1        = init_stream(VIDEO, OUTPUT, 1, ACTIVE, "i2cat_rocks");
    out_str2        = init_stream(VIDEO, OUTPUT, 2, ACTIVE, "i2cat_rocks_2nd");
    transmitter     = init_transmitter(out_str_list, 24.0);
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
        
        while(!stop){
                       
            pthread_rwlock_rdlock(&in_str_list->lock);
            pthread_rwlock_rdlock(&out_str_list->lock);
            
            out_str = out_str_list->first;
            in_str = in_str_list->first;
            
            pthread_rwlock_unlock(&out_str_list->lock);
            pthread_rwlock_unlock(&in_str_list->lock);
            
            while(in_str != NULL && out_str != NULL){     
                if (out_str->video->encoder == NULL && in_str->video->decoder != NULL) {
                    in_frame = curr_out_frame(in_str->video->decoded_frames);
                    if (in_frame == NULL){
                        continue;
                    }
                    
                    set_video_frame_cq(out_str->video->decoded_frames, RAW, 
                                         in_frame->width, 
                                         in_frame->height);
                    set_video_frame_cq(out_str->video->coded_frames, H264,
                                         in_frame->width, 
                                         in_frame->height);
                    init_encoder(out_str->video);
                    c_update_server(server);
                    continue;
                }
                
                in_frame = curr_out_frame(in_str->video->decoded_frames);
                out_frame = curr_in_frame(out_str->video->decoded_frames);
                              
                if (in_frame == NULL || out_frame == NULL){
                    continue;
                }
                
                memcpy(out_frame->buffer, 
                       in_frame->buffer, 
                       in_frame->buffer_len);
                out_frame->buffer_len 
                    = out_frame->buffer_len;
                    
                remove_frame(in_str->video->decoded_frames);
                put_frame(out_str->video->decoded_frames);
                
                in_str = in_str->next;
                out_str = out_str->next;
            }
            
            gettimeofday(&b, NULL);
            if (b.tv_sec - a.tv_sec >= 120){
                stop = TRUE;
            }  if (out_str_list->count < 2 && b.tv_sec - a.tv_sec >= 50){
                 //Adding 2nd outgoing stream
                add_stream(out_str_list, out_str2);               
                //Adding 2nd incoming participant
                add_participant(receiver->participant_list, in_p2);
            }  else {
                usleep(5000);
            }
        }       
        
    }
    
    printf("Stopping all managers\n");
    
    c_stop_server(server);
    stop_transmitter(transmitter);
    stop_receiver(receiver);
    destroy_stream_list(in_str_list);
    destroy_stream_list(out_str_list);
}
