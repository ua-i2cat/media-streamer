#ifdef HAVE_CONFIG_H
#include "config.h"
#include "config_unix.h"
#endif // HAVE_CONFIG_H
#include "debug.h"
#include "perf.h"
#include "rtp/rtp.h"
#include "rtp/rtp_callback.h"
#include "rtp/pbuf.h"
#include "rtp/rtpdec.h"


int decode_frame(struct coded_data *cdata, void *rx_data)
{
        int ret = TRUE;
        int len;
        rtp_packet *pckt = NULL;
        char *data;
        uint32_t data_pos;
        uint32_t tmp;
        uint32_t substream;
        int i;
	
        struct recieved_data *buffers = (struct recieved_data *) rx_data; 
        for (i = 0; i < (int) MAX_SUBSTREAMS; ++i) {
		buffers->buffer_len[i] = 0;
                buffers->buffer_num[i] = 0;
                buffers->frame_buffer[i] = NULL;
        }

        int k = 0, m = 0, c = 0, seed = 0; // LDGM
        int buffer_number, buffer_length;

        int pt;
        bool buffer_swapped = false;

        while (cdata != NULL) {
                uint32_t *hdr;
                pckt = cdata->data;

                pt = pckt->pt;
                hdr = (uint32_t *)(void *) pckt->data;
                data_pos = ntohl(hdr[1]);
                tmp = ntohl(hdr[0]);

                substream = tmp >> 22;
                buffer_number = tmp & 0x3ffff;
                buffer_length = ntohl(hdr[2]);

                if(pt == PT_VIDEO) {
                        len = pckt->data_len - sizeof(video_payload_hdr_t);
                        data = (char *) hdr + sizeof(video_payload_hdr_t);
                } else if (pt == PT_VIDEO_LDGM) {
                        len = pckt->data_len - sizeof(ldgm_video_payload_hdr_t);
                        data = (char *) hdr + sizeof(ldgm_video_payload_hdr_t);

                        tmp = ntohl(hdr[3]);
                        k = tmp >> 19;
                        m = 0x1fff & (tmp >> 6);
                        c = 0x3f & tmp;
                        seed = ntohl(hdr[4]);
                } else {
                        fprintf(stderr, "[decoder] Unknown packet type: %d.\n", pckt->pt);
                        //exit_uv(1);i
			printf("unknown packet type\n");
                        ret = FALSE;
                        goto cleanup;
                }

                if(substream >= MAX_SUBSTREAMS) {
                        fprintf(stderr, "[decoder] received substream ID %d. Expecting at most %d substreams. Did you set -M option?\n",
                                        substream, MAX_SUBSTREAMS);
                        printf("wrong maxsubstreams\n");
                        ret = FALSE;
                        goto cleanup;
                }

                if(!buffers->frame_buffer[substream]) {
                	buffers->frame_buffer[substream] = (char *) malloc(buffer_length);
                }

                buffers->buffer_num[substream] = buffer_number;
                buffers->buffer_len[substream] = buffer_length;


                memcpy(buffers->frame_buffer[substream] + data_pos, (unsigned char*) data,len);


                cdata = cdata->nxt;
        }

        if(!pckt) {
		printf("no packet, dude!\n");
                ret = FALSE;
                goto cleanup;
        }

        assert(ret == TRUE);

	cleanup:
        ;
        unsigned int frame_size = 0;

        return ret;
}
