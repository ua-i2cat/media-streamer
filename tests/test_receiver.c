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
  
  	stream_list = init_stream_list();
  
//  	add_participant(part_list, 2, INPUT, NULL, 0);

  	receiver = init_receiver(stream_list, 5004);
    participant_data_t *p1 = init_participant(1, INPUT, NULL, 0);
    participant_data_t *p2 = init_participant(2, INPUT, NULL, 0);
  	add_participant(receiver->participant_list, p1);
  	add_participant(receiver->participant_list, p2);
  
  	if (start_receiver(receiver)) {

        printf("First 200 frames to disk\n");
	  
        int i = 0, c=0;
        while(i < 700){
            pthread_rwlock_rdlock(&stream_list->lock);
            stream = stream_list->first;
            if (stream == NULL){
                pthread_rwlock_unlock(&stream_list->lock);
                continue;	
            }	
            if (stream->video->new_decoded_frame){
                if (F_video_rx0 == NULL) {
                    printf("recording rx frame0...\n");
                    F_video_rx0 = fopen(OUTPUT_PATH0, "wb");
                }
                pthread_rwlock_rdlock(&stream->video->decoded_frame->lock);
                fwrite(stream->video->decoded_frame->buffer, stream->video->decoded_frame->buffer_len, 1, F_video_rx0);
                pthread_rwlock_unlock(&stream->video->decoded_frame->lock);
                stream->video->new_decoded_frame = FALSE;
                printf("Frame %d by stream 0\n", i);
                i++;
            }
            pthread_rwlock_unlock(&stream_list->lock);
        }

        i=0;
        while(i < 700){
            pthread_rwlock_rdlock(&stream_list->lock);
            stream = stream_list->first->next;
            if (stream == NULL){
                pthread_rwlock_unlock(&stream_list->lock);
                continue;	
            }	
            if (stream->video->new_decoded_frame){
				
                if (F_video_rx1 == NULL) {
                    printf("recording rx frame1...\n");
                    F_video_rx1 = fopen(OUTPUT_PATH1, "wb");
                }
                pthread_rwlock_rdlock(&stream->video->decoded_frame->lock);
                fwrite(stream->video->decoded_frame->buffer, stream->video->decoded_frame->buffer_len, 1, F_video_rx1);
                pthread_rwlock_unlock(&stream->video->decoded_frame->lock);
                stream->video->new_decoded_frame = FALSE;
                printf("Frame %d by stream 1\n", i);
                i++;
            }
            pthread_rwlock_unlock(&stream_list->lock);
        }	
		// if (i == 300){
		// 	printf("Disabling flow\n");
  //  			set_stream_state(stream, NON_ACTIVE);
  //    		printf("Flow disabled\n");
  //    		sleep(10);
	 // 		printf("Enabling flow\n");
	 // 		set_stream_state(stream, ACTIVE);
	 // 		printf("Flow enabled\n");
	 // 		i++;
		// }
    
        stop_receiver(receiver);
        printf("Stopped receiver\n");
        destroy_stream_list(stream_list);

        printf("Finished\n");
    }
}
