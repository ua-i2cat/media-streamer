#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"

FILE *F_video_rx=NULL;
char *OUTPUT_PATH = "/home/palau/Videos/rx_frame.yuv";

int main(){
  participant_list_t *part_list;
  stream_list_t *stream_list;
  receiver_t *receiver;
  
  part_list = init_participant_list();
  stream_list = init_stream_list();
  
  add_participant(part_list, 1, INPUT, RTP, NULL, 0);
  
  receiver = init_receiver(part_list, stream_list, 5004);
  
  if (start_receiver(receiver)) {

	printf("First 200 frames to disk\n");
	  
    int i = 0;
	while(i < 200){
		pthread_rwlock_rdlock(&stream_list->lock);
		if (stream_list->first != NULL){
			pthread_mutex_lock(&stream_list->first->video.new_decoded_frame_lock);
			if (stream_list->first->video.new_decoded_frame){
				if (F_video_rx == NULL) {
					printf("recording rx frame...\n");
					F_video_rx = fopen(OUTPUT_PATH, "wb");
				}
				pthread_rwlock_rdlock(&stream_list->first->video.lock);
				fwrite(stream_list->first->video.decoded_frame, stream_list->first->video.decoded_frame_len, 1,F_video_rx);
				pthread_rwlock_unlock(&stream_list->first->video.lock);
				stream_list->first->video.new_decoded_frame = FALSE;
				i++;
			}
			pthread_mutex_unlock(&stream_list->first->video.new_decoded_frame_lock);
		}
		pthread_rwlock_unlock(&stream_list->lock);
    }
    
	printf("Disabling flow\n");
    set_stream_state(stream_list->first, NON_ACTIVE);
    printf("Flow disabled\n");
	sleep(10);
	printf("Enabling flow\n");
	set_stream_state(stream_list->first, ACTIVE);
	printf("Flow enabled\n");
    
	printf("Next 400 frames to disk\n");
	
	i=0;
	while(i < 400){
		pthread_rwlock_rdlock(&stream_list->lock);
		if (stream_list->first != NULL){
			pthread_rwlock_rdlock(&stream_list->first->lock);
			pthread_mutex_lock(&stream_list->first->video.new_decoded_frame_lock);
			if (stream_list->first->video.new_decoded_frame){
				if (F_video_rx == NULL) {
					printf("recording rx frame...\n");
					F_video_rx = fopen(OUTPUT_PATH, "wb");
				}
				pthread_rwlock_rdlock(&stream_list->first->video.lock);
				fwrite(stream_list->first->video.decoded_frame, stream_list->first->video.decoded_frame_len, 1,F_video_rx);
				pthread_rwlock_unlock(&stream_list->first->video.lock);
				stream_list->first->video.new_decoded_frame = FALSE;
				i++;
			}
			pthread_mutex_unlock(&stream_list->first->video.new_decoded_frame_lock);
			pthread_rwlock_unlock(&stream_list->first->lock);
		}
		pthread_rwlock_unlock(&stream_list->lock);
    }
    
    
    stop_receiver(receiver);
    destroy_stream_list(stream_list);
    destroy_participant_list(part_list);

	printf("Finished\n");
  }
}
