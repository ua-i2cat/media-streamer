/*
 *  participants.h - Participants and sessions definitions.
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
 * @file participants.h
 * @brief Participants data structures and functions.
 *
 */

#ifndef __PARTICIPANTS_H__
#define __PARTICIPANTS_H__

//#include <semaphore.h>
#include "rtpdec.h"
#include "video.h"
#include "module.h"
#include "commons.h"
#include "debug.h"

typedef struct participant_data participant_data_t;

typedef struct stream stream_t;

typedef struct rtp_session {
    uint32_t port;
    char *addr;
    struct rtp *rtp;
    struct tx *tx_session;
} rtp_session_t;

typedef struct participant_list {
    pthread_rwlock_t lock;
    int count;
    participant_data_t *first;
    participant_data_t *last;
} participant_list_t;

struct participant_data {
    pthread_mutex_t lock;
    uint32_t ssrc;
    unsigned int id;
    uint8_t active;
    participant_data_t *next;
    participant_data_t *previous;
    io_type_t type;
    rtp_session_t *rtp;
    stream_t *stream;
};

/**
 * Initializes a Participants list.
 * @param get_media_time_ptr Callback to get the media_time field address.
 * @return participant_list_t *.
 */
participant_list_t *init_participant_list(void);

/**
 * Initializes a Participants list.
 * @param get_media_time_ptr Callback to get the media_time field address.
 * @return participant_list_t *.
 */
void destroy_participant_list(participant_list_t *list);

/**
 * Initializes a Participants list.
 * @param get_media_time_ptr Callback to get the media_time field address.
 * @return participant_list_t *.
 */
void add_participant(participant_list_t *list, participant_data_t *participant);

/**
 * Initializes a Participants list.
 * @param get_media_time_ptr Callback to get the media_time field address.
 * @return participant_list_t *.
 */
bool remove_participant(participant_list_t *list, unsigned int id);

/**
 * Initializes a Participants list.
 * @param get_media_time_ptr Callback to get the media_time field address.
 * @return participant_list_t *.
 */
participant_data_t *get_participant_id(participant_list_t *list, unsigned int id);

/**
 * Initializes a Participants list.
 * @param get_media_time_ptr Callback to get the media_time field address.
 * @return participant_list_t *.
 */
participant_data_t *get_participant_ssrc(participant_list_t *list, uint32_t ssrc);

/**
 * Initializes a Participants list.
 * @param get_media_time_ptr Callback to get the media_time field address.
 * @return participant_list_t *.
 */
participant_data_t *get_participant_non_init(participant_list_t *list);

/**
 * Initializes a Participants list.
 * @param get_media_time_ptr Callback to get the media_time field address.
 * @return participant_list_t *.
 */
int set_participant_ssrc(participant_data_t *participant, uint32_t ssrc);

/**
 * Initializes a Participants list.
 * @param get_media_time_ptr Callback to get the media_time field address.
 * @return participant_list_t *.
 */
participant_data_t *init_participant(unsigned int id, io_type_t type, char *addr, uint32_t port);

#endif //__PARTICIPANTS_H__

