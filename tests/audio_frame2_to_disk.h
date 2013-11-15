/*
 * To file audio_frame2 recorder.
 *
 * Usage:   Compile the .c, include the .h and call the function.
 *
 * By Txor <jordi.casas@i2cat.net>
 */
#ifndef __AUDIO_FRAME2_TO_DISK__
#define __AUDIO_FRAME2_TO_DISK__

#include "audio.h"

void write_audio_frame2_to_file (char *path, audio_frame2 *frame);
void add_audio_frame2_to_file (char *path, audio_frame2 *frame);

#endif //__AUDIO_FRAME2_TO_DISK__

