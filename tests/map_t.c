/*
 * File:   map_t.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on Jun 5, 2016, 4:32:13 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "map.h"
#include "hash.h"
#include "packet.h"

/*
 * CUnit Test Suite
 */

int init_suite(void);
int clean_suite(void);

// Check hash function
void check_hash() {
    
    char *key = "127.0.0.1:8000";
    uint32_t hash = trudpHashSuperFast(key, strlen(key) + 1);
    printf("\nHash1 of key %s = %010u ", key, hash);
    hash = trudpHashFast((ub1*)key, strlen(key) + 1, 0);
    printf("\nHash2 of key %s = %010u ", key, hash);
    
    key = "127.0.0.1:8001";
    hash = trudpHashSuperFast(key, strlen(key) + 1);
    printf("\nHash1 of key %s = %010u ", key, hash);
    hash = trudpHashFast((ub1*)key, strlen(key) + 1, 0);
    printf("\nHash2 of key %s = %010u ", key, hash);

    key = "192.168.101.11:8000";
    hash = trudpHashSuperFast(key, strlen(key) + 1);
    printf("\nHash1 of key %s = %010u ", key, hash);
    hash = trudpHashFast((ub1*)key, strlen(key) + 1, 0);
    printf("\nHash2 of key %s = %010u ", key, hash);

    key = "192.168.101.11:8001";
    hash = trudpHashSuperFast(key, strlen(key) + 1);
    printf("\nHash1 of key %s = %010u ", key, hash);
    hash = trudpHashFast((ub1*)key, strlen(key) + 1, 0);
    printf("\nHash2 of key %s = %010u \n   ", key, hash);
    
    CU_ASSERT(2 * 2 == 4);
}

static int randIpOct() {
    
    // 1 - 253
    return (rand() % 253) + 1;
}

static int randPort() {
    
    // 1000 - 17800
    return ( (rand() % (17800 - 1000)) + 1000);
}

#define BUFFER_LEN  64
char ip_str[BUFFER_LEN];

static char* randIpPort() {
        
    snprintf(ip_str, BUFFER_LEN, "%d.%d.%d.%d:%d", randIpOct(), randIpOct(), randIpOct(), randIpOct(), randPort() );
    
    return ip_str;
}

// Check hash table. Add and get several records
void check_map() {
    
    int i;
    const size_t NUM_KEYS = 10000;

    srand(trudpGetTimestamp());

    // Create keys and data
    char **key = malloc(sizeof(char*) * NUM_KEYS);
    size_t *key_length = malloc(sizeof(*key_length) * NUM_KEYS);
    
    char **data = malloc(sizeof(char*) * NUM_KEYS);
    size_t *data_length  = malloc(sizeof(*data_length) * NUM_KEYS);
            
    for(i = 0; i < NUM_KEYS; i++) {
        
        char *k = randIpPort();
        
        key[i] = malloc(BUFFER_LEN);
        key_length[i] = strlen(k) + 1;        
        memcpy(key[i], k, key_length[i]);
        
        data[i] = malloc(BUFFER_LEN);
        data_length[i] = snprintf(data[i], BUFFER_LEN, 
                "Hello TR-UDP hash table - %d!", i) + 1;
        
        //printf("\n %s - \"%s\" ", key[i], data[i]);
    }
    
    // Create new map
    trudpMapData *map = trudpMapNew(NUM_KEYS, 1);
    CU_ASSERT_PTR_NOT_NULL(map);
    
    // Add and Get data from map
    uint32_t t_beg = trudpGetTimestamp();    
    for(i = 0; i < NUM_KEYS; i++) {

        // Add to map
        trudpMapAdd(map, key[i], key_length[i], data[i], data_length[i]);

        // Get from map
        size_t d_length;
        void *d = trudpMapGet(map, key[i], key_length[i], &d_length);
        CU_ASSERT_FATAL(d != (void*)-1);
        CU_ASSERT_EQUAL_FATAL(data_length[i], d_length);
        CU_ASSERT_STRING_EQUAL(d, data[i]);
    }
    CU_ASSERT_EQUAL(NUM_KEYS, trudpMapSize(map));
    printf("\n %d records add/get, time: %.3f ms, number of collisions: %u ", 
            (int)NUM_KEYS, (trudpGetTimestamp() - t_beg) / 1000.0, map->collisions );
    
    // Update data of existing key value (add existing key)
    char *data_new = "This is new data for this key ...";
    size_t data_new_length = strlen(data_new) + 1;    
    // Add to map
    trudpMapAdd(map, key[1], key_length[1], data_new, data_new_length);  
    
    // Get updated key from map
    size_t d_length;
    void *d = trudpMapGet(map, key[1], key_length[1], &d_length);
    CU_ASSERT_FATAL(d != (void*)-1);
    CU_ASSERT_EQUAL_FATAL(data_new_length, d_length);
    CU_ASSERT_STRING_EQUAL(d, data_new);
    
    // Delete key from map
    int rv = trudpMapDelete(map, key[1], key_length[1]);
    CU_ASSERT(!rv);
    CU_ASSERT_EQUAL(NUM_KEYS - 1, trudpMapSize(map));
    
    // Add deleted key
    trudpMapAdd(map, key[1], key_length[1], data[1], data_length[1]);
    
    // Loop through map using iterator
    t_beg = trudpGetTimestamp();
    // Create map iterator
    trudpMapIterator *it = trudpMapIteratorNew(map);
    CU_ASSERT_PTR_NOT_NULL_FATAL(it);
    
    i = 0;
    //printf("\n display %d records by iterator loop: \n", (int)map->length);
    while(trudpMapIteratorNext(it)) {
        i++;
        trudpMapElementData *el = trudpMapIteratorElement(it);
        size_t key_lenth;
        /*void *key =*/ trudpMapIteratorElementKey(el, &key_lenth);
        size_t data_lenth;
        /*void *data =*/ trudpMapIteratorElementData(el, &data_lenth);
        //printf("\n #%d, idx: %u, hash: %010u, key: %s, data: %s ", 
        //       i, it->idx, el->hash, (char*)key, (char*)data);        
    }
    CU_ASSERT(i == map->length);
    // Destroy map iterator
    trudpMapIteratorDestroy(it);
    
    printf("\n %d records read in iterator loop, time: %.3f ms \n   ", 
            (int)map->length, (trudpGetTimestamp() - t_beg) / 1000.0);
        
    // Destroy map
    trudpMapDestroy(map);
    
    // Free keys and data
    for(i = 0; i < NUM_KEYS; i++) { free(key[i]); free(data[i]); }
    free(key);
    free(data);
    free(key_length);
    free(data_length);
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
    if ((NULL == CU_add_test(pSuite, "check hash functions", check_hash)) 
     || (NULL == CU_add_test(pSuite, "check TR-UDP map", check_map)) 
            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    return 0;
}
