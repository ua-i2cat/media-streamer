/*
 *  circular_queue.h - Thread safe circular queue.
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
 *            David Cassany <david.cassany@i2cat.net>
 *            Marc Palau <marc.palau@i2cat.net>
 */

/**
 * @file circular_queue.h
 * @brief Thread safe (one reader, one writer) circular queue composed of 'bags' where generic objects can be stored.
 */

#ifndef __CIRCULAR_QUEUE_H__
#define __CIRCULAR_QUEUE_H__

#include "commons.h"

typedef enum {
    CIRCULAR_QUEUE_MID,
    CIRCULAR_QUEUE_EMPTY,
    CIRCULAR_QUEUE_FULL
} cq_level_t;

typedef struct bag {
    void *pocket;
    unsigned int *media_time;
} bag_t;

#ifdef STATS
typedef struct circular_queue_stats {
    unsigned int delay_sum;
    unsigned int remove_counter;
    float delay;
    float fps;
    unsigned int put_counter;
    unsigned int fps_sum;
    unsigned int last_frame_time;
} circular_queue_stats_t;
#endif

typedef struct circular_queue {
    int rear;
    int front;
    int max;
    cq_level_t level;
#ifdef STATS
    circular_queue_stats_t stats;
#endif
    void *(*init_object)(void *);
    void (*destroy_object)(void *);
    unsigned int *(*get_media_time_ptr)(void *);
    bag_t **bags;
} circular_queue_t;

/**
 * Initializes a circular queue.
 * @param max Maximum number of bags.
 * @param init_object Callback to initialize an empty object inside a bag.
 * @param init_data Initialization data to be passed to the init_object function.
 * @param destroy_object Callback to destroy an object from a bag.
 * @param get_media_time_ptr Callback to get the media_time field address.
 * @return Circular queue instance if succeeded, NULL otherwise.
 */
circular_queue_t *cq_init(
        int max,
        void *(*init_object)(void *),
        void *init_data,
        void (*destroy_object)(void *),
        unsigned int *(*get_media_time_ptr)(void *));

/**
 * Destroys the circular queue.
 * @param cq Circular queue instance.
 */
void cq_destroy(circular_queue_t* cq);

/**
 * Returns the pointer to the front bag, used to get (read) an object from a bag.
 * @param cq Circular queue instance.
 * @return A pointer to the front element or NULL if the queue is empty.
 */
void* cq_get_front(circular_queue_t *cq);

/**
 * Advances the front one position, used to remove (forget) the current front bag.
 * @param cq Circular queue instance.
 */
void cq_remove_bag(circular_queue_t *cq);

/**
 * Returns the pointer to the rear bag, used to add (fill) an object into a bag.
 * @param cq Circular queue instance.
 * @return A pointer to the rear bag or NULL if the queue is full.
 */
void *cq_get_rear(circular_queue_t *cq);

/**
 * Advances the rear one position, used to add a new rear bag.
 * @param cq Circular queue instance.
 */
void cq_add_bag(circular_queue_t *cq);

/**
 * Mercilessly removes (forgets) the rear element if the queue is full.
 * @param cq Circular queue instance.
 */
void cq_flush(circular_queue_t *cq);

/**
 * Destroys and reconfigures each bag of the queue.
 * @param cq Circular queue instance.
 * @param init_data Outter initialization data structure.
 */
void cq_reconfig(circular_queue_t *cq, void *init_data);

#endif //__CIRCULAR_QUEUE_H__

