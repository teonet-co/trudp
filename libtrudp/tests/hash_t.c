/*
 * File:   hash_t.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on Jun 5, 2016, 4:32:13 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "../hash.h"

/*
 * CUnit Test Suite
 */

int init_suite(void);
int clean_suite(void);

// Check hash function
void check_hash() {
    
    char *key = "127.0.0.1:8000";
    uint32_t hash = SuperFastHash(key, strlen(key) + 1);
    printf("\nHash of key %s = %u ", key, hash);
    
    key = "127.0.0.1:8001";
    hash = SuperFastHash(key, strlen(key) + 1);
    printf("\nHash of key %s = %u ", key, hash);

    key = "192.168.101.11:8000";
    hash = SuperFastHash(key, strlen(key) + 1);
    printf("\nHash of key %s = %u ", key, hash);

    key = "192.168.101.11:8001";
    hash = SuperFastHash(key, strlen(key) + 1);
    printf("\nHash of key %s = %u ", key, hash);
    
    CU_ASSERT(2 * 2 == 4);
}

int hashSuiteAdd() {
    
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    pSuite = CU_add_suite("TR-UDP hash", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "check hash function", check_hash)) 
     //|| (NULL == CU_add_test(pSuite, "test2", test2))) {
            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    return 0;
}
