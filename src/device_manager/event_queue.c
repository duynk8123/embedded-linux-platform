#include "device_manager/event_queue.h"
#include "logger/logger.h"
/**
 * \brief           Init queue event
 * \param[in]       queue   :queue event
 * \retval          EVENT_QUEUE_OK
 * \retval          EVENT_QUEUE_MUTEX_INIT_FAILED
 * \retval          EVENT_QUEUE_COND_INIT_FAILED     
 */
event_queue_result_t event_queue_init(event_queue_t *queue){
    /*local variables*/
    int result      = EVENT_QUEUE_ERROR;
    queue->head    = 0;
    queue->tail    = 0;
    queue->count   = 0;
    queue->closed  = false;

    /*Create resources*/
    result = pthread_mutex_init(&queue->mutex,NULL);

    if(result != EVENT_QUEUE_OK){
        LOG_ERROR("[EVENT_QUEUE] Cannot create mutex for event queue");
        queue->closed = true;
        return EVENT_QUEUE_MUTEX_INIT_FAILED;
    }
    result = pthread_cond_init(&queue->cond, NULL);

    if(result != EVENT_QUEUE_OK){
        LOG_ERROR("[EVENT_QUEUE] Cannot create condition variable for event queue");
        pthread_mutex_destroy(&queue->mutex);
        queue->closed = true;
        return EVENT_QUEUE_COND_INIT_FAILED;
    }
    return EVENT_QUEUE_OK;
}
/**
 * \brief           Push an event to queue
 */
event_queue_result_t event_queue_push(event_queue_t *queue, const event_t *event){
    /*local variables*/
    int result      = EVENT_QUEUE_ERROR;
    /*check null input param*/
    if(queue == NULL || event == NULL){
        LOG_ERROR("[EVENT_QUEUE] NULL PARAM");
        return EVENT_QUEUE_INVALID_PARAM;
    }
    result = pthread_mutex_lock(&queue->mutex);
    if(result != EVENT_QUEUE_OK){
        LOG_ERROR("[EVENT_QUEUE] Can not lock semaphore");
        return EVENT_QUEUE_LOCK_MUTEX_FAILED;
    }
    /*check queue closed?*/
    if(queue->closed == true){
        result = EVENT_QUEUE_CLOSED;
        LOG_ERROR("[EVENT_QUEUE] Queue event is closed");
    }
    else{
        /*check queue full*/
        if(queue->count == EVENT_QUEUE_CAPACITY){
            result = EVENT_QUEUE_FULL;
            LOG_INFO("[EVENT_QUEUE] Queue FULL");
        }
        else{
            queue->buffer[queue->tail] = *event;
            queue->tail = (queue->tail + 1) % EVENT_QUEUE_CAPACITY;
            queue->count++;
            /*signal consumer get event*/
            /*Note: push event Ok -> signal consumer fail -> should not retry*/
            result = pthread_cond_signal(&queue->cond);
            if(result != EVENT_QUEUE_OK){
                LOG_ERROR("[EVENT_QUEUE] Signal to consumer fail");
                result = EVENT_QUEUE_SIGNAL_FAILED;
            }
        }
    }
    if(pthread_mutex_unlock(&queue->mutex) != 0){
        LOG_ERROR("[EVENT_QUEUE] Unlock mutex failed");
        return EVENT_QUEUE_UNLOCK_MUTEX_FAILED;
    }
    return result;
}
/**
 * \brief           Pop an event to queue
 */
event_queue_result_t event_queue_pop(event_queue_t *queue, event_t *event){
    /*local variables*/
    int result      = EVENT_QUEUE_ERROR;
    /*check null input param*/
    if(queue == NULL || event == NULL){
        LOG_ERROR("[EVENT_QUEUE] NULL PARAM");
        return EVENT_QUEUE_INVALID_PARAM;
    }
    result = pthread_mutex_lock(&queue->mutex);
    if(result != EVENT_QUEUE_OK){
        LOG_ERROR("[EVENT_QUEUE] Lock mutex failed");
        return EVENT_QUEUE_LOCK_MUTEX_FAILED;
    }
    /*check queue open and empty*/
    while(queue->count == 0 && queue->closed == false){
       result = pthread_cond_wait(&queue->cond,&queue->mutex);
       if(result != EVENT_QUEUE_OK){
            result = EVENT_QUEUE_COND_WAIT_FAILED;
            pthread_mutex_unlock(&queue->mutex);
            return result;
        }
    }
    if(queue->count == 0 && queue->closed == true){
       result = EVENT_QUEUE_CLOSED;
       LOG_INFO("[EVENT_QUEUE] Queue close");
    }
    else{
        *event = queue->buffer[queue->head];
        queue->head = (queue->head + 1) % EVENT_QUEUE_CAPACITY;
        queue->count--;
        result = EVENT_QUEUE_OK;
    }
    if(pthread_mutex_unlock(&queue->mutex) != EVENT_QUEUE_OK){
        LOG_ERROR("[EVENT_QUEUE] Unlock mutex failed");
        return EVENT_QUEUE_UNLOCK_MUTEX_FAILED;
    }
    return result;
}
/**
 * \brief           Close event queue
 */
event_queue_result_t event_queue_close(event_queue_t *queue){
    /*local variables*/
    int result      = EVENT_QUEUE_OK;
    if(queue == NULL){
        LOG_ERROR("[EVENT_QUEUE] Error param");
        return EVENT_QUEUE_INVALID_PARAM;
    }
    if(queue->closed){
        LOG_ERROR("[EVENT_QUEUE] Queue already closed");
        return EVENT_QUEUE_CLOSED;
    }
    result = pthread_mutex_lock(&queue->mutex);
    if(result != EVENT_QUEUE_OK){
        LOG_ERROR("[EVENT_QUEUE] Lock mutex failed");
        return EVENT_QUEUE_LOCK_MUTEX_FAILED;
    }
    /*close queue*/
    queue->closed = true;
    /*notify all thread wake up*/
    result = pthread_cond_broadcast(&queue->cond);
    if(result != EVENT_QUEUE_OK){
        LOG_ERROR("[EVENT_QUEUE] Notify all thread failed");
        pthread_mutex_unlock(&queue->mutex);
        return EVENT_QUEUE_COND_BROADCAST_FAILED;
    }
    if(pthread_mutex_unlock(&queue->mutex) != EVENT_QUEUE_OK){
        LOG_ERROR("[EVENT_QUEUE] Unlock mutex failed");
        return EVENT_QUEUE_UNLOCK_MUTEX_FAILED;
    }

    return result;
}
/**
 * \brief          Deinit queue
 */
event_queue_result_t event_queue_deinit(event_queue_t *queue)
{
    if(queue == NULL){
        LOG_ERROR("[EVENT_QUEUE] Error param");
        return EVENT_QUEUE_INVALID_PARAM;
    }

    if(pthread_mutex_destroy(&queue->mutex) != 0){
        LOG_ERROR("[EVENT_QUEUE] Destroy mutex failed");
        return EVENT_QUEUE_MUTEX_DESTROY_FAILED;
    }

    if(pthread_cond_destroy(&queue->cond) != 0){
        LOG_ERROR("[EVENT_QUEUE] Destroy conditional variable failed");
        return EVENT_QUEUE_COND_DESTROY_FAILED;
    }

    return EVENT_QUEUE_OK;
}