/*
 *  audio_processor.c - Audio stream processor based on video_data.c
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
 *  Authors:  Jordi "Txor" Casas Ríos <jordi.casas@i2cat.net>
 */

#include <stdlib.h>
#include "debug.h"
#include "audio_processor.h"
#include "audio_config.h"
#include "circular_queue.h"
#include "audio_frame2.h"
#include "utils.h"

// private data
struct bag_init_val {
    int chan;
    int size;
};

// Input sizes that return AUDIO_INTERNAL_SIZE size on applying resampling.
// Sample rates:              8K  16K  32K  48K
int resampler_size_values[4] = { 50, 100, 200, 300 };

// private functions
static void *decoder_thread(void *arg);
static void *encoder_thread(void *arg);
static void *bag_init(void *arg);
static void bag_destroy(void *arg);
static int normalize_get_size();
static void normalize_extract(audio_frame2 *src, audio_frame2 *dst, int size);
static void audio_frame_format(audio_frame2 *src, struct audio_desc *desc);
static void audio_frame_append(audio_frame2 *src, audio_frame2 *dst);

static void *decoder_thread(void* arg)
{
    audio_processor_t *ap = (audio_processor_t *) arg;

    audio_frame2 *frame, *output_frame, *normal;

    normal = audio_frame2_init();
    bool normalized;

    while(ap->run) {
        if(ap->coded_cq->level != CIRCULAR_QUEUE_EMPTY) {
            // TODO: Channel muxing

            // Get the frame to process
            if ((frame = (audio_frame2 *)cq_get_front(ap->coded_cq)) != NULL) {

                // Get the place to save the resampled frame (last step).
                if ((output_frame = (audio_frame2 *)cq_get_rear(ap->decoded_cq)) != NULL) {
                    resampler_set_resampled(ap->resampler, output_frame);

                    // TODO: always get the same size, not only when the frame is too big.
                    normalized = false;
                    if (frame->data_len[0] > ap->internal_frame_size) {
                        normalize_extract(frame, normal, ap->internal_frame_size);
                        normalized = true;
                    }

                    // Decompress audio_frame2
                    if (normalized) {
                        frame = audio_codec_decompress(ap->compression_config, normal);
                    }
                    else {
                        frame = audio_codec_decompress(ap->compression_config, frame);
                        cq_remove_bag(ap->coded_cq);
                    }

                    // Resample audio_frame2
                    frame = resampler_resample(ap->resampler, frame);

                    // Commit the cq changes.
                    cq_add_bag(ap->decoded_cq);
                }
            }
        }
    }
    audio_frame2_free(normal);

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
    audio_frame2 *frame;
    struct bag_init_val *init_object = (struct bag_init_val *)init;
    int channels = init_object->chan;
    int max_size = init_object->size;

    frame = rtp_audio_frame2_init();
    rtp_audio_frame2_allocate(frame, channels, max_size);

    return (void *)frame;
}

static void bag_destroy(void *bag)
{
    rtp_audio_frame2_free((audio_frame2 *)bag);
}

static int normalize_get_size(audio_processor_t *ap)
{
    int size, bps_factor, index;

    switch(ap->external_config->sample_rate) {
        case 8000:
            index = 0;
            break;
        case 16000:
            index = 1;
            break;
        case 32000:
            index = 2;
            break;
        case 48000:
            index = 3;
            break;
    }
    bps_factor = ap->internal_config->bps / ap->external_config->bps;
    size = resampler_size_values[(int)index] / bps_factor;

    return size;
}

static void normalize_extract(audio_frame2 *src, audio_frame2 *dst, int size)
{
    char *buf;
    if ((buf = malloc(src->data_len[0])) == NULL) {
        error_msg("normalize_extract: malloc: out of memory!");
        return;
    }

    audio_frame2_allocate(dst, src->ch_count, size);

    dst->bps = src->bps;
    dst->sample_rate = src->sample_rate;
    dst->codec = src->codec;

    for (int i = 0; i < src->ch_count; i++) {
        memcpy(buf, src->data[i], src->data_len[i]);

        src->data_len[i] = src->data_len[i] - size;
        memcpy(src->data[i], buf + size, src->data_len[i]);

        dst->data_len[i] = size;
        memcpy(dst->data[i], buf, size);
    }

    free(buf);
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
    for (int i = 0; i < src->ch_count; i++) {
        memcpy(dst->data[i] + dst->data_len[i], src->data[i], src->data_len[i]);
        dst->data_len[i] += src->data_len[i];
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
    init_data.size = (AUDIO_INTERNAL_SAMPLE_RATE * AUDIO_INTERNAL_BPS);
    ap->decoded_cq = cq_init(CIRCULAR_QUEUE_SIZE, bag_init, bag_destroy, &init_data);

    switch(ap->role) {
        case DECODER:
            ap->worker = decoder_thread;
            // Coded circular queue
            init_data.chan = AUDIO_DEFAULT_CHANNELS;
            init_data.size = (AUDIO_DEFAULT_SAMPLE_RATE * AUDIO_DEFAULT_BPS);
            ap->coded_cq = cq_init(CIRCULAR_QUEUE_SIZE, bag_init, bag_destroy, &init_data);
            break;
        case ENCODER:
            ap->worker = encoder_thread;
            // Coded circular queue
            init_data.chan = AUDIO_DEFAULT_CHANNELS;
            init_data.size = AUDIO_DEFAULT_SIZE;
            ap->coded_cq = cq_init(CIRCULAR_QUEUE_SIZE, bag_init, bag_destroy, &init_data);
            break;
        default:
            break;
    }

    return ap;
}

void ap_destroy(audio_processor_t *ap)
{
    ap->run = FALSE;
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
            ap->internal_frame_size = normalize_get_size(ap);
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
    ap->run = TRUE;
    if (pthread_create(&ap->thread, NULL, ap->worker, (void *)ap) != 0)
        ap->run = FALSE;
}

struct audio_desc *ap_get_config(audio_processor_t *ap)
{
    return ap->external_config;
}

