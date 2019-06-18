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
 *
 * \file   trudp_utils.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on July 21, 2016, 12:33 AM
 */

#ifndef TRUDP_UTILS_H
#define TRUDP_UTILS_H

#include <stddef.h>
#include <time.h>

#if defined(__linux__)
#include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

char *trudpMakeKey(char *addr, int port, int channel, size_t *key_length);
char *formatMessage(const char *fmt, ...);
char *sformatMessage(char *str_to_free, const char *fmt, ...);
struct timeval *usecToTv(struct timeval *tv, uint32_t usec);


#ifdef __cplusplus
}
#endif

#endif /* TRUDP_UTILS_H */
