#ifndef _PARTICIPANT_H_
#define _PARTICIPANT_H_

#include "stream.h"

typedef enum transport_protocol {
    RTP;
    RTSP;
} transport_protocol_t;

typedef enum participant_type {
    INPUT;
    OUTPUT;
} participant_type_t;

typedef enum rtp_session {
    uint32_t port;
    char *addr;
} rtp_session_t;

typedef enum rtsp_session {
    uint32_t port;
    char *addr;
    // TODO: RTSP fields
} rtsp_session_t;

typedef struct participant_struct {
    uint32_t id;
    participant_type_t type;
    transport_protocol_t protocol;
    uint32_t ssrc;
    union {
        rtp_session_t *rtp;
        rtsp_session_t *rtsp;
    };
    stream_t **streams;
    participant_t *prev;
    participant_t *next;
} participant_t;

typedef struct participant_list {
    pthread_rwlock_t lock;
    int count;
    participant_t *first;
    participant_t *last;
} participant_list_t;

participant_list_t *init_participant_list(void);

encoder_t *init_encoder(participant_t *participant);

decoder_t *init_decoder(participant_t *participant);

int add_participant(participant_list_t *list,
                    uint32_t id,
                    participant_type_t type,
                    transport_protocol_t protocol,
                    uint32_t ssrc);

int add_input_participant(participant_list_t *list,
                          uint32_t id,
                          transport_protocol_t protocol,
                          uint32_t ssrc);

int add_output_participant(participant_list_t *list,
                           uint32_t id,
                           transport_protocol_t protocol);

int remove_participant(participant_list_t *list, uint32_t id);

participant_t *get_participant_id(participant_list_t *list,
                                  uint32_t id);

participant_t *get_participant_ssrc(participant_list_t *list,
                                    uint32_t ssrc);

#endif
