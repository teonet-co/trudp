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

/**
 * UDP client server helper module
 */

#if defined(_MSC_VER)
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "udp.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#include "teobase/socket.h"
#include "teobase/types.h"
#include "teobase/logging.h"
#include "teobase/platform.h"

#include "trudp_utils.h"
#include "trudp_options.h"

// Global teocli options
extern bool trudpOpt_DBG_sendto;
extern bool trudpOpt_DBG_dumpUdpData;
extern trudpUdpDataSentCallback_t trudpOpt_STAT_udpDataSentCallback;
extern trudpUdpDataReceivedCallback_t trudpOpt_STAT_udpDataReceivedCallback;

// UDP / UDT functions
#define _trudpUdpSocket(domain, type, protocol) socket(domain, type, protocol)
#define _trudpUdpBind(fd, addr, addr_len) bind(fd, addr, addr_len)
#define NUMBER_OF_TRY_PORTS 1000

// Local functions
static void _trudpCallUdpDataSentCallback(int bytes_sent);
static void _trudpCallUdpDataReceivedCallback(int bytes_received);
#ifdef RESERVED
static int  _trudpUdpIsReadable(int sd, uint32_t timeOut);
static int  _trudpUdpIsWritable(int sd, uint32_t timeOut);
static ssize_t _trudpUdpReadEventLoop(int fd, void *buffer, size_t buffer_size,
        __SOCKADDR_ARG remaddr, socklen_t *addr_length, int timeout);
#endif

/**
 * Set socket or FD to non blocking mode
 *
 * @param fd
 */
void trudpUdpSetNonblock(int fd) { // deprecated

    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    //-------------------------
    // Set the socket I/O mode: In this case FIONBIO
    // enables or disables the blocking mode for the
    // socket based on the numerical value of iMode.
    // If iMode = 0, blocking is enabled;
    // If iMode != 0, non-blocking mode is enabled.

    int iResult;
    u_long iMode = 1;

    iResult = ioctlsocket(fd, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
      printf("ioctlsocket failed with error: %ld\n", iResult);

    #else
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    #endif
}


void _trudpCallUdpDataSentCallback(int bytes_sent) {
    trudpUdpDataSentCallback_t callback_copy =
        trudpOpt_STAT_udpDataSentCallback;

    if (callback_copy != NULL) {
        callback_copy(bytes_sent);
    }
}

void _trudpCallUdpDataReceivedCallback(int bytes_received) {
    trudpUdpDataReceivedCallback_t callback_copy =
        trudpOpt_STAT_udpDataReceivedCallback;

    if (callback_copy != NULL) {
        callback_copy(bytes_received);
    }
}


/**
 * Make address from the IPv4 numbers-and-dots notation and integer port number
 * into binary form
 *
 * @param addr
 * @param port
 * @param remaddr
 * @param addr_length
 * @return
 */
int trudpUdpMakeAddr(const char *addr, int port, __SOCKADDR_ARG remaddr, socklen_t *len) {
    struct addrinfo hints, *res;
    char port_ch[10];
    sprintf(port_ch, "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    int status = getaddrinfo(addr, port_ch, &hints, &res);
    if (status != 0) {
        LTRACK_E("trudpUdpMakeAddr", "getnameinfo: %s", gai_strerror(status));
        return -1;
    }
    *len = res->ai_addrlen;
    memset(remaddr, 0, *len);
    memcpy(remaddr, res->ai_addr, res->ai_addrlen);

    freeaddrinfo(res);

    return 0;
}

/**
 * Get address and port from address structure
 *
 * @param[in] remaddr
 * @param[out] port Pointer to port to get port integer
 * @return Pointer to address string(must be freed)
 */
 const char *trudpUdpGetAddr(__CONST_SOCKADDR_ARG remaddr, socklen_t remaddr_len, int *port) {
    char host[NI_MAXHOST] = { 0 };
    char service[NI_MAXSERV] = { 0 };

    int s = getnameinfo(remaddr, remaddr_len, host, sizeof(host), service, NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV);
    if (s != 0) {
        LTRACK_E("trudpUdpGetAddr", "getnameinfo: %s", gai_strerror(s));
    }
    (void)s;// \TODO: need to handle the error code

    size_t addr_len = strlen(host)+1;
    char *addr = malloc(addr_len);
    memset(addr, '\0', addr_len);
    memcpy(addr, host, addr_len);

    if(port) {
        *port = atoi(service);
    }

    return (const char *)addr;
}

/**
 * Create and bind UDP socket for CLIENT
 *
 * @param[in,out] port Pointer to Port number
 * @param[in] allow_port_increment_f Allow port increment flag
 * @return File descriptor or error if return value < 0:
 *         -1 - cannot create socket; -2 - can't bind on port
 */
int trudpUdpBindRaw_cli(const char* addr, int *port, int allow_port_increment_f) {
    struct addrinfo hints;
    struct addrinfo *rp;
    struct addrinfo *res;
    memset(&hints, '\0', sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    int fd, s;

    struct sockaddr_in sa;
    int rc = inet_pton(AF_INET, addr, &(sa.sin_addr));
    if (rc == 1) { /* valid IPv4 */
        hints.ai_family = AF_INET;
    } else {
        struct sockaddr_in6 sa6;
        rc = inet_pton(AF_INET6, addr, &(sa6.sin6_addr));
        if (rc == 1) { /* valid IPv6 */
            hints.ai_family = AF_INET6;
        }
    }

    // Bind the socket to any valid IP address and a specific port, increment
    // port if busy
    for (int i = 0;;) {
        char port_ch[10];
        sprintf(port_ch, "%d", *port);

        s = getaddrinfo(NULL, port_ch, &hints, &res);

        if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            return -3;
        }

        for (rp = res; rp != NULL; rp = rp->ai_next) {
            fd = _trudpUdpSocket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (fd == -1) continue;

            if (_trudpUdpBind(fd, rp->ai_addr, rp->ai_addrlen) == 0) {
                goto success_bind;
            }

            teosockClose(fd);
        }

        ++(*port);

        if(allow_port_increment_f && i++ < NUMBER_OF_TRY_PORTS) {
            continue;
        } else {
            freeaddrinfo(res);
            return -2;
        }
    }

success_bind:
    freeaddrinfo(res);
    return fd;
}

/**
 * Create and bind UDP socket for SERVER
 *
 * @param[in,out] port Pointer to Port number
 * @param[in] allow_port_increment_f Allow port increment flag
 * @return File descriptor or error if return value < 0:
 *         -1 - cannot create socket; -2 - can't bind on port
 */
int trudpUdpBindRaw(int *port, int allow_port_increment_f) {
    struct addrinfo hints;
    struct addrinfo *rp;
    struct addrinfo *res;
    memset(&hints, '\0', sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    int fd, s;

    // Bind the socket to any valid IP address and a specific port, increment
    // port if busy
    for (int i = 0;;) {
        char port_ch[10];
        sprintf(port_ch, "%d", *port);

        hints.ai_family = AF_INET6;
        s = getaddrinfo(NULL, port_ch, &hints, &res);

        if (s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            freeaddrinfo(res);
            return -3;
        }

        for (rp = res; rp != NULL; rp = rp->ai_next) {
            fd = _trudpUdpSocket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (fd == -1) continue;

            if (_trudpUdpBind(fd, rp->ai_addr, rp->ai_addrlen) == 0) {

                int off = 0;
                setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&off, sizeof(off));

                goto success_bind;
            }

            teosockClose(fd);
        }

        ++(*port);

        if(allow_port_increment_f && i++ < NUMBER_OF_TRY_PORTS) {
            continue;
        } else {
            freeaddrinfo(res);
            return -2;
        }
    }

success_bind:
    freeaddrinfo(res);
    return fd;
}

/**
 * Simple UDP recvfrom wrapper
 *
 * @param fd Socket descriptor
 * @param buffer Buffer to store received data
 * @param buffer_size The size of buffer
 * @param remaddr Remote address to receive from
 * @param addr_length The length of @a remaddr argument
 * @return The amount of bytes received or -1 on error
 */
teosockRecvfromResult trudpUdpRecvfrom(
    int fd, uint8_t *buffer, size_t buffer_size, __SOCKADDR_ARG remaddr,
    socklen_t *addr_length, size_t *received_length, int *error) {
    if (received_length == NULL) {
        return TEOSOCK_RECVFROM_UNKNOWN_ERROR;
    }

    teosockRecvfromResult recvfrom_result = teosockRecvfrom(fd, buffer, buffer_size, remaddr, addr_length, received_length, error);

    if (recvfrom_result == TEOSOCK_RECVFROM_DATA_RECEIVED) {
        if (trudpOpt_DBG_dumpUdpData) {
            char hexdump[32];
            if (buffer != NULL && buffer_size > 0) {
                dump_bytes(hexdump, sizeof(hexdump), buffer, *received_length);
            } else {
                strcpy(hexdump, "(null)");
            }

            dump_bytes(hexdump, sizeof(hexdump), buffer, *received_length);
            LTRACK("TrUdp",
                   "Received %u bytes using recvfrom() starting with %s",
                   (uint32_t)(*received_length), hexdump);
        }

        _trudpCallUdpDataReceivedCallback(*received_length);
    }

    return recvfrom_result;
}

#ifdef RESERVED
/**
 * Wait while socket read available or timeout occurred
 *
 * @param sd File descriptor
 * @param timeOut Timeout in uSec
 *
 * @return -1 - error; 0 - timeout; >0 ready
 */
static int _trudpUdpIsReadable(int sd, uint32_t timeOut) {

    int rv = 1;

    fd_set socketReadSet;
    FD_ZERO(&socketReadSet);
    FD_SET(sd,&socketReadSet);
    struct timeval tv;
    usecToTv(&tv, timeOut);

    rv = select(sd + 1, &socketReadSet, NULL, NULL, &tv);

    return rv;
}
#endif

#ifdef RESERVED
/**
 * Wait while socket write available or timeout occurred
 *
 * @param sd File descriptor
 * @param timeOut Timeout in uSec
 *
 * @return -1 - error; 0 - timeout; >0 ready
 */
static int _trudpUdpIsWritable(int sd, uint32_t timeOut) {

    int rv = 1;

    fd_set socketWriteSet;
    FD_ZERO(&socketWriteSet);
    FD_SET(sd,&socketWriteSet);
    struct timeval tv;
    usecToTv(&tv, timeOut);

    rv = select(sd + 1, NULL, &socketWriteSet, NULL, &tv);
    //if(rv <= 0) printf("isWritable timeout\n");

    return rv;
}
#endif

/**
 * Simple UDP sendto wrapper
 *
 * @param fd Socket descriptor
 * @param buffer Buffer with data to send
 * @param buffer_size The size of data to send
 * @param remaddr Address to send data to
 * @param addrlen The length of @a remaddr argument
 * @return The amount of bytes sent or -1 on error
 */
 ssize_t trudpUdpSendto(int fd, const uint8_t* buffer, size_t buffer_size,
        __CONST_SOCKADDR_ARG remaddr, socklen_t addr_length) {
    CLTRACK(trudpOpt_DBG_sendto, "TrUdp", "Sending %u bytes using sendto().",
             (uint32_t)buffer_size);

    if (trudpOpt_DBG_dumpUdpData) {
        char hexdump[32];
        if (buffer != NULL && buffer_size > 0) {
            dump_bytes(hexdump, sizeof(hexdump), buffer, buffer_size);
        } else {
            strcpy(hexdump, "(null)");
        }

        LTRACK("TrUdp", "Sending %u bytes using sendto() starting with %s",
               (uint32_t)buffer_size, hexdump);
    }

    ssize_t sendlen = 0;

    //if(waitSocketWriteAvailable(fd, 1000000) > 0) {

    // Write UDP data
    int flags = 0;
    sendlen = sendto(fd, buffer, buffer_size, flags, remaddr, addr_length);
    //}

    if (sendlen == -1) {
#if defined(TEONET_OS_WINDOWS)
        int recv_errno = WSAGetLastError();
#else
        int recv_errno = errno;
#endif
        // TODO: Use thread safe error formatting function.
        // TODO: On Windows use correct error formatting function.
        LTRACK_E("TrUdp",
                 "Sending %u bytes using sendto() failed with error %" PRId32 ": %s",
                 (uint32_t)buffer_size, recv_errno, strerror(recv_errno));
    } else if ((size_t)sendlen != buffer_size) {
        LTRACK_E("TrUdp",
                 "Sending using sendto() sent only %u bytes of %u.",
                 (uint32_t)sendlen, (uint32_t)buffer_size);
    } else {
        _trudpCallUdpDataSentCallback(sendlen);
    }

    return sendlen;
}

#ifdef RESERVED
/**
 * Wait socket data during timeout and call callback if data received
 *
 * @param fd
 * @param buffer
 * @param buffer_size
 * @param remaddr
 * @param addr_length
 * @param timeout Timeout of wait socket read event in ms
 *
 * @return 0 - if disconnected or 1 other way
 */
static ssize_t _trudpUdpReadEventLoop(int fd, void *buffer, size_t buffer_size,
        __SOCKADDR_ARG remaddr, socklen_t *addr_length, int timeout) {

    int rv;
    fd_set rfds;
    struct timeval tv;
    ssize_t recvlen = 0;

    // Watch server_socket to see when it has input.
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    // Wait up to 50 ms. */
    usecToTv(&tv, timeout * 1000);
    rv = select((int)fd + 1, &rfds, NULL, NULL, &tv);

    // Error
    if (rv == -1) printf("select() handle error\n");

    // Timeout
    else if(!rv) { // Idle or Timeout event

        //send_l0_event(con, EV_L_IDLE, NULL, 0);
    }
    // There is a data in fd
    else {
        recvlen = trudpUdpRecvfrom(fd, buffer, buffer_size,
                (__SOCKADDR_ARG)remaddr, addr_length);
    }

    return recvlen;
}
#endif
