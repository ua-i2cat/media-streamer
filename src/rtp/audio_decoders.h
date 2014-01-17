/*
 *  audio_decoders.h
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of media-streamer.
 *  This file is a modification from the ultragrid project hosted at
 *  http://seth.ics.muni.cz/git/ultragrid.git, you can read its original
 *  license and its original authors in the original file. 
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
 *  Authors:  Jordi "Txor" Casas Ríos <txorlings@gmail.com>,
 *            David Cassany <david.cassany@i2cat.net>
 *            Marc Palau <marc.palau@i2cat.net>
 */

/**
 * @file circular_queue.h
 * @brief Old decode_audio_frame and new decode_audio_frame_mulaw functions and suppot structures.
 */


#ifndef __AUDIO_DECODERS_H__
#define __AUDIO_DECODERS_H__

struct state_audio_decoder {
    audio_frame2 *frame;

    struct resampler *resampler;

    struct audio_desc *desc;
};

struct coded_data;

int decode_audio_frame(struct coded_data *cdata, void *data);
int decode_audio_frame_mulaw(struct coded_data *cdata, void *data);

#endif //__AUDIO_DECODERS_H__
