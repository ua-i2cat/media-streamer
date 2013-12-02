/*
 *  audio_processor.h
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
 *  Authors:  Jordi "Txor" Casas Ríos <jordi.casas@i2cat.net>,
 *            David Cassany <david.cassany@i2cat.net>,
 *            Ignacio Contreras <ignacio.contreras@i2cat.net>,
 *            Marc Palau <marc.palau@i2cat.net>
 */

/**
 * @file audio_processor.h
 * @brief Audio processor contains two circular queues, an audio configuration and encoder/decoder threads. It is responsible for transforming audio to/from internal format.
 *
 */

#ifndef __AUDIO_DATA_H__
#define __AUDIO_DATA_H__

#include "circular_queue.h"
#include "resampler.h"
#include "codec.h"
#include "audio.h"
#include "commons.h"

typedef struct audio_processor {
    role_t role;
    circular_queue_t *decoded_cq;
    circular_queue_t *coded_cq;

    // Audio configurations
    struct audio_desc *external_config;
    struct audio_desc *internal_config;
    struct resampler *resampler;
    struct audio_codec_state *compression_config;
    int normalized_frame_size;

    // Thread data
    int run;
    void *worker;
    pthread_t thread;
} audio_processor_t;

/**
 * Initializes the audio processor
 * @param role Defines the role of this audio processor (encoder or decoder)
 * @return audio_processor_t * if succeeded, NULL otherwise
 */
audio_processor_t *ap_init(role_t role);

/**
 * Destroys the audio processor
 * @param ap Target audio_processor_t
 */
void ap_destroy(audio_processor_t *ap);

/**
 * Configure the external and internal audio format.
 * @param ap Target audio_processor_t.
 * @param bps External bytes per second value.
 * @param sample_rate External sample rate.
 * @param channels External number of channels.
 * @param codec External codification type.
 */
void ap_config(audio_processor_t *ap, int bps, int sample_rate, int channels, audio_codec_t codec);

/**
 * Starts the worker thread.
 * @param ap The audio_processor_t where the thread operates.
 * @return decoder_thread_t * if succeeded, NULL otherwise.
 */
void ap_worker_start(audio_processor_t *ap);

#endif //__AUDIO_DATA_H__

