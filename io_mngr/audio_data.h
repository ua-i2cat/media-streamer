#ifndef __AUDIO_DATA_H__
#define __AUDIO_DATA_H__

//#include <semaphore.h>
//#include "config_unix.h"
//#include "types.h"
//#include "video_data_frame.h"


typedef enum audio_type {
    ENCODER,
    DECODER,
    NONE
} audio_type_t;

typedef struct decoder_thread {
    pthread_t thread;
    uint8_t run;
    struct state_decompress *sd;
    uint32_t last_seqno;
} decoder_thread_t;

typedef struct encoder_thread {
    pthread_t thread;
    uint8_t run;

    int index;

    struct video_frame *frame;  // TODO: should be gone!
                                // redundant with stream->coded_frame
    //pthread_mutex_t output_lock;
    //pthread_cond_t output_cond;
    struct compress_state *cs;
} encoder_thread_t;

typedef struct audio_data {
    audio_type_t type;
    audio_frame_cq_t *decoded_frames;
    audio_frame_cq_t *coded_frames;
    uint32_t interlacing;  //TODO: fix this. It has to be UG enum
    uint32_t fps;       //TODO: fix this. It has to be UG enum
    uint32_t seqno;
    union {
        struct encoder_thread *encoder;
        struct decoder_thread *decoder;
    };
} audio_data_t;

decoder_thread_t *init_decoder(audio_data_t *data);
encoder_thread_t *init_encoder(audio_data_t *data);

void start_decoder(audio_data_t *v_data);
void destroy_decoder(decoder_thread_t *decoder);
void destroy_encoder(audio_data_t *data);
void stop_decoder(audio_data_t *data);
void stop_encoder(audio_data_t *data);

audio_data_t *init_audio_data(audio_type_t type);
int destroy_audio_data(audio_data_t *data);

#endif //__AUDIO_DATA_H__

