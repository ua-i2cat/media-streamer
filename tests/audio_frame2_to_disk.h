/*
 * To file audio_frame2 recorder.
 *
 * Usage:   Compile the .c, include the .h and call the function.
 *
 * By Txor <jordi.casas@i2cat.net>
 */
#ifndef __AUDIO_FRAME2_TO_DISK_H__
#define __AUDIO_FRAME2_TO_DISK_H__

#include "audio.h"

void write_audio_frame2(char *preffix, const audio_frame2 *frame, int bps, bool verbose);
void write_audio_frame2_channels(char *preffix, const audio_frame2 *frame, bool verbose);

#endif //__AUDIO_FRAME2_TO_DISK_H__

