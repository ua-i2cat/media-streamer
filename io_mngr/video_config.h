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

#define VIDEO_CIRCULAR_QUEUE_SIZE 16

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080
#define MAX_COUNTER 255

#define DEFAULT_FPS 25
#define PIXEL_FORMAT RGB

#define VIDEO_INTERNAL_WIDTH MAX_WIDTH
#define VIDEO_INTERNAL_HEIGHT MAX_HEIGHT
#define VIDEO_INTERNAL_CODEC RAW
#define VIDEO_INTERNAL_FPS DEFAULT_FPS

#define VIDEO_EXTERNAL_WIDTH MAX_WIDTH
#define VIDEO_EXTERNAL_HEIGHT MAX_HEIGHT
#define VIDEO_EXTERNAL_CODEC RAW
#define VIDEO_EXTERNAL_FPS DEFAULT_FPS

#endif //__VIDEO_CONFIG_H__ 

