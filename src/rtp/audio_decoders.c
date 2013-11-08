
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

/*
 * Second version that uses external audio configuration,
 * now it uses a struct state_audio_decoder instead an audio_frame2.
 * It does multi-channel handling.
 */
int decode_audio_frame_mulaw(struct coded_data *cdata, void *data)
{
    struct state_audio_decoder *audio = (struct state_audio_decoder *)data;

    if(!cdata) return false;

    // Reconfiguration.
    if (audio->frame->bps != audio->desc->bps ||
            audio->frame->sample_rate != audio->desc->sample_rate ||
            audio->frame->ch_count != audio->desc->ch_count ||
            audio->frame->codec != audio->desc->codec)
    {
        rtp_audio_frame2_allocate(audio->frame, audio->desc->ch_count, audio->desc->sample_rate * audio->desc->bps);
        audio->frame->bps = audio->desc->bps;
        audio->frame->sample_rate = audio->desc->sample_rate;
        audio->frame->ch_count = audio->desc->ch_count;
        audio->frame->codec = audio->desc->codec;
    }

    // Initial setup
    for (int ch = 0 ; ch < audio->frame->ch_count ; ch ++) {
        audio->frame->data_len[ch] = 0;
    }

    // Check-if-there-is-only-one-channel optimization.
    if (audio->frame->ch_count == 1) {
        char *to = audio->frame->data[0];

        while (cdata != NULL) {
            // Get the data to copy into the frame.
            char *from = cdata->data->data;

            // See if the data fits.
            if (cdata->data->data_len <= (int)(audio->frame->max_size - audio->frame->data_len[0])) {
                // Copy the data
                memcpy(to, from, cdata->data->data_len);
                // Update the pointer and the counter.
                to += cdata->data->data_len;
                audio->frame->data_len[0] += cdata->data->data_len;
            } else {
                // Filled it out, exit now.
                return true;
            }

            cdata = cdata->nxt;
        }
    } else { // Multi-channel case.

        /*
         * Unoptimized version of the multi-channel handling.
         * TODO: Optimize it! It's a matrix transpose.
         *       Take a look at http://stackoverflow.com/questions/1777901/array-interleaving-problem
         */

        char *to;
        int samples_copied = 0;

        while (cdata != NULL) {
            // Check that the amount of data on cdata->data->data is congruent with 0 modulus audio->frame->ch_count.
            if (cdata->data->data_len % audio->frame->ch_count != 0) {
                // printf something?
                return false;
            }

            // If there is space on the current audio_frame2 buffer.
            if (cdata->data->data_len <= (int)(audio->frame->max_size - samples_copied)) {
                char *from = cdata->data->data;

                // For each group of samples.
                for (int g = 0 ; g < (cdata->data->data_len / audio->frame->ch_count) ; g++) {
                    // Iterate throught each channel.
                    for (int ch = 0 ; ch < audio->frame->ch_count ; ch ++) {
                        // Copy the current sample from the RTP packet to the audio_frame2.
                        to = audio->frame->data[ch];
                        to += samples_copied * sizeof(uint8_t);

                        memcpy(to, from, sizeof(uint8_t));

                        from += sizeof(uint8_t);
                        audio->frame->data_len[ch]++;
                    }

                    samples_copied++;
                }
            } else {
                // Filled audio_frame2 out, exit now.
                return true;
            }

            cdata = cdata->nxt;
        }

    }

    return true;
}

