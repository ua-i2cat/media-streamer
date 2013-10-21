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
#define MAX_PARTICIPANT_STREAMS 4

typedef struct participant_data participant_data_t;

typedef enum {
    INPUT,
    OUTPUT
} participant_type_t;

typedef enum {
    RTP,
    RTSP
} participant_protocol_t

struct participant_data {
    pthread_mutex_t lock;
    participant_type_t type;
    uint32_t ssrc;
    uint32_t id;
    participant_data_t *next;
    participant_data_t *previous;
    participant_protocol_t protocol;
    union {
        rtp_session_t *rtp;
        rtsp_session_t *rtsp;
    }
    struct rtp_session *session;
    struct recieved_data *rx_data;

    // One participant may have more than one stream.
    uint8_t streams_count;
    stream_t *streams[MAX_PARTICIPANT_STREAMS];
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
    // TODO: rtp thread management
} rtp_session_t;

typedef struct rtsp_session {
    uint32_t port;
    char *addr;
    // TODO: rtsp thread management
} rtsp_session_t;


participant_list_t *init_participant_list(void);
void destroy_participant_list(participant_list_t *list);

int add_participant(participant_list_t *list, int id, char *dst, uint32_t port, participant_type_t type);
int remove_participant(participant_list_t *list, uint32_t id);

participant_data_t *get_participant_id(participant_list_t *list, uint32_t id);
participant_data_t *get_participant_ssrc(participant_list_t *list, uint32_t ssrc);

participant_data_t *init_participant(int id, char *dst, uint32_t port, participant_type_t type, participant_protocol_t protocol);
void set_active_participant(participant_data_t *participant, uint8_t active);
void destroy_participant(participant_data_t *src);

int add_participant_stream(participant_data_t *participant, stream_data_t *stream);
int remove_participant_stream(participant_data_t *participant, stream_data_t *stream);

#endif
