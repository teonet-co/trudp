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
 * 
 * \file   hash.h
 * \author \author Kirill Scherba <kirill@scherba.ru>
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

uint32_t trudpHashSuperFast (const char * data, int len);
ub4 trudpHashFast(ub1 *k, ub4 length, ub4 initval); /* the previous hash, or an arbitrary value */

#ifdef __cplusplus
}
#endif

#endif /* HASH_H */

