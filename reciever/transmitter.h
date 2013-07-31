#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "participants.h"

int start_out_manager(participant_list_t *list);
int stop_out_manager(void);
void transmitter_destroy_encoder_thread(encoder_thread_t **encoder);

#endif
