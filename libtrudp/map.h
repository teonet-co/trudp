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

/* 
 * File:   map.h
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on June 6, 2016, 12:26 PM
 */

#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif
    
#define HASH_TABLE_SIZE 100
#define HASH_TABLE_INITVAL 77557755
    
typedef struct trudpMapData {
    
    size_t length;
    trudpQueue **q;
    uint32_t collisions;
    size_t hash_map_size;
    
} trudpMapData;    

trudpMapData *trudpMapNew(size_t size);
void trudpMapDestroy(trudpMapData *ht);
void *trudpMapAdd(trudpMapData *ht, void *key, size_t key_length, void *data, size_t data_length);
void *trudpMapGet(trudpMapData *ht, void *key, size_t key_length, size_t *data_length);
int trudpMapDelete(trudpMapData *map, void *key, size_t key_length);

size_t trudpMapSize(trudpMapData *ht);


#ifdef __cplusplus
}
#endif

#endif /* MAP_H */

