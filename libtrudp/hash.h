/** 
 * \file   hash.h
 * \author kirill
 *
 * Created on June 5, 2016, 4:29 PM
 */

#ifndef HASH_H
#define HASH_H

#include <stdint.h>

#include "queue.h"

#define HASH_TABLE_SIZE 100
#define HASH_TABLE_INITVAL 77557755

#ifdef __cplusplus
extern "C" {
#endif
    
typedef unsigned long int ub4; /* unsigned 4-byte quantities */
typedef unsigned char ub1; /* unsigned 1-byte quantities */  

typedef struct trudpHashTdata {
    
    size_t size;
    trudpQueue *q[];
    
} trudpHashTdata;

uint32_t SuperFastHash (const char * data, int len);
ub4 hash_f(ub1 *k, ub4 length, ub4 initval); /* the previous hash, or an arbitrary value */

trudpHashTdata *trudpHashTnew(size_t size);
void trudpHashTdestroy(trudpHashTdata *ht);
void *trudpHashTAdd(trudpHashTdata *ht, void *key, size_t key_length, void *data, size_t data_length);
void *trudpHashTGet(trudpHashTdata *ht, void *key, size_t key_length, size_t *data_length);

#ifdef __cplusplus
}
#endif

#endif /* HASH_H */

