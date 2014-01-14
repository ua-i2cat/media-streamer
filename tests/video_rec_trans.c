#include <signal.h>
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"
#include "io_mngr/c_basicRTSPOnlyServer.hh"

#define LIVE_TIME 120

static volatile bool stop = false;

// Triggers the stops condition of the program.
static void finish_handler(int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGALRM:
            stop = true;
            break;
        default:
            break;
    }
}

int main(){

    struct timeval a, b;

    //video_frames structures
    video_frame2 *in_frame;
    video_frame2 *out_frame;

    //Receiver structures
    stream_list_t *in_str_list;
    stream_list_t *dummy_audio_str_list2;
    stream_t *in_str;
    receiver_t *receiver;

    //Transmitter structures
    stream_list_t *out_str_list;
    stream_list_t *dummy_audio_str_list1;
    stream_t *out_str;
    transmitter_t *transmitter;

    //Outgoing streams and incomping streams
    stream_t *out_str1;
    stream_t *out_str2;

    //RTSP server
    rtsp_serv_t *server = (rtsp_serv_t*) malloc(sizeof(rtsp_serv_t));

    //Incoming participants
    participant_t *in_p1;
    participant_t *in_p2;

    //Attach signal handler and start alarm
    signal(SIGINT, finish_handler);
    alarm(LIVE_TIME);

    //Initialization of all data
    in_str_list     = init_stream_list();
    out_str_list    = init_stream_list();
    dummy_audio_str_list1    = init_stream_list();
    dummy_audio_str_list2    = init_stream_list();
    out_str1        = init_stream(VIDEO, OUTPUT, 1, ACTIVE, "i2cat_rocks");
    out_str2        = init_stream(VIDEO, OUTPUT, 2, ACTIVE, "i2cat_rocks_2nd");
    in_str          = init_stream(VIDEO, INPUT, rand(), I_AWAIT, NULL);
    transmitter     = init_transmitter(out_str_list, dummy_audio_str_list1, 20.0);
    server          = init_rtsp_server(8554, transmitter);
    receiver        = init_receiver(in_str_list, dummy_audio_str_list2, 5004, 5006);
    in_p1           = init_participant(1, INPUT, NULL, 0);
    in_p2           = init_participant(2, INPUT, NULL, 0);

    //Timestamp of start time
    gettimeofday(&a, NULL);
    gettimeofday(&b, NULL);

    //Adding 1st outgoing stream
    add_stream(out_str_list, out_str1);
    //Starting RTSP server
    c_start_server(server);
    //Allocating place for unknown incoming stream
    vp_reconfig_external(in_str->video, 0, 0, H264);
    //Adding 1st incoming stream and participant
    add_participant_stream(in_str, in_p1);
    add_stream(receiver->video_stream_list, in_str);

    //Start receiving
    if (start_receiver(receiver) && start_transmitter(transmitter)) {

        printf("Forwarding video for %i seconds\n", LIVE_TIME);

        while(!stop){

            pthread_rwlock_rdlock(&in_str_list->lock);
            pthread_rwlock_rdlock(&out_str_list->lock);

            out_str = out_str_list->first;
            in_str = in_str_list->first;

            pthread_rwlock_unlock(&out_str_list->lock);
            pthread_rwlock_unlock(&in_str_list->lock);

            while(in_str != NULL && out_str != NULL && !stop) {

                if ((in_frame = cq_get_front(in_str->video->decoded_cq)) != NULL) {

                    if (out_str->video->internal_config->width != in_frame->width ||
                            out_str->video->internal_config->height != in_frame->height ||
                            out_str->video->external_config->width != in_frame->width ||
                            out_str->video->external_config->height != in_frame->height) {
                        vp_reconfig_internal(out_str->video, 
                                in_frame->width, 
                                in_frame->height,
                                RAW);
                        vp_reconfig_external(out_str->video,
                                in_frame->width, 
                                in_frame->height,
                                H264);
                        vp_worker_start(out_str->video);
                        c_update_server(server);
                    } else {

                        if ((out_frame = cq_get_rear(out_str->video->decoded_cq)) != NULL) {
                            memcpy(out_frame->buffer, 
                                    in_frame->buffer, 
                                    in_frame->buffer_len);
                            out_frame->buffer_len 
                                = out_frame->buffer_len;

                            cq_remove_bag(in_str->video->decoded_cq);
                            cq_add_bag(out_str->video->decoded_cq);

                            in_str = in_str->next;
                            out_str = out_str->next;
                        }
                    }
                }
            }

            gettimeofday(&b, NULL);
            if (out_str_list->count < 2 && b.tv_sec - a.tv_sec >= 50){
                //Adding 2nd incoming participant
                in_str = init_stream(VIDEO, INPUT, rand(), I_AWAIT, NULL);
                vp_reconfig_external(in_str->video, 0, 0, H264);
                add_participant_stream(in_str, in_p2);
                add_stream(receiver->video_stream_list, in_str);
                //Adding 2nd outgoing stream
                add_stream(out_str_list, out_str2);               
            }  else {
                usleep(5000);
            }
        }       

    }

    printf("Stopping all managers\n");

    c_stop_server(server);
    stop_transmitter(transmitter);
    destroy_transmitter(transmitter);
    stop_receiver(receiver);
    destroy_receiver(receiver);
    destroy_stream_list(in_str_list);
    destroy_stream_list(out_str_list);
}

