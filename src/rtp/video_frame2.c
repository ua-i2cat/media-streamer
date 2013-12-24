/*
 *  video_frame2.c
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of UG-Modules RTP library.
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
 *  Authors:  Jordi "Txor" Casas Ríos <jordi.casas@i2cat.net>,
 */

#include <stdlib.h>
#include <unistd.h>
#include "video_frame2.h"
#include "debug.h"

video_frame2 *rtp_video_frame2_init()
{
    video_frame2 *frame;
    if ((frame = malloc(sizeof(video_frame2))) == NULL) {
        error_msg("rtp_video_frame2_init: malloc: out of memory");
        return NULL;
    }

    // Data
    frame->buffer = NULL;
    frame->buffer_len = 0;
    // Stats
    frame->media_time = 0;
    frame->seqno = 0;
    // Control
    frame->type = BFRAME;

    return frame;
}

void rtp_video_frame2_allocate(video_frame2 *frame, unsigned int width, unsigned int height, codec_t codec)
{
    frame->width = width;
    frame->height = height;
    frame->codec = codec;
    
    if (width == 0 || height == 0){
        width = VIDEO_FRAME2_MAX_WIDTH;
        height = VIDEO_FRAME2_MAX_HEIGHT;
    }

    frame->buffer_len = width * height * 3; 
    if ((frame->buffer = realloc(frame->buffer, frame->buffer_len)) == NULL) {
        error_msg("rtp_video_frame2_allocate: realloc: out of memory!");
    }
}

void rtp_video_frame2_free(video_frame2 *frame)
{
    free(frame->buffer);
    free(frame);
}

