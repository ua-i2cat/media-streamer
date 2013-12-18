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
 *  Authors:  Jordi "Txor" Casas Ríos <jordi.casas@i2cat.net>,
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

//#include <semaphore.h>
//#include "config_unix.h"
//#include "types.h"
//#include "video_data_frame.h"
#include "circular_queue.h"
#include "commons.h"

// Video frame related data goes to video specific .h ???
typedef enum frame_type {
    INTRA,
    BFRAME,
    OTHER
} frame_type_t;

// Video frame related data goes to video specific .h ???
typedef struct video_frame_data {
    uint8_t *buffer;
    uint32_t buffer_len;
    uint32_t curr_seqno;
    uint32_t width;
    uint32_t height;
    uint32_t media_time;
    uint32_t seqno;
    frame_type_t frame_type;
    codec_t codec;
} video_data_frame_t;

//typedef struct decoder_thread {
//    pthread_t thread;
//    uint8_t run;
//    struct state_decompress *sd;
//} decoder_thread_t;

//typedef struct encoder_thread {
//    pthread_t thread;
//    uint8_t run;
//
//    int index;
//
//    struct video_frame *frame;  // TODO: should be gone!
//                                // redundant with stream->coded_frame
//    struct compress_state *cs;
//} encoder_thread_t;

typedef struct video_processor {
    role_t type;
    video_frame_cq_t *decoded_frames;
    video_frame_cq_t *coded_frames;

    // Video configurations
    uint32_t interlacing;  //TODO: fix this. It has to be UG enum
    uint32_t fps;       //TODO: fix this. It has to be UG enum
    uint32_t seqno;
    uint32_t bitrate;
    uint32_t lost_coded_frames;

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
 * Configure the external and internal video format.
 * @param vp Target video_processor_t.
 * @param bps External bytes per second value.
 * @param sample_rate External sample rate.
 * @param channels External number of channels.
 * @param codec External codification type.
 */
void vp_config(video_processor_t *vp, int bps, int sample_rate, int channels, video_codec_t codec);

/**
 * Starts the worker thread.
 * @param vp The video_processor_t where the thread operates.
 * @return decoder_thread_t * if succeeded, NULL otherwise.
 */
void vp_worker_start(video_processor_t *vp);

/**
 * Returns the so called external configuration.
 * @param vp The target video_processor_t.
 * @return struct audio_desc *
 */
struct audio_desc *vp_get_config(video_processor_t *vp);

//decoder_thread_t *init_decoder(video_processor_t *data);
//encoder_thread_t *init_encoder(video_processor_t *data);

#endif //__VIDEO_PROCESSOR_H__

