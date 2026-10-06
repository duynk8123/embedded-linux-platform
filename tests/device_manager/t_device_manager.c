#include "device_manager/device_manager.h"
#include <unistd.h>
#include "logger/logger.h"
/*
    Overview test case:
        INIT -> START -> STOPPED -> DEINIT
        Check state when start (Init -> start)
        Check state when stopped (Running -> Stopping -> Stopped)
        Test push 1 event
        Test push 3 different events
        Test worker drain queue after stop
        Invalid param
*/

void test_init_deinit(device_manager_t *device_mngr);
void test_start(device_manager_t *device_mngr);
void test_stop(device_manager_t *device_mngr);
void test_sensor_data_event(device_manager_t *device_mngr);
void test_multi_event(device_manager_t *device_mngr);
void test_drain_before_exit(device_manager_t *device_mngr);
/* test null param for all function */
void test_null_param(device_manager_t *device_mngr);
void test_null_param_init(device_manager_t *device_mngr);
void test_null_param_start(device_manager_t *device_mngr);
void test_null_param_stop(device_manager_t *device_mngr);
void test_null_param_deinit(device_manager_t *device_mngr);

/* test double call function */
void test_double_operation_init(device_manager_t *device_mngr);
void test_double_operation_start(device_manager_t *device_mngr);
void test_double_operation_stop(device_manager_t *device_mngr);
void test_double_operation_deinit(device_manager_t *device_mngr);
int main(){
    LOG_TEST("TEST START");
    /* init global variable */
    device_manager_t device_mngr;
    device_manager_t *device_mngr_null = NULL;
    /*execute test*/
    test_init_deinit(&device_mngr);
    test_start(&device_mngr);
    test_stop(&device_mngr);
    test_sensor_data_event(&device_mngr);
    test_multi_event(&device_mngr);
    test_drain_before_exit(&device_mngr);
    /* test null para, */
    test_null_param_init(device_mngr_null);
    test_null_param_start(device_mngr_null);
    test_null_param_stop(device_mngr_null);
    test_null_param_deinit(device_mngr_null);
    /* test double call function */
    test_double_operation_init(&device_mngr);
    test_double_operation_start(&device_mngr);
    test_double_operation_stop(&device_mngr);
    test_double_operation_deinit(&device_mngr);    
    LOG_TEST("TEST END");
}
void test_init_deinit(device_manager_t *device_mngr){
    LOG_TEST("TEST test_init_deinit START");
    /* local variables */
    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_OK;

    result = device_manager_init(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Init failed");
        return;
    }
    result = device_manager_start(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Start failed");
        return;
    }    
    sleep(3);

    result = device_manager_stop(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Stop failed");
        return;
    }
    result = device_manager_deinit(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Deinit failed");
        return;
    }
    if(expected_result == result){
        LOG_TEST("[PASS] test_init_deinit");
    }
    LOG_TEST(" TEST test_init_deinit STOP ");

}

void test_start(device_manager_t *device_mngr){
    /* local variables */
    LOG_TEST(" TEST test_start  START ");
    device_manager_result_t result;
    device_manager_state_t expected_result = DEVICE_MANAGER_STATE_RUNNING;

    result = device_manager_init(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Init failed");
    }

    result = device_manager_start(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Start failed");
        return;
    }  
    if(expected_result == device_mngr->state){
        LOG_TEST("[PASS] test_start");
    }
    else LOG_TEST("[FAILED] test_start");
    result = device_manager_stop(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Stop failed");
        return;
    }
    result = device_manager_deinit(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Deinit failed");
        return;
    }
    LOG_TEST(" TEST test_start STOP  ");

}
void test_stop(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_stop  START ");
    device_manager_result_t result;
    device_manager_state_t expected_result = DEVICE_MANAGER_STATE_STOPPED;

    result = device_manager_init(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Init failed");
    }

    result = device_manager_start(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Start failed");
        return;
    }  
    result = device_manager_stop(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Stop failed");
        return;
    }
    if(expected_result == device_mngr->state){
        LOG_TEST("[PASS] test_stop");
    }
    else LOG_TEST("[FAILED] test_stop");
    result = device_manager_deinit(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Deinit failed");
        return;
    }
    
    LOG_TEST(" TEST test_stop  STOP  ");

}

void test_sensor_data_event(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_sensor_data_event  START ");
    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_OK;

    result = device_manager_init(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Init failed");
    }

    result = device_manager_start(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Start failed");
        return;
    }  
    event_t event = {
        .type = EVENT_SENSOR_DATA
        };
    event_queue_push( &device_mngr->event_queue, &event );

    sleep(1);

    result = device_manager_stop(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Stop failed");
        return;
    }
    result = device_manager_deinit(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Deinit failed");
        return;
    }
    if(expected_result == result){
        LOG_TEST("[PASS] test_sensor_data_event");
    }
    else LOG_TEST("[FAILED] test_sensor_data_event");  
    LOG_TEST(" TEST test_sensor_data_event  STOP  ");
}
void test_multi_event(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_multi_event  START ");
    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_OK;

    result = device_manager_init(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Init failed");
    }

    result = device_manager_start(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Start failed");
        return;
    }  
    event_t event = {
        .type = EVENT_SENSOR_DATA
        };
    event_queue_push( &device_mngr->event_queue, &event );
    
    event.type = EVENT_SENSOR_ERROR;
    event_queue_push( &device_mngr->event_queue, &event );

    event.type = EVENT_SYSTEM_STOP;
    event_queue_push( &device_mngr->event_queue, &event );

    sleep(2);

    result = device_manager_stop(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Stop failed");
        return;
    }
    result = device_manager_deinit(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Deinit failed");
        return;
    }
    if(expected_result == result){
        LOG_TEST("[PASS] test_multi_event");
    }
    else LOG_TEST("[FAILED] test_multi_event");  
    LOG_TEST(" TEST test_multi_event  STOP  ");
}
void test_drain_before_exit(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_drain_before_exit  START ");
    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_OK;
    int count = 0;

    result = device_manager_init(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Init failed");
    } 
    event_t event = {
        .type = EVENT_SENSOR_DATA
        };
    for(int i = 0 ; i<200; i++){
        event_queue_push( &device_mngr->event_queue, &event );
        count++;
    }

    result = device_manager_start(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Start failed");
        return;
    } 

    result = device_manager_stop(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Stop failed");
        return;
    }

    result = device_manager_deinit(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Deinit failed");
        return;
    }

    if(expected_result == result){
        LOG_TEST("[PASS] test_drain_before_exit");
    }
    else LOG_TEST("[FAILED] test_drain_before_exit");  
    LOG_TEST(" TEST test_drain_before_exit  STOP  ");
}
void test_null_param_init(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_null_param_init  START ");
    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_INVALID_PARAM;

    result = device_manager_init(device_mngr);

    if(expected_result == result){
        LOG_TEST("[PASS] test_null_param");
    }else LOG_TEST("[FAILED] test_null_param");  
    LOG_TEST(" TEST test_null_param_init  STOP  ");
}

void test_null_param_start(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_null_param_start  START ");
    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_INVALID_PARAM;

    result = device_manager_start(device_mngr);

    if(expected_result == result){
        LOG_TEST("[PASS] test_null_param_start");
    }else LOG_TEST("[FAILED] test_null_param_start");  
    LOG_TEST(" TEST test_null_param_start  STOP  ");
}
void test_null_param_stop(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_null_param_stop  START ");
    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_INVALID_PARAM;

    result = device_manager_stop(device_mngr);

    if(expected_result == result){
        LOG_TEST("[PASS] test_null_param_stop");
    }else LOG_TEST("[FAILED] test_null_param_stop");  
    LOG_TEST(" TEST test_null_param_stop  STOP  ");
}
void test_null_param_deinit(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_null_param_deinit  START ");
    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_INVALID_PARAM;

    result = device_manager_deinit(device_mngr);
    if(expected_result == result){
        LOG_TEST("[PASS] test_null_param_deinit");
    }else LOG_TEST("[FAILED] test_null_param_deinit");  
    LOG_TEST(" TEST test_null_param_deinit  STOP  ");
}

/* test double call function */
void test_double_operation_init(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_double_operation_init  START ");

    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_INVALID_STATE;
    /* init variable */
    device_manager_state_t state = DEVICE_MANAGER_STATE_NOT_INIT;

    device_mngr->state = state;

    /* double call init function */
    result = device_manager_init(device_mngr);

    result = device_manager_init(device_mngr);

    if(expected_result == result){
        LOG_TEST("[PASS] test_double_operation_init");
    }else LOG_TEST("[FAILED] test_double_operation_init");  

    LOG_TEST(" TEST test_double_operation_init  STOP  ");
}

void test_double_operation_start(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_double_operation_start  START ");

    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_INVALID_STATE;
    /* init variable */
    device_manager_state_t state = DEVICE_MANAGER_STATE_NOT_INIT;

    device_mngr->state = state;

    result = device_manager_init(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Init failed");
    } 
    /* double call start function */
    result = device_manager_start(device_mngr);

    result = device_manager_start(device_mngr);

    if(expected_result == result){
        LOG_TEST("[PASS] test_double_operation_start");
    }else LOG_TEST("[FAILED] test_double_operation_start");  

    /* deinit for other test */
    result = device_manager_stop(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Stop failed");
        return;
    }

    result = device_manager_deinit(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Deinit failed");
        return;
    }
    LOG_TEST(" TEST test_double_operation_start  STOP  ");
}


void test_double_operation_stop(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_double_operation_stop  START ");

    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_INVALID_STATE;
    /* init variable */
    device_manager_state_t state = DEVICE_MANAGER_STATE_NOT_INIT;

    device_mngr->state = state;

    /* init => start => stop */
    result = device_manager_init(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Init failed");
    } 
    result = device_manager_start(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Start failed");
    }
    /* double call stop function */ 
    result = device_manager_stop(device_mngr);

    result = device_manager_stop(device_mngr);

    if(expected_result == result){
        LOG_TEST("[PASS] test_double_operation_stop");
    }else LOG_TEST("[FAILED] test_double_operation_stop");  
    /* deinit for other test */
    result = device_manager_deinit(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Deinit failed");
    }
    LOG_TEST(" TEST test_double_operation_stop  STOP  ");
}

void test_double_operation_deinit(device_manager_t *device_mngr){
    LOG_TEST(" TEST test_double_operation_deinit  START ");

    device_manager_result_t result;
    device_manager_result_t expected_result = DEVICE_MANAGER_INVALID_STATE;
    /* init variable */
    device_manager_state_t state = DEVICE_MANAGER_STATE_NOT_INIT;

    device_mngr->state = state;

    /* init => start => stop => deinit */
    result = device_manager_init(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Init failed");
    } 
    result = device_manager_start(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Start failed");
    }

    result = device_manager_stop(device_mngr);
    if(result != DEVICE_MANAGER_OK){
        LOG_TEST("[ERROR] Stop failed");
        return;
    }
    /* double call function deinit */

    result = device_manager_deinit(device_mngr);

    result = device_manager_deinit(device_mngr);

    if(expected_result == result){
        LOG_TEST("[PASS] test_double_operation_deinit");
    }else LOG_TEST("[FAILED] test_double_operation_deinit");  

    LOG_TEST(" TEST test_double_operation_deinit  STOP  ");    
}