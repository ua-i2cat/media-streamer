#ifndef _STREAM_H_
#define _STREAM_H_

#include "config_unix.h"
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
       - like the encoder?
    */

    uint8_t *data;
    uint32_t data_len;
    struct state_compress *sc;
} encoder_t;

typedef enum stream_type {
    AUDIO,
    VIDEO
} stream_type_t;

typedef struct audio_stream {
    // TODO
} audio_stream_t;

typedef struct video_stream {
    codec_t codec;
    uint32_t width;
    uint32_t height;
    uint8_t *frame;  // TODO: char *?
    uint32_t frame_len;
} video_stream_t;

typedef struct stream {
    stream_type_t type;
    uint32_t ssrc;
    uint8_t active;
    union {
        audio_stream_t audio;
        video_stream_t video;
    };
    union {
        encoder_t *encoder;
        decoder_t *decoder;
    };
} stream_t;

decoder_t *init_decoder_thread(stream_t *stream);
decoder_t *init_encoder_thread(stream_t *stream);

#endif
