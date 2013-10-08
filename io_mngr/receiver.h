#include "participants.h"

typedef struct receiver {
  participant_list_t 	*list;
  int 			port;
  pthread_t		th_id;
  uint8_t		run;
  struct rtp 		*session;
  struct pdb 		*part_db;
} receiver_t;


int start_receiver(receiver_t *recv);

receiver_t *init_receiver(participant_list_t *list, int port);

int stop_receiver(receiver_t *receiver);

void init_decoder(participant_data_t *src);

void *receiver_thread(receiver_t *receiver); 

void *decoder_th(void* participant);
