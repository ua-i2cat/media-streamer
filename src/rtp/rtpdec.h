#define MAX_SUBSTREAMS  1

#include "pbuf.h"

int decode_frame(struct coded_data *cdata, void *decode_data);

int decode_frame_h264(struct coded_data *cdata, void *rx_data);