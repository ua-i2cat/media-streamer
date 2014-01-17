/*
 *  participants.h - Participants and its RTP sessions.
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
 * @brief Participants data structures and its RTP sessions.
 *
 */

#ifndef __PARTICIPANTS_H__
#define __PARTICIPANTS_H__

#include "rtpdec.h"
#include "commons.h"

typedef struct participant participant_t;

typedef struct stream stream_t;

typedef struct rtp_session {
    uint32_t port;
    char *addr;
    struct rtp *rtp;
    struct tx *tx_session;
} rtp_session_t;

typedef struct participant {
    pthread_mutex_t lock;
    uint32_t ssrc;
    unsigned int id;
    uint8_t active;
    participant_t *next;
    participant_t *previous;
    io_type_t type;
    rtp_session_t *rtp;
    stream_t *stream;
} participant_t;

/**
 * Initializes a participant and its RTP session.
 * @param id Internal identifier.
 * @param type Type of input/output.
 * @param addr IP destination of the RTP session.
 * @param port Port of the RTP session.
 * @return The participant instance.
 */
participant_t *init_participant(unsigned int id, io_type_t type, char *addr, uint32_t port);

/**
 * Destroys a participant and its RTP session.
 * @param participant Participant instance.
 */
void destroy_participant(participant_t *participant);

/**
 * Setter for the SSRC field.
 * @param participant Participant instance.
 * @param ssrc RTP flux identifier.
 */
void set_participant_ssrc(participant_t *participant, uint32_t ssrc);

#endif //__PARTICIPANTS_H__

