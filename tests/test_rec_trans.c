#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"

int main(){
  participant_list_t *rec_list, *trans_list;
  receiver_t *receiver;
  
  rec_list = init_participant_list();
  trans_list = init_participant_list();
  
  add_participant(rec_list, 1, 1280, 720, H264, NULL, 0, INPUT);
  add_participant(trans_list, 1, 1280, 720, H264, "127.0.0.1", 6004, OUTPUT);
  
  receiver = init_receiver(rec_list, 5004);
  
  if (start_receiver(receiver)) {

    start_out_manager(trans_list, 25);
  
	printf("Transmit up to 1000 frames to disk\n");
	  
    int i = 0;
	while(i < 1000){
		usleep(10*1000);

        pthread_mutex_lock(&rec_list->first->lock);
		if (rec_list->first->new_frame){

            
			pthread_rwlock_wrlock(&trans_list->lock);

            if (pthread_mutex_trylock(&trans_list->first->lock) == 0) {
                    memcpy(trans_list->first->frame, (char *) rec_list->first->frame, rec_list->first->frame_length);
                    rec_list->first->new_frame = FALSE;
                    trans_list->first->new_frame = TRUE;
                    i++;
                    pthread_mutex_unlock(&trans_list->first->lock);
            }
            pthread_rwlock_unlock(&trans_list->lock); 
            
        }
        pthread_mutex_unlock(&rec_list->first->lock);
	}
  }
    
    

  printf(" deallocating resources and terminating threads\n");
  stop_out_manager();
  stop_receiver(receiver);
  destroy_participant_list(trans_list);
  destroy_participant_list(rec_list);
  pthread_join(receiver->th_id, NULL);
}
