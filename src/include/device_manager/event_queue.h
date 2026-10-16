#pragma once
#include "event.h"
#include <pthread.h>
#include <stdbool.h>
#define EVENT_QUEUE_CAPACITY 200

/*result of event queue*/
typedef enum {
    EVENT_QUEUE_OK = 0,
    /*queue state error*/
    EVENT_QUEUE_CLOSED, 

    /*parameter*/
    EVENT_QUEUE_INVALID_PARAM,

    /*mutex*/
    EVENT_QUEUE_LOCK_MUTEX_FAILED,
    EVENT_QUEUE_UNLOCK_MUTEX_FAILED,
    EVENT_QUEUE_MUTEX_DESTROY_FAILED,
    EVENT_QUEUE_MUTEX_INIT_FAILED,

    /*Conditional variable*/
    EVENT_QUEUE_COND_WAIT_FAILED,
    EVENT_QUEUE_SIGNAL_FAILED,
    EVENT_QUEUE_COND_BROADCAST_FAILED,
    EVENT_QUEUE_COND_INIT_FAILED,
    EVENT_QUEUE_COND_DESTROY_FAILED,

    /*other errors*/
    EVENT_QUEUE_ERROR = -1
} event_queue_result_t;

/*event queue */
typedef struct {
    event_t buffer[EVENT_QUEUE_CAPACITY];

    size_t head;
    size_t tail;
    size_t count;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_cond_t not_empty_cond;
    pthread_cond_t not_full_cond;
    bool closed;

} event_queue_t;

event_queue_result_t event_queue_init(event_queue_t *queue);
event_queue_result_t event_queue_deinit(event_queue_t *queue);
event_queue_result_t event_queue_push(event_queue_t *queue, const event_t *event);
event_queue_result_t event_queue_pop(event_queue_t *queue, event_t *event);
event_queue_result_t event_queue_close(event_queue_t *queue);
