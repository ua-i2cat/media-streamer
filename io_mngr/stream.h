#ifndef _STREAM_H_
#define _STREAM_H_

#include "config_unix.h"
#include "types.h"
#include <pthread.h>

typedef struct decoder {
    pthread_t thread;
    uint8_t run;

    pthread_mutex_t lock;
    pthread_cond_t notify_frame;
    uint8_t new_frame;

    uint8_t *data;
    uint32_t data_len;
    struct state_decompress *sd;
} decoder_t;

typedef struct encoder {
    pthread_t thread;
    uint8_t run;

    pthread_mutex_t lock;

    /* TODO:
       thread management:
       - like the decoder?
    */

    int index;
    struct video_frame *frame;
    uint8_t *input_frame;
    uin32_t input_frame_len;

    sem_t input_sem;
    sem_t output_sem;

    uint8_t *data;
    uint32_t data_len;
    struct state_compress *sc;
} encoder_t;

typedef enum stream_type {
    AUDIO,
    VIDEO
} stream_type_t;

typedef struct audio_data {
    // TODO
} audio_data_t;

typedef struct video_data {
    codec_t codec;
    uint32_t width;
    uint32_t height;
    uint8_t *frame;  // TODO: char *?
    uint32_t frame_len;
} video_data_t;

typedef struct stream_data {
    stream_type_t type;
    uint32_t id;
    uint8_t active;
    stream_data_t *prev;
    stream_data_t *next;
    union {
        audio_data_t audio;
        video_data_t video;
    };
    union {
        encoder_t *encoder;
        decoder_t *decoder;
    };
} stream_data_t;

typedef struct stream_list {
    pthread_rwlock_t lock;
    int count;
    stream_data_t *first;
    stream_data_t *last;
} stream_list_t;

decoder_t *init_decoder(stream_data_t *stream);
decoder_t *init_encoder(stream_data_t *stream);

int *destroy_decoder(stream_data_t *stream);
int *destroy_encoder(straem_data_t *stream);

stream_list_t *init_stream_list(void);
int *destroy_stream_list(stream_list_t *list);

stream_data_t *init_video_stream(stream_type_t type, uint32_t id, uint8_t active);
int destroy_stream(stream_data_t *stream);

int set_stream_video_data(stream_data_t *stream, codec_t codec,
                          uint32_t width, uint32_t height);
// TODO set_stream_audio_data

int add_stream(stream_list_t *list, stream_data_t *stream);
int remove_stream(stream_list *list, uint32_t id);

#endif
