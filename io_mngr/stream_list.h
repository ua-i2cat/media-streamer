/*
 *  stream_list.h - Stream list.
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
 * @file stream_list.h
 * @brief Linked list of streams.
 *
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
 * @return The stream list instance if succeeded, NULL otherwise.
 */
stream_list_t *init_stream_list(void);

/**
 * Destroys a stream list.
 * @param list Stream list instance.
 */
void destroy_stream_list(stream_list_t *list);

/**
 * Adds a stream to the end of the list.
 * @param list Stream list instance.
 * @param stream Stream instance.
 */
void add_stream(stream_list_t *list, stream_t *stream);

/**
 * Removes a stream, identified with its internal id, from the list.
 * @param list Stream list instance.
 * @param id Stream internal id.
 * @return True if succeeded, false otherwise.
 */
bool remove_stream(stream_list_t *list, unsigned int id);

/**
 * Gets the instance of a stream identified with a given id.
 * @param list Stream list instance.
 * @param id Stream internal id.
 * @return The stream instance if it was found, NULL otherwise.
 */
stream_t *get_stream_id(stream_list_t *list, unsigned int id);

/**
 * Gets the instance of a participant, identified with the given id, that is linked by a stream from the list.
 * @param list Stream list instance.
 * @param id Participant internal id.
 * @return The participant instance if it was found, NULL otherwise.
 */
participant_t *get_participant_stream_id(stream_list_t *list, unsigned int id);

/**
 * Gets the instance of a participant, identified with the given ssrc, that is linked by a stream from the list.
 * @param list Stream list instance.
 * @param ssrc Participant ssrc.
 * @return The participant instance if it was found, NULL otherwise.
 */
participant_t *get_participant_stream_ssrc(stream_list_t *list, uint32_t ssrc);

/**
 * Gets the instance of an unitialized (ssrc equal to 0) participant that is linked bya stream from the list.
 * @param list Stream list instance.
 * @return The participant instance if it was found, NULL otherwise.
 */
participant_t *get_participant_stream_non_init(stream_list_t *list);

#endif //__STREAM_LIST_H__

