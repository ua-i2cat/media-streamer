#include "config_unix.h"
#include "types.h"

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

typedef enum frame_type {
    INTRA,
    BFRAME,
    OTHER
} frame_type_t;

typedef struct video_data_frame {
    pthread_rwlock_t lock;
	uint32_t width;
	uint32_t height;
	uint8_t *buffer;  // TODO: char *?
    uint32_t buffer_len;
    uint32_t seqno;
    uint32_t media_time;
    frame_type_t frame_type;
    codec_t codec;
} video_data_frame_t;

video_data_frame_t *init_video_data_frame();
int destroy_video_data_frame(video_data_frame_t *frame);
int set_video_data_frame(video_data_frame_t *frame, codec_t codec, uint32_t width, uint32_t height);
