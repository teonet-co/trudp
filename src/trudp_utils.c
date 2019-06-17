/*
 * The MIT License
 *
 * Copyright 2016-2018 Kirill Scherba <kirill@scherba.ru>.
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
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
//#include <time.h>


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

// \todo vformatMessage does not work under MinGW
#define KSN_BUFFER_SM_SIZE 256; //2048;//256

static char *vformatMessage(const char *fmt, va_list ap) {

    int size = KSN_BUFFER_SM_SIZE; /* Guess we need no more than 100 bytes */
    char *p, *np;
    va_list ap_copy;
    int n;

    if((p = malloc(size)) == NULL) return NULL;

    while(1) {

        // Try to print in the allocated space
        va_copy(ap_copy,ap);
        n = vsnprintf(p, size, fmt, ap_copy);
//        printf("n = %d\n", n);
        va_end(ap_copy);

        // Check error code
        //if(n < 0) return NULL;

        // If that worked, return the string
        if(n > 0 && n < size) return p;

        // Else try again with more space
        size = size + KSN_BUFFER_SM_SIZE; // Precisely what is needed
        if((np = realloc(p, size)) == NULL) {
            free(p);
            return NULL;
        }
        else p = np;
    }
}

/**
 * Create formated message in new null terminated string
 *
 * @param fmt Format string like in printf function
 * @param ... Parameter
 *
 * @return Null terminated string, should be free after using or NULL on error
 */
char *formatMessage(const char *fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    char *p = vformatMessage(fmt, ap);
    va_end(ap);

    return p;
}

/**
 * Create formated message in new null terminated string, and free str_to_free
 *
 * @param str_to_free
 * @param fmt Format string like in printf function
 * @param ... Parameter
 *
 * @return Null terminated string, should be free after using or NULL on error
 */
char *sformatMessage(char *str_to_free, const char *fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    char *p = vformatMessage(fmt, ap);
    va_end(ap);

    if(str_to_free != NULL) free(str_to_free);

    return p;
}

/**
 * Convert uSec time to timeval structure
 *
 * @param tv [out] Pointer to struct timeval to save time to
 * @param usec Time in uSec
 *
 * @return Pointer to the input struct timeval
 */
struct timeval *usecToTv(struct timeval *tv, uint32_t usec) {

    if(usec) {
        tv->tv_sec  = usec / 1000000;
        tv->tv_usec = usec % 1000000;
    } else {
        tv->tv_sec  = 0;
        tv->tv_usec = 0;
    }

    return tv;
}
