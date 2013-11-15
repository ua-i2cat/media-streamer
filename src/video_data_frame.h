#include "config_unix.h"
#include "types.h"

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

#define CQ_FULL 1
#define CQ_OK 0
#define CQ_EMPTY -1

typedef enum frame_type {
    INTRA,
    BFRAME,
    OTHER
} frame_type_t;

typedef struct video_frame_data {
    uint8_t *buffer;
    uint32_t buffer_len;
    uint32_t curr_seqno;
    uint32_t width;
    uint32_t height;
    uint32_t media_time;
    frame_type_t frame_type;
    codec_t codec;
} video_frame_data_t

typedef struct video_frame_cq {
    uint32_t rear;
    uint32_t front;
    uint32_t max;
    uint32_t timeout; //in usec
    int state;
	video_frame_data_t **frames;
} video_frame_cq_t;

video_frame_cq_t *init_video_frame_cq(uint32_t max, uint32_t timeout);
int destroy_video_frame_cq(video_frame_cq_t *frame_cq);
int set_video_frame_cq(video_frame_cq_t *frame_cq, codec_t codec, uint32_t width, uint32_t height);
video_data_frame_t* get_in_frame(video_frame_cq_t *frame_cq);
int increase_rear_frame(video_frame_cq_t *frame_cq);
