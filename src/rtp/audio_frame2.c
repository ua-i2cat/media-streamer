/*
 *  audio_frame2.c
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of media-streamer.
 *  It is an implementation of some utils for the struct audio_frame2 used to
 *  decouple audio_frame2 from the audio part.
 *  This file is based on src/audio/utils.c, you can read its license and its 
 *  original authors it that file.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "config_unix.h"
#include "config_win32.h"
#endif // HAVE_CONFIG_H

#include "rtp/audio_frame2.h"

rtp_audio_codec_info_t rtp_audio_codec_info[] = {
    [AC_NONE] = { "(none)", 0 }, 
    [AC_PCM] = { "PCM", 0x0001 },
    [AC_ALAW] = { "A-law", 0x0006 },
    [AC_MULAW] = { "u-law", 0x0007 },
    [AC_ADPCM_IMA_WAV] = { "ADPCM", 0x0011 },
    [AC_SPEEX] = { "speex", 0xA109 },
    [AC_OPUS] = { "OPUS", 0x7375704F },
    [AC_G722] = { "G.722", 0x028F },
    [AC_G726] = { "G.726", 0x0045 },
};

int audio_codec_info_len = sizeof(rtp_audio_codec_info)/sizeof(rtp_audio_codec_info_t);

audio_frame2 *rtp_audio_frame2_init()
{
    audio_frame2 *ret = (audio_frame2 *) calloc(1, sizeof(audio_frame2));
    return ret;
}

void rtp_audio_frame2_allocate(audio_frame2 *frame, int nr_channels, int max_size)
{
    assert(nr_channels <= MAX_AUDIO_CHANNELS);

    frame->max_size = max_size;
    frame->ch_count = nr_channels;

    for(int i = 0; i < MAX_AUDIO_CHANNELS; ++i) {
        free(frame->data[i]);
        frame->data[i] = NULL;
        frame->data_len[i] = 0;
    }

    for(int i = 0; i < nr_channels; ++i) {
        frame->data[i] = malloc(max_size);
    }
}

void rtp_audio_frame2_free(audio_frame2 *frame)
{
    if(!frame)
        return;
    for(int i = 0; i < MAX_AUDIO_CHANNELS; ++i) {
        free(frame->data[i]);
    }
    free(frame);
}

uint32_t rtp_get_audio_tag(audio_codec_t codec)
{
    return rtp_audio_codec_info[codec].tag;
}

audio_codec_t rtp_get_audio_codec_to_tag(uint32_t tag)
{
    for(int i = 0; i < audio_codec_info_len; ++i) {
        if(rtp_audio_codec_info[i].tag == tag) {
            return i;
        }
    }
    return AC_NONE;
}

