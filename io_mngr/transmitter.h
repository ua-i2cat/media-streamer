/**
 * @file transmitter.h
 * @brief Output manager functions.
 *
 * Allows the compression and transmition over RTP of uncompressed video
 * (RGB interleaved 4:4:4, according to <em>ffmpeg</em>: <em>rgb24</em>),
 * using the participants.h API.
 *
 */

#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "participants.h"

#define DEFAULT_FPS 24
#define DEFAULT_RECV_PORT 12006 // just trying to not interfere with anything
#define DEFAULT_RTCP_BW 5 * 1024 * 1024 * 10
#define DEFAULT_TTL 255
#define DEFAULT_SEND_BUFFER_SIZE 1920 * 1080 * 4 * sizeof(char) * 10
#define PIXEL_FORMAT RGB
#define MTU 1300 // 1400

typedef struct transmitter {
    uint32_t run;
    participant_list_t *participants;
    stream_list_t *stream_list;
    float fps;
    float wait_time;
    uint32_t recv_port;
    uint32_t ttl;
    uint64_t send_buffer_size;
    uint32_t mtu;
} transmitter_t;


transmitter_t *init_transmitter(stream_list_t *list, float fps);
int start_transmitter(transmitter_t *transmitter);
int stop_transmitter(transmitter_t *transmitter);

int init_transmission(participant_data_t *participant, transmitter_t *transmitter);
int stop_transmission(participant_data_t *participant);

int add_transmitter_participant(transmitter_t *transmitter, uint32_t id, char *addr, uint32_t port);

#endif
