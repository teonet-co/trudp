/** 
 * \file   hash.h
 * \author kirill
 *
 * Created on June 5, 2016, 4:29 PM
 */

#ifndef HASH_H
#define HASH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    
typedef unsigned long int ub4; /* unsigned 4-byte quantities */
typedef unsigned char ub1; /* unsigned 1-byte quantities */  

uint32_t SuperFastHash (const char * data, int len);
ub4 hash_f(ub1 *k, ub4 length, ub4 initval); /* the previous hash, or an arbitrary value */

#ifdef __cplusplus
}
#endif

#endif /* HASH_H */

