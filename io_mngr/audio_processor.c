/*
 *  audio_processor.c - Audio stream processor based on video_processor.c
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of io_mngr.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:  Jordi "Txor" Casas Ríos <txorlings@gmail.com>
 */

#include "audio_processor.h"
#include "audio_frame2.h"
#include "audio_config.h"
#include "utils.h"
#include "debug.h"

// Decoder thread constants
#define DECODER_THREAD_WAIT_TIMEOUT 250
#define DECODER_THREAD_CHECK_PERIOD 15

// private data
struct bag_init_val {
    int chan;
    int size;
};

// private functions
static void *decoder_thread(void *arg);
static void *encoder_thread(void *arg);
static void *bag_init(void *arg);
static void bag_destroy(void *arg);
static unsigned int *get_media_time_ptr(void *frame);
static int normalize_get_decompress_size();
static int normalize_get_resample_size();
static void audio_frame_extract(audio_frame2 *src, audio_frame2 *dst);
static void audio_frame_format(audio_frame2 *src, struct audio_desc *desc);
static void audio_frame_append(audio_frame2 *src, audio_frame2 *dst);
static void audio_frame_copy(audio_frame2 *src, audio_frame2 *dst);
static void audio_frame_check_internal_reconfig(audio_frame2 *dst);

static void *decoder_thread(void* arg)
{
    audio_processor_t *ap = (audio_processor_t *) arg;

    audio_frame2 *frame,
                 *output_frame,
                 *norm_frame,
                 *decomp_frame;

    norm_frame = audio_frame2_init();
    audio_frame2_allocate(norm_frame,
            ap->internal_config->ch_count,
            ap->internal_conversion_decompress_size);

    decomp_frame = audio_frame2_init();
    audio_frame2_allocate(decomp_frame,
            ap->internal_config->ch_count,
            ap->internal_conversion_resample_size);

    while(ap->run) {
        usleep(THREAD_SLEEP_TIMEOUT);
        if(ap->coded_cq->level != CIRCULAR_QUEUE_EMPTY) {
            // TODO: Channel muxing

            // Get the frame to process
            if ((frame = (audio_frame2 *)cq_get_front(ap->coded_cq)) != NULL) {

                // Get the place to save the resampled frame (last step).
                if ((output_frame = (audio_frame2 *)cq_get_rear(ap->decoded_cq)) != NULL) {

                    // Reconfigure auxiliar frames if needed
                    if (norm_frame->max_size != ap->internal_conversion_decompress_size) {
                        audio_frame2_allocate(norm_frame,
                                ap->external_config->ch_count,
                                ap->internal_conversion_decompress_size);
                    }
                    if (decomp_frame->max_size != ap->internal_conversion_resample_size) {
                        audio_frame2_allocate(decomp_frame,
                                ap->external_config->ch_count,
                                ap->internal_conversion_resample_size);
                    }

                    // Reconfigure output_frame if needed
                    audio_frame_check_internal_reconfig(output_frame);

                    // Normalize number of samples,
                    // norm_frame must point to the desired data.
                    audio_frame_format(norm_frame, ap->external_config);
                    if ((unsigned int)frame->data_len[0] > ap->internal_conversion_decompress_size) {
                        // Too many samples on the coded frame,
                        // extract normalized amount of frames.
                        audio_frame_extract(frame, norm_frame);
                    } else if ((unsigned int)frame->data_len[0] < ap->internal_conversion_decompress_size) {
                        // Not ehought samples on the coded frame,
                        // eat as many samples as possible from the next frames,
                        // fill the surplus space with zeros if needed.
                        audio_frame2 *iterator_frame = frame;
                        bool hungry = true;
                        while (hungry) {
                            audio_frame_extract(iterator_frame, norm_frame);
                            // It ate the current frame?
                            if (iterator_frame->data_len[0] == 0) {
                                cq_remove_bag(ap->coded_cq);
                                // It is still hungry?
                                if ((unsigned int)norm_frame->data_len[0] < norm_frame->max_size) {
                                    // There is more food?
                                    struct timeval a, b;
                                    gettimeofday(&a, NULL);
                                    a.tv_usec += DECODER_THREAD_WAIT_TIMEOUT;
                                    while ((iterator_frame = (audio_frame2 *)cq_get_front(ap->coded_cq)) == NULL && a.tv_usec > b.tv_usec) {
                                        usleep(DECODER_THREAD_CHECK_PERIOD);
                                        gettimeofday(&b, NULL);
                                    }
                                    if (iterator_frame == NULL) {
                                        // No food arrivals, stop eating.
                                        hungry = false;
                                    }
                                } else {
                                    // Just filled its appetite!
                                    hungry = false;
                                }

                            } else {
                                // It left food, it can't be hungry.
                                hungry = false;
                            }
                        }
                    } else {
                        // The samples on the coded frame fits in the output_frame,
                        // copy it.
                        audio_frame_copy(frame, norm_frame);
                        cq_remove_bag(ap->coded_cq);
                    }

                    // Decompress
                    audio_codec_state_set_out(ap->compression_config, decomp_frame);
                    (void)audio_codec_decompress(ap->compression_config, norm_frame);

                    // Resample (if needed)
                    if (!resampler_compare_sample_rate(ap->resampler, ap->external_config->sample_rate)) {
                        resampler_set_resampled(ap->resampler, output_frame);
                        (void)resampler_resample(ap->resampler, decomp_frame);
                    } else {
                        audio_frame_copy(decomp_frame, output_frame);
                    }

                    // Add the bag
                    cq_add_bag(ap->decoded_cq);
                }
            }
        }
    }
    audio_frame2_free(norm_frame);

    pthread_exit((void *)NULL);    
}

static void *encoder_thread(void *arg)
{
    audio_processor_t *ap = (audio_processor_t *)arg;

    audio_frame2 *frame, *output_frame, *uncompressed, *tmp_frame;

    tmp_frame = audio_frame2_init();
    audio_frame2_allocate(tmp_frame, ap->external_config->ch_count, AUDIO_DEFAULT_SIZE);
    // On hot reconfiguration it can't be done here
    resampler_set_resampled(ap->resampler, tmp_frame);

    while (ap->run) {
        usleep(THREAD_SLEEP_TIMEOUT);
        if(ap->decoded_cq->level != CIRCULAR_QUEUE_EMPTY) {
            // TODO: Channel muxing

            // Get the frame to process
            if ((frame = (audio_frame2 *)cq_get_front(ap->decoded_cq)) != NULL) {

                // Get the place to save the resampled frame (last step).
                if ((output_frame = (audio_frame2 *)cq_get_rear(ap->coded_cq)) != NULL) {

                    // Resample audio_frame2
                    uncompressed = resampler_resample(ap->resampler, frame);

                    // Compress audio_frame2 and append it to ap->coded_cq
                    audio_frame_format(output_frame, ap->external_config);
                    while ((frame = audio_codec_compress(ap->compression_config, uncompressed)) != NULL) {
                        audio_frame_append(frame, output_frame);
                        uncompressed = NULL;
                    }
                }

                // Commit the cq changes.
                cq_remove_bag(ap->decoded_cq);
                cq_add_bag(ap->coded_cq);
            }
        }
    }
    audio_frame2_free(tmp_frame);

    pthread_exit((void *)NULL);
}

static void *bag_init(void *init)
{
    struct bag_init_val *init_object = (struct bag_init_val *)init;
    audio_frame2 *frame;

    if ((frame = rtp_audio_frame2_init()) == NULL) {
        error_msg("bag_init: out of memory");
        return NULL;
    }

    rtp_audio_frame2_allocate(frame, init_object->chan, init_object->size);

    return (void *)frame;
}

static void bag_destroy(void *bag)
{
    rtp_audio_frame2_free((audio_frame2 *)bag);
}

static unsigned int *get_media_time_ptr(void *frame)
{
    audio_frame2 *f = (audio_frame2 *)frame;
    unsigned int *time = &f->media_time;
    return time;
}

// TODO: Rethink about internal audio_frame2 max size.
static int normalize_get_decompress_size(audio_processor_t *ap)
{
    int bps_factor, size;

    // What if external bps is greater than internal bps?
    bps_factor = ap->internal_config->bps / ap->external_config->bps;
    size = ap->internal_conversion_resample_size / bps_factor;

    return size;
}

// TODO: Rethink about internal audio_frame2 max size.
static int normalize_get_resample_size(audio_processor_t *ap)
{
    int rate_factor, size;

    rate_factor = ap->internal_config->sample_rate / ap->external_config->sample_rate;
    size = AUDIO_INTERNAL_SIZE / rate_factor;

    return size;
}

static void audio_frame_extract(audio_frame2 *src, audio_frame2 *dst)
{
    int remaining_data;
    int ins = 0;
    int outs = 0;

    // Calculate the data fitting and reserve memory for data movements if needed.
    if ((remaining_data = src->data_len[0] - (dst->max_size - dst->data_len[0])) < 0) {
        remaining_data = 0;
    }

    // Copy data from src to dst.
    for (int ch = 0; ch < src->ch_count; ch++) {
        for (ins = 0, outs = dst->data_len[0];
                ins < src->data_len[0] && (unsigned int)outs < dst->max_size;
                ins++, outs++) {
            dst->data[ch][outs] = src->data[ch][ins];
            dst->data_len[ch]++;
        }
    }

    // Move remainig data to the start of src buffer if needed.
    if (remaining_data != 0) {
        char *buf;
        if ((buf = malloc(remaining_data)) == NULL) {
            error_msg("audio_frame_extract: malloc: out of memory!");
            return;
        }
        for (int ch = 0; ch < src->ch_count; ch++) {
            int j, k, s;
            for (j = 0, s = ins;
                    s < src->data_len[ch];
                    j++, s++) {
                buf[j] = src->data[ch][s];
            }
            src->data_len[ch] = 0;
            for (k = 0; k < j; k++) {
                src->data[ch][k] = buf[k];
                src->data_len[ch]++;
            }
        }
        free(buf);
    }
    else {
        for (int ch = 0; ch < src->ch_count; ch++) {
            src->data_len[ch] = 0;
        }
    }
}

static void audio_frame_format(audio_frame2 *src, struct audio_desc *desc)
{
    src->bps = desc->bps;
    src->sample_rate = desc->sample_rate;
    src->ch_count = desc->ch_count;
    src->codec = desc->codec;
    for (int i = 0; i < src->ch_count; i++) {
        src->data_len[i] = 0;
    }
}

static void audio_frame_append(audio_frame2 *src, audio_frame2 *dst)
{
    int ch, ins, outs;
    for (ch = 0; ch < src->ch_count; ch++) {
        for (ins = 0, outs = dst->data_len[0];
                ins < src->data_len[0] && (unsigned int)outs < dst->max_size;
                ins++, outs++) {
            dst->data[ch][outs] = src->data[ch][ins];
            dst->data_len[ch] ++;
        }
    }
}

static void audio_frame_copy(audio_frame2 *src, audio_frame2 *dst)
{
    if ((unsigned int)src->data_len[0] <= dst->max_size) {
        dst->bps = src->bps;
        dst->sample_rate = src->sample_rate;
        dst->ch_count = src->ch_count;
        dst->codec = src->codec;
        for (int ch = 0; ch < src->ch_count; ch++) {
            memcpy(dst->data[ch],
                    src->data[ch],
                    src->data_len[ch]);
            dst->data_len[ch] = src->data_len[ch];
        }
    }
}

static void audio_frame_check_internal_reconfig(audio_frame2 *dst)
{
    if (dst->max_size != AUDIO_INTERNAL_SIZE ||
            dst->ch_count != AUDIO_INTERNAL_CHANNELS ||
            dst->bps != AUDIO_INTERNAL_BPS ||
            dst->sample_rate != AUDIO_INTERNAL_SAMPLE_RATE ||
            dst->codec != AUDIO_INTERNAL_CODEC) {
        dst->bps = AUDIO_INTERNAL_BPS;
        dst->sample_rate = AUDIO_INTERNAL_SAMPLE_RATE;
        dst->codec = AUDIO_INTERNAL_CODEC;
        audio_frame2_allocate(dst,
                AUDIO_INTERNAL_CHANNELS,
                AUDIO_INTERNAL_SIZE);

    }
}

// Visible functions
audio_processor_t *ap_init(role_t role)
{
    struct bag_init_val init_data;
    audio_processor_t *ap;

    if ((ap = malloc(sizeof(audio_processor_t))) == NULL) {
        error_msg("ap_init: malloc: out of memory!");
        return NULL;
    }

    if ((ap->external_config = malloc(sizeof(struct audio_desc))) == NULL) {
        error_msg("ap_init: malloc: out of memory!");
        return NULL;
    }
    if ((ap->internal_config = malloc(sizeof(struct audio_desc))) == NULL) {
        error_msg("ap_init: malloc: out of memory!");
        return NULL;
    }

    ap->role = role;
    // Default values
    ap->internal_config->bps = AUDIO_INTERNAL_BPS;
    ap->internal_config->sample_rate = AUDIO_INTERNAL_SAMPLE_RATE;
    ap->internal_config->ch_count = AUDIO_INTERNAL_CHANNELS;
    ap->internal_config->codec = AUDIO_INTERNAL_CODEC;
    ap_config(ap, AUDIO_DEFAULT_BPS, AUDIO_DEFAULT_SAMPLE_RATE, AUDIO_DEFAULT_CHANNELS, AUDIO_DEFAULT_CODEC);
    // Decoded circular queue
    init_data.chan = AUDIO_INTERNAL_CHANNELS;
    init_data.size = AUDIO_INTERNAL_SIZE;
    ap->decoded_cq = cq_init(
            AUDIO_CIRCULAR_QUEUE_SIZE,
            bag_init,
            &init_data,
            bag_destroy,
            get_media_time_ptr);

    switch(ap->role) {
        case DECODER:
            ap->worker = decoder_thread;
            // Coded circular queue
            init_data.chan = AUDIO_DEFAULT_CHANNELS;
            init_data.size = (AUDIO_DEFAULT_SAMPLE_RATE * AUDIO_DEFAULT_BPS);
            ap->coded_cq = cq_init(
                    AUDIO_CIRCULAR_QUEUE_SIZE,
                    bag_init,
                    &init_data,
                    bag_destroy,
                    get_media_time_ptr);
            break;
        case ENCODER:
            ap->worker = encoder_thread;
            // Coded circular queue
            init_data.chan = AUDIO_DEFAULT_CHANNELS;
            init_data.size = AUDIO_DEFAULT_SIZE;
            ap->coded_cq = cq_init(
                    AUDIO_CIRCULAR_QUEUE_SIZE,
                    bag_init,
                    &init_data,
                    bag_destroy,
                    get_media_time_ptr);
            break;
        default:
            break;
    }

    return ap;
}

void ap_destroy(audio_processor_t *ap)
{
    ap->run = false;
    pthread_join(ap->thread, NULL);
    cq_destroy(ap->decoded_cq);
    cq_destroy(ap->coded_cq);
    free(ap->external_config);
    free(ap->internal_config);
    free(ap);
}

void ap_config(audio_processor_t *ap, int bps, int sample_rate, int channels, audio_codec_t codec)
{
    // External and internal audio configuration
    ap->external_config->bps = bps;
    ap->external_config->sample_rate = sample_rate;
    ap->external_config->ch_count = channels;
    ap->external_config->codec = codec;

    switch(ap->role) {
        case DECODER:
            // Specific configurations and conversion bussines logic.
            ap->compression_config = audio_codec_init(ap->external_config->codec, AUDIO_DECODER);
            ap->resampler = resampler_prepare(ap->internal_config->sample_rate);
            ap->internal_conversion_resample_size = normalize_get_resample_size(ap);
            ap->internal_conversion_decompress_size = normalize_get_decompress_size(ap);
            break;

        case ENCODER:
            // Specific configurations and conversion bussines logic.
            ap->compression_config = audio_codec_init(ap->external_config->codec, AUDIO_CODER);
            ap->resampler = resampler_prepare(ap->external_config->sample_rate);
            break;

        default:
            break;
    }
}

void ap_worker_start(audio_processor_t *ap)
{
    ap->run = true;
    if (pthread_create(&ap->thread, NULL, ap->worker, (void *)ap) != 0)
        ap->run = false;
}

struct audio_desc *ap_get_config(audio_processor_t *ap)
{
    return ap->external_config;
}

