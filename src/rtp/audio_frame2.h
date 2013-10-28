/*
 * FILE:    rtp/audio_frame2.h
 * AUTHORS: Jordi "Txor" Casas Ríos   <jordi.casas@i2cat.net>
 *
 * Provide to RTP library some coupled tools from audio subsystem.
 *
 */

// audio_codec_t and audio_frame2 are imported from audio/audio.h
#include "audio/audio.h"

#ifndef _AUDIO_FRAME2_H_
#define _AUDIO_FRAME2_H_

// Copy of audio_codec_info_t at audio/codec.h:80
typedef struct {
    const char *name;
    /** @var tag
     *  @brief TwoCC if defined, otherwise we define our tag
     */
    uint32_t    tag;
} rtp_audio_codec_info_t;

// Copy of functions from audio/utils.c
audio_frame2 *rtp_audio_frame2_init(void);
void rtp_audio_frame2_allocate(audio_frame2 *, int nr_channels, int max_size);
void rtp_audio_frame2_free(audio_frame2 *);

uint32_t rtp_get_audio_tag(audio_codec_t codec);
audio_codec_t rtp_get_audio_codec_to_tag(uint32_t audio_tag);

#endif // _AUDIO_FRAME2_H_
