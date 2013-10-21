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

#define I_AWAIT 2

typedef struct participant_data participant_data_t;

typedef enum {INPUT, OUTPUT} participant_type_t;

struct participant_data {
    pthread_mutex_t lock;
    uint32_t ssrc;
    uint32_t id;
    participant_data_t *next;
    participant_data_t *previous;
    participant_type_t type;
    struct rtp_session *session;
    struct recieved_data *rx_data;
    stream_t **streams;
};

typedef struct participant_list {
    pthread_rwlock_t lock;
    int count;
    participant_data_t *first;
    participant_data_t *last;
} participant_list_t;

typedef struct rtp_session {
    uint32_t port;
    char *addr;
} rtp_session_t;


participant_list_t *init_participant_list(void);
void destroy_participant_list(participant_list_t *list);


int add_participant(participant_list_t *list, int id, int width, int height, codec_t codec, char *dst, uint32_t port, participant_type_t type);

participant_data_t *get_participant_id(participant_list_t *list, uint32_t id);
participant_data_t *get_participant_ssrc(participant_list_t *list, uint32_t ssrc);

int remove_participant(participant_list_t *list, uint32_t id);


void destroy_participant(participant_data_t *src);

void set_active_participant(participant_data_t *participant, uint8_t active);

participant_data_t *init_participant(int id, int width, int height, codec_t codec, char *dst, uint32_t port, participant_type_t type);

#endif
