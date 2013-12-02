#include <semaphore.h>
#include "config_unix.h"
#include "types.h"
#include "video_data_frame.h"
#include "commons.h"

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

typedef struct video_data {
    role_t type;
    video_frame_cq_t *decoded_frames;
    video_frame_cq_t *coded_frames;
    uint32_t interlacing;  //TODO: fix this. It has to be UG enum
    uint32_t fps;       //TODO: fix this. It has to be UG enum
    uint32_t seqno;
    union {
        struct encoder_thread *encoder;
        struct decoder_thread *decoder;
    };
} video_data_t;

decoder_thread_t *init_decoder(video_data_t *data);
encoder_thread_t *init_encoder(video_data_t *data);

void start_decoder(video_data_t *v_data);
void destroy_decoder(decoder_thread_t *decoder);
void destroy_encoder(video_data_t *data);
void stop_decoder(video_data_t *data);
void stop_encoder(video_data_t *data);

video_data_t *init_video_data(role_t type);
int destroy_video_data(video_data_t *data);
