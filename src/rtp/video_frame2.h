/*
 *  video_frame2.h
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of UG-Modules RTP library.
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
 *            David Cassany <david.cassany@i2cat.net>
 *            Marc Palau <marc.palau@i2cat.net>
 */

/**
 * @file video_frame2.h
 * @brief Video frame configuration.
 */

/**
 * @file video_frame2.h
 * @brief video_frame2 structure and methods.
 */

#ifndef __VIDEO_FRAME2_H__
#define __VIDEO_FRAME2_H__

#include "types.h"

#define VIDEO_FRAME2_MAX_WIDTH 1920
#define VIDEO_FRAME2_MAX_HEIGHT 1080

typedef enum video_frame2_type {
    INTRA,
    BFRAME,
    OTHER
} video_frame2_type_t;

typedef struct
{
    // Config
    unsigned int width;
    unsigned int height;
    codec_t codec;
    // Data
    char *buffer;
    unsigned int buffer_len;
    // Stats
    double fps;
    unsigned int media_time;
    unsigned int seqno;
    // Control
    video_frame2_type_t type;
} video_frame2;

/**
 * Create and initialize a video_frame2 with some default values.
 * @return video_frame2 * if succeeded, NULL otherwise.
 */
video_frame2 *rtp_video_frame2_init(void);

/**
 * Allocate memory space for the audio_frame2 buffer with specified video frame size, also configures the codec type.
 * @param frame Target video_frame2.
 * @param width New width for the video frame.
 * @param height New height for the video frame.
 * @param codec New codec type for the video frame.
 */
void rtp_video_frame2_allocate(video_frame2 *frame, unsigned int width, unsigned int height, codec_t codec);

/**
 * Frees the allocated memory for a video_frame2
 * @param frame Target video_frame2.
 */
void rtp_video_frame2_free(video_frame2 *frame);

#endif //__VIDEO_FRAME2_H__

