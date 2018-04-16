/*
 * File:   map_t.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on Jun 5, 2016, 4:32:13 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "map.h"
#include "hash.h"

/*
 * CUnit Test Suite
 */

int init_suite(void);
int clean_suite(void);

long long timeInMilliseconds(void) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

// Check hash function
void check_hash() {
    
    char *key = "127.0.0.1:8000";
    uint32_t hash = teoHashSuperFast(key, strlen(key) + 1);
    printf("\n\tHash1 of key %s = %010u ", key, hash);
    hash = teoHashFast((ub1*)key, strlen(key) + 1, 0);
    printf("\n\tHash2 of key %s = %010u ", key, hash);
    
    key = "127.0.0.1:8001";
    hash = teoHashSuperFast(key, strlen(key) + 1);
    printf("\n\tHash1 of key %s = %010u ", key, hash);
    hash = teoHashFast((ub1*)key, strlen(key) + 1, 0);
    printf("\n\tHash2 of key %s = %010u ", key, hash);

    key = "192.168.101.11:8000";
    hash = teoHashSuperFast(key, strlen(key) + 1);
    printf("\n\tHash1 of key %s = %010u ", key, hash);
    hash = teoHashFast((ub1*)key, strlen(key) + 1, 0);
    printf("\n\tHash2 of key %s = %010u ", key, hash);

    key = "192.168.101.11:8001";
    hash = teoHashSuperFast(key, strlen(key) + 1);
    printf("\n\tHash1 of key %s = %010u ", key, hash);
    hash = teoHashFast((ub1*)key, strlen(key) + 1, 0);
    printf("\n\tHash2 of key %s = %010u \n   ", key, hash);
    
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
void _check_map(const size_t NUM_KEYS) {
    
    int i;
    //const size_t NUM_KEYS = 250000;

    srand(timeInMilliseconds());

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
                "Hello Teo ccl hash table - %d!", i) + 1;
        
        //printf("\n %s - \"%s\" ", key[i], data[i]);
    }
    
    // Create new map
    teoMapData *map = teoMapNew(NUM_KEYS, 1);
    CU_ASSERT_PTR_NOT_NULL(map);
    
    // Add and Get data from map
    uint64_t t_beg = timeInMilliseconds();    
    for(i = 0; i < NUM_KEYS; i++) {

        // Add to map
        teoMapAdd(map, key[i], key_length[i], data[i], data_length[i]);

        // Get from map
        size_t d_length;
        void *d = teoMapGet(map, key[i], key_length[i], &d_length);
        CU_ASSERT_FATAL(d != (void*)-1);
        CU_ASSERT_EQUAL_FATAL(data_length[i], d_length);
        CU_ASSERT_STRING_EQUAL(d, data[i]);
    }
    CU_ASSERT_EQUAL(NUM_KEYS, teoMapSize(map));
    printf("\n\t%d records add/get, time: %.3f ms, number of collisions: %u ", 
            (int)NUM_KEYS, (timeInMilliseconds() - t_beg) / 1000.0, map->collisions );
    
    // Update data of existing key value (add existing key)
    char *data_new = "This is new data for this key ...";
    size_t data_new_length = strlen(data_new) + 1;    
    // Add to map
    teoMapAdd(map, key[1], key_length[1], data_new, data_new_length);  
    
    // Get updated key from map
    size_t d_length;
    void *d = teoMapGet(map, key[1], key_length[1], &d_length);
    CU_ASSERT_FATAL(d != (void*)-1);
    CU_ASSERT_EQUAL_FATAL(data_new_length, d_length);
    CU_ASSERT_STRING_EQUAL(d, data_new);
    
    // Delete key from map
    int rv = teoMapDelete(map, key[1], key_length[1]);
    CU_ASSERT(!rv);
    CU_ASSERT_EQUAL(NUM_KEYS - 1, teoMapSize(map));
    
    // Add deleted key
    teoMapAdd(map, key[1], key_length[1], data[1], data_length[1]);
    
    // Loop through map using iterator
    t_beg = timeInMilliseconds();
    // Create map iterator
    teoMapIterator *it = teoMapIteratorNew(map);
    CU_ASSERT_PTR_NOT_NULL_FATAL(it);
    
    i = 0;
    //printf("\n display %d records by iterator loop: \n", (int)map->length);
    while(teoMapIteratorNext(it)) {
        i++;
        teoMapElementData *el = teoMapIteratorElement(it);
        size_t key_lenth;
        /*void *key =*/ teoMapIteratorElementKey(el, &key_lenth);
        size_t data_lenth;
        /*void *data =*/ teoMapIteratorElementData(el, &data_lenth);
        //printf("\n #%d, idx: %u, hash: %010u, key: %s, data: %s ", 
        //       i, it->idx, el->hash, (char*)key, (char*)data);        
    }
    CU_ASSERT(i == map->length);
    // Destroy map iterator
    teoMapIteratorDestroy(it);
    
    printf("\n\t%d records read in iterator loop, time: %.3f ms \n   ", 
            (int)map->length, (timeInMilliseconds() - t_beg) / 1000.0);
        
    // Destroy map
    teoMapDestroy(map);
    
    // Free keys and data
    for(i = 0; i < NUM_KEYS; i++) { free(key[i]); free(data[i]); }
    free(key);
    free(data);
    free(key_length);
    free(data_length);
}

void check_map() {
    size_t num = 11, mul = 6, num_keys = 55;
    
    for(int i=0; i < num; i++, num_keys = (i < (num + 1)/2 ? num_keys * mul : num_keys / mul ))
        _check_map(num_keys);
}

// Binary keys check
void check_binary_key() {
    
    const size_t NUM_KEYS = 20;
    
    // Create new map
    teoMapData *map = teoMapNew(NUM_KEYS, 1);
    CU_ASSERT_PTR_NOT_NULL(map);
    
    int key = 25;
    int data = 125;
    
    // Add to map
    teoMapAdd(map, &key, sizeof(key) , &data, sizeof(data));
    
    // Get from map
    size_t d_length;
    void *d = teoMapGet(map, &key, sizeof(key), &d_length);
    CU_ASSERT_FATAL(d != (void*)-1);
    CU_ASSERT_EQUAL_FATAL(sizeof(data), d_length);
    CU_ASSERT_EQUAL(*(int*)d, data);

    // Destroy map
    teoMapDestroy(map);        
}

int mapSuiteAdd() {
    
    CU_pSuite pSuite = NULL;

    /* Add a suite to the registry */
    pSuite = CU_add_suite("Teo ccl hash map", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "check hash functions", check_hash)) 
     || (NULL == CU_add_test(pSuite, "check map functions", check_map)) 
     || (NULL == CU_add_test(pSuite, "check binary keys", check_binary_key)) 
            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    return 0;
}
