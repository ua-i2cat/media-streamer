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
  
#include "debug.h"
#include "circular_queue.h"

circular_queue_t *cq_init(int max, void *(*init_object)(), void (*destroy_object)(void *)) {

    if (max <= 1){
        error_msg("video frame queue must have at least 2 positions");
        return NULL;
    }

    if (circular_queue_t* cq = malloc(sizeof(circular_queue_t)) == NULL) {
        error_msg("cq_init malloc out of memory!");
        return NULL;
    }
    cq->rear = 0;
    cq->front = 0;
    cq->max = max;
    cq->level = CQ_EMPTY;
    cq->init_object = init_object;
    cq->destroy_object = destroy_object;
    for (int i = 0; i < max; i++) {
        cq->bags[i] = cq->init_object();
    }

    return cq;
}

void cq_destroy(circular_queue_t* cq) {

    for(int i = 0; i < cq->max; i++) {
        cq->destroy_object(cq->bags[i]);
    }
    free(cq);
}

void *cq_get_rear(circular_queue_t *cq) {

    if (cq->level == CQ_FULL) {
        return NULL;
    }
    return cq->bags[cq->rear];
}

void cq_add_bag(circular_queue_t *cq) {

    int r =  (cq->rear + 1) % cq->max;
    if (r == cq->front) {
        cq->level = CQ_FULL;
    } else {
        cq->level = CQ_OK;
    }
    cq->rear = r;
}

void* cq_get_front(circular_queue_t *cq) {

    if (cq->level == CQ_EMPTY) {
        return NULL;
    }

    return cq->bags[cq->front];
}

void cq_remove_bag(circular_queue_t *cq) {

    int f = (cq->front + 1) % cq->max;
    if (f == cq->rear) {
        cq->level = CQ_EMPTY;
    } else {
        cq->level = CQ_OK;
    }
    cq->front = f;
}

void cq_flush(circular_queue_t *cq) {

    if (cq->level == CQ_FULL){
        int r = (cq->rear + 1) % cq->max;
        if (r != cq->rear){
            cq->level = CQ_OK;
            cq->rear = r;
        }
    }
}

