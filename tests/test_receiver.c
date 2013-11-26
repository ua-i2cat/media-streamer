#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"

FILE *F_video_rx0=NULL, *F_video_rx1=NULL ;
char *OUTPUT_PATH0 = "./rx_frame1.yuv";
char *OUTPUT_PATH1 = "./rx_frame2.yuv";

int main(){
  	stream_list_t *stream_list;
  	stream_data_t *stream;
  	receiver_t *receiver;
    video_data_frame_t *decoded_frame;
  
  	stream_list = init_stream_list();
  
//  	add_participant(part_list, 2, INPUT, NULL, 0);

  	receiver = init_receiver(stream_list, 5004, 5006);
    participant_data_t *p1 = init_participant(1, INPUT, NULL, 0);
    participant_data_t *p2 = init_participant(2, INPUT, NULL, 0);
    
  	//Allocating place for unknown incoming stream
    stream = init_stream(VIDEO, INPUT, rand(), I_AWAIT, NULL);
    set_video_frame_cq(stream->video->coded_frames, H264, 0, 0);
    //Adding 1st incoming stream and participant
    add_participant_stream(stream, p1);
    add_stream(receiver->stream_list, stream);
    
    //Allocating place for unknown incoming stream
    stream = init_stream(VIDEO, INPUT, rand(), I_AWAIT, NULL);
    set_video_frame_cq(stream->video->coded_frames, H264, 0, 0);
    //Adding 1st incoming stream and participant
    add_participant_stream(stream, p2);
    add_stream(receiver->stream_list, stream);
  
  	if (start_receiver(receiver)) {

        printf("First 200 frames to disk of first participant\n");
	  
        int i = 0;
        while(i < 200){
            usleep(1000);
            pthread_rwlock_rdlock(&stream_list->lock);
            stream = stream_list->first;
            if (stream == NULL){
                pthread_rwlock_unlock(&stream_list->lock);
                continue;	
            }

            decoded_frame = curr_out_frame(stream->video->decoded_frames);
            if (decoded_frame == NULL){
                pthread_rwlock_unlock(&stream_list->lock);
                  continue;
            }
            
            if (F_video_rx0 == NULL) {
                F_video_rx0 = fopen(OUTPUT_PATH0, "wb");
            }
            fwrite(decoded_frame->buffer, decoded_frame->buffer_len, 1, F_video_rx0);
            printf("Frame %d by stream 0\n", i);
            i++;
            remove_frame(stream->video->decoded_frames);
            pthread_rwlock_unlock(&stream_list->lock);
        }

        printf("First 200 frames to disk of second participant\n");
        i=0;
        while(i < 200){
            usleep(100);
            pthread_rwlock_rdlock(&stream_list->lock);
            stream = stream_list->first->next;
            if (stream == NULL){
                pthread_rwlock_unlock(&stream_list->lock);
                continue;	
            }	
            decoded_frame = curr_out_frame(stream->video->decoded_frames);
            if (decoded_frame == NULL){
                pthread_rwlock_unlock(&stream_list->lock);
                continue;
            }
            if (F_video_rx1 == NULL) {
                printf("recording rx frame0...\n");
                F_video_rx1 = fopen(OUTPUT_PATH1, "wb");
            }

            fwrite(decoded_frame->buffer, decoded_frame->buffer_len, 1, F_video_rx1);

            printf("Frame %d by stream 1\n", i);
            i++;
            remove_frame(stream->video->decoded_frames);
            pthread_rwlock_unlock(&stream_list->lock);
        }
    
        stop_receiver(receiver);
        destroy_receiver(receiver);
        printf("Stopped receiver\n");
        destroy_stream_list(stream_list);
        printf("Finished\n");
    }
}
