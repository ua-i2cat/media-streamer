#include "config.h"
#include "io_mngr/participants.h"
#include "io_mngr/receiver.h"
#include "io_mngr/transmitter.h"

int main(){
  participant_list_t *rec_list, *trans_list;
  receiver_t *receiver;
  rtsp_serv_t *server;
  participant_data_t *participant_tmpl, *pt, *participant;
  
  rec_list = init_participant_list();
  trans_list = init_participant_list();
  participant_tmpl = init_participant(0, 0, 0, 0, H264, NULL, 0, OUTPUT);
  
  server = malloc(sizeof(rtsp_serv_t));
  
  server->descriptionString = "i2CAT Rocks!";
  server->streamName = "i2catrocks";
  server->port = 8554;
  server->participant_tmpl = participant_tmpl;
  server->list = trans_list;
  
  participant = init_participant(0, 1, 0, 0, H264, NULL, 0, INPUT);
  add_participant(rec_list, participant);

  receiver = init_receiver(rec_list, 6004);
  
  if (start_receiver(receiver)) {
  
	printf("Transmit up to 100000 frames\n");
	
    int i = 0;
	while(i < 100000){
		usleep(5*1000);

        pthread_mutex_lock(&rec_list->first->lock);
		if (rec_list->first->new_frame){
     
			pthread_rwlock_wrlock(&trans_list->lock);
			if (server->participant_tmpl->width == 0 || server->participant_tmpl->height == 0){
				pthread_mutex_lock(&server->participant_tmpl->lock);
				server->participant_tmpl->width = rec_list->first->width;
				server->participant_tmpl->height = rec_list->first->height;
				pthread_mutex_unlock(&server->participant_tmpl->lock);
				start_out_manager(trans_list, 25, server);
			}
	
			pt = trans_list->first;
			while(pt != NULL){
				if (trans_list->count != 0 && pthread_mutex_trylock(&pt->lock) == 0) {
					memcpy(pt->frame, (char *) rec_list->first->frame, rec_list->first->frame_length);
					pt->new_frame = TRUE;
					rec_list->first->new_frame = FALSE;
					i++;
					pthread_mutex_unlock(&pt->lock);
				}
				pt = pt->next;
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
