/*
 *  stream.h - Audio or video stream entity.
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
 * @file stream.h
 * @brief The stream contains all the necessary data to implement a node of audio or video stream list and to link with its corresponding participant.
 *
 */

#ifndef __STREAM_H__
#define __STREAM_H__

#include "video_processor.h"
#include "audio_processor.h"
#include "participants_list.h"

typedef enum stream_type {
    AUDIO,
    VIDEO
} stream_type_t;

typedef enum stream_state {
    ACTIVE,
    I_AWAIT,
    NON_ACTIVE
} stream_state_t;

typedef struct stream {
    stream_type_t type;
    io_type_t io_type;
    stream_state_t state;
    char *stream_name;
    unsigned int id;
    participant_list_t *plist;
    struct stream *prev;
    struct stream *next;
    union {
        audio_processor_t *audio;
        video_processor_t *video;
    };
} stream_t;

/**
 * Initializes a stream.
 * @param type Type of media.
 * @param io_type Type of input/output.
 * @param id Internal identification.
 * @param state Stream current status.
 * @param stream_name Stream name.
 * @return The stream instance, NULL otherwise.
 */
stream_t *init_stream(stream_type_t type, io_type_t io_type, unsigned int id, stream_state_t state, char *stream_name);

/**
 * Destroys a stream.
 * @param stream The stream instance.
 */
void destroy_stream(stream_t *stream);

/**
 * Setter for the stream state.
 * @param stream The stream instance.
 * @param state New stream_state_t value.
 */
void set_stream_state(stream_t *stream, stream_state_t state);

/**
 * Add a participant to the stream.
 * @param stream The stream instance.
 * @param participant Target participant_t.
 */
void add_participant_stream(stream_t *stream, participant_t *participant);

/**
 * Remove a participant, idenified with its id, from a stream.
 * @param stream The stream instance.
 * @param id Participant internal id.
 * @return True if succeeded, false otherwise.
 */
bool remove_participant_from_stream(stream_t *stream, unsigned int id);

#endif //__STREAM_H__

