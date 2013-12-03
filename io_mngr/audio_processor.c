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
#include "audio.h"

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

static void *decoder_thread(void* arg) {

    audio_processor_t *ap = (audio_processor_t *) arg;

    audio_frame2 *frame, *output_frame;

    while(ap->run) {
        if(ap->coded_cq->level != CIRCULAR_QUEUE_EMPTY) {
            // TODO: Frame size normalization
            // TODO: Channel muxing

            // Get the frame to process
            if ((frame = (audio_frame2 *)cq_get_front(ap->coded_cq)) != NULL) {

                // Get the place to save the resampled frame (last step).
                if ((output_frame = (audio_frame2 *)cq_get_rear(ap->decoded_cq)) != NULL) {
                    resampler_set_resampled(ap->resampler, output_frame);

                    // Decompress audio_frame2
                    frame = audio_codec_decompress(ap->compression_config, frame);

                    // Resample audio_frame2
                    frame = resampler_resample(ap->resampler, frame);

                    // Commit the cq changes.
                    cq_remove_bag(ap->coded_cq);
                    cq_add_bag(ap->decoded_cq);
                }
            }
        }
    }

    pthread_exit((void *)NULL);    
}

static void *encoder_thread(void *arg) {

    audio_processor_t *ap = (audio_processor_t *)arg;

    audio_frame2 *frame = NULL;

    while (ap->run) {
        if(ap->decoded_cq->level != CIRCULAR_QUEUE_FULL) {
            //TODO: Encoder code
            frame++;
        }

    }

    pthread_exit((void *)NULL);
}

static void *bag_init(void *init) {

    audio_frame2 *frame;
    struct bag_init_val *init_object = (struct bag_init_val *)init;
    int channels = init_object->chan;
    int max_size = init_object->size;

    frame = rtp_audio_frame2_init();
    rtp_audio_frame2_allocate(frame, channels, max_size);

    return (void *)frame;
}

static void bag_destroy(void *bag) {

    rtp_audio_frame2_free((audio_frame2 *)bag);
}

audio_processor_t *ap_init(role_t role) {

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
    ap_config(ap, AUDIO_DEFAULT_BPS, AUDIO_DEFAULT_SAMPLE_RATE, AUDIO_DEFAULT_CHANNELS, AUDIO_DEFAULT_CODEC);
    // Coded circular queue
    init_data.chan = AUDIO_DEFAULT_CHANNELS;
    init_data.size = (AUDIO_DEFAULT_SAMPLE_RATE * AUDIO_DEFAULT_BPS);
    ap->coded_cq = cq_init(CIRCULAR_QUEUE_SIZE, bag_init, bag_destroy, &init_data);
    // Decoded circular queue
    init_data.chan = AUDIO_INTERNAL_CHANNELS;
    init_data.size = (AUDIO_INTERNAL_SAMPLE_RATE * AUDIO_INTERNAL_BPS);
    ap->decoded_cq = cq_init(CIRCULAR_QUEUE_SIZE, bag_init, bag_destroy, &init_data);

    switch(ap->role) {
        case DECODER:
            ap->worker = decoder_thread;
            break;
        case ENCODER:
            ap->worker = encoder_thread;
            break;
        default:
            break;
    }

    ap_worker_start(ap);

    return ap;
}

void ap_destroy(audio_processor_t *ap) {

    ap->run = FALSE;
    pthread_join(ap->thread, NULL);
    cq_destroy(ap->decoded_cq);
    cq_destroy(ap->coded_cq);
    free(ap->external_config);
    free(ap->internal_config);
    free(ap);
}

void ap_config(audio_processor_t *ap, int bps, int sample_rate, int channels, audio_codec_t codec) {

    // External and internal audio configuration
    ap->external_config->bps = bps;
    ap->external_config->sample_rate = sample_rate;
    ap->external_config->ch_count = channels;
    ap->external_config->codec = codec;
    ap->internal_config->bps = AUDIO_INTERNAL_BPS;
    ap->internal_config->sample_rate = AUDIO_INTERNAL_SAMPLE_RATE;
    ap->internal_config->ch_count = AUDIO_INTERNAL_CHANNELS;
    ap->internal_config->codec = AUDIO_INTERNAL_CODEC;

    switch(ap->role) {
        case DECODER:
            // Specific configurations and conversion bussines logic.
            ap->compression_config = audio_codec_init(ap->external_config->codec, AUDIO_DECODER);
            ap->resampler = resampler_prepare(ap->internal_config->sample_rate);
            break;

        case ENCODER:
            // Specific configurations and conversion bussines logic.
            ap->compression_config = audio_codec_init(ap->external_config->codec, AUDIO_CODER);
            //TODO
            break;

        default:
            break;
    }
}

void ap_worker_start(audio_processor_t *ap) {

    ap->run = TRUE;
    if (pthread_create(&ap->thread, NULL, ap->worker, (void *)ap) != 0)
        ap->run = FALSE;
}

struct audio_desc *ap_get_config(audio_processor_t *ap) {

    return ap->external_config;
}

