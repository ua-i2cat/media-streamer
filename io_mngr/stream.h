/*
 *  stream.h - Stream definitions.
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

#ifndef __STREAM_H__
#define __STREAM_H__

#include "video_processor.h"
#include "audio_processor.h"
#include "participants.h"

typedef enum stream_type {
    AUDIO,
    VIDEO
} stream_type_t;

typedef enum stream_state{
    ACTIVE,
    I_AWAIT,
    NON_ACTIVE
} stream_state_t;

typedef struct stream_data {
    stream_type_t type;
    io_type_t io_type;
    stream_state_t state;
    char *stream_name;
    uint32_t id;
    participant_list_t *plist;
    struct stream_data *prev;
    struct stream_data *next;
    union {
        audio_processor_t *audio;
        video_processor_t *video;
    };
} stream_data_t;

/**
 * Initializes a stream.
 * @return stream_data_t * if succeeded, NULL otherwise.
 */
stream_data_t *init_stream(stream_type_t type, io_type_t io_type, uint32_t id, stream_state_t state, char *stream_name);

/**
 * Destroys a stream.
 * @param stream Target stream_data_t.
 */
void destroy_stream(stream_data_t *stream);

/**
 * Set the stream state.
 * @param stream Target stream_data_t.
 * @param state New stream_state_t value.
 */
void set_stream_state(stream_data_t *stream, stream_state_t state);

/**
 * Add a participant to the stream.
 * @param stream Target stream_data_t.
 * @param participant Target participant_data_t.
 */
void add_participant_stream(stream_data_t *stream, participant_data_t *participant);

/**
 * Remove a participant, idenified with its id, from a stream.
 * @param stream Target stream_data_t.
 * @param id Target participant id.
 * @return TRUE if succeeded, FALSE otherwise.
 */
int remove_participant_from_stream(stream_data_t *stream, uint32_t id);

#endif //__STREAM_H__

