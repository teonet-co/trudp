/*
 * File:   tr-udp_t.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on Jun 1, 2016, 12:36:15 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

/*
 * CUnit Test Suite
 */

int init_suite(void);
int clean_suite(void);

void trudpTest1() {
    CU_ASSERT(2 * 2 == 4);
}

void trudpTest2() {
    CU_ASSERT(2 * 2 == 5);
}

int trUdpSuiteAdd() {
    
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    pSuite = CU_add_suite("TR-UDP main cUnit test", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "test1", trudpTest1)) 
     //|| (NULL == CU_add_test(pSuite, "test2", trudpTest2))
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
