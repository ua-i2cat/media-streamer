/*
 *  stream.h
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

#ifndef __STREAM_H__
#define __STREAM_H__

#include <pthread.h>
#include <semaphore.h>
#include "video_data.h"
#include "audio_processor.h"
#include "participants.h"
#include "commons.h"

typedef enum stream_type {
    AUDIO,
    VIDEO
} stream_type_t;

typedef enum stream_state{
    ACTIVE,
    I_AWAIT,
    NON_ACTIVE
} stream_state_t;

typedef struct audio_data {
    // TODO
} audio_data_t;

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
        video_data_t *video;
    };
} stream_data_t;

typedef struct stream_list {
    pthread_rwlock_t lock;
    int count;
    stream_data_t *first;
    stream_data_t *last;
} stream_list_t;

/**
 * Initializes a stream list.
 * @return stream_list_t * if succeeded, NULL otherwise.
 */
stream_list_t *init_stream_list(void);

/**
 * Destroys a stream list.
 * @param list Target stream_list_t.
 */
void destroy_stream_list(stream_list_t *list);

/**
 * Initializes a stream.
 * @return stream_data_t * if succeeded, NULL otherwise.
 */
stream_data_t *init_stream(stream_type_t type, io_type_t io_type, uint32_t id, stream_state_t state, float fps, char *stream_name);

/**
 * Destroys a stream.
 * @param stream Target stream_data_t.
 */
void destroy_stream(stream_data_t *stream);

/**
 * Attaches a stream to the first free position of a stream list.
 * @param list Target stream_list_t.
 * @param stream Target stream_data_t.
 * @return TRUE if succeeded, FALSE otherwise.
 */
int add_stream(stream_list_t *list, stream_data_t *stream);

/**
 * Removes a stream, identified with its id, from a stream list.
 * @param list Target stream_list_t.
 * @param id Target stream id.
 * @return TRUE if succeeded, FALSE otherwise.
 */
int remove_stream(stream_list_t *list, uint32_t id);

// TODO set_stream_audio_data

/**
 * Get a pointer to the stream identified with an id from a stream list.
 * @param list Target stream_list_t.
 * @param id Target stream id.
 * @return stream_data_t * if the list had streams, NULL if the list was empty.
 */
stream_data_t *get_stream_id(stream_list_t *list, uint32_t id);

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
 * Removes a participant, idenified with its id, from a stream.
 * @param stream Target stream_data_t.
 * @param id Target participant id.
 * @return TRUE if succeeded, FALSE otherwise.
 */
int remove_participant_from_stream(stream_data_t *stream, uint32_t id);

//TODO: rethink these function names
/**
 * Get the pointer to the first participant with an id from a stream list.
 * @param list Target stream_list_t.
 * @param id Target participant id.
 * @return participant_data_t * to the participant identified with id if it was found, NULL otherwise.
 */
participant_data_t *get_participant_stream_id(stream_list_t *list, uint32_t id);

/**
 * Get the pointer to the first participant with an ssrc from a stream list.
 * @param list Target stream_list_t.
 * @param ssrc Target participant ssrc.
 * @return participant_data_t * to the participant identified with ssrc if it was found, NULL otherwise.
 */
participant_data_t *get_participant_stream_ssrc(stream_list_t *list, uint32_t ssrc);

/**
 * Get the first uninitialized participant from a stream list.
 * @param list Target stream_list_t.
 * @return participant_data_t * to the participant, NULL otherwise.
 */
participant_data_t *get_participant_stream_non_init(stream_list_t *list);

#endif //__STREAM_H__

