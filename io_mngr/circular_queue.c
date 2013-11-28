/**
 * @file circular_queue.c
 * @brief Thread resistant circular queue.
 *
 */

#include "debug.h"
#include "circular_queue.h"

circular_queue_t *cq_init(int max, void *(*init_object)(), void (*destroy_object)(void *)) {

    if (max <= 1){
        error_msg("video frame queue must have at least 2 positions");
        return NULL;
    }

    circular_queue_t* cq = malloc(sizeof(circular_queue_t));
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

int cq_destroy(circular_queue_t* cq) {

    for(int i = 0; i < cq->max; i++) {
        cq->destroy_object(cq->bags[i]);
    }
    free(cq);

    return TRUE;
}

void *cq_get_rear(circular_queue_t *cq) {

    if (cq->level == CQ_FULL) {
        return NULL;
    }
    return cq->bags[cq->rear];
}

int cq_add_bag(circular_queue_t *cq) {

    int r;

    r =  (cq->rear + 1) % cq->max;
    if (r == cq->front) {
        cq->level = CQ_FULL;
    } else {
        cq->level = CQ_OK;
    }
    cq->rear = r;

    return TRUE;
}

void* cq_get_front(circular_queue_t *cq) {

    if (cq->level == CQ_EMPTY) {
        return NULL;
    }

    return cq->bags[cq->front];
}

int cq_remove_bag(circular_queue_t *cq) {

    int f;

    f =  (cq->front + 1) % cq->max;
    if (f == cq->rear) {
        cq->level = CQ_EMPTY;
    } else {
        cq->level = CQ_OK;
    }
    cq->front = f;

    return TRUE;
}

int cq_flush(circular_queue_t *cq) {

    int r;

    if (cq->level == CQ_FULL){
        r = (cq->rear + 1) % cq->max;
        if (r != cq->rear){
            cq->level = CQ_OK;
            cq->rear = r;
        }
    }

    return TRUE;
}

