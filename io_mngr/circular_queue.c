/*
 *  circular_queue.c
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

//#include <stdlib.h>
//#include <unistd.h>
#include "debug.h"
#include "circular_queue.h"

circular_queue_t *cq_init(int max, void *(*init_object)(void *), void *init_data, void (*destroy_object)(void *), unsigned int *(*get_media_time_ptr)(void *))
{
    if (max <= 1){
        error_msg("video frame queue must have at least 2 positions");
        return NULL;
    }

    circular_queue_t* cq;
    if ((cq = malloc(sizeof(circular_queue_t))) == NULL) {
        error_msg("cq_init malloc out of memory!");
        return NULL;
    }
    cq->rear = 0;
    cq->front = 0;
    cq->max = max;
    cq->level = CIRCULAR_QUEUE_EMPTY;
#ifdef STATS
    cq->delay_sum = 0;
    cq->remove_counter = 0;
    cq->delay = 0;
    cq->fps = 0.0;
    cq->put_counter = 0;
    cq->fps_sum = 0;
    cq->last_frame_time = 0;
#endif
    cq->init_object = init_object;
    cq->destroy_object = destroy_object;
    cq->get_media_time_ptr = get_media_time_ptr;
    cq->bags = malloc(sizeof(bag_t *) * max);
    for (int i = 0; i < max; i++) {
        cq->bags[i] = malloc(sizeof(bag_t *));
        cq->bags[i]->pocket = cq->init_object(init_data);
        cq->bags[i]->media_time = cq->get_media_time_ptr(cq->bags[i]->pocket);
    }

    return cq;
}

void cq_destroy(circular_queue_t* cq)
{
    for(int i = 0; i < cq->max; i++) {
        cq->destroy_object(cq->bags[i]->pocket);
        free(cq->bags[i]);
    }
    free(cq->bags);
    free(cq);
}

void *cq_get_rear(circular_queue_t *cq)
{
    if (cq->level == CIRCULAR_QUEUE_FULL) {
        return NULL;
    }
    return cq->bags[cq->rear]->pocket;
}

void cq_add_bag(circular_queue_t *cq)
{
    int r =  (cq->rear + 1) % cq->max;
    if (r == cq->front) {
        cq->level = CIRCULAR_QUEUE_FULL;
    } else {
        cq->level = CIRCULAR_QUEUE_MID;
    }
    cq->rear = r;

#ifdef STATS
    frame_cq->stats->put_counter++;
    local_time = get_local_mediatime_us();
    frame_cq->stats->fps_sum += local_time - frame_cq->stats->last_frame_time;
    frame_cq->stats->last_frame_time = local_time;

    if (frame_cq->stats->put_counter == MAX_COUNTER) {
        frame_cq->stats->fps = (frame_cq->stats->put_counter * 1000000) / frame_cq->stats->fps_sum;
        frame_cq->stats->put_counter = 0;
        frame_cq->stats->fps_sum = 0;
    }
#endif
}

void* cq_get_front(circular_queue_t *cq)
{
    if (cq->level == CIRCULAR_QUEUE_EMPTY) {
        return NULL;
    }

    return cq->bags[cq->front]->pocket;
}

void cq_remove_bag(circular_queue_t *cq)
{
#ifdef STATS
    frame_cq->stats->delay_sum += get_local_mediatime_us() - cq->bags[cq->front]->media_time;
    frame_cq->stats->remove_counter++;
    if (frame_cq->stats->remove_counter == MAX_COUNTER) {
        frame_cq->stats->delay = frame_cq->stats->delay_sum/frame_cq->stats->remove_counter;
        frame_cq->stats->delay_sum = 0;
        frame_cq->stats->remove_counter = 0;
    }
#endif

    int f = (cq->front + 1) % cq->max;
    if (f == cq->rear) {
        cq->level = CIRCULAR_QUEUE_EMPTY;
    } else {
        cq->level = CIRCULAR_QUEUE_MID;
    }
    cq->front = f;
}

void cq_flush(circular_queue_t *cq)
{
    if (cq->level == CIRCULAR_QUEUE_FULL) {
        cq->front = (cq->front + (cq->max - 1))
            % cq->max;
        cq->level = CIRCULAR_QUEUE_MID;
    }
}

void cq_reconfig(circular_queue_t *cq, void *init_data)
{
    for (int i = 0; i < cq->max; i++) {
        cq->destroy_object(cq->bags[i]->pocket);
        cq->bags[i]->pocket = cq->init_object(init_data);
        cq->bags[i]->media_time = cq->get_media_time_ptr(cq->bags[i]->pocket);
    }
}

