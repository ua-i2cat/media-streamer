/*
 *  stream_list.h - Stream list definitions.
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

#ifndef __STREAM_LIST_H__
#define __STREAM_LIST_H__

#include "stream.h"

typedef struct stream_list {
    pthread_rwlock_t lock;
    int count;
    stream_t *first;
    stream_t *last;
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
 * Attaches a stream to the first free position of a stream list.
 * @param list Target stream_list_t.
 * @param stream Target stream_t.
 * @return true if succeeded, false otherwise.
 */
void add_stream(stream_list_t *list, stream_t *stream);

/**
 * Removes a stream, identified with its id, from a stream list.
 * @param list Target stream_list_t.
 * @param id Target stream id.
 * @return true if succeeded, false otherwise.
 */
bool remove_stream(stream_list_t *list, unsigned int id);

/**
 * Get a pointer to the stream identified with an id from a stream list.
 * @param list Target stream_list_t.
 * @param id Target stream id.
 * @return stream_t * if the list had streams, NULL if the list was empty.
 */
stream_t *get_stream_id(stream_list_t *list, unsigned int id);

//TODO: rethink these function names
/**
 * Get the pointer to the first participant with an id from a stream list.
 * @param list Target stream_list_t.
 * @param id Target participant id.
 * @return participant_data_t * to the participant identified with id if it was found, NULL otherwise.
 */
participant_data_t *get_participant_stream_id(stream_list_t *list, unsigned int id);

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

#endif //__STREAM_LIST_H__

