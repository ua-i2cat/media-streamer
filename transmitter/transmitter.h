#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "rtp/rtp.h"
#include "video_compress.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/sem.h>

struct rtp_session {
    uint32_t    port;
    char        *addr;
    //struct rtp  *rtp;
};

struct participant_data {
    pthread_mutex_t         lock;
    int                     *new;
    uint32_t                ssrc;
    char                    *frame;
    int                     frame_length;
    struct participant_data *next;
    struct participant_data *previous;
    uint32_t                width;
    uint32_t                height;
    codec_t                 codec;
    struct rtp_session      *session;
    union {
        struct encoder_th   *encoder;
        struct decoder_th   *decoder;
    };
};

struct encoder_th {
    pthread_t               thread;
    struct rtpenc_th        *rtpenc;
    struct compress_state   *sc;
    int                     index;
    struct video_frame      *frame;
    sem_t                   input_sem;
    sem_t                   output_sem;
};

struct decoder_th {
};

struct rtpenc_th {
    pthread_t   *thread;
};

struct participant_list {
    pthread_mutex_t         lock;
    int                     count;
    struct participant_data *first;
    struct participant_data *last;
};

int start_out_manager(struct participant_list *list, uint32_t port);
int stop_out_manager(void);

#endif