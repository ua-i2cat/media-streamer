#define MAX_SUBSTREAMS  1

#include "rtp/pbuf.h"

#define INTRA  1
#define BFRAME 0
#define OTHER  2

struct stream_info{
	uint32_t width;
	uint32_t height;
};

struct recieved_data{
    uint32_t buffer_len[MAX_SUBSTREAMS];
    uint32_t buffer_num[MAX_SUBSTREAMS];
    char *frame_buffer[MAX_SUBSTREAMS];
    uint8_t frame_type; 
    struct stream_info info;
};

int decode_frame(struct coded_data *cdata, void *decode_data);

int decode_frame_h264(struct coded_data *cdata, void *rx_data);

int init_rx_data(struct recieved_data *rx_data);