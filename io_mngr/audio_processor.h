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
#include "audio.h"

typedef enum role {
    ENCODER,
    DECODER,
    NONE
} role_t;

typedef struct decoder_thread {
    pthread_t thread;
    int run;
} decoder_thread_t;

typedef struct encoder_thread {
    pthread_t thread;
    int run;
} encoder_thread_t;

typedef struct audio_processor {
    role_t role;
    circular_queue_t *decoded_cq;
    circular_queue_t *coded_cq;

    // Audio configurations
    int bps;                /* bytes per sample */
    int sample_rate;
    int ch_count;		/* count of channels */
    audio_codec_t codec;

//    int seqno;
    union {
        struct encoder_thread *encoder;
        struct decoder_thread *decoder;
    };
} audio_processor_t;

/**
 * Initializes the audio processor
 * @param role Defines the role of this audio processor (encoder or decoder)
 * @return audio_processor_t * if succeeded, NULL otherwise
 */
audio_processor_t *ap_init(role_t role);

/**
 * Destroys the audio processor
 * @param max Target audio_processor_t
 */
void ap_destroy(audio_processor_t *data);

/**
 * Initializes the decoder thread
 * @param ap The audio_processor_t where the thread operates
 * @return decoder_thread_t * if succeeded, NULL otherwise
 */
decoder_thread_t *ap_decoder_init(audio_processor_t *ap);

/**
 * Destroys the audio processor
 * @param max Target audio_processor_t
 */
void ap_decoder_destroy(decoder_thread_t *decoder);

/**
 * TODO
 * @param TODO
 * @return TODO
 */
void ap_decoder_start(audio_processor_t *ap);

/**
 * TODO
 * @param TODO
 * @return TODO
 */
void ap_decoder_stop(audio_processor_t *ap);

/**
 * TODO
 * @param TODO
 * @return TODO
 */
encoder_thread_t *ap_encoder_init(audio_processor_t *ap);

/**
 * TODO
 * @param TODO
 * @return TODO
 */
void ap_encoder_destroy(audio_processor_t *ap);

/**
 * TODO
 * @param TODO
 * @return TODO
 */
void ap_encoder_stop(audio_processor_t *ap);

#endif //__AUDIO_DATA_H__

