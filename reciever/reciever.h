
typedef struct reciever {
  participant_list_t 	*list;
  int 			port;
  pthread_t		th_id;
  uint8_t		run;
  struct rtp 		*session;
  struct pdb 		*part_db;
} reciever_t;


int start_reciever(reciever_t *recv);

int stop_reciever(reciever_t *reciever);