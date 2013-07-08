#include "rtp/rtpenc.h"

void tx_init_h264(void);

void tx_send_base_h264(struct tile *tile, struct rtp *rtp_session,
                       uint32_t ts, int send_m, codec_t color_spec,
                       double input_fps, enum interlacing_t interlacing,
                       unsigned int substream, int fragment_offset);

void rtpenc_h264_stats_print(void);
