/*
 *  video_frame2.h
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
 *            David Cassany <david.cassany@i2cat.net>
 *            Marc Palau <marc.palau@i2cat.net>
 */

/**
 * @file video_frame2.h
 * @brief Video frame configuration.
 */

#ifndef __VIDEO_FRAME2_H__
#define __VIDEO_FRAME2_H__

#include "types.h"

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
    uint32_t media_time;
    uint32_t seqno;
    
    // Control
    video_frame2_type_t type;
} video_frame2;

#endif //__VIDEO_FRAME2_H__

