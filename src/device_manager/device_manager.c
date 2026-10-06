#include "device_manager/device_manager.h"
#include "logger/logger.h"
/**
 * PURPOSE: GET AND PROCESS EVENT
 */
/*
    Declaration forward
*/
// static void process_event(device_manager_t *device_mngr,const event_t *event);
static void process_event(const event_t *event);

/** 
 * \brief Function worker of device manager module
 */
void* worker_fnc(void* arg){
    LOG_INFO("[DEVICE_MNGR] Worker start");
    device_manager_t *device_mngr = (device_manager_t *)arg;
    event_queue_result_t result = EVENT_QUEUE_OK;
    event_t event;
    /* this thread will out when all events are popped so that no need to check state*/
    while(1){
        /* get event */
        result = event_queue_pop(&device_mngr->event_queue,&event);

        if(result == EVENT_QUEUE_CLOSED) break;   

        if(result != EVENT_QUEUE_OK) break;
        
        /* process event */
        //process_event(device_mngr,&event);
        process_event(&event);
    }
    LOG_INFO("[DEVICE_MNGR] Worker exit");
    return NULL;
}

/**
 * \brief Init device manager module
 */
device_manager_result_t device_manager_init(device_manager_t *device_mngr){
    device_manager_result_t result = DEVICE_MANAGER_ERROR;
    event_queue_result_t result_event_queue = EVENT_QUEUE_ERROR;
    /* check null param */
    if(device_mngr == NULL){
        LOG_ERROR("[DEVICE_MNGR] Invalid parameter");
        return DEVICE_MANAGER_INVALID_PARAM;
    }
    /* check state of device manager */
    if(device_mngr->state != DEVICE_MANAGER_STATE_NOT_INIT){
        LOG_ERROR("[DEVICE_MNGR] Invalid state");
        return DEVICE_MANAGER_INVALID_STATE;
    }
    /* init event queue */
    result_event_queue = event_queue_init(&device_mngr->event_queue);
    if(result_event_queue == EVENT_QUEUE_OK){
        result = DEVICE_MANAGER_OK;
        /* set state into INIT */
        device_mngr->state = DEVICE_MANAGER_STATE_INIT;
    }
    else{
        LOG_ERROR("[DEVICE_MNGR] Error init device manager");
        return DEVICE_MANAGER_ERROR;
    }

    return result;
}

/**
 * \brief Start thread worker to get event from producer
 */
device_manager_result_t device_manager_start(device_manager_t *device_mngr){
    device_manager_result_t result = DEVICE_MANAGER_ERROR;
    /* check null param */
    if(device_mngr == NULL){
        LOG_ERROR("[DEVICE_MNGR] Invalid parameter");
        return DEVICE_MANAGER_INVALID_PARAM;
    }
    /* check state of device manager */
    if(device_mngr->state != DEVICE_MANAGER_STATE_INIT){
        LOG_ERROR("[DEVICE_MNGR] Invalid state");
        return DEVICE_MANAGER_INVALID_STATE;
    }
    /* create thread */
    result = pthread_create(&device_mngr->worker,NULL,worker_fnc,device_mngr);

    if(result == DEVICE_MANAGER_OK){
        /*change state into RUNNING */
        device_mngr->state = DEVICE_MANAGER_STATE_RUNNING;
        device_mngr->running = true;
    }
    else    return DEVICE_MANAGER_ERROR;
    return result;
}

/**
 * \brief Stop device manager module 
 */
device_manager_result_t device_manager_stop(device_manager_t *device_mngr){
    device_manager_result_t result = DEVICE_MANAGER_ERROR;
    event_queue_result_t result_event_queue = EVENT_QUEUE_ERROR;
    /* check null param */
    if(device_mngr == NULL){
        LOG_ERROR("[DEVICE_MNGR] Invalid parameter");
        return DEVICE_MANAGER_INVALID_PARAM;
    }
    /* check state of device manager */
    if(device_mngr->state != DEVICE_MANAGER_STATE_RUNNING){
        LOG_ERROR("[DEVICE_MNGR] Invalid state");
        return DEVICE_MANAGER_INVALID_STATE;
    }
    
    result_event_queue = event_queue_close(&device_mngr->event_queue);
    if(result_event_queue != EVENT_QUEUE_OK){
        return DEVICE_MANAGER_ERROR;
    } 

    result = pthread_join(device_mngr->worker,NULL);

    if(result == DEVICE_MANAGER_OK && result_event_queue == EVENT_QUEUE_OK){
        device_mngr->state = DEVICE_MANAGER_STATE_STOPPED;
    }
    return result;
}

/**
 * \brief Deinit device manager module
 */
device_manager_result_t device_manager_deinit(device_manager_t *device_mngr){
    device_manager_result_t result = DEVICE_MANAGER_ERROR;
    event_queue_result_t result_event_queue = EVENT_QUEUE_ERROR;

    /* check null param */
    if(device_mngr == NULL){
        LOG_ERROR("[DEVICE_MNGR] Invalid parameter");
        return DEVICE_MANAGER_INVALID_PARAM;
    }
    /* state must be stopped */
    if(device_mngr->state != DEVICE_MANAGER_STATE_STOPPED)
    {
        LOG_ERROR("[DEVICE_MNGR] Invalid state");
        return DEVICE_MANAGER_INVALID_STATE;
    }
    result_event_queue = event_queue_deinit(&device_mngr->event_queue);
    if(result_event_queue == EVENT_QUEUE_OK){
        result = DEVICE_MANAGER_OK;
        /* after deiniting completely, change into not init to start new lifecycle*/
        device_mngr->state = DEVICE_MANAGER_STATE_NOT_INIT;
    }
    else{
        LOG_ERROR("Event queue deinit failed");
        return DEVICE_MANAGER_ERROR;
    }
   
    return result;
}

static void process_event(
    //device_manager_t *device_mngr,
    const event_t *event)
{
    /* Avoid unsed variables*/
    // device_manager_t *local_device_mngr = device_mngr;
    switch(event->type)
    {
        case EVENT_SENSOR_DATA:
            LOG_INFO("[DEVICE_MNGR] Sensor data");
            break;

        case EVENT_SENSOR_ERROR:
            LOG_INFO("[DEVICE_MNGR] Sensor error");
            break;

        case EVENT_SYSTEM_STOP:
            LOG_INFO("[DEVICE_MNGR] Stop request");
            break;

        default:
            LOG_ERROR("[DEVICE_MNGR] Unknown event");
            break;
    }
}