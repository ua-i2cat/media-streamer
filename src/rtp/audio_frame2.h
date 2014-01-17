/*
 *  audio_frame2.h
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of media-streamer.
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
 *  Authors:  Jordi "Txor" Casas Ríos <txorlings@gmail.com>
 */

/**
 * @file audio_frame2.h
 * @brief Provide some coupled tools from audio subsystem to RTP library.
 */

#include "audio.h"

#ifndef __AUDIO_FRAME2_H__
#define __AUDIO_FRAME2_H__

typedef struct {
    const char *name;
    uint32_t    tag;
} rtp_audio_codec_info_t;

audio_frame2 *rtp_audio_frame2_init(void);
void rtp_audio_frame2_allocate(audio_frame2 *, int nr_channels, int max_size);
void rtp_audio_frame2_free(audio_frame2 *);

uint32_t rtp_get_audio_tag(audio_codec_t codec);
audio_codec_t rtp_get_audio_codec_to_tag(uint32_t audio_tag);

#endif // __AUDIO_FRAME2_H__
