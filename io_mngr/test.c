#include "config.h"
#include "participants.h"
#include "receiver.h"

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
  
    /*sleep(10);
  
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
