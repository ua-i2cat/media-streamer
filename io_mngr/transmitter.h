/*
 *  transmitter.h - Manager for transmitting media streams.
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
 *  Authors:  Jordi "Txor" Casas Ríos <txorlings@gmail.com>,
 *            David Cassany <david.cassany@i2cat.net>,
 *            Ignacio Contreras <ignacio.contreras@i2cat.net>,
 *            Marc Palau <marc.palau@i2cat.net>
 */

/**
 * @file transmitter.h
 * @brief Transmitter internal data and functions.
 * Allows the compression and transmition over RTP of uncompressed video
 * (RGB interleaved 4:4:4, according to <em>ffmpeg</em>: <em>rgb24</em>),
 * using the participants.h API.
 * Also allows the compression and transmition over RTP of compressed audio
 * with configurable format using the participants.h API.
 */

#ifndef __TRANSMITTER_H__
#define __TRANSMITTER_H__

#include "stream_list.h"
#include "participants.h"

typedef struct transmitter {
    // Video data
    uint32_t video_run;
    pthread_t video_thread;
    stream_list_t *video_stream_list;
    float fps;
    float wait_time;
    uint32_t video_recv_port;
    uint32_t video_ttl;
    uint64_t video_send_buffer_size;
    uint32_t video_mtu;

    // Audio data
    uint32_t audio_run;
    pthread_t audio_thread;
    stream_list_t *audio_stream_list;
    uint32_t audio_recv_port;
    uint32_t audio_ttl;
    uint64_t audio_send_buffer_size;
    uint32_t audio_mtu;
} transmitter_t;

/**
 * Initializes a transmitter.
 * @param video_stream_list Video stream list instance.
 * @param audio_stream_list Audio stream list instance.
 * @return The transmitter instance if success, NULL otherwise.
 */
transmitter_t *init_transmitter(stream_list_t *video_stream_list, stream_list_t *audio_stream_list);

/**
 * Starts both audio and video transmitter threads.
 * @param transmitter The transmitter instance.
 * @return True if both threads are running, false otherwise.
 */
bool start_transmitter(transmitter_t *transmitter);

/**
 * Stops both audio and video transmitter threads.
 * @param transmitter The transmitter instance.
 */
void stop_transmitter(transmitter_t *transmitter);

/**
 * Destroys a transmitter if its threads are stopped.
 * @param transmitter The transmitter instance.
 * @return true if success, false otherwise.
 */
bool destroy_transmitter(transmitter_t *transmitter);

#endif //__TRANSMITTER_H__

