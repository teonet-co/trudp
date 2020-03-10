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

#include "packet.h"

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
    trudpPacket* packetDATA = trudpPacketDATAcreateNew(packetDATAid, 0, data, data_length, &packetLength);
    uint8_t* packetDATA_buffer = (uint8_t*)packetDATA;
    CU_ASSERT_PTR_NOT_NULL_FATAL(trudpPacketCheck(packetDATA_buffer, packetLength));

    // Check getter functions
    CU_ASSERT_EQUAL(packetDATAid, trudpPacketGetId(packetDATA));
    CU_ASSERT(!memcmp(data, trudpPacketGetData(packetDATA), data_length));
    CU_ASSERT_EQUAL(data_length, trudpPacketGetDataLength(packetDATA));
    CU_ASSERT_EQUAL(TRU_DATA, trudpPacketGetType(packetDATA));
    CU_ASSERT(trudpPacketGetTimestamp(packetDATA) <= trudpGetTimestamp());

    // Create & check ACK packet
    trudpPacket* packetACK = trudpPacketACKcreateNew(packetDATA);
    uint8_t* packetACK_buffer = (uint8_t*)packetACK;
    CU_ASSERT_PTR_NOT_NULL_FATAL(trudpPacketCheck(packetACK_buffer, trudpPacketACKlength()));

    // Create & check RESET packet
    trudpPacket *packetRESET = trudpPacketRESETcreateNew(GET_ID(),0);
    uint8_t* packetRESET_buffer = (uint8_t*)packetRESET;
    CU_ASSERT_PTR_NOT_NULL_FATAL(trudpPacketCheck(packetRESET_buffer, trudpPacketRESETlength()));

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
