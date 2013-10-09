#include "config.h"
#include "participants.h"
#include "receiver.h"

FILE *F_video_rx=NULL;
char *OUTPUT_PATH = "rx_frame.yuv";


int main(){
  participant_list_t *list;
  receiver_t *receiver;
  
  list = init_participant_list();
  
  add_participant(list, 1, 1280, 720, H264, NULL, 0, INPUT);
  
  receiver = init_receiver(list, 5004);
  
  if (start_receiver(receiver)) {
  
	printf("First 100 frames to disk\n");
	  
    int i = 0;
	while(i < 100){
		usleep(10*1000);
		if (list->first->new_frame){
			pthread_mutex_lock(&list->first->lock);
			if (F_video_rx == NULL) {
				printf("recording rx frame...\n");
				F_video_rx = fopen(OUTPUT_PATH, "wb");
			}
			
			list->first->new_frame = FALSE;
			fwrite(list->first->frame, list->first->frame_length, 1,F_video_rx);
			i++;
			pthread_mutex_unlock(&list->first->lock);
		}
    }
    
	printf("Disabling flow\n");
    set_active_participant(list->first, FALSE);
	sleep(15);
	set_active_participant(list->first, TRUE);
	printf("Enabling flow\n");
    
	printf("First 100 frames to disk\n");
	
	i = 0;
	while(i < 100){
		usleep(10*1000);
		if (list->first->new_frame){
			pthread_mutex_lock(&list->first->lock);
			if (F_video_rx == NULL) {
				printf("recording rx frame...\n");
				F_video_rx = fopen(OUTPUT_PATH, "wb");
			}
			
			list->first->new_frame = FALSE;
			fwrite(list->first->frame, list->first->frame_length, 1,F_video_rx);
			i++;
			pthread_mutex_unlock(&list->first->lock);
		}
    }
    
    printf("Stopping reciever\n");
    
    stop_receiver(receiver);
	
	printf("Awaiting decoders to finish\n");
  
    pthread_join(receiver->th_id, NULL);
  }
}
