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

// Utilities functions ========================================================

#include <stdio.h>

#include "trudp_utils.h"
#include "trudp_const.h"

/**
 * Make TR-UDP map key
 *
 * @param addr String with IP address
 * @param port Port number
 * @param channel Cannel number 0-15
 * @param key_length [out] Pointer to keys length (may be NULL)
 *
 * @return Static buffer with key ip:port:channel
 */
char *trudpMakeKey(char *addr, int port, int channel, size_t *key_length) 
{

    static char buf[MAX_KEY_LENGTH];
    size_t kl = snprintf(buf, MAX_KEY_LENGTH, "%s:%u:%u", addr, port, channel);
    if(key_length) *key_length = kl;

    return buf;
}
