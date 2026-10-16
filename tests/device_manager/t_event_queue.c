#include "device_manager/event_queue.h"
#include "device_manager/event.h"
#include <stdatomic.h>
#include <unistd.h>

/*
    Overview test case:
        Init → Push → Pop
        Producer will wait when queue Full 
        Fail push event when queue is close
        Drain remaining events after close

        Basic consumer
        Basic shutdown

        Push 10000 event
        Multi producer
        Multi consumer
        Multi producer and multi consumer
*/

/* functional test */
void test_push_pop(event_queue_t *queue);                   
void test_blocking_queue_full(event_queue_t *queue);             
void test_queue_close(event_queue_t *queue);            
void test_drain_after_close(event_queue_t *queue);      

/* intergration test */
void test_consumer(event_queue_t *queue);               
void test_shutdown(event_queue_t *queue);        

/* stress test */
void test_10k_event(event_queue_t *queue);                  //1 producer + 1 consumer + 10k events
void test_multi_producer(event_queue_t *queue);             //4 producer + 1 consumer + 40k events          
void test_multi_consumer(event_queue_t *queue);             //1 producer + 1 consumer + 40k events
void test_multi_consumer_producer(event_queue_t *queue);    //4 producer + 4 consumer + 100k events

/* global variable */
volatile int pop_count = 0;
atomic_int count_test_multi_consumer = 0;
atomic_int count_test_10k_event = 0;
atomic_int count_pop_test_multi_producer_consumer = 0;
atomic_int count_push_test_multi_producer_consumer = 0;
/* thread function */
void* producer_thread(void *arg);
void* consumer_thread(void *arg);
void* consumer_thread_test_multi_producer(void *arg);
void* producer_thread_test_multi_producer(void *arg);
void* consumer_thread_test_multi_consumer(void *arg);
void* producer_thread_test_multi_consumer(void *arg);
void* producer_thread_10k_event(void *arg);
void* consumer_thread_10k_event(void *arg);
void* producer_thread_test_multi_producer_consumer(void *arg);
void* consumer_thread_test_multi_producer_consumer(void *arg);
/**/
int main(){
    printf("\n========== TEST START ==========\n");
    event_queue_t t_event_queue;
    /*execute test*/
    test_push_pop(&t_event_queue);
    test_blocking_queue_full(&t_event_queue);
    test_queue_close(&t_event_queue);
    test_drain_after_close(&t_event_queue);
    test_consumer(&t_event_queue);
    test_shutdown(&t_event_queue);
    test_10k_event(&t_event_queue);
    test_multi_producer(&t_event_queue);
    test_multi_consumer(&t_event_queue);
    test_multi_consumer_producer(&t_event_queue);
    printf("\n========== TEST END ==========\n");
}
void test_push_pop(event_queue_t *queue){
    printf("\n========== test_push_pop START ==========\n");
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
    printf("\n========== test_push_pop STOP ==========\n");
}
void test_blocking_queue_full(event_queue_t *queue){
    printf("\n========== test_blocking_queue_full START ==========\n");
    pthread_t tid[2];
    event_queue_result_t result = EVENT_QUEUE_ERROR;
    //event_queue_result_t expected_value = EVENT_QUEUE_OK;
    int thread_result;

    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    thread_result= pthread_create(&tid[0],NULL,producer_thread,queue);
    if(thread_result != 0){
        printf("[ERROR] Create consumer thread failed\n");
        event_queue_deinit(queue);
        return;
    }
    sleep(3);

    /* create consumer thread */
    thread_result= pthread_create(&tid[1],NULL,consumer_thread,queue);
    if(thread_result != 0){
        printf("[ERROR] Create consumer thread failed\n");
        event_queue_deinit(queue);
        return;
    }

    pthread_join(tid[0],NULL);

    /* close queue to out while() */
    result = event_queue_close(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Close queue event failed\n");
    }

    pthread_join(tid[1],NULL);

    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }
    printf("\n========== test_blocking_queue_full STOP ==========\n");
}
void test_queue_close(event_queue_t *queue){
    printf("\n========== test_queue_close START ==========\n");
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
        printf("[PASS] test_queue_close\n");
    } else{
        printf( "test_queue_close Fail, result=%d\n", result);
    }
    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Denit queue event failed\n");
    }
    printf("\n========== test_queue_close STOP ==========\n");
}

void test_consumer(event_queue_t *queue){
    printf("\n========== test_consumer START   ==========\n");
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

    result = event_queue_close(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Close queue event failed\n");
    }

    pthread_join(tid,NULL);

    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }
    printf("\n========== test_consumer STOP   ==========\n");
}

void test_shutdown(event_queue_t *queue){
    printf("\n========== test_shutdown START  ==========\n");
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
    printf("\n========== test_queue_close STOP ==========\n");
}

void test_drain_after_close(event_queue_t *queue){
    printf("\n========== test_drain_after_close START ==========\n");
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
        printf("[ERROR] Pop event 1st failed\n");
    } else printf("Event is %d\n",event_out.type);
    
    /*get the 2nd event*/
    result = event_queue_pop(queue,&event_out);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Pop event 2nd failed\n");
    } else printf("Event is %d\n",event_out.type);

    /*get the 3rd event*/
    result = event_queue_pop(queue,&event_out);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Pop event 3rd failed\n");
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
    printf("\n========== test_drain_after_close STOP    ==========\n");
}

/* stress test */
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
    if(pop_count == 10000*4){
        printf("[PASS] test_multi_producer\n");
    }else {
        printf("[FAIL] test_multi_producer\n");
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

void test_10k_event(event_queue_t *queue){
    printf("\n========== test_10k_event START           ==========\n");
    pthread_t tid[2];
    event_queue_result_t result = EVENT_QUEUE_ERROR;
    //event_queue_result_t expected_value = EVENT_QUEUE_OK;
    int thread_result;

    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    thread_result= pthread_create(&tid[0],NULL,producer_thread_10k_event,queue);
    if(thread_result != 0){
        printf("[ERROR] Create consumer thread failed\n");
        event_queue_deinit(queue);
        return;
    }

    sleep(3);

    /* create consumer thread */
    thread_result= pthread_create(&tid[1],NULL,consumer_thread_10k_event,queue);
    if(thread_result != 0){
        printf("[ERROR] Create consumer thread failed\n");
        event_queue_deinit(queue);
        return;
    }

    pthread_join(tid[0],NULL);

    /* close queue to out while() */
    result = event_queue_close(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Close queue event failed\n");
    }

    pthread_join(tid[1],NULL);

    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }
    printf("\n========== test_10k_event STOP        ==========\n");
}
void test_multi_consumer_producer(event_queue_t *queue){
    printf("\n========== TEST test_multi_consumer_producer  START ==========\n");
    pthread_t tid[8];
    event_queue_result_t result = EVENT_QUEUE_ERROR;
    //event_queue_result_t expected_value = EVENT_QUEUE_OK;
    int producer_thread_result[4];
    int consumer_thread_result[4];
    result = event_queue_init(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Init queue event failed\n");
    }
    /* create consumer thread */
    for(int i = 0; i<4 ; i++){
        consumer_thread_result[i] = pthread_create(&tid[i],NULL,consumer_thread_test_multi_producer_consumer,queue);
        if(consumer_thread_result[i] != 0){
            printf("[ERROR] Create consumer thread failed\n");
            event_queue_deinit(queue);
            return;
        }
    }
    
    /* create producer thread */
    for(int i = 4; i<8 ; i++){
        producer_thread_result[i-4] = pthread_create(&tid[i],NULL,producer_thread_test_multi_producer_consumer,queue);
        if(producer_thread_result[i-4] != 0){
            printf("[ERROR] Create producer thread failed\n");
            event_queue_deinit(queue);
            return;
        }
    }
    
    /* wait all producers finished */
    for(int i =  4; i < 8; i++){
        pthread_join(tid[i], NULL);
    }
    /* wait for all event push OK */
    event_queue_close(queue);

    /* wait for all consumers finished */
    for(int i = 0; i < 4; i++){
        pthread_join(tid[i], NULL);
    }

    /* get 2 counts of consumers and producers */
    uint32_t push_count = atomic_load(&count_push_test_multi_producer_consumer);

    uint32_t pop_count = atomic_load(&count_pop_test_multi_producer_consumer);

    /* after all thread exit check the result */
    printf("push_count %d pop_count %d\n",push_count,pop_count);
    if(push_count == pop_count){
        printf("[PASS] test_multi_consumer_producer\n");
    }else {
        printf("[FAIL] test_multi_consumer_producer\n");
    }
    result = event_queue_deinit(queue);
    if(result != EVENT_QUEUE_OK){
        printf("[ERROR] Deinit queue event failed\n");
    }
    printf("\n========== TEST test_multi_consumer_producer END ==========\n");
}

/* thread function implementation */

void* producer_thread(void *arg){
    event_queue_t *queue = arg;
    event_t event = {.type = EVENT_SYSTEM_START};

    for(int i = 0; i < EVENT_QUEUE_CAPACITY + 2; i++){
        event_queue_push(queue, &event);
    }

    return NULL;
}
void* consumer_thread(void *arg){
    event_queue_t *queue = (event_queue_t *) arg;

    event_t event;
    printf("Consumer: waiting...\n");
    while(1){
        event_queue_result_t result = event_queue_pop(queue, &event);
        if(result == EVENT_QUEUE_CLOSED) break;
    }
    return NULL;
}
/* thread for test_multi_producer */

void* consumer_thread_test_multi_producer(void *arg){
    event_queue_t *queue = (event_queue_t *)arg;
    event_queue_result_t result = EVENT_QUEUE_OK;
    event_t event;
    while(1){
        result =  event_queue_pop(queue, &event);
        if(result == EVENT_QUEUE_CLOSED) break;

        if(result == EVENT_QUEUE_OK)  pop_count++;
    }
    return NULL;
}
void* producer_thread_test_multi_producer(void *arg){
    event_queue_t *queue = (event_queue_t *) arg;

    event_t event;
    event.type = EVENT_SYSTEM_START;
    for(int i = 0 ; i< 10000; i++){
        event_queue_push(queue, &event);
    }
    return NULL;
}
/* thread for test_multi_consumer */
void* consumer_thread_test_multi_consumer(void *arg){
    event_queue_t *queue = (event_queue_t *)arg;
    event_queue_result_t result = EVENT_QUEUE_OK;
    event_t event;
    
    while(1){
        result =  event_queue_pop(queue, &event);
        if(result == EVENT_QUEUE_CLOSED) break;

        if(result == EVENT_QUEUE_OK)  {
            int count = atomic_fetch_add( &count_test_multi_consumer, 1) + 1;
            if(count == 10000*4){
                printf("[PASS] test_multi_consumer\n");
            }
        }
    }
    return NULL;
}
void* producer_thread_test_multi_consumer(void *arg){
    event_queue_t *queue = (event_queue_t *) arg;

    event_t event;
    event.type = EVENT_SYSTEM_START;
    for(int i = 0 ; i< 10000*4; i++){
       event_queue_push(queue, &event);
    }
    return NULL;
}
void* producer_thread_10k_event(void *arg){
    event_queue_t *queue = arg;
    event_t event = {.type = EVENT_SYSTEM_START};

    for(int i = 0; i < 10000; i++){
        event_queue_push(queue, &event);
    }
    return NULL;
}
void* consumer_thread_10k_event(void *arg){
    event_queue_t *queue = (event_queue_t *) arg;

    event_t event;
    while(1){
        event_queue_result_t result = event_queue_pop(queue, &event);
        if(result == EVENT_QUEUE_OK){
            int count = atomic_fetch_add( &count_test_multi_consumer, 1) + 1;
            if(count == 10000){
                printf("[PASS] test_10k_event\n");
            }
        }
        if(result == EVENT_QUEUE_CLOSED) break;

    }
    return NULL;
}
void* producer_thread_test_multi_producer_consumer(void *arg){
    event_queue_t *queue = (event_queue_t *) arg;
    event_queue_result_t result = EVENT_QUEUE_OK;
    event_t event;
    event.type = EVENT_SYSTEM_START;
    for(int i = 0 ; i< 25000; i++){
        result = event_queue_push(queue, &event);
        if(result == EVENT_QUEUE_OK)  {
           atomic_fetch_add( &count_push_test_multi_producer_consumer, 1);
        }
    }
    return NULL;
}
void* consumer_thread_test_multi_producer_consumer(void *arg){
    event_queue_t *queue = (event_queue_t *)arg;
    event_queue_result_t result = EVENT_QUEUE_OK;
    event_t event;
    
    while(1){
        result =  event_queue_pop(queue, &event);
        if(result == EVENT_QUEUE_CLOSED) break;

        if(result == EVENT_QUEUE_OK)  {
           atomic_fetch_add( &count_pop_test_multi_producer_consumer, 1);
        }
    }
    return NULL;
}