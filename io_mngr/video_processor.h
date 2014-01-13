/*
 *  video_processor.h
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
 *  Authors:  Jordi "Txor" Casas Ríos <txorlings@gmail.com>,
 *            David Cassany <david.cassany@i2cat.net>,
 *            Ignacio Contreras <ignacio.contreras@i2cat.net>,
 *            Marc Palau <marc.palau@i2cat.net>
 */

/**
 * @file video_processor.h
 * @brief Video processor contains two circular queues, a video configuration and encoder/decoder threads. It is responsible for transforming video to/from internal format.
 *
 */

#ifndef __VIDEO_PROCESSOR_H__
#define __VIDEO_PROCESSOR_H__

//#include <pthread.h>
#include "types.h"
#include "video_frame2.h"
#include "circular_queue.h"
#include "commons.h"
#include "module.h"

typedef struct video_processor
{
    // Common data
    role_t role;
    circular_queue_t *decoded_cq;
    circular_queue_t *coded_cq;

    // Video configuration
    struct video_desc *external_config;
    struct video_desc *internal_config;

    struct state_decompress *decompressor;

    struct compress_state *compressor;
    struct module module;

    // Stream stats
    unsigned int seqno;
    unsigned int bitrate;
    unsigned int lost_coded_frames; 

    // Thread data
    int run;
    void *worker;
    pthread_t thread;
} video_processor_t;

/**
 * Initializes the video processor
 * @param role Defines the role of this video processor (encoder or decoder)
 * @return video_processor_t * if succeeded, NULL otherwise
 */
video_processor_t *vp_init(role_t role);

/**
 * Destroys the video processor
 * @param vp Target video_processor_t
 */
void vp_destroy(video_processor_t *vp);

/**
 * Configure the internal video format.
 * @param vp Target video_processor_t.
 * @param width New width value (Use 0 for avoid to change it).
 * @param height New height value (Use 0 for avoid to change it).
 * @param codec New codec type.
 */
void vp_reconfig_internal(video_processor_t *vp, unsigned int width, unsigned int height, codec_t codec);

/**
 * Configure the external video format.
 * @param vp Target video_processor_t.
 * @param width New width value (Use 0 for avoid to change it).
 * @param height New height value (Use 0 for avoid to change it).
 * @param codec New codec type.
 */
void vp_reconfig_external(video_processor_t *vp, unsigned int width, unsigned int height, codec_t codec);

/**
 * Starts the worker thread.
 * @param vp The video_processor_t where the thread operates.
 * @return decoder_thread_t * if succeeded, NULL otherwise.
 */
void vp_worker_start(video_processor_t *vp);

#endif //__VIDEO_PROCESSOR_H__

