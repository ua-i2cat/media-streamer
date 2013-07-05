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

struct rtp_nal_t {
    uint8_t *data;
    int size;
};

static uint8_t *rtp_find_startcode_internal(uint8_t *start, uint8_t *end);
uint8_t *rtp_find_startcode(uint8_t *p, uint8_t *end);
int rtp_parse_nal_units(uint8_t *buf_in, int size, struct rtp_nal_t *nals, int *nnals);

static uint8_t *rtp_find_startcode_internal(uint8_t *start, uint8_t *end)
{
    uint8_t *p = start;
    uint8_t *pend = end;

    const uint8_t *a = p + 4 - ((intptr_t)p & 3);

    for (pend -= 3; p < a && p < pend; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }

    for (pend -= 3; p < pend; p += 4) {
        uint32_t x = *(const uint32_t*)p;
        //if ((x - 0x01000100) & (~x) & 0x80008000) // little endian
        //if ((x - 0x00010001) & (~x) & 0x00800080) // big endian
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

    for (pend += 3; p < pend; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1) {
            return p;
        }
    }

    return pend + 3;
}

uint8_t *rtp_find_startcode(uint8_t *p, uint8_t *end) {
    uint8_t *out = rtp_find_startcode_internal(p, end);
    if (p < out && out < end && !out[-1]) {
        out--;
    }
    return out;
}

int rtp_parse_nal_units(uint8_t *buf_in, int size, struct rtp_nal_t *nals, int *nnals)
{
    uint8_t *p = buf_in;
    uint8_t *end = p + size;
    uint8_t *nal_start, *nal_end;

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


    uint8_t *data = (uint8_t *)tile->data;
    int data_len = tile->data_len;

    struct rtp_nal_t nals[MAX_NALS];
    int nnals;


    rtp_parse_nal_units(data, data_len, nals, &nnals);

    printf("[DEBUG] %d NAL units found in buffer", nnals);

    char pt = 96; // h264
    int cc = 0;
    uint32_t csrc = 0;
    
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
                                (char *)nal.data, nal.size, extn, extn_len,
                                extn_type);
        if (err != 0) {
            printf("[ERROR] There was a problem sending the RTP packet\n");
        } else {
            printf("[INFO] NAL sent over RTP - %d bytes\n", nal.size);
        }
    }
}
