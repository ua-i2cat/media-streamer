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

typedef enum {
    RTP,
    RTSP
} participant_protocol_t;

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
    sem_t semaphore;
    // TODO: rtp thread management
} rtp_session_t;

typedef struct rtsp_session {
    uint32_t port;
    char *addr;
    // TODO: rtsp thread management
} rtsp_session_t;

struct participant_data {
    pthread_mutex_t lock;
    uint32_t ssrc;
    uint32_t id;
    uint8_t active;
    participant_data_t *next;
    participant_data_t *previous;
    participant_protocol_t protocol;
    io_type_t type;
    union {
        rtp_session_t rtp;
        rtsp_session_t rtsp;
    };
    // One participant may have more than one stream.
    uint8_t streams_count;
    stream_data_t *streams[MAX_PARTICIPANT_STREAMS];
};


participant_list_t *init_participant_list(void);
void destroy_participant_list(participant_list_t *list);

int add_participant(participant_list_t *list, int id, io_type_t part_type, participant_protocol_t prot_type, char *addr, uint32_t port);
int remove_participant(participant_list_t *list, uint32_t id);

participant_data_t *get_participant_id(participant_list_t *list, uint32_t id);
participant_data_t *get_participant_ssrc(participant_list_t *list, uint32_t ssrc);

participant_data_t *init_participant(uint32_t id, io_type_t type, participant_protocol_t protocol, char *addr, uint32_t port);
void set_active_participant(participant_data_t *participant, uint8_t active);
void destroy_participant(participant_data_t *src);

int add_participant_stream(participant_data_t *participant, stream_data_t *stream);
int remove_participant_stream(participant_data_t *participant, stream_data_t *stream);

#endif
