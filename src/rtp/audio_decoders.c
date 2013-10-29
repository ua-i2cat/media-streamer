/*
 * AUTHOR:   N.Cihan Tas
 * MODIFIED: Ladan Gharai
 *           Colin Perkins
 *           Martin Benes     <martinbenesh@gmail.com>
 *           Lukas Hejtmanek  <xhejtman@ics.muni.cz>
 *           Petr Holub       <hopet@ics.muni.cz>
 *           Milos Liska      <xliska@fi.muni.cz>
 *           Jiri Matela      <matela@ics.muni.cz>
 *           Dalibor Matura   <255899@mail.muni.cz>
 *           Ian Wesley-Smith <iwsmith@cct.lsu.edu>
 * 
 * This file implements a linked list for the playout buffer.
 *
 * Copyright (c) 2003-2004 University of Southern California
 * Copyright (c) 2003-2004 University of Glasgow
 * Copyright (c) 2005-2010 CESNET z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *      This product includes software developed by the University of Southern
 *      California Information Sciences Institute. This product also includes
 *      software developed by CESNET z.s.p.o.
 * 
 * 4. Neither the name of the University, Institute, CESNET nor the names of
 *    its contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "rtp/rtp.h"
#include "rtp/rtp_callback.h"
#include "rtp/pbuf.h"
#include "rtp/audio_decoders.h"
#include "rtp/audio_frame2.h"

int decode_audio_frame(struct coded_data *cdata, void *data)
{
        audio_frame2 *frame = (audio_frame2 *) data;

        int input_channels = 0;
        int bps, sample_rate, channel;
        static int prints = 0;

        if(!cdata) return false;

        while (cdata != NULL) {
                char *data;
                // for definition see rtp_callbacks.h
                uint32_t *audio_hdr = (uint32_t *)(void *) cdata->data->data;
                unsigned int length;

                length = cdata->data->data_len - sizeof(audio_payload_hdr_t);
                data = cdata->data->data + sizeof(audio_payload_hdr_t);

                /* we receive last channel first (with m bit, last packet) */
                /* thus can be set only with m-bit packet */
                if(cdata->data->m) {
                        input_channels = ((ntohl(audio_hdr[0]) >> 22) & 0x3ff) + 1;
                }

                // we have:
                // 1) last packet, then we have just set total channels
                // 2) not last, but the last one was processed at first
                assert(input_channels > 0);

                channel = (ntohl(audio_hdr[0]) >> 22) & 0x3ff;
                sample_rate = ntohl(audio_hdr[3]) & 0x3fffff;
                bps = (ntohl(audio_hdr[3]) >> 26) / 8;
                uint32_t audio_tag = ntohl(audio_hdr[4]);

                /*
                 * Reconfiguration
                 */
                audio_codec_t audio_codec = rtp_get_audio_codec_to_tag(audio_tag);
                if (frame->ch_count != input_channels ||
                                frame->bps != bps ||
                                frame->sample_rate != sample_rate ||
                                frame->codec != audio_codec) { 
                        rtp_audio_frame2_allocate(frame, input_channels, sample_rate * bps/* 1 sec */); 
                        frame->bps = bps;
                        frame->sample_rate = sample_rate;
                        frame->codec = audio_codec;
                }

                unsigned int offset = ntohl(audio_hdr[1]);
                unsigned int buffer_len = ntohl(audio_hdr[2]);

                if(offset + length <= frame->max_size) {
                        memcpy(frame->data[channel] + offset, data, length);
                } else { /* discarding data - buffer to small */
                        if(++prints % 100 == 0)
                                fprintf(stdout, "Warning: "
                                                "discarding audio data "
                                                "- buffer too small\n");
                }

                /* buffer size same for every packet of the frame */
                if(buffer_len <= frame->max_size) {
                        frame->data_len[channel] = buffer_len;
                } else { /* overflow */
                        frame->data_len[channel] =
                                frame->max_size;
                }

                cdata = cdata->nxt;
        }

        return true;
}

int decode_audio_frame_ulaw(struct coded_data *cdata, void *data)
{
        audio_frame2 *frame = (audio_frame2 *) data;

        int input_channels = 0;
        int bps, sample_rate, channel;
        static int prints = 0;

        if(!cdata) return false;

        while (cdata != NULL) {
                char *data;
                // for definition see rtp_callbacks.h
                uint32_t *audio_hdr = (uint32_t *)(void *) cdata->data->data;
                unsigned int length;

                length = cdata->data->data_len - sizeof(audio_payload_hdr_t);
                data = cdata->data->data + sizeof(audio_payload_hdr_t);

                /* we receive last channel first (with m bit, last packet) */
                /* thus can be set only with m-bit packet */
                if(cdata->data->m) {
                        input_channels = ((ntohl(audio_hdr[0]) >> 22) & 0x3ff) + 1;
                }

                // we have:
                // 1) last packet, then we have just set total channels
                // 2) not last, but the last one was processed at first
                assert(input_channels > 0);

                channel = (ntohl(audio_hdr[0]) >> 22) & 0x3ff;
                sample_rate = ntohl(audio_hdr[3]) & 0x3fffff;
                bps = (ntohl(audio_hdr[3]) >> 26) / 8;
                uint32_t audio_tag = ntohl(audio_hdr[4]);

                /*
                 * Reconfiguration
                 */
                audio_codec_t audio_codec = rtp_get_audio_codec_to_tag(audio_tag);
                if (frame->ch_count != input_channels ||
                                frame->bps != bps ||
                                frame->sample_rate != sample_rate ||
                                frame->codec != audio_codec) { 
                        rtp_audio_frame2_allocate(frame, input_channels, sample_rate * bps/* 1 sec */); 
                        frame->bps = bps;
                        frame->sample_rate = sample_rate;
                        frame->codec = audio_codec;
                }

                unsigned int offset = ntohl(audio_hdr[1]);
                unsigned int buffer_len = ntohl(audio_hdr[2]);

                if(offset + length <= frame->max_size) {
                        memcpy(frame->data[channel] + offset, data, length);
                } else { /* discarding data - buffer to small */
                        if(++prints % 100 == 0)
                                fprintf(stdout, "Warning: "
                                                "discarding audio data "
                                                "- buffer too small\n");
                }

                /* buffer size same for every packet of the frame */
                if(buffer_len <= frame->max_size) {
                        frame->data_len[channel] = buffer_len;
                } else { /* overflow */
                        frame->data_len[channel] =
                                frame->max_size;
                }

                cdata = cdata->nxt;
        }

        return true;
}
