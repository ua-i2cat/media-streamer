/*
 *  audio_config.h - Compile time configurations.
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
 * @brief Compile time audio configurations like internal format and queue size.
 */

#ifndef __AUDIO_CONFIG_H__ 
#define __AUDIO_CONFIG_H__ 

#include "audio.h"

#define CIRCULAR_QUEUE_SIZE 10

#define AUDIO_INTERNAL_SIZE 300

#define AUDIO_INTERNAL_BPS 2
#define AUDIO_INTERNAL_SAMPLE_RATE 48000
#define AUDIO_INTERNAL_CHANNELS 1
#define AUDIO_INTERNAL_CODEC AC_PCM

#define AUDIO_DEFAULT_BPS 1
#define AUDIO_DEFAULT_SAMPLE_RATE 8000
#define AUDIO_DEFAULT_CHANNELS 1
#define AUDIO_DEFAULT_CODEC AC_MULAW

#endif //__AUDIO_CONFIG_H__ 

