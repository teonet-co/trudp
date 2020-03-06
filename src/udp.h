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

/*
 * File:   udp.h
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on June 2, 2016, 12:36 PM
 */

#ifndef UDP_H
#define UDP_H

#if defined(HAVE_MINGW) || defined(_WIN32)

    #define WIN32_LEAN_AND_MEAN
    // TODO: Stop using deprecated functions and remove this define.
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;

    #define __SOCKADDR_ARG struct sockaddr *__restrict
    #define __CONST_SOCKADDR_ARG const struct sockaddr *

    #ifndef _SSIZE_T_DEFINED
    typedef intptr_t ssize_t;
    #define _SSIZE_T_DEFINED
    #endif
#elif defined(__ANDROID__) || defined(__APPLE__)
    #define __SOCKADDR_ARG struct sockaddr *__restrict
    #define __CONST_SOCKADDR_ARG const struct sockaddr *
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
#elif defined(__linux__) && defined(__x86_64__)
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
#endif

#include "trudp_api.h"

#ifdef __cplusplus
extern "C" {
#endif

TRUDP_API ssize_t trudpUdpSendto(int fd, const uint8_t* buffer, size_t buffer_size,
        __CONST_SOCKADDR_ARG remaddr, socklen_t addr_length);
TRUDP_API int trudpUdpBindRaw(int *port, int allow_port_increment_f);
TRUDP_API char *trudpUdpGetAddr(__CONST_SOCKADDR_ARG remaddr, int *port);

TRUDP_API ssize_t trudpUdpRecvfrom(int fd, uint8_t* buffer, size_t buffer_size,
        __SOCKADDR_ARG remaddr, socklen_t *addr_length);
TRUDP_API int trudpUdpMakeAddr(const char *addr, int port, __SOCKADDR_ARG remaddr,
        socklen_t *addr_len);


#ifdef __cplusplus
}
#endif

#endif /* UDP_H */
