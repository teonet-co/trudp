/*
 * The MIT License
 *
 * Copyright 2016 Kirill Scherba <kirill@scherba.ru>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "map.h"

// Local functions
static inline trudpQueueData *_trudpMapValueDataToQueueData(trudpMapValueData *mvd);

trudpMapData *trudpMapNew(size_t size) {

    int i;
    trudpMapData *map = (trudpMapData *)malloc(sizeof(trudpMapData));

    // Fill parameters
    map->q = (trudpQueue **)malloc(size * sizeof(trudpQueue*));
    map->hash_map_size = size;
    map->collisions = 0;
    map->length = 0;

    // Create Hash table
    for(i = 0; i < size; i++) map->q[i] = trudpQueueNew();

    return map;
}

trudpMapData *trudpMapResize(trudpMapData *map, size_t size) {

    // Show mime of  resize for testing
    #define _SHOW_MSG_ 1
    #if _SHOW_MSG_
    printf("\n resize map from %d to %d, time: ", (int)map->hash_map_size, (int)size);
    uint32_t t_stop, t_beg = trudpGetTimestamp();
    #endif

    int i = 0;
    trudpMapData *map_new = trudpMapNew(size); // (trudpMapData *)malloc(sizeof(trudpMapData));

    // Loop through existing map and add it elements to new map
    trudpMapIterator *it = trudpMapIteratorNew(map);
    while(trudpMapIteratorNext(it)) {
        trudpMapValueData *el = trudpMapIteratorElement(it);

        // \todo Show 9-n records instead of 31
        printf("\n #%d hash: %010u, key: %s, value: %s ", i, el->hash, el->data, el->data + el->key_length);
//        i++;
//        continue;
        
        trudpQueueData *qd = _trudpMapValueDataToQueueData(el);
        int idx = el->hash % map_new->hash_map_size;
        trudpQueuePut(map_new->q[idx], qd);
        map_new->length++;
        

//        size_t key_length;
//        void *key = trudpMapIteratorElementKey(el, &key_length);
//        size_t data_length;
//        void *data = trudpMapIteratorElementData(el, &data_length);
//        trudpMapAdd(map_new, key, key_length, data, data_length);

        i++;
    }

    // Destroy map iterator
    trudpMapIteratorDestroy(it);

    // Free existing queues and move queues pointer of new map to existing
    for(i = 0; i < map->hash_map_size; i++) {
        //trudpQueueFree(map->q[i]);
        free(map->q[i]);
    }
    free(map->q);
    map->q = map_new->q;
    map->hash_map_size = size;
    map->collisions = 0;
    map->length = map_new->length;
    free(map_new);

    #if _SHOW_MSG_
    t_stop = (trudpGetTimestamp() - t_beg);
    printf("%.3f ms ", t_stop / 1000.0);
    #endif

    return map;
}

void trudpMapDestroy(trudpMapData *map) {

    if(map) {

        int i;
        for(i = 0; i < map->hash_map_size; i++) {
            trudpQueueDestroy(map->q[i]);
        }
        free(map->q);
        free(map);
    }
}

/**
 * Calculate hash for the key
 *
 * @param key Pointer to key
 * @param key_length Key length
 * @return
 */
static inline uint32_t trudpMapHash(void *key, size_t key_length) {

    // Select one of several hash functions
    #define _USE_HASH_ 0
    #if _USE_HASH_ == 0
    uint32_t hash = hash_f(key, key_length, HASH_TABLE_INITVAL);
    #elif _USE_HASH_ == 1
    uint32_t hash = SuperFastHash(key, key_length);
    #endif

    return hash;
}

/**
 * Get key data from hash table
 *
 * @param map Pointer to trudpHashTdata
 * @param key Key
 * @param key_length Key length
 * @param hash Hash of key
 * @param data_length [out] Pointer to data length
 *
 * @return Pointer to Data of selected key or (void*)-1 if not found
 */
static void *_trudpMapGet(trudpMapData *map, void *key, size_t key_length,
        uint32_t hash, size_t *data_length) {

    void *data = (void*)-1;
    *data_length = 0;

    int idx = hash % map->hash_map_size;
    trudpMapValueData *htd;
    trudpQueueData *tqd;
    trudpQueueIterator *it = trudpQueueIteratorNew(map->q[idx]);
    if(it != NULL) {
      while((tqd = trudpQueueIteratorNext(it))) {

        htd = (trudpMapValueData *)tqd->data;
        if(htd->hash == hash) {

            if(key_length == htd->key_length &&
               !memcmp(htd->data, key, key_length)) {

                *data_length = htd->data_length;
                data = htd->data + htd->key_length;
                break;
            }
            else map->collisions++;
        }
      }
      trudpQueueIteratorFree(it);
    }

    return data;
}

/**
 * Get pointer to trudpMapValueData from data pointer returned by trudpMapGet
 * @param tqd_data
 * @return
 */
static inline trudpMapValueData *_trudpMapGetValueData(void *tqd_data,
        uint32_t key_length) {

    return tqd_data - key_length - sizeof(trudpMapValueData);
}

/**
 * Get pointer to maps queue data from pointer to trudpMapValueData
 * @param mvd
 * @return
 */
static inline trudpQueueData *_trudpMapValueDataToQueueData(trudpMapValueData *mvd) {
    return mvd ? (trudpQueueData *)((void*)mvd - sizeof(trudpQueueData)) : NULL;
}

/**
 * Add (or update) key data to the map
 *
 * @param map
 * @param key
 * @param key_length
 * @param data
 * @param data_length
 * @return Data of added key or (void*)-1 at error
 */
void *trudpMapAdd(trudpMapData *map, void *key, size_t key_length, void *data,
        size_t data_length) {

    void *r_data = (void*)-1;

    if(!data) data_length = 0;

    // Create and fill Data structure
    size_t htd_length = sizeof(trudpMapValueData) + key_length + data_length;
    trudpMapValueData *htd = (trudpMapValueData *) malloc(htd_length);
    htd->hash = trudpMapHash(key, key_length);
    htd->key_length = key_length;
    htd->data_length = data_length;
    memcpy(htd->data, key, key_length);
    if(data_length) memcpy(htd->data + htd->key_length, data, data_length);

    // Check that key exist and add data to map if not exists
    void *tqd_data = NULL;
    size_t d_length;
    trudpQueueData *tqd;
    // Add data to map
    if((tqd_data = _trudpMapGet(map, key, key_length, htd->hash, &d_length)) == (void*) -1) {
        int idx = htd->hash % map->hash_map_size;
        tqd = trudpQueueAdd(map->q[idx], (void*)htd, htd_length);
        if(tqd) {
            map->length++;

            // Resize if needed
            if(map->length > map->hash_map_size * 3)
                trudpMapResize(map, map->hash_map_size * 10);
        }
    }
    // Update existing key data
    else {
        trudpMapValueData *htd_existing = _trudpMapGetValueData(tqd_data, key_length);
        tqd = _trudpMapValueDataToQueueData(htd_existing);
        int idx = htd->hash % map->hash_map_size;
        tqd = trudpQueueUpdate(map->q[idx], (void*)htd, htd_length, tqd);
    }

    // Free allocated data
    free(htd);

    // Set pointers to trudpMapValueData and returned data
    if(tqd) {
        htd = (trudpMapValueData *)tqd->data;
        r_data = htd->data_length ? htd->data + htd->key_length : NULL;
    }

    return r_data;
}

/**
 * Get key data from hash table
 *
 * @param map Pointer to trudpMapData
 * @param key Key
 * @param key_length Key length
 * @param data_length [out] Pointer to data length
 *
 * @return Data of selected key or (void*)-1 if not found
 */
void *trudpMapGet(trudpMapData *map, void *key, size_t key_length,
        size_t *data_length) {

    uint32_t hash = trudpMapHash(key, key_length);
    void *data = _trudpMapGet(map, key, key_length, hash, data_length);

    return (void*)-1 || _trudpMapGetValueData(data, key_length)->data_length ? data : NULL;

}

/**
 * Delete keys element from map
 *
 * @param map
 * @param key
 * @param key_length
 * @return Zero at success, or errors: -1 - keys element not found
 */
int trudpMapDelete(trudpMapData *map, void *key, size_t key_length) {

    int rv = -1;

    size_t data_length;
    uint32_t hash = trudpMapHash(key, key_length);
    void *data = _trudpMapGet(map, key, key_length, hash, &data_length);
    if(data != (void*)-1) {
        trudpMapValueData *mvd = _trudpMapGetValueData(data, key_length);
        trudpQueueData *tqd = _trudpMapValueDataToQueueData(mvd);
        int idx = mvd->hash % map->hash_map_size;
        rv = trudpQueueDelete(map->q[idx], tqd);
        if(!rv) {
            map->length--;

            // Resize if needed
            if(map->hash_map_size > 10 && map->length < (map->hash_map_size / 10) * 3)
                trudpMapResize(map, map->hash_map_size / 10);

        }
    }

    return rv;
}

/**
 * Get number of elements in TR-UPD map
 *
 * @param map Pointer to trudpMapData
 *
 * @return Number of elements in TR-UPD map
 */
inline size_t trudpMapSize(trudpMapData *map) {

    return map ? map->length : -1;
}

trudpMapIterator *trudpMapIteratorNew(trudpMapData *map) {

    trudpMapIterator *map_it = (trudpMapIterator*)malloc(sizeof(trudpMapIterator));
    if(map_it) {
        map_it->it = trudpQueueIteratorNew(map->q[0]);
        map_it->idx = 0;
        map_it->map = map;
        map_it->tmv = NULL;
    }

    return map_it;
}

int trudpMapIteratorDestroy(trudpMapIterator *map_it) {

    if(map_it) {
        trudpQueueIteratorFree(map_it->it);
        free(map_it);
    }

    return 0;
}

trudpMapValueData *trudpMapIteratorNext(trudpMapIterator *map_it) {

    if(!map_it) return NULL;

    trudpQueueData *tqd;
    trudpMapValueData *tmv = NULL;

    while(!(tqd = trudpQueueIteratorNext(map_it->it))) {
        if(++map_it->idx < map_it->map->hash_map_size) {
            trudpQueueIteratorReset(map_it->it, map_it->map->q[map_it->idx]);
        }
        else break;
    }

    if(tqd) {
        tmv = (trudpMapValueData *)tqd->data;
        map_it->tmv = tmv;
    }

    return tmv;
}

inline trudpMapValueData *trudpMapIteratorElement(trudpMapIterator *map_it) {
    return map_it ? map_it->tmv : NULL;
}

inline void *trudpMapIteratorElementKey(trudpMapValueData *tmv, size_t *key_length) {

    *key_length = tmv->key_length;
    return tmv->data;
}

inline void *trudpMapIteratorElementData(trudpMapValueData *tmv, size_t *data_length) {

    *data_length = tmv->data_length;
    return tmv->data + tmv->key_length;
}
