#pragma once
#include <stdio.h>
#include <stdint.h>


typedef enum {
    EVENT_SYSTEM_START,
    EVENT_SYSTEM_STOP, 
    EVENT_SENSOR_DATA,
    EVENT_SENSOR_ERROR
} event_type_t;

/*payload*/
typedef struct {
    int sensor_id;
    int value;
} sensor_data_t;

/*error code*/
typedef struct {
    int sensor_id;
    int error_code;
} sensor_error_t;

/*event*/
typedef struct {
    event_type_t type;
    uint64_t timestamp;

    union {
        sensor_data_t sensor_data;
        sensor_error_t sensor_error;
    } payload;

} event_t;
