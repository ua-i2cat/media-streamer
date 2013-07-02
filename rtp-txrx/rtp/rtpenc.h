
#include "test/video.h"


//#define CLOCK_REALTIME 1
#define GET_STARTTIME clock_gettime(CLOCK_REALTIME, &start)
#define GET_STOPTIME clock_gettime(CLOCK_REALTIME, &stop)
#define GET_DELTA delta = stop.tv_nsec - start.tv_nsec

unsigned int buffer_id;
unsigned mtu = 1000;
int bitrate;
long packet_rate;

void tx_init();

void tx_send_base(struct tile *tile, struct rtp *rtp_session,
                uint32_t ts, int send_m,
                codec_t color_spec, double input_fps,
                enum interlacing_t interlacing, unsigned int substream,
                int fragment_offset);
