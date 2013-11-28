/**
 * @file circular_queue.h
 * @brief Thread resistant circular queue.
 *
 */

#ifndef __CIRCULAR_QUEUE_H__
#define __CIRCULAR_QUEUE_H__

enum {
    CQ_OK,
    CQ_EMPTY,
    CQ_FULL
} cq_level;

typedef struct circular_queue {
    int rear;
    int front;
    int max;
    cq_level level;
    void *(*init_object)();
    void (*destroy_object)(void *);
    void **bags;
} circular_queue_t;

circular_queue_t *cq_init(int max, void *(*init_object)(), void (*destroy_object)(void *));
int cq_destroy(circular_queue_t* cq);
void *cq_get_rear(circular_queue_t *cq);
int cq_add_bag(circular_queue_t *cq);
void* cq_get_front(circular_queue_t *cq);
int cq_remove_bag(circular_queue_t *cq);
int cq_flush(circular_queue_t *cq);

#endif //__CIRCULAR_QUEUE_H__

