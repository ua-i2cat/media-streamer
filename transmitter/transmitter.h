#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "participants.h"

int start_out_manager(struct participant_list *list, uint32_t port);
int stop_out_manager(void);

#endif
