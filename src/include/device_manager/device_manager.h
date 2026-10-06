#pragma once
#include "event_queue.h"
/*
    Life cycle of device manager
*/
typedef enum{
    DEVICE_MANAGER_STATE_NOT_INIT,
    DEVICE_MANAGER_STATE_INIT,
    DEVICE_MANAGER_STATE_RUNNING,
    DEVICE_MANAGER_STATE_STOPPED
} device_manager_state_t;

typedef struct {
    device_manager_state_t state;
    event_queue_t event_queue;
    
    pthread_t worker;
    bool running;
} device_manager_t;

typedef enum{
    DEVICE_MANAGER_OK,
    DEVICE_MANAGER_INVALID_PARAM,
    DEVICE_MANAGER_INVALID_STATE,
    DEVICE_MANAGER_ERROR = -1
}device_manager_result_t;

device_manager_result_t device_manager_init(device_manager_t *device_mngr);
device_manager_result_t device_manager_start(device_manager_t *device_mngr);
device_manager_result_t device_manager_stop(device_manager_t *device_mngr);
device_manager_result_t device_manager_deinit(device_manager_t *device_mngr);
void* worker_fnc(void* arg);