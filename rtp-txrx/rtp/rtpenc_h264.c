#include "config.h"
#include "config_unix.h"
#include "debug.h"
#include "ntp.h"
#include "tv.h"
#include "rtp/rtp.h"
#include "rtp/pbuf.h"
#include "pdb.h"
#include "rtp/rtp_callback.h"
#include "tfrc.h"
#include "rtp/rtpenc_h264.h"

#define MAX_NALS 1024

static const uint8_t *rtp_find_startcode_internal(const uint8_t *p, const uint8_t *end)
{
    const uint8_t *a = p + 4 - ((intptr_t)p & 3);

    for (end -= 3; p < a && p < end; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }

    for (end -= 3; p < end; p += 4) {
        uint32_t x = *(const uint32_t*)p;
//      if ((x - 0x01000100) & (~x) & 0x80008000) // little endian
//      if ((x - 0x00010001) & (~x) & 0x00800080) // big endian
        if ((x - 0x01010101) & (~x) & 0x80808080) { // generic
            if (p[1] == 0) {
                if (p[0] == 0 && p[2] == 1)
                    return p;
                if (p[2] == 0 && p[3] == 1)
                    return p+1;
            }
            if (p[3] == 0) {
                if (p[2] == 0 && p[4] == 1)
                    return p+2;
                if (p[4] == 0 && p[5] == 1)
                    return p+3;
            }
        }
    }

    for (end += 3; p < end; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }

    return end + 3;
}

const uint8_t *rtp_find_startcode(const uint8_t *p, const uint8_t *end){
    const uint8_t *out= rtp_find_startcode_internal(p, end);
    if(p<out && out<end && !out[-1]) out--;
    return out;
}

struct rtp_nal_t {
    uint8_t *data;
    int size;
};

int rtp_parse_nal_units(const uint8_t *buf_in, int size, struct rtp_nal_t *nals, int *nnals)
{
    const uint8_t *p = buf_in;
    const uint8_t *end = p + size;
    const uint8_t *nal_start, *nal_end;

    size = 0;
    nal_start = rtp_find_startcode(p, end);
    *nnals = 0;
    for (;;) {
        while (nal_start < end && !*(nal_start++));
        if (nal_start == end)
            break;

        nal_end = rtp_find_startcode(nal_start, end);
        int nal_size = 4 + nal_end - nal_start;
        size += nal_size;

        nals[(*nnals)].data = nal_start;
        nals[(*nnals)].size = nal_size;
        (*nnals)++;

        nal_start = nal_end;
    }
    return size;
}


void tx_init_h264()
{
    buffer_id =  lrand48() & 0x3fffff;
    bitrate = 6618;
    packet_rate = 1000 * 1000 * 8 / bitrate;
}

void tx_send_base_h264(struct tile *tile, struct rtp *rtp_session,
                       uint32_t ts, int send_m, codec_t color_spec,
                       double input_fps, enum interlacing_t interlacing,
                       unsigned int substream, int fragment_offset)
{


    char *data = tile->data;
    int data_len = tile->data_len;

    struct rtp_nal_t nals[MAX_NALS];
    int nnals;


    rtp_parse_nal_units(data, data_len, nals, &nnals);

    char pt = 96; // h264
    int cc = 0;
    uint32_t csrc = NULL;
    
    char *extn = 0;
    uint16_t extn_len = 0;
    uint16_t extn_type = 0;

    // according to previous code
    int headers_len = 40; //+ (sizeof(video_payload_hdr_t));

    int i;
    for (i = 0; i < nnals; i++) {
        struct rtp_nal_t nal = nals[i];

        int rtp_size = headers_len + nal.size;
        if (rtp_size >= 1000) {
            printf("[WARNING] RTP packet size exceeds the MTU size\n");
        }
        
        int err = rtp_send_data(rtp_session, ts, pt, send_m, cc, &csrc,
                                nal.data, nal.size, extn, extn_len,
                                extn_type);
        if (err != 0) {
            printf("[ERROR] There was a problem sending the RTP packet\n");
        } else {
            printf("[INFO] NAL sent over RTP - %d bytes\n", nal.size);
        }
    }
}
