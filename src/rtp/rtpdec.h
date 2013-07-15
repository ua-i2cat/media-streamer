#define MAX_SUBSTREAMS  1

struct recieved_data{
    uint32_t buffer_len[MAX_SUBSTREAMS];
    uint32_t buffer_num[MAX_SUBSTREAMS];
    char *frame_buffer[MAX_SUBSTREAMS];
};

int decode_frame(struct coded_data *cdata, void *decode_data);