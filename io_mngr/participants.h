/**
 * @file participants.h
 * @brief Participants data structures and functions.
 *
 */

#ifndef _PARTICIPANTS_H_
#define _PARTICIPANTS_H_

#include <semaphore.h>
#include "config_unix.h"
#include "rtpdec.h"
#include "video.h"
#include "stream.h"
#include "debug.h"

#define MAX_PARTICIPANT_STREAMS 4

typedef struct participant_data participant_data_t;

typedef struct participant_list {
    pthread_rwlock_t lock;
    int count;
    participant_data_t *first;
    participant_data_t *last;
} participant_list_t;

typedef struct rtp_session {
    pthread_t thread;
    uint8_t run;
    uint32_t port;
    char *addr;
} rtp_session_t;

struct participant_data {
    pthread_mutex_t lock;
    uint32_t ssrc;
    uint32_t id;
    uint8_t active;
    participant_data_t *next;
    participant_data_t *previous;
    io_type_t type;
    rtp_session_t rtp;
    // One participant may have more than one stream.
    uint8_t has_stream;
    stream_data_t *stream;
};


participant_list_t *init_participant_list(void);
void destroy_participant_list(participant_list_t *list);

int add_participant(participant_list_t *list, participant_data_t *participant);
int remove_participant(participant_list_t *list, uint32_t id);

participant_data_t *get_participant_id(participant_list_t *list, uint32_t id);
participant_data_t *get_participant_ssrc(participant_list_t *list, uint32_t ssrc);
participant_data_t *get_participant_non_init(participant_list_t *list);
int get_participant_from_stream_id(participant_list_t *list, uint32_t stream_id);

int set_participant(participant_data_t *participant, uint32_t ssrc);

participant_data_t *init_participant(uint32_t id, io_type_t type, char *addr, uint32_t port);
void set_active_participant(participant_data_t *participant, uint8_t active);
void destroy_participant(participant_data_t *src);

int add_participant_stream(participant_data_t *participant, stream_data_t *stream);
int remove_participant_stream(participant_data_t *participant);

#endif
