#define MAX_SUBSTREAMS  1

#include "rtp/pbuf.h"

struct recieved_data{
    uint32_t buffer_len[MAX_SUBSTREAMS];
    uint32_t buffer_num[MAX_SUBSTREAMS];
    char *frame_buffer[MAX_SUBSTREAMS];
    uint8_t bframe;
	uint8_t iframe;
};

int decode_frame(struct coded_data *cdata, void *decode_data);

int decode_frame_h264(struct coded_data *cdata, void *rx_data);