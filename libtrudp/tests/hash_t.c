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
    printf("\nHash of key %s = %010u ", key, hash);
    hash = hash_f(key, strlen(key) + 1, 0);
    printf("\nHash of key %s = %010u ", key, hash);
    
    key = "127.0.0.1:8001";
    hash = SuperFastHash(key, strlen(key) + 1);
    printf("\nHash of key %s = %010u ", key, hash);
    hash = hash_f(key, strlen(key) + 1, 0);
    printf("\nHash of key %s = %010u ", key, hash);

    key = "192.168.101.11:8000";
    hash = SuperFastHash(key, strlen(key) + 1);
    printf("\nHash of key %s = %010u ", key, hash);
    hash = hash_f(key, strlen(key) + 1, 0);
    printf("\nHash of key %s = %010u ", key, hash);

    key = "192.168.101.11:8001";
    hash = SuperFastHash(key, strlen(key) + 1);
    printf("\nHash of key %s = %010u ", key, hash);
    hash = hash_f(key, strlen(key) + 1, 0);
    printf("\nHash of key %s = %010u ", key, hash);
    
    CU_ASSERT(2 * 2 == 4);
}

// check hash table
void check_hash_table() {
    
    trudpHashTdata *th = trudpHashTnew(10);
    CU_ASSERT_PTR_NOT_NULL(th);
    
    char *key = "127.0.0.1:8000";
    size_t key_length = strlen(key) + 1;
    char *data = "Hello TR-UDP hash table!";
    size_t data_length = strlen(data) + 1;
    trudpHashTAdd(th, key, key_length, data, data_length);
    
    size_t d_length;
    void *d = trudpHashTGet(th, key, key_length, &d_length);
    CU_ASSERT_FATAL(d != (void*)-1)
    CU_ASSERT_STRING_EQUAL(d, data);
    
    trudpHashTdestroy(th);
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
     || (NULL == CU_add_test(pSuite, "check hash table", check_hash_table)) 
            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    return 0;
}
