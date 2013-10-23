#ifndef _STREAM_H_
#define _STREAM_H_

#include "config_unix.h"
#include "types.h"
#include <pthread.h>
#include <semaphore.h>

typedef struct decoder_thread {
    pthread_t thread;
    uint8_t run;

    pthread_mutex_t lock;
    pthread_cond_t notify_frame;
    uint8_t new_frame;

    struct state_decompress *sd;
} decoder_thread_t;

typedef struct encoder_thread {
    pthread_t thread;
    uint8_t run;

    pthread_mutex_t lock;

    /* TODO:
       thread management:
       - like the decoder?
    */

    int index;

    sem_t input_sem;
    sem_t output_sem;

    struct compress_state *cs;
} encoder_thread_t;

typedef enum stream_type {
    AUDIO,
    VIDEO
} stream_type_t;

typedef enum io_type {
    INPUT,
    OUTPUT
} io_type_t;

typedef struct audio_data {
    // TODO
} audio_data_t;

typedef struct video_data {
    pthread_rwlock_t lock;
    pthread_rwlock_t coded_frame_seqno_lock;
    pthread_rwlock_t decoded_frame_seqno_lock;
    codec_t codec;
    uint32_t width;
    uint32_t height;
    uint8_t *decoded_frame;  // TODO: char *?
    uint32_t decoded_frame_len;
    uint32_t decoded_frame_seqno;
    uint8_t *coded_frame;
    uint32_t coded_frame_len;
    uint32_t coded_frame_seqno;
} video_data_t;

typedef struct stream_data {
    pthread_rwlock_t lock;
    stream_type_t type;
    io_type_t io_type;
    uint32_t id;
    uint8_t active;
    uint8_t new_frame;
    struct stream_data *prev;
    struct stream_data *next;
    union {
        audio_data_t audio;
        video_data_t video;
    };
    union {
        struct encoder_thread *encoder;
        struct decoder_thread *decoder;
    };
} stream_data_t;

typedef struct stream_list {
    pthread_rwlock_t lock;
    int count;
    stream_data_t *first;
    stream_data_t *last;
} stream_list_t;

decoder_thread_t *init_decoder(stream_data_t *stream);
encoder_thread_t *init_encoder(stream_data_t *stream);

void start_decoder(stream_data_t *stream);

void destroy_decoder(decoder_thread_t *decoder);
void destroy_encoder(encoder_thread_t *encoder);

stream_list_t *init_stream_list(void);
void destroy_stream_list(stream_list_t *list);

stream_data_t *init_stream(stream_type_t type, io_type_t io_type, uint32_t id, uint8_t active);
int destroy_stream(stream_data_t *stream);

int set_stream_video_data(stream_data_t *stream, codec_t codec,
                          uint32_t width, uint32_t height);
// TODO set_stream_audio_data

int add_stream(stream_list_t *list, stream_data_t *stream);
int remove_stream(stream_list_t *list, uint32_t id);

stream_data_t *get_stream_id(stream_list_t *list, uint32_t id);

#endif
