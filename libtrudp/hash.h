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

uint32_t SuperFastHash (const char * data, int len);


#ifdef __cplusplus
}
#endif

#endif /* HASH_H */

