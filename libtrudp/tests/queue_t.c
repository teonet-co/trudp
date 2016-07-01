/*
 * File:   queue.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 30, 2016, 11:53:18 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CUnit/Basic.h>

#include "queue.h"

/*
 * CUnit Test Suite
 */

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

//#include "header_t.c"

/**
 * Create/destroy TR-UDP queue test
 */
void queue_create_test() {

    // Create new queue
    trudpQueue *q = trudpQueueNew();
    CU_ASSERT_PTR_NOT_NULL_FATAL(q);

    // Destroy queue
    int rv = trudpQueueDestroy(q);
    CU_ASSERT(!rv);
}

/**
 * Add elements to TR-UDP queue test
 */
void add_elements_to_queue() {
    
    #undef NO_MESSAGES
    #define NO_MESSAGES 1
    
    // Create new queue
    trudpQueue *q = trudpQueueNew();
    CU_ASSERT_PTR_NOT_NULL_FATAL(q);
    
    int i;
    trudpQueueData *qd, *qd2;
    size_t data_length;
    const size_t num_elements = 3;
    const char *data_format = "Hello-%u!";
    size_t data_buf_length = strlen(data_format) + 1 + 10;
    char data[num_elements][data_buf_length];
    
    // Add elements to queue
    #if !NO_MESSAGES
    printf("\n");
    printf("    Add %u elements to Queue:\n", (unsigned long)num_elements);
    #endif
    for(i = 0; i < num_elements; i++) {
        //char *data = "Hello-1!";
        snprintf(data[i], data_buf_length, data_format, i+1);
        data_length = strlen(data[i]) + 1;
        qd = trudpQueueAdd(q, (void*)data[i], data_length);
        #if !NO_MESSAGES
        printf("      data_length: %u, pointer: %p, data: %s\n", 
                (unsigned long)data_length, data[i], data[i]);
        #endif
        CU_ASSERT(q->length); // length not null
        CU_ASSERT(q->last == qd); // last element equal to this
        CU_ASSERT(q->first && q->last); // first and last element are present
        CU_ASSERT(i || (!i && q->first == qd)); // for fist added: first element equal to this
        CU_ASSERT(i || (!i && !qd->prev)); // for fist added: previous is null
        CU_ASSERT(!i || (i && qd->prev)); // for non fist added: previous is not null
        CU_ASSERT(!qd->next); // next is null
        CU_ASSERT_PTR_NOT_NULL_FATAL(qd);
        CU_ASSERT_STRING_EQUAL_FATAL(data[i], qd->data);
        CU_ASSERT_EQUAL_FATAL(data_length, qd->data_length);
        CU_ASSERT_EQUAL_FATAL(trudpQueueSize(q), i+1);
    }
    
    // Loop through all queue elements
    #if !NO_MESSAGES
    printf("    Retrieve %d elements from Queue:\n", (unsigned long)num_elements);
    #endif
    i = 0;
    qd = q->first;
    while(qd) {      
        #if !NO_MESSAGES
        printf("      qd->data: \"%s\", data[i]: \"%s\"\n", qd->data, data[i]);
        #endif
        CU_ASSERT_STRING_EQUAL_FATAL(qd->data, data[i++]);
        qd = qd->next;
    }
    #if !NO_MESSAGES
    printf("    ");
    #endif

    // Move first element to the end of queue
    qd = q->first;
    qd2 = qd->next;
    trudpQueueMoveToEnd(q, qd);
    CU_ASSERT(q->last == qd);
    CU_ASSERT(q->first == qd2);
    CU_ASSERT(q->length == num_elements);
    
    // Delete first
    qd = q->first->next;
    trudpQueueDeleteFirst(q);
    CU_ASSERT(q->length == num_elements-1);
    CU_ASSERT(qd == q->first);
    
    // Delete last
    qd = q->last->prev;
    trudpQueueDeleteLast(q);
    CU_ASSERT(q->length == num_elements-2);
    CU_ASSERT(qd == q->last);
    
    // Delete all by deleting first
    while(!trudpQueueDeleteFirst(q));
    CU_ASSERT(!q->length);
    CU_ASSERT(!q->first && !q->last);
    
    // Destroy queue
    int rv = trudpQueueDestroy(q);
    CU_ASSERT(!rv);    
}

/**
 * Check Queue iterator
 */
void check_queue_iterator() {
    
    #undef NO_MESSAGES
    #define NO_MESSAGES 1
    
    // Create new queue
    trudpQueue *q = trudpQueueNew();
    CU_ASSERT_PTR_NOT_NULL_FATAL(q);
    
    int i;
    trudpQueueData *qd;
    size_t data_length;
    const size_t num = 3;
    const char *data_format = "Hello-%u!";
    size_t data_buf_length = strlen(data_format) + 1 + 10;
    char data[num][data_buf_length];
    
    // Add elements to queue
    #if !NO_MESSAGES
    printf("\n");
    printf("    Add %d elements to Queue:\n", (unsigned long)num);
    #endif    
    for(i = 0; i < num; i++) {
        //char *data = "Hello-1!";
        snprintf(data[i], data_buf_length, data_format, i+1);
        data_length = strlen(data[i]) + 1;
        qd = trudpQueueAdd(q, (void*)data[i], data_length);
        #if !NO_MESSAGES
        printf("      data_length: %u, pointer: %p, data: %s\n", 
                (unsigned long)data_length, data[i], data[i]);
        #endif
        CU_ASSERT_PTR_NOT_NULL_FATAL(qd);
        CU_ASSERT_STRING_EQUAL_FATAL(data[i], qd->data);
        CU_ASSERT_EQUAL_FATAL(data_length, qd->data_length);
        CU_ASSERT_EQUAL_FATAL(trudpQueueSize(q), i+1);
    }

    // Retrieve elements from Queue using iterator
    #if !NO_MESSAGES
    printf("    Retrieve %u elements from Queue using iterator:\n", (unsigned long)num);
    #endif

    i = 0;
    trudpQueueIterator *it = trudpQueueIteratorNew(q);
    if(it != NULL) {
        
        while(trudpQueueIteratorNext(it)) {
            
            qd = trudpQueueIteratorElement(it);
            #if !NO_MESSAGES
            printf("      qd->data: \"%s\", data[i]: \"%s\"\n", qd->data, data[i]);
            #endif
            CU_ASSERT_STRING_EQUAL_FATAL(qd->data, data[i++]);            
        }
        trudpQueueIteratorFree(it);
    }

    // Destroy queue
    int rv = trudpQueueDestroy(q);
    CU_ASSERT(!rv);    
}

/**
 * Delete elements from TR-UDP Queue
 */
void delete_elements_from_queue() {
    
    #undef NO_MESSAGES
    #define NO_MESSAGES 1
    
    // Create new queue
    trudpQueue *q = trudpQueueNew();
    CU_ASSERT_PTR_NOT_NULL_FATAL(q);
    
    int i;
    trudpQueueData *qd;
    size_t data_length;
    const size_t num = 8;
    const char *data_format = "Hello-%u!";
    size_t data_buf_length = strlen(data_format) + 1 + 10;
    char data[num][data_buf_length];
    
    // Add elements to queue
    #if !NO_MESSAGES
    printf("\n");
    printf("    Add %d elements to Queue:\n", (unsigned long)num);
    #endif    
    for(i = 0; i < num; i++) {
        //char *data = "Hello-1!";
        snprintf(data[i], data_buf_length, data_format, i+1);
        data_length = strlen(data[i]) + 1;
        qd = trudpQueueAdd(q, (void*)data[i], data_length);
        #if !NO_MESSAGES
        printf("      data_length: %u, pointer: %p, data: %s\n", 
                (unsigned long)data_length, data[i], data[i]);
        #endif
        CU_ASSERT_PTR_NOT_NULL_FATAL(qd);
        CU_ASSERT_STRING_EQUAL_FATAL(data[i], qd->data);
        CU_ASSERT_EQUAL_FATAL(data_length, qd->data_length);
        CU_ASSERT_EQUAL_FATAL(trudpQueueSize(q), i+1);
    }
    
    // Delete elements from queue
    // Retrieve elements from Queue using iterator
    #if !NO_MESSAGES
    printf("    Delete elements from Queue:\n");
    #endif
    i = 0;
    size_t newNum, deletedNum = 0;
    trudpQueueIterator *it = trudpQueueIteratorNew(q);
    if(it != NULL) {
        
        while(trudpQueueIteratorNext(it)) {
            
            if(i%2) {
                qd = trudpQueueIteratorElement(it);
                //trudpQueueIteratorNext(it);
                #if !NO_MESSAGES
                printf("      delete qd->data: \"%s\"\n", qd->data);
                #endif
                deletedNum++;
                trudpQueueDelete(q, qd);
            }
            i++;
        }
        trudpQueueIteratorFree(it);
    }
    newNum = trudpQueueSize(q);
    #if !NO_MESSAGES
    printf("      %u elements deleted from queue\n", (unsigned long)deletedNum);
    printf("      new number of elements: %u\n", (unsigned long)newNum);
    #endif
    CU_ASSERT_EQUAL_FATAL(num - deletedNum, newNum);

    // Retrieve elements from Queue using iterator
    #if !NO_MESSAGES
    printf("    Retrieve elements from Queue:\n");
    #endif
    it = trudpQueueIteratorNew(q);
    if(it != NULL) {
        
        while(trudpQueueIteratorNext(it)) {
            
            qd = trudpQueueIteratorElement(it);
            #if !NO_MESSAGES
            printf("      qd->data: \"%s\"\n", qd->data);
            #endif
        }
        trudpQueueIteratorFree(it);
    }

    // Destroy queue
    int rv = trudpQueueDestroy(q);
    CU_ASSERT(!rv);    
}

int packetSuiteAdd();
int timedQueueSuiteAdd();
int trUdpSuiteAdd();
int hashSuiteAdd();

int main() {
    
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("TR-UDP queue", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add this module tests to the suite */
    if ((NULL == CU_add_test(pSuite, "queue create/destroy", queue_create_test)) ||
        (NULL == CU_add_test(pSuite, "add elements to queue", add_elements_to_queue)) ||
        (NULL == CU_add_test(pSuite, "check queue iterator", check_queue_iterator)) ||
        (NULL == CU_add_test(pSuite, "delete elements from queue", delete_elements_from_queue)) ) {
        
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    packetSuiteAdd();
    timedQueueSuiteAdd();
    trUdpSuiteAdd();
    hashSuiteAdd();

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
