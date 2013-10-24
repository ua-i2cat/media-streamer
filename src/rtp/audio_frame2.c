/*
 * FILE:    audio/utils.c
 * AUTHORS: Martin Benes     <martinbenesh@gmail.com>
 *          Lukas Hejtmanek  <xhejtman@ics.muni.cz>
 *          Petr Holub       <hopet@ics.muni.cz>
 *          Milos Liska      <xliska@fi.muni.cz>
 *          Jiri Matela      <matela@ics.muni.cz>
 *          Dalibor Matura   <255899@mail.muni.cz>
 *          Ian Wesley-Smith <iwsmith@cct.lsu.edu>
 *
 * Copyright (c) 2005-2010 CESNET z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *      This product includes software developed by CESNET z.s.p.o.
 * 
 * 4. Neither the name of CESNET nor the names of its contributors may be used 
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */

/*
 *
 * Implementation of some utils for the struct audio_frame2
 * Used to decouple audio_frame2 from the audio part.
 *
 */ 

//#include "audio/audio.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "config_unix.h"
#include "config_win32.h"
#endif // HAVE_CONFIG_H

#include "rtp/audio_frame2.h"
//#include "audio/codec.h"
//#include "audio/utils.h" 

audio_codec_info_t audio_codec_info[] = {
    [AC_NONE] = { "(none)", 0 }, 
    [AC_PCM] = { "PCM", 0x0001 },
    [AC_ALAW] = { "A-law", 0x0006 },
    [AC_MULAW] = { "u-law", 0x0007 },
    [AC_ADPCM_IMA_WAV] = { "ADPCM", 0x0011 },
    [AC_SPEEX] = { "speex", 0xA109 },
    [AC_OPUS] = { "OPUS", 0x7375704F }, // == Opus, the TwoCC isn't defined
    [AC_G722] = { "G.722", 0x028F },
    [AC_G726] = { "G.726", 0x0045 },
};

int audio_codec_info_len = sizeof(audio_codec_info)/sizeof(audio_codec_info_t);

audio_frame2 *audio_frame2_init()
{
    audio_frame2 *ret = (audio_frame2 *) calloc(1, sizeof(audio_frame2));
    return ret;
}

void audio_frame2_allocate(audio_frame2 *frame, int nr_channels, int max_size)
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

void audio_frame2_free(audio_frame2 *frame)
{
    if(!frame)
        return;
    for(int i = 0; i < MAX_AUDIO_CHANNELS; ++i) {
        free(frame->data[i]);
    }
    free(frame);
}

uint32_t get_audio_tag(audio_codec_t codec)
{
    return audio_codec_info[codec].tag;
}

audio_codec_t get_audio_codec_to_tag(uint32_t tag)
{
    for(int i = 0; i < audio_codec_info_len; ++i) {
        if(audio_codec_info[i].tag == tag) {
            return i;
        }
    }
    return AC_NONE;
}

