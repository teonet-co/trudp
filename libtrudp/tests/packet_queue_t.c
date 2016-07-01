/*
 * File:   header_t.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 30, 2016, 7:41:24 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <CUnit/Basic.h>

#include "packet.h"
#include "packet_queue.h"

/*
 * CUnit Test Suite
 */

int init_suite(void);
int clean_suite(void);

/**
 * Timed queue test
 */
void timed_queue() {
    
    uint32_t id = 0;
    #define GET_ID() ++id
    trudpPacketQueueData *tqd;
    
    // Create timed queue
    trudpPacketQueue *tq = trudpPacketQueueNew();
    
    // Create 1 DATA packet
    char *data = "Header with Hello!";
    size_t packetLength, data_length = strlen(data) + 1;
    uint32_t packet_id = GET_ID();
    void *packetDATA = trudpPacketDATAcreateNew(packet_id, 0, data, data_length, &packetLength);
    CU_ASSERT_FATAL(trudpPacketCheck(packetDATA, packetLength));
    CU_ASSERT_EQUAL(trudpPacketGetId(packetDATA), packet_id);
    
    // Create 2 DATA packet
    char *data2 = "Header with Hello 2!";
    size_t packetLength2, data_length2 = strlen(data2) + 1;
    uint32_t packet_id2 = GET_ID();
    void *packetDATA2 = trudpPacketDATAcreateNew(packet_id2, 0, data2, data_length2, &packetLength2);
    CU_ASSERT_FATAL(trudpPacketCheck(packetDATA2, packetLength2));
    CU_ASSERT_EQUAL(trudpPacketGetId(packetDATA2), packet_id2);
    
    // Add 1 DATA packet to timed queue
    tqd = trudpPacketQueueAdd(tq, packetDATA, packetLength, trudpGetTimestampFull() + 10000);
    CU_ASSERT_PTR_NOT_NULL_FATAL(tqd);
    CU_ASSERT_EQUAL_FATAL(trudpPacketGetId(tqd->packet), packet_id);    
    
    // Add 2 DATA packet to timed queue
    tqd = trudpPacketQueueAdd(tq, packetDATA2, packetLength2, trudpGetTimestampFull() + 10000);
    CU_ASSERT_PTR_NOT_NULL_FATAL(tqd);
    CU_ASSERT_EQUAL_FATAL(trudpPacketGetId(tqd->packet), packet_id2);    
    
    // Find DATA packet by Id
    CU_ASSERT_PTR_EQUAL_FATAL(tqd, trudpPacketQueueFindById(tq, packet_id2));

    // Find DATA packet by time
    usleep(10000);
    tqd = trudpPacketQueueFindById(tq, packet_id); // Get packet with Id 1
    CU_ASSERT_PTR_EQUAL_FATAL(tqd, trudpPacketQueueFindByTime(tq, trudpGetTimestampFull()));
    
    // Delete packet from timed queue
    tqd = trudpPacketQueueFindByTime(tq, trudpGetTimestampFull());
    trudpPacketQueueDelete(tq, tqd);
    CU_ASSERT_FATAL(trudpQueueSize(tq->q) == 1);
    
    // Free timed queue
    trudpPacketQueueFree(tq);
    CU_ASSERT_FATAL(trudpQueueSize(tq->q) == 0);
    
    // Free DATA packet
    trudpPacketCreatedFree(packetDATA);
    
    // Destroy timed queue
    trudpPacketQueueDestroy(tq);
}

void test2() {
    CU_ASSERT(2 * 2 == 5);
}

int timedQueueSuiteAdd() {
    
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    pSuite = CU_add_suite("TR-UDP packet queue", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "add/delete/find packets in packet queue", timed_queue)) 
      //||(NULL == CU_add_test(pSuite, "test2", test2))
            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    return 0;
}