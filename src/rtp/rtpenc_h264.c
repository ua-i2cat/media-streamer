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

#define RTPENC_H264_MAX_NALS 1024
#define RTPENC_H264_PT 96

struct rtp_nal_t {
    uint8_t *data;
    int size;
};

int rtpenc_h264_nals_recv;
int rtpenc_h264_nals_sent_nofrag;
int rtpenc_h264_nals_sent_frag;
int rtpenc_h264_nals_sent;

void rtpenc_h264_stats_print()
{
    printf("[RTPENC][STATS] Total recv NALs: %d\n", rtpenc_h264_nals_recv);
    printf("[RTPENC][STATS] Unfragmented sent NALs: %d\n", rtpenc_h264_nals_sent_nofrag);
    printf("[RTPENC][STATS] Fragmented sent NALs: %d\n", rtpenc_h264_nals_sent_frag);
    printf("[RTPENC][STATS] NAL fragments sent: %d\n", rtpenc_h264_nals_sent - rtpenc_h264_nals_sent_nofrag);
    printf("[RTPENC][STATS] Total sent NALs: %d\n", rtpenc_h264_nals_sent);
}

static uint8_t *rtpenc_h264_find_startcode_internal(uint8_t *start, uint8_t *end);
uint8_t *rtpenc_h264_find_startcode(uint8_t *p, uint8_t *end);
int rtpenc_h264_parse_nal_units(uint8_t *buf_in, int size, struct rtp_nal_t *nals, int *nnals);


#ifdef DEBUG /* Just define debug functions if needed */
static void rtpenc_h264_debug_print_nal_recv_info(uint8_t *header);
static void rtpenc_h264_debug_print_nal_sent_info(uint8_t *header, int size);
static void rtpenc_h264_debug_print_fragment_sent_info(uint8_t *header, int size);
static void rtpenc_h264_debug_pint_payload_bytes(uint8_t *payload, int number);

static void rtpenc_h264_debug_pint_payload_bytes(uint8_t *payload, int number)
{
	int i;
	for (i = 0; i < number; i++) {
		debug_msg("NAL payload byte[%d] = %x\n", i, (int)payload[i]);
	}
}

static void rtpenc_h264_debug_print_nal_recv_info(uint8_t *header)
{
    debug_msg("NAL recv header: %d %d %d %d %d %d %d %d\n",
            ((*header) & 0x80) >> 7, ((*header) & 0x40) >> 6,
            ((*header) & 0x20) >> 5, ((*header) & 0x10) >> 4,
            ((*header) & 0x08) >> 3, ((*header) & 0x04) >> 2,
            ((*header) & 0x02) >> 1, ((*header) & 0x01));
    int type = (int)(*header & 0x1f);
    int nri = (int)((*header & 0x60) >> 5);
    debug_msg("NAL recv type: %d\n", type);
    debug_msg("NAL recv NRI: %d\n", nri);
}

static void rtpenc_h264_debug_print_nal_sent_info(uint8_t *header, int size)
{
    debug_msg("NAL sent over RTP (%d bytes)\n", size);
    debug_msg("NAL sent header: %d %d %d %d %d %d %d %d\n",
            ((*header) & 0x80) >> 7, ((*header) & 0x40) >> 6,
            ((*header) & 0x20) >> 5, ((*header) & 0x10) >> 4,
            ((*header) & 0x08) >> 3, ((*header) & 0x04) >> 2,
            ((*header) & 0x02) >> 1, ((*header) & 0x01));
    int type = (int)(*header & 0x1f);
    int nri = (int)((*header & 0x60) >> 5);
    debug_msg("NAL sent type: %d\n", type);
    debug_msg("NAL sent NRI: %d\n", nri);
}

static void rtpenc_h264_debug_print_fragment_sent_info(uint8_t *header, int size)
{
    char frag_class;
    switch((header[1] & 0xE0) >> 5) {
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
    debug_msg("NAL fragment (%c) sent over RTP (%d bytes)\n", frag_class, size);
}
#endif /* DEBUG */

static uint8_t *rtpenc_h264_find_startcode_internal(uint8_t *start, uint8_t *end)
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

uint8_t *rtpenc_h264_find_startcode(uint8_t *p, uint8_t *end) {
    uint8_t *out = rtpenc_h264_find_startcode_internal(p, end);
    if (p < out && out < end && !out[-1]) {
        out--;
    }
    return out;
}

int rtpenc_h264_parse_nal_units(uint8_t *buf_in, int size, struct rtp_nal_t *nals, int *nnals)
{
    uint8_t *p = buf_in;
    uint8_t *end = p + size;
    uint8_t *nal_start;
    uint8_t *nal_end;

    size = 0;
    nal_start = rtpenc_h264_find_startcode(p, end);
    *nnals = 0;
    for (;;) {
        while (nal_start < end && !*(nal_start++));
        if (nal_start == end)
            break;

        nal_end = rtpenc_h264_find_startcode(nal_start, end);
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
    packet_rate = 1000 * mtu * 8 / bitrate;

    rtpenc_h264_nals_recv = 0;
    rtpenc_h264_nals_sent_nofrag = 0;
    rtpenc_h264_nals_sent_frag = 0;
    rtpenc_h264_nals_sent = 0;
}

void tx_send_base_h264(struct tile *tile, struct rtp *rtp_session,
                       uint32_t ts, int send_m, codec_t color_spec,
                       double input_fps, enum interlacing_t interlacing,
                       unsigned int substream, int fragment_offset)
{

    UNUSED(color_spec);
    UNUSED(input_fps);
    UNUSED(interlacing);
    UNUSED(substream);
    UNUSED(fragment_offset);

    uint8_t *data = (uint8_t *)tile->data;
    int data_len = tile->data_len;

    struct rtp_nal_t nals[RTPENC_H264_MAX_NALS];
    int nnals = 0;
    rtpenc_h264_parse_nal_units(data, data_len, nals, &nnals);

    rtpenc_h264_nals_recv += nnals;
    #ifdef DEBUG
    debug_msg("%d NAL units found in buffer\n", nnals);
    #endif

    char pt = RTPENC_H264_PT;
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
            debug_msg("RTP packet size exceeds the MTU size\n");
            #endif
            fragmentation = 1;
        }

        uint8_t *nal_header = nal.data;
        int nal_header_size = 1;

        uint8_t *nal_payload = nal.data + nal_header_size;
        int nal_payload_size = nal.size - nal_header_size;

        #ifdef DEBUG
        rtpenc_h264_debug_print_nal_recv_info(nal_header);
        #endif

        const char type = (char)(*nal_header & 0x1f);
        const char nri = (char)((*nal_header & 0x60) >> 5);

        char info_type;
        if (type >= 1 && type <= 23) {
            info_type = 1;
        }
        else {
            info_type = type;
        }

        switch (info_type) {
        case 0:
        case 1:
            #ifdef DEBUG
            debug_msg("Unfragmented or reconstructed NAL type\n");
            #endif
            break;
        default:
            error_msg("Non expected NAL type %d\n", (int)info_type);
            return; // TODO maybe just warn and don't fail?
            break;
        }

        if (!fragmentation) {
#ifdef DEBUG
            rtpenc_h264_debug_pint_payload_bytes(nal_payload, 6);
#endif
            int err = rtp_send_data_hdr(rtp_session, ts, pt, send_m, cc, &csrc,
                                        (char *)nal_header, nal_header_size,
                                        (char *)nal_payload, nal_payload_size, extn, extn_len,
                                        extn_type);
            if (err < 0) {
                error_msg("There was a problem sending the RTP packet\n");
            }
            else {
                rtpenc_h264_nals_sent_nofrag++;
                rtpenc_h264_nals_sent++;
                #ifdef DEBUG
                rtpenc_h264_debug_print_nal_sent_info(nal_header, nal_payload_size + nal_header_size);
                #endif
            }
        }
        else {
            const char fu_indicator = 28 | (nri << 5); // new type, fragmented

            uint8_t frag_header[2];
            int frag_header_size = 2;

            frag_header[0] = fu_indicator;
            frag_header[1] = type | (1 << 7); // start, initial fu_header

            uint8_t *frag_payload = nal_payload;
            int frag_payload_size = nal_max_size - frag_header_size;

            int remaining_payload_size = nal_payload_size;

            while (remaining_payload_size + 2 > nal_max_size) {
#ifdef DEBUG
                rtpenc_h264_debug_pint_payload_bytes(frag_payload, 6);
#endif
                int err = rtp_send_data_hdr(rtp_session, ts, pt, send_m, cc, &csrc,
                                            (char *)frag_header, frag_header_size,
                                            (char *)frag_payload, frag_payload_size, extn, extn_len,
                                            extn_type);
                if (err < 0) {
                    error_msg("There was a problem sending the RTP packet\n");
                }
                else {
                    rtpenc_h264_nals_sent++;
                    #ifdef DEBUG
                    rtpenc_h264_debug_print_fragment_sent_info(frag_header, frag_payload_size + frag_header_size);
                    #endif
                }

                remaining_payload_size -= frag_payload_size;
                frag_payload += frag_payload_size;
                
                frag_header[1] &= ~(1 << 7); // not end, not start 
            }

            frag_header[1] |= 1 << 6; // end
#ifdef DEBUG
            rtpenc_h264_debug_pint_payload_bytes(frag_payload, 6);
#endif

            int err = rtp_send_data_hdr(rtp_session, ts, pt, send_m, cc, &csrc,
                            (char *)frag_header, frag_header_size,
                            (char *)frag_payload, remaining_payload_size, extn, extn_len,
                            extn_type);            
            if (err < 0) {
                error_msg("There was a problem sending the RTP packet\n");
            }
            else {
                rtpenc_h264_nals_sent_frag++; // Each fragmented NAL has one E (end) NAL fragment
                rtpenc_h264_nals_sent++;
                #ifdef DEBUG
                rtpenc_h264_debug_print_fragment_sent_info(frag_header, remaining_payload_size + frag_header_size);
                #endif
            }
        }
    }
}
