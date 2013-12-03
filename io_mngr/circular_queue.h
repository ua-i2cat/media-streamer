/*
 *  circular_queue.h
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
 * @brief Thread resistant circular queue composed of 'bags', there you can store generic objects.
 */

#ifndef __CIRCULAR_QUEUE_H__
#define __CIRCULAR_QUEUE_H__

typedef enum {
    CIRCULAR_QUEUE_MID,
    CIRCULAR_QUEUE_EMPTY,
    CIRCULAR_QUEUE_FULL
} cq_level_t;

typedef struct circular_queue {
    int rear;
    int front;
    int max;
    cq_level_t level;
    void *(*init_object)(void *);
    void (*destroy_object)(void *);
    void **bags;
} circular_queue_t;

/**
 * Initializes a 'bag' circular queue.
 * @param max Maximum number of 'bags'.
 * @param init_object Callback to initialize 'bag' objects.
 * @param destroy_object Callback to destroy 'bag' objects.
 * @param init_data Initialization data to be passed to the init_object function.
 * @return circular_queue_t * if succeeded, NULL otherwise.
 */
circular_queue_t *cq_init(int max, void *(*init_object)(void *), void (*destroy_object)(void *), void *init_data);

/**
 * Destroys the circular queue.
 * @param max Target circular_queue_t.
 */
void cq_destroy(circular_queue_t* cq);

/**
 * Returns the pointer from the front 'bag', useful to get the element from the 'bag'.
 * @param max Target circular_queue_t.
 * @return A pointer to the front element or NULL if the queue is empty.
 */
void* cq_get_front(circular_queue_t *cq);

/**
 * Advances the front one position removing the latter front 'bag' from filled 'bags' set.
 * @param max Target circular_queue_t.
 */
void cq_remove_bag(circular_queue_t *cq);

/**
 * Returns the pointer pointing to the free 'bag' from rear, useful for fill the 'bag'.
 * @param max Target circular_queue_t.
 * @return A pointer to the free rear 'bag' or NULL if the queue is full.
 */
void *cq_get_rear(circular_queue_t *cq);

/**
 * Advances the rear one position adding the latter rear element to the filled 'bags' set.
 * @param max Target circular_queue_t.
 */
void cq_add_bag(circular_queue_t *cq);


/**
 * Mercilessly forgets the rear element only if the queue is full.
 * @param max Target circular_queue_t.
 */
void cq_flush(circular_queue_t *cq);

#endif //__CIRCULAR_QUEUE_H__

