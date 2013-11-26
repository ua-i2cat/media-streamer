/*
 * A resampler_resample wrapper:
 * Transform a greater audio_frame2 to a smaller one,
 * then call resample with that one and
 * then group the resulting audio_frame2s together into one with
 * a original size.
 *
 * Usage:   Compile the .c, include the .h and call the function.
 *
 * By Txor <jordi.casas@i2cat.net>
 */
#ifndef __RESIZED_RESAMPLE_H__
#define __RESIZED_RESAMPLE_H__

#include "audio.h"
#include "resampler.h"

audio_frame2 *resize_resample(struct resampler *s, audio_frame2 *frame, void *callback, int max_size);

#endif //__RESIZED_RESAMPLE_H__

