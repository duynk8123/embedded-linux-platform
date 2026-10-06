#include "device_manager/event_queue.h"
#include "device_manager/event.h"
#include <unistd.h>

/*
    Overview test case:
        Init → Push → Pop
        Queue Full
        Queue Close
        Consumer Block → Producer Wakeup
        Consumer Block → Close Queue Wakeup
        Drain remaining events after close
        Multi Producer
        Multi Consumer
*/
void test_push_pop(event_queue_t *queue);               //normal test
void test_queue_full(event_queue_t *queue);             //abnormal test
void test_queue_close(event_queue_t *queue);            //abnormal test
void test_consumer(event_queue_t *queue);               //normal test
void test_shutdown(event_queue_t *queue);               //abnormal test
void test_drain_after_close(event_queue_t *queue);      //normal test
void test_multi_producer(event_queue_t *queue);         
void test_multi_consumer(event_queue_t *queue);
//thread function
void* consumer_thread(void *arg){
    event_queue_t *queue = (event_queue_t *) arg;

    event_t event;

    printf("Consumer: waiting...\n");

    event_queue_result_t result =
        event_queue_pop(queue, &event);
    
    printf("Consumer: wake up, result = %d, event %d\n", result, event.type);

    return NULL;
}
//thread for test_multi_producer
int push_count=0;
int pop_count=0;
void* consumer_thread_test_multi_producer(void *arg){
    event_queue_t *queue = (event_queue_t *)arg;
    event_queue_result_t result = EVENT_QUEUE_OK;
    event_t event;
    while(1){
        result =  event_queue_pop(queue, &event);
        if(result == EVENT_QUEUE_CLOSED) break;

        if(result == EVENT_QUEUE_OK)  pop_count++;
        printf(
            "Consumer: result = %d, event = %d, count:%d\n",
            result,
            event.type,
            pop_count
        );
    }
    return NULL;
}
void* producer_thread_test_multi_producer(void *arg){
    event_queue_t *queue = (event_queue_t *) arg;

    event_t event;
    event.type = EVENT_SYSTEM_START;
    for(int i = 0 ; i<10; i++){
        event_queue_result_t result = event_queue_push(queue, &event);

        if(result == EVENT_QUEUE_OK) push_count++;
        printf(
            "Producer: push result = %d\n",
            result
        );
    }
    return NULL;
}
//thread for test_multi_consumer
volatile int count_test_multi_consumer=0;
void* consumer_thread_test_multi_consumer(void *arg){
    event_queue_t *queue = (event_queue_t *)arg;
    event_queue_result_t result = EVENT_QUEUE_OK;
    event_t event;
    
    while(1){
        result =  event_queue_pop(queue, &event);
        if(result == EVENT_QUEUE_CLOSED) break;

        if(result == EVENT_QUEUE_OK)  count_test_multi_consumer++;
        printf(
            "Consumer: result = %d, event = %d, count:%d\n",
            result,
            event.type,
            count_test_multi_consumer
        );
        if(count_test_multi_consumer == 100*4){
            printf("[PASS] test_multi_producer\n");
        }
    }
    return NULL;
}
void* producer_thread_test_multi_consumer(void *arg){
    event_queue_t *queue = (event_queue_t *) arg;

    event_t event;
    event.type = EVENT_SYSTEM_START;
    for(int i = 0 ; i<40; i++){
        event_queue_result_t result = event_queue_push(queue, &event);
        printf(
            "Producer: push result = %d\n",
            result
        );
    }
    return NULL;
}
int main(){
    printf("\n========== TEST START ==========\n");
    event_queue_t t_event_queue;
    // pthread_mutex_t lock;
    /*execute test*/
    test_push_pop(&t_event_queue);
    test_queue_full(&t_event_queue);
    test_queue_close(&t_event_queue);
    test_consumer(&t_event_queue);
    test_shutdown(&t_event_queue);
    test_drain_after_close(&t_event_queue);
    test_multi_producer(&t_event_queue);
    test_multi_consumer(&t_event_queue);
    printf("\n========== TEST END ==========\n");
}
void test_push_pop(event_queue_t *queue){
    event_queue_result_t result = EVENT_QUEUE_ERROR;
    // event_queue_result_t expected_value = EVENT_QUEUE_OK;

    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    /*test event*/
    event_t event_in = {
        .type = EVENT_SYSTEM_START
    };
    result = event_queue_push(queue,&event_in);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Push event failed\n");
    }
    /*get the event*/
    event_t event_out;
    result = event_queue_pop(queue,&event_out);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Pop event failed\n");
    }
    else {
        if(event_in.type == event_out.type){
            printf("[PASS] test_push_pop\n");
        }
    }
    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }
}
void test_queue_full(event_queue_t *queue){
    event_queue_result_t result = EVENT_QUEUE_ERROR;
    event_queue_result_t expected_value = EVENT_QUEUE_FULL;

    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    /*test event*/
    event_t event_in = {
        .type = EVENT_SYSTEM_START
    };
    for (int i=0 ; i < EVENT_QUEUE_CAPACITY;i++){
        result = event_queue_push(queue,&event_in);
        if(result != EVENT_QUEUE_OK){
            printf("[ERROR] Push event failed\n");
        }
    }
    /* push event out of range */
    result = event_queue_push(queue,&event_in);

   /* check expected value */
    if(expected_value == result){
        printf("[PASS] test_queue_full\n");
    }
    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }
}
void test_queue_close(event_queue_t *queue){
    event_queue_result_t result = EVENT_QUEUE_ERROR;
    event_queue_result_t expected_value = EVENT_QUEUE_CLOSED;

    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    result = event_queue_close(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Close queue event failed\n");
    }
    /*test event*/
    event_t event_in = {
        .type = EVENT_SYSTEM_START
    };
    result = event_queue_push(queue,&event_in);

    /*check expected value*/
    if(expected_value == result){
        printf("[PASS]test_queue_close\n");
    } else{
        printf(
            "test_queue_close Fail, result=%d\n",
            result
            );
    }
    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Denit queue event failed\n");
    }
}

void test_consumer(event_queue_t *queue){
    pthread_t tid;
    event_queue_result_t result = EVENT_QUEUE_ERROR;
    //event_queue_result_t expected_value = EVENT_QUEUE_OK;
    int thread_result;

    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    /* create consumer thread */
    thread_result= pthread_create(&tid,NULL,consumer_thread,queue);
    if(thread_result != 0){
        printf("[ERROR] Create consumer thread failed\n");
        event_queue_deinit(queue);
        return;
    }
    /* to ensure the consumer thread is waiting */
    sleep(1);
    /*test event*/
    event_t event_in = {
        .type = EVENT_SYSTEM_START
    };
    result = event_queue_push(queue,&event_in);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Push event failed\n");
    }
    pthread_join(tid,NULL);

    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }
}

void test_shutdown(event_queue_t *queue){
    pthread_t tid;
    event_queue_result_t result = EVENT_QUEUE_ERROR;
    //event_queue_result_t expected_value = EVENT_QUEUE_OK;
    int thread_result;

    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    /* create consumer thread */
    thread_result= pthread_create(&tid,NULL,consumer_thread,queue);
    if(thread_result != 0){
        printf("[ERROR] Create consumer thread failed\n");
        event_queue_deinit(queue);
        return;
    }
    /* to ensure the consumer thread is waiting */
    sleep(1);
    result = event_queue_close(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }

    pthread_join(tid,NULL);

    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }

}

void test_drain_after_close(event_queue_t *queue){

    event_queue_result_t result = EVENT_QUEUE_ERROR;
    // event_queue_result_t expected_value = EVENT_QUEUE_OK;

    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    /*test event*/
    event_t event_in = {
        .type = EVENT_SYSTEM_START
    };
    result = event_queue_push(queue,&event_in);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Push event failed\n");
    }
    /*put 2nd event*/
    event_in.type = EVENT_SENSOR_DATA;
    result = event_queue_push(queue,&event_in);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Push event failed\n");
    }
    /*put 3rd event*/
    event_in.type = EVENT_SENSOR_ERROR;
    result = event_queue_push(queue,&event_in);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Push event failed\n");
    }
    /*close*/
    result = event_queue_close(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    /*get the 1st event*/
    event_t event_out;
    result = event_queue_pop(queue,&event_out);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Pop event failed\n");
    } else printf("Event is %d\n",event_out.type);
    
    /*get the 2nd event*/
    result = event_queue_pop(queue,&event_out);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Pop event failed\n");
    } else printf("Event is %d\n",event_out.type);

    /*get the 3rd event*/
    result = event_queue_pop(queue,&event_out);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Pop event failed\n");
    } else printf("Event is %d\n",event_out.type);

    /*get the event to return error*/
    result = event_queue_pop(queue,&event_out);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Pop event failed\n");
    } else printf("Event is %d\n",event_out.type);

    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }
}


void test_multi_producer(event_queue_t *queue){
    printf("\n========== TEST test_multi_producer  START ==========\n");
    pthread_t tid[5];
    event_queue_result_t result = EVENT_QUEUE_ERROR;
    //event_queue_result_t expected_value = EVENT_QUEUE_OK;
    int producer_thread_result[4];
    int consumer_thread_result;
    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    /* create consumer thread */
    consumer_thread_result = pthread_create(&tid[4],NULL,consumer_thread_test_multi_producer,queue);
    if(consumer_thread_result != 0){
        printf("[ERROR] Create consumer thread failed\n");
        event_queue_deinit(queue);
        return;
    }
    
    /* create producer thread */
    for(int i = 0; i<4 ; i++){
        producer_thread_result[i] = pthread_create(&tid[i],NULL,producer_thread_test_multi_producer,queue);
        if(producer_thread_result[i] != 0){
            printf("[ERROR] Create producer thread failed\n");
            event_queue_deinit(queue);
            return;
        }
    }
    
    /* wait all producers finished */
    for(int i = 0; i < 4; i++){
        pthread_join(tid[i], NULL);
    }
    /*wait for all event push OK*/
    event_queue_close(queue);

    pthread_join(tid[4],NULL);
    /* after all thread exit check the result */
    if(push_count == pop_count){
        printf("[PASS] test_multi_producer\n");
    }
    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }
    printf("\n========== TEST test_multi_producer END ==========\n");
}

void test_multi_consumer(event_queue_t *queue){
    printf("\n========== TEST test_multi_consumer  START ==========\n");
    pthread_t tid[5];
    event_queue_result_t result = EVENT_QUEUE_ERROR;
    //event_queue_result_t expected_value = EVENT_QUEUE_OK;
    int producer_thread_result;
    int consumer_thread_result[4];
    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    /* create consumer thread */
    for(int i = 0; i<4 ; i++){
        consumer_thread_result[i] = pthread_create(&tid[i],NULL,consumer_thread_test_multi_consumer,queue);
        if(consumer_thread_result[i] != 0){
            printf("[ERROR] Create consumer thread failed\n");
            event_queue_deinit(queue);
            return;
        }
    }
    /* create producer thread */
    producer_thread_result = pthread_create(&tid[4],NULL,producer_thread_test_multi_consumer,queue);
    if(producer_thread_result != 0){
        printf("[ERROR] Create producer thread failed\n");
        event_queue_deinit(queue);
        return;
    }
    /* wait producer finished */
    pthread_join(tid[4], NULL);

    /*wait for all event push OK*/
    event_queue_close(queue);
    for(int i =0; i<4; i++){
        pthread_join(tid[i],NULL);
    }

    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }
    printf("\n========== TEST test_multi_consumer END ==========\n");
}