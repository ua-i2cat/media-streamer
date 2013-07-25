#include "participants.h"
#include "reciever.h"

int main(){
  participant_list_t *list;
  reciever_t *reciever;
  
  list = init_participant_list();
  
  add_participant(list, 1, 1920, 1080, H264, NULL, 0, INPUT);
  add_participant(list, 2, 1920, 1080, H264, NULL, 0, INPUT);
  add_participant(list, 3, 1920, 1080, H264, NULL, 0, INPUT);
  add_participant(list, 4, 1920, 1080, H264, NULL, 0, INPUT);
  add_participant(list, 5, 1920, 1080, H264, NULL, 0, INPUT);

  reciever = init_reciever(list, 5004);
  
  if (start_reciever(reciever)) {
  
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
  
    stop_reciever(reciever);
  
    pthread_join(reciever->th_id, NULL);
  }
}