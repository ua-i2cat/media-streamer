/*
 *  receiver.h - Manager for the received media streams.
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
 *  Authors:  Jordi "Txor" Casas Ríos <txorlings@gmail.com>,
 *            David Cassany <david.cassany@i2cat.net>,
 *            Ignacio Contreras <ignacio.contreras@i2cat.net>,
 *            Marc Palau <marc.palau@i2cat.net>
 */

/**
 * @file receiver.h
 * @brief The receiver can manage multiple input streams for audio and video.
 */

#ifndef __RECEIVER_H__
#define __RECEIVER_H__

#include "stream_list.h"
#include "participants.h"

typedef struct receiver
{
    // Video data
    stream_list_t *video_stream_list;
    int video_port;
    pthread_t video_th_id;
    uint8_t video_run;
    struct rtp *video_session;
    struct pdb *video_part_db;

    // Audio data
    stream_list_t *audio_stream_list;
    int audio_port;
    pthread_t audio_th_id;
    uint8_t audio_run;
    struct rtp *audio_session;
    struct pdb *audio_part_db;
} receiver_t;

/**
 * Initializes a receiver.
 * @param video_stream_list Video stream list instance.
 * @param audio_stream_list Audio stream list instance.
 * @param video_port Port to bind RTP video session.
 * @param audio_port Port to bind RTP audio session.
 * @return The receiver instance if success, NULL otherwise.
 */
receiver_t *init_receiver(stream_list_t *video_stream_list, stream_list_t *audio_stream_list, uint32_t video_port, uint32_t audio_port);

/**
 * Starts both audio and video receiver threads.
 * @param receiver The receiver instance.
 * @return True if both threads are running, false otherwise.
 */
bool start_receiver(receiver_t *receiver);

/**
 * Stops both audio and video receiver threads.
 * @param receiver The receiver instance.
 */
void stop_receiver(receiver_t *receiver);

/**
 * Destroys receiver instance if its threads are stopped.
 * @param receiver The receiver instance.
 * @return True if it was destroyed, false otherwise.
 */
int destroy_receiver(receiver_t *receiver);

#endif //__RECEIVER_H__

