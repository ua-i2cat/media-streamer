/**
 * @file receiver.h
 * @brief Receiver internal data and functions.
 *
 */

#ifndef __RECEIVER_H__
#define __RECEIVER_H__

#include "stream.h"
#include "participants.h"

typedef struct receiver {
    // Video data
    stream_list_t *stream_list;
    int port;
    pthread_t th_id;
    uint8_t run;
    struct rtp *session;
    struct pdb *part_db;

    // Audio data
    stream_list_t *audio_stream_list;
    int audio_port;
    pthread_t audio_th_id;
    uint8_t audio_run;
    struct rtp *audio_session;
    struct pdb *audio_part_db;
} receiver_t;


receiver_t *init_receiver(stream_list_t *stream_list, uint32_t video_port, uint32_t audio_port);
int start_receiver(receiver_t *receiver);
int stop_receiver(receiver_t *receiver);
int destroy_receiver(receiver_t *receiver);

#endif //__RECEIVER_H__

