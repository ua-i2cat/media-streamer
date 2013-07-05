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
#define RTP_H264_PT 96

//#define DEBUG

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

    #ifdef DEBUG
    printf("[RTPENC][DEBUG] %d NAL units found in buffer\n", nnals);
    #endif

    char pt = RTP_H264_PT;
    int cc = 0;
    uint32_t csrc = 0;
    
    char *extn = 0;
    uint16_t extn_len = 0;
    uint16_t extn_type = 0;

    int i;
    for (i = 0; i < nnals; i++) {
        struct rtp_nal_t nal = nals[i];

        int fragmentation = 0;
        int nal_max_size = mtu - 40;
        if (nal.size > nal_max_size) {
            #ifdef DEBUG
            printf("[RTPENC][WARNING] RTP packet size exceeds the MTU size\n");
            #endif
            fragmentation = 1;
        }

        char *nal_header = (char *)nal.data;
        int nal_header_size = 1;
        
        char *nal_payload = (char *)(nal.data + nal_header_size);
        int nal_payload_size = nal.size - nal_header_size;

        #ifdef DEBUG
        printf("[RTPENC][DEBUG] NAL header: %d %d %d %d %d %d %d %d\n",
                ((*nal_header) & 0x80) >> 7, ((*nal_header) & 0x40) >> 6,
                ((*nal_header) & 0x20) >> 5, ((*nal_header) & 0x10) >> 4,
                ((*nal_header) & 0x08) >> 3, ((*nal_header) & 0x04) >> 2,
                ((*nal_header) & 0x02) >> 1, ((*nal_header) & 0x01));
        #endif

        const char type = (char)(*nal_header & 0x1f);
        const char nri = (char)((*nal_header & 0x60) >> 5);
        
        #ifdef DEBUG
        printf("[RTPENC][DEBUG] NAL type: %d\n", (int)type);
        printf("[RTPENC][DEBUG] NAL NRI: %d\n", (int)nri);
        #endif

        char info_type;
        if (type >= 1 || type <= 23) {
            info_type = 1;
        }
        else {
            info_type = type;
        }

        #ifdef DEBUG
        switch (info_type) {
        case 0:
        case 1:
            printf("[RTPENC][DEBUG] Unfragmented or reconstructed NAL type\n");
            break;
        default:
            printf("[RTPENC][ERROR] Non expected NAL type %d\n", (int)info_type);
            return;
            break;
        }
        #else
        if (info_type != 0 && info_type != 1) {
            printf("[RTPENC][ERROR] Non expected NAL type %d\n", (int)info_type);
        }
        #endif

        if (!fragmentation) {
            int err = rtp_send_data_hdr(rtp_session, ts, pt, send_m, cc, &csrc,
                                        nal_header, nal_header_size,
                                        nal_payload, nal_payload_size, extn, extn_len,
                                        extn_type);
            if (err <= 0) {
                printf("[RTPENC][ERROR] There was a problem sending the RTP packet\n");
            } else {
                #ifdef DEBUG
                printf("[RTPENC][INFO] NAL sent over RTP (%d bytes)\n", nal.size);
                #endif
            }
        }
        else {
            const char fu_indicator = 28 | (nri << 5); // new type, fragmented
            char fu_header = type | (1 << 7); // start

            char frag_header[2];
            int frag_header_size = 2;

            frag_header[0] = fu_indicator;
            frag_header[1] = fu_header;

            char *frag_payload = (char *)nal_payload;
            int frag_payload_size = nal_max_size - frag_header_size;

            int remaining_payload_size = nal_payload_size;

            while (remaining_payload_size + 2 > nal_max_size) {
                
                int err = rtp_send_data_hdr(rtp_session, ts, pt, send_m, cc, &csrc,
                                            frag_header, frag_header_size,
                                            frag_payload, frag_payload_size, extn, extn_len,
                                            extn_type);
                if (err <= 0) {
                    printf("[RTPENC][ERROR] There was a problem sending the RTP packet\n");
                }
                else {
                    #ifdef DEBUG
                    char frag_class;
                    switch((frag_header[1] & 0xE0) >> 5) {
                    case 0:
                        frag_class = '0';
                        break;
                    case 2:
                        frag_class = 'E';
                        break;
                    case 4:
                        frag_class = 'S';
                        break;
                    default:
                        frag_class = '!';
                        break;
                    }
                    printf("[RTPENC][INFO] NAL fragment (%c) sent over RTP (%d bytes)\n", frag_class, frag_payload_size + frag_header_size);
                    #endif
                }
             
                remaining_payload_size -= frag_payload_size;
                frag_payload += frag_payload_size;
                
                frag_header[1] &= ~(1 << 7); // not end, not start 
            }

            frag_header[1] |= 1 << 6; // end

            int err = rtp_send_data_hdr(rtp_session, ts, pt, send_m, cc, &csrc,
                            frag_header, frag_header_size,
                            frag_payload, remaining_payload_size, extn, extn_len,
                            extn_type);
            
            if (err <= 0) {
                printf("[RTPENC][ERROR] There was a problem sending the RTP packet\n");
            }
            else {
                #ifdef DEBUG
                char frag_class;
                switch((frag_header[1] & 0xE0) >> 5) {
                case 0:
                    frag_class = '0';
                    break;
                case 2:
                    frag_class = 'E';
                    break;
                case 4:
                    frag_class = 'S';
                    break;
                default:
                    frag_class = '!';
                    break;
                }
                printf("[RTPENC][INFO] NAL fragment (%c) sent over RTP (%d bytes)\n",
                    frag_class, remaining_payload_size + frag_header_size);
                #endif
            }

        }
    }
}
