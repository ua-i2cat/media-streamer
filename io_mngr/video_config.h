/*
 *  video_config.h - Compile time video configurations.
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

/**
 * @file audio_config.h
 * @brief Compile time video configurations like internal format and queue size.
 */

#ifndef __VIDEO_CONFIG_H__ 
#define __VIDEO_CONFIG_H__ 

#include "video_frame2.h"

#define INITIAL_VIDEO_RECV_BUFFER_SIZE  ((4*1920*1080)*110/100) //command line net.core setup: sysctl -w net.core.rmem_max=9123840

#define VIDEO_CIRCULAR_QUEUE_SIZE 16

#define VIDEO_MAX_COUNTER 255

#define VIDEO_DEFAULT_PIXEL_FORMAT RGB

#define VIDEO_DEFAULT_FPS 25.0
#define VIDEO_DEFAULT_MEDIA_TIME 0
#define VIDEO_DEFAULT_SEQNO 0

#define VIDEO_DEFAULT_TYPE OTHER

#define VIDEO_DEFAULT_INTERLACING PROGRESSIVE
#define VIDEO_DEFAULT_TILE_COUNT 0

#define VIDEO_DEFAULT_INTERNAL_WIDTH VIDEO_FRAME2_MAX_WIDTH
#define VIDEO_DEFAULT_INTERNAL_HEIGHT VIDEO_FRAME2_MAX_HEIGHT
#define VIDEO_DEFAULT_INTERNAL_CODEC VIDEO_DEFAULT_PIXEL_FORMAT

#define VIDEO_DEFAULT_EXTERNAL_WIDTH VIDEO_FRAME2_MAX_WIDTH
#define VIDEO_DEFAULT_EXTERNAL_HEIGHT VIDEO_FRAME2_MAX_HEIGHT
#define VIDEO_DEFAULT_EXTERNAL_CODEC H264

#endif //__VIDEO_CONFIG_H__ 

