/*
 * FILE:    audio/audio.h
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

#ifndef _AUDIO_FRAME2_H_
#define _AUDIO_FRAME2_H_

#define MAX_AUDIO_CHANNELS      8

typedef struct {
    const char *name;
    /** @var tag
     *  @brief TwoCC if defined, otherwise we define our tag
     */
    uint32_t    tag;
} audio_codec_info_t;

// Internal definition, must match the definition from audio part.
// TODO: Import it from audio part.
typedef enum {
    AC_NONE,
    AC_PCM,
    AC_ALAW,
    AC_MULAW,
    AC_ADPCM_IMA_WAV,
    AC_SPEEX,
    AC_OPUS,
    AC_G722,
    AC_G726,
} audio_codec_t;

typedef struct
{
    int bps;                /* bytes per sample */
    int sample_rate;
    char *data[MAX_AUDIO_CHANNELS]; /* data should be at least 4B aligned */
    int data_len[MAX_AUDIO_CHANNELS];           /* size of useful data in buffer */
    int ch_count;		/* count of channels */
    unsigned int max_size;  /* maximal size of data in buffer */
    audio_codec_t codec;
} audio_frame2;

audio_frame2 *audio_frame2_init(void);
void audio_frame2_allocate(audio_frame2 *, int nr_channels, int max_size);
void audio_frame2_free(audio_frame2 *);

uint32_t get_audio_tag(audio_codec_t codec);
audio_codec_t get_audio_codec_to_tag(uint32_t audio_tag);

#endif // _AUDIO_FRAME2_H_
