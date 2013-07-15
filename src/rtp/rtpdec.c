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
        //struct vcodec_state *pbuf_data = (struct vcodec_state *) decode_data;
        //struct state_decoder *decoder = pbuf_data->decoder;
        //struct video_frame *frame = decoder->frame;

        int ret = TRUE;
        //uint32_t offset;
        int len;
        rtp_packet *pckt = NULL;
        //unsigned char *source;
        char *data;
        uint32_t data_pos;
        //int prints=0;
        //struct tile *tile = NULL;
        uint32_t tmp;
        uint32_t substream;

        int i;
        //struct linked_list *pckt_list[MAX_SUBSTREAMS];
        // the following is just LDGM related optimalization - normally we fill up
        // allocated buffers when we have compressed data. But in case of LDGM, there
        // is just the LDGM buffer present, so we point to it instead to copying
        struct recieved_data *buffers = (struct recieved_data *) rx_data; // for FEC or compressed data
        for (i = 0; i < (int) MAX_SUBSTREAMS; ++i) {
                //pckt_list[i] = ll_create();
        		buffers->buffer_len[i] = 0;
                buffers->buffer_num[i] = 0;
                buffers->frame_buffer[i] = NULL;
        }

        //perf_record(UVP_DECODEFRAME, frame);

        // We have no framebuffer assigned, exitting
        //if(!decoder->display) {
        //        return FALSE;
        //}
        
        //int k = 0, m = 0, c = 0, seed = 0; // LDGM
        int buffer_number, buffer_length;

        // first, dispatch "messages"
        /*if(decoder->set_fps) {
                struct vcodec_message *msg = malloc(sizeof(struct vcodec_message));
                struct fps_changed_message *data = malloc(sizeof(struct fps_changed_message));
                msg->type = FPS_CHANGED;
                msg->data = data;
                data->val = decoder->set_fps;
                data->interframe_codec = is_codec_interframe(decoder->codec);
                simple_linked_list_append(pbuf_data->messages, msg);
                decoder->set_fps = 0;
        }*/

        int pt;
        //bool buffer_swapped = false;

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

                //printf("[DECODER] substream = %u\n", substream);

                if(pt == PT_VIDEO) {
                        len = pckt->data_len - sizeof(video_payload_hdr_t);
                        data = (char *) hdr + sizeof(video_payload_hdr_t);

                        //printf("[DECODER]  packet data (first byte) = %x and total length = %d\n",data[0],len);
//                } else if (pt == PT_VIDEO_LDGM) {
//                        len = pckt->data_len - sizeof(ldgm_video_payload_hdr_t);
//                        data = (char *) hdr + sizeof(ldgm_video_payload_hdr_t);
//
//                        tmp = ntohl(hdr[3]);
//                        k = tmp >> 19;
//                        m = 0x1fff & (tmp >> 6);
//                        c = 0x3f & tmp;
//                        seed = ntohl(hdr[4]);
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
                        // the guess is valid - we start with highest substream number (anytime - since it holds a m-bit)
                        // in next iterations, index is valid
                        /*if(substream == 1 || substream == 3) {
                                fprintf(stderr, "[decoder] Guessing mode: ");
                                if(substream == 1) {
                                        decoder_set_video_mode(decoder, VIDEO_STEREO);
                                } else {
                                        decoder_set_video_mode(decoder, VIDEO_4K);
                                }
                                decoder->received_vid_desc.width = 0; // just for sure, that we reconfigure in next iteration
                                fprintf(stderr, "%s\n", get_video_mode_description(decoder->video_mode));
                        } else {
                                exit_uv(1);
                        }*/
                        // we need skip this frame (variables are illegal in this iteration
                        // and in case that we got unrecognized number of substreams - exit
                        printf("wrong maxsubstreams\n");
                        ret = FALSE;
                        goto cleanup;
                }

                if(!buffers->frame_buffer[substream]) {
                	buffers->frame_buffer[substream] = (char *) malloc(buffer_length);
                }

                buffers->buffer_num[substream] = buffer_number;
                buffers->buffer_len[substream] = buffer_length;

                //printf("[DECODER] buffer_length = %d /  data_pos = %d /  len = %d /  first byte data = %x\n",buffer_length, data_pos,len,data[0]);

                //ll_insert(pckt_list[substream], data_pos, len);
                
                //if (pt == PT_VIDEO && decoder->decoder_type == LINE_DECODER) {

		//} else { /* PT_VIDEO_LDGM or external decoder */
               // if (pt == PT_VIDEO) {
                        memcpy(buffers->frame_buffer[substream] + data_pos, (unsigned char*) data,len);

               // }
        //}

                cdata = cdata->nxt;
        }

        if(!pckt) {
        		printf("no packet, dude!\n");
                ret = FALSE;
                goto cleanup;
        }

        assert(ret == TRUE);

        // format message
	/*
        struct ldgm_data *ldgm_data = malloc(sizeof(struct ldgm_data));
        ldgm_data->buffer_len = malloc(sizeof(buffer_len));
        ldgm_data->buffer_num = malloc(sizeof(buffer_num));
        ldgm_data->recv_buffers = malloc(sizeof(recv_buffers));
        ldgm_data->pckt_list = malloc(sizeof(pckt_list));
        ldgm_data->k = k;
        ldgm_data->m = m;
        ldgm_data->c = c;
        ldgm_data->seed = seed;
        ldgm_data->substream_count = decoder->max_substreams;
        ldgm_data->pt = pt;
        ldgm_data->poisoned = false;
        memcpy(ldgm_data->buffer_len, buffer_len, sizeof(buffer_len));
        memcpy(ldgm_data->buffer_num, buffer_num, sizeof(buffer_num));
        memcpy(ldgm_data->recv_buffers, recv_buffers, sizeof(recv_buffers));
        memcpy(ldgm_data->pckt_list, pckt_list, sizeof(pckt_list));

        pthread_mutex_lock(&decoder->lock);
        {
                if(decoder->ldgm_data && !decoder->slow) {
                        fprintf(stderr, "Your computer is too SLOW to play this !!!\n");
                        decoder->slow = true;
                } else {
                        decoder->slow = false;
                }

                while (decoder->ldgm_data) {
                        pthread_cond_wagettimeofday(&curr_time, NULL);it(&decoder->ldgm_boss_cv, &decoder->lock);
                }

                decoder->ldgm_data = ldgm_data;

                
                pthread_cond_signal(&decoder->ldgm_worker_cv);
        }
        pthread_mutex_unlock(&decoder->lock);
        
        */

cleanup:
        ;
        //unsigned int frame_size = 0;

        /*for(i = 0; i < (int) (sizeof(pckt_list) / sizeof(struct linked_list *)); ++i) {

                if(ret != TRUE) {
                        free(recv_buffers[i]);
                        ll_destroy(pckt_list[i]);
                }

                frame_size += buffer_len[i];
        }

        pbuf_data->max_frame_size = max(pbuf_data->max_frame_size, frame_size);

        if(ret) {
                decoder->displayed++;
                pbuf_data->decoded++;
        } else {
                decoder->dropped++;
        }

        if(decoder->displayed % 600 == 599) {
                fprintf(stderr, "Decoder statistics: %lu displayed frames / %lu frames dropped (%lu corrupted)\n",
                                decoder->displayed, decoder->dropped, decoder->corrupted);
        }*/

        return ret;
}