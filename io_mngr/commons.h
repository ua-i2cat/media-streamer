/*
 *  commons.h
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
 *            David Cassany <david.cassany@i2cat.net>
 */

/**
 * @file circular_queue.h
 * @brief Common definitions for io_mngr entities.
 */

#ifndef __COMMONS_H__
#define __COMMONS_H__

#define DEFAULT_RTCP_BW 5 * 1024 * 1024 * 10
#define DEFAULT_TTL 255
#define DEFAULT_SEND_BUFFER_SIZE 1920 * 1080 * 4 * sizeof(char) * 10
#define MTU 1300 // 1400

typedef enum role {
    ENCODER,
    DECODER,
    NONE
} role_t;

typedef enum io_type {
    INPUT,
    OUTPUT
} io_type_t;

#endif //__COMMONS_H__

