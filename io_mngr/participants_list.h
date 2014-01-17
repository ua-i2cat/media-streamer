/*
 *  participants_list.h - Participant list.
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
 * @file participants.h
 * @brief Participants list data structures and functions.
 *
 */

#ifndef __PARTICIPANTS_LIST_H__
#define __PARTICIPANTS_LIST_H__

#include "participants.h"

typedef struct participant_list {
    pthread_rwlock_t lock;
    int count;
    participant_t *first;
    participant_t *last;
} participant_list_t;

/**
 * Initializes a Participant list.
 * @return The participant list instance.
 */
participant_list_t *init_participant_list(void);

/**
 * Destroys a Participant list.
 * @param list Participant list instance.
 */
void destroy_participant_list(participant_list_t *list);

/**
 * Adds a participant to a participant list.
 * @param list Participant list instance.
 * @param participant Participant instance.
 */
void add_participant(participant_list_t *list, participant_t *participant);

/**
 * Removes a participant from a participant list using its id.
 * @param list Participant list instance.
 * @param id Participant internal identification.
 * @return True if the participant was removed, false otherwise.
 */
bool remove_participant(participant_list_t *list, unsigned int id);

/**
 * Get a participant instance searching by its id.
 * @param list Participant list instance.
 * @param id Participant internal identification.
 * @return The participant instance.
 */
participant_t *get_participant_id(participant_list_t *list, unsigned int id);

/**
 * Get a participant instance searching by its ssrc.
 * @param list Participant list instance.
 * @param ssrc RTP flux identifier.
 * @return The participant instance.
 */
participant_t *get_participant_ssrc(participant_list_t *list, uint32_t ssrc);

/**
 * Get the first unitialized participant instance whose ssrc field is 0.
 * @param list Participant list instance.
 * @return The participant instance if it was found, NULL otherwise.
 */
participant_t *get_participant_non_init(participant_list_t *list);

#endif //__PARTICIPANTS_LIST_H__

