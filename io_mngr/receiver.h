#include "stream.h"
#include "participants.h"

typedef struct receiver {
  stream_list_t	*stream_list;
  int port;
  pthread_t	th_id;
  uint8_t run;
  struct rtp *session;
  struct pdb *part_db;
} receiver_t;


receiver_t *init_receiver(stream_list_t *stream_list, uint32_t port);

int stop_receiver(receiver_t *receiver);
int start_receiver(receiver_t *receiver);
int destroy_receiver(receiver_t *receiver);
