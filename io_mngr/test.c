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
  add_participant(list, 2, 1280, 720, H264, NULL, 0, INPUT);
  add_participant(list, 3, 1280, 720, H264, NULL, 0, INPUT);
  add_participant(list, 4, 1280, 720, H264, NULL, 0, INPUT);
  add_participant(list, 5, 1280, 720, H264, NULL, 0, INPUT);

  receiver = init_receiver(list, 5004);
  
  if (start_receiver(receiver)) {
  
    /*
    while(1){
      usleep(10*1000);
      if (list->first->new_frame){
	pthread_mutex_lock(&list->first->lock);
	if (F_video_rx == NULL) {
	    printf("recording rx frame...\n");
	    F_video_rx = fopen(OUTPUT_PATH, "wb");
	}

	fwrite(list->first->frame, list->first->frame_length, 1,F_video_rx);
	pthread_mutex_unlock(&list->first->lock);
      }
    }
    
    sleep(10);
  
    pthread_rwlock_wrlock(&list->lock);
    if (remove_participant(list,3))
      printf("PARTICIPANT REMOVED\n\n");
    pthread_rwlock_unlock(&list->lock);
    
    
    
    printf("List count: %d\n\n", list->count);
    
    pthread_rwlock_wrlock(&list->lock);
    if (remove_participant(list,2))
      printf("PARTICIPANT REMOVED\n\n");
    pthread_rwlock_unlock(&list->lock);
    
    
    printf("List count: %d\n\n", list->count);
  
    sleep(3);
  
    stop_receiver(receiver);*/
  
    pthread_join(receiver->th_id, NULL);
  }
}
