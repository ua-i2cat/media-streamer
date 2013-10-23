#include "participants.h"

typedef struct receiver {
  participant_list_t *participant_list;
  stream_list_t	*stream_list;
  int port;
  pthread_t	th_id;
  uint8_t run;
  struct rtp *session;
  struct pdb *part_db;
} receiver_t;


int start_receiver(receiver_t *recv);

receiver_t *init_receiver(participant_list_t *participant_list, stream_list_t *stream_list, uint32_t port);

int stop_receiver(receiver_t *receiver);