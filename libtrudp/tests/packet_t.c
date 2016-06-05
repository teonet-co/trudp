/*
 * File:   header_t.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 30, 2016, 7:41:24 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CUnit/Basic.h>

#include "../packet.h"

/*
 * CUnit Test Suite
 */

int init_suite(void);
int clean_suite(void);

void create_headers() {
    
    uint32_t id = 0;
    #define GET_ID() ++id
    
    // Create & check DATA packet
    char *data = "Header with Hello!";
    size_t packetLength, data_length = strlen(data) + 1;
    uint32_t packetDATAid = GET_ID();
    void *packetDATA = trudpPacketDATAcreateNew(packetDATAid, data, data_length, &packetLength);
    CU_ASSERT(trudpPacketCheck(packetDATA, packetLength));
    
    // Check getter functions
    CU_ASSERT_EQUAL(packetDATAid, trudpPacketGetId(packetDATA));
    CU_ASSERT(!memcmp(data, trudpPacketGetData(packetDATA), data_length));
    CU_ASSERT_EQUAL(data_length, trudpPacketGetDataLength(packetDATA));
    CU_ASSERT_EQUAL(TRU_DATA, trudpPacketGetType(packetDATA));
    CU_ASSERT(trudpPacketGetTimestamp(packetDATA) <= trudpGetTimestamp());
    
    // Create & check ACK packet
    void *packetACK = trudpPacketACKcreateNew(packetDATA);
    CU_ASSERT(trudpPacketCheck(packetACK, trudpPacketACKlength()));
    
    // Create & check RESET packet
    void *packetRESET = trudpPacketRESETcreateNew(GET_ID());
    CU_ASSERT(trudpPacketCheck(packetRESET, trudpPacketRESETlength()));    
    
    // Free packets
    trudpPacketCreatedFree(packetRESET); 
    trudpPacketCreatedFree(packetACK);        
    trudpPacketCreatedFree(packetDATA);
}

int packetSuiteAdd() {
    
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    pSuite = CU_add_suite("TR-UDP packet", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "create headers", create_headers)) 
      //||(NULL == CU_add_test(pSuite, "test2", test2))
            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    return 0;
}
