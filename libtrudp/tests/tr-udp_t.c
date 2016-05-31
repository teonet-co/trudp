/*
 * File:   tr-udp_t.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on Jun 1, 2016, 12:36:15 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include "../tr-udp.h"

/*
 * CUnit Test Suite
 */

int init_suite(void);
int clean_suite(void);

/**
 * Create and Destroy TR-UDP
 */
void create_test() {
    
    // Create TR-UDP
    trudpData *td = trudpNew();
    CU_ASSERT_PTR_NOT_NULL(td);
    
    // Destroy TR-UDP
    trudpDestroy(td);
}

/**
 * Send data test
 */
void send_data_test() {

    // Create TR-UDP
    trudpData *td = trudpNew();
    CU_ASSERT_PTR_NOT_NULL(td);

    // Send data
    char *data = "HelloTR-UDP!";
    size_t data_length = strlen(data) + 1;
    CU_ASSERT(trudpSendData(td, data, data_length) > 0);
    CU_ASSERT(trudpQueueSize(td->sendQueue->q) == 1);
    CU_ASSERT_PTR_NOT_NULL(trudpTimedQueueFindById(td->sendQueue, 1));
    
    // Destroy TR-UDP
    trudpDestroy(td);
}

/**
 * Process received packet
 */
void process_received_packet_test() {

    // Create TR-UDP
    trudpData *td = trudpNew();
    CU_ASSERT_PTR_NOT_NULL(td);

    // \todo process received packet test
    
    // Destroy TR-UDP
    trudpDestroy(td);
}


int trUdpSuiteAdd() {
    
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    pSuite = CU_add_suite("TR-UDP main module", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "create test", create_test)) 
     || (NULL == CU_add_test(pSuite, "send data test", send_data_test))
     || (NULL == CU_add_test(pSuite, "process received packet test", process_received_packet_test))       
    
            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
