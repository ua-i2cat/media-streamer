#include "participants.h"

typedef struct reciever {
  participant_list_t 	*list;
  int 			port;
  pthread_t		th_id;
  uint8_t		run;
  struct rtp 		*session;
  struct pdb 		*part_db;
} reciever_t;


int start_reciever(reciever_t *recv);

reciever_t *init_reciever(participant_list_t *list, int port);

int stop_reciever(reciever_t *reciever);

void init_decoder(participant_data_t *src);

void *reciever_thread(reciever_t *reciever); 

void *decoder_th(void* participant);
