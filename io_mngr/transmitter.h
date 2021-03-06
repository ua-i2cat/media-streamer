/*
 *  transmitter.h
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of io_mngr.
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

#include "stream.h"
#include "participants.h"

// TODO: Put away format constants
#define DEFAULT_FPS 24
#define DEFAULT_RTCP_BW 5 * 1024 * 1024 * 10
#define DEFAULT_TTL 255
#define DEFAULT_SEND_BUFFER_SIZE 1920 * 1080 * 4 * sizeof(char) * 10
#define PIXEL_FORMAT RGB
#define MTU 1300 // 1400

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
    //float fps;
    //float wait_time;
    uint32_t audio_recv_port;
    uint32_t audio_ttl;
    uint64_t audio_send_buffer_size;
    uint32_t audio_mtu;
} transmitter_t;

/**
 * Initializes a transmitter object with both audio and video.
 * @param video_stream_list Initialized stream_list_t to use for video.
 * @param audio_stream_list Initialized stream_list_t to use for audio.
 * @return A pointer to the generated transmitter_t object, NULL otherwise.
 */
transmitter_t *init_transmitter(stream_list_t *video_stream_list, stream_list_t *audio_stream_list, float fps);

/**
 * Starts both audio and video transmitter threads.
 * @param transmitter The transmitter_t target.
 * @return Returns TRUE if both threads are running, FALSE otherwise.
 */
int start_transmitter(transmitter_t *transmitter);

/**
 * Stops both audio and video transmitter threads.
 * @param transmitter The transmitter_t target.
 */
void stop_transmitter(transmitter_t *transmitter);

/**
 * Frees the transmitter_t object if the threads are stopped.
 * @param transmitter The transmitter_t target.
 * @return TRUE if freed, FALSE otherwise.
 */
int destroy_transmitter(transmitter_t *transmitter);

#endif //__TRANSMITTER_H__

