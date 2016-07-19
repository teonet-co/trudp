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

/**
 * UDP client server helper module
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>

// C11 present
#if __STDC_VERSION__ >= 201112L
//extern int inet_aton (const char *__cp, struct in_addr *__inp) __THROW;
#endif

#include "udp.h"
#include "utils.h"

// UDP / UDT functions
#define ksn_socket(domain, type, protocol) socket(domain, type, protocol)
#define ksn_bind(fd, addr, addr_len) bind(fd, addr, addr_len)
#define NUMBER_TRY_PORTS 1000

/**
 * Set socket or FD to non blocking mode
 *
 * @param fd
 */
static void set_nonblock(int fd) {

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

/**
 * Convert host name to IP
 * 
 * @param remaddr
 * @param server
 */
static void hostToIp(struct sockaddr_in *remaddr, const char *server) {

    struct hostent *hostp;

    if((remaddr->sin_addr.s_addr = inet_addr(server)) == 
            (unsigned long)INADDR_NONE) {

        /* When passing the host name of the server as a */
        /* parameter to this program, use the gethostbyname() */
        /* function to retrieve the address of the host server. */
        /***************************************************/
        /* get host address */
        hostp = gethostbyname(server);
        if(hostp == (struct hostent *)NULL) {
            // ...
        }
        else memcpy(&remaddr->sin_addr, hostp->h_addr, sizeof(remaddr->sin_addr));
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
int trudpUdpMakeAddr(const char *addr, int port, __SOCKADDR_ARG remaddr,
        socklen_t *addr_length) {

    if(*addr_length < sizeof(struct sockaddr_in)) return -3;

    *addr_length = sizeof(struct sockaddr_in); // length of addresses
    memset((void *)remaddr, 0, *addr_length);
    ((struct sockaddr_in*)remaddr)->sin_family = AF_INET;
    ((struct sockaddr_in*)remaddr)->sin_port = htons(port);
//    #ifndef HAVE_MINGW
//    if(inet_aton(addr, &((struct sockaddr_in*)remaddr)->sin_addr) == 0) {
//        return(-2);
//    }
//    #else
    //((struct sockaddr_in*)remaddr)->sin_addr.s_addr = inet_addr(addr);
    hostToIp((struct sockaddr_in*)remaddr, addr);
//    #endif

    return 0;
}

/**
 * Get address and port from address structure
 *
 * @param remaddr
 * @param port Pointer to port to get port integer
 * @return Pointer to address string
 */
inline char *trudpUdpGetAddr(__CONST_SOCKADDR_ARG remaddr, int *port) {

    char *addr = inet_ntoa(((struct sockaddr_in*)remaddr)->sin_addr); // IP to string
    if(port) *port = ntohs(((struct sockaddr_in*)remaddr)->sin_port); // Port to integer

    return addr;
}

/**
 * Create and bind UDP socket for client/server
 *
 * @param[in,out] port Pointer to Port number
 * @param[in] allow_port_increment_f Allow port increment flag
 * @return File descriptor or error if return value < 0:
 *         -1 - cannot create socket; -2 - can't bind on port
 */
int trudpUdpBindRaw(int *port, int allow_port_increment_f) {

    int i, fd;
    struct sockaddr_in addr;	// Our address

    // Create an UDP socket
    if((fd = ksn_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) <= 0) {
        perror("cannot create socket\n");
        return -1;
    }

    memset((char *)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to any valid IP address and a specific port, increment
    // port if busy
    for(i=0;;) {

        addr.sin_port = htons(*port);

        // Try to bind
        if(ksn_bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            fprintf(
                stderr,
                "can't bind on port %d, try next port number ...\n",
                *port
            );
            (*port)++;
            if(allow_port_increment_f && i++ < NUMBER_TRY_PORTS) continue;
            else return -2;
        }

        // Bind successfully
        else {
            set_nonblock(fd);
            break;
        }
    }

    return fd;
}

/**
 * Simple UDP recvfrom wrapper
 *
 * @param fd
 * @param buffer
 * @param buffer_size
 * @param remaddr
 * @param addr_length
 * @return 
 */
inline ssize_t trudpUdpRecvfrom(int fd, void *buffer, size_t buffer_size,
        __SOCKADDR_ARG remaddr, socklen_t *addr_length) {

    int flags = 0;

    // Read UDP data
    ssize_t recvlen = recvfrom(fd, buffer, buffer_size, flags,
            (__SOCKADDR_ARG)remaddr, addr_length);

    return recvlen;
}

/**
 * Wait while socket read available or timeout occurred
 *
 * @param sd File descriptor
 * @param timeOut Timeout in uSec
 *
 * @return -1 - error; 0 - timeout; >0 ready
 */
int isReadable(int sd, uint32_t timeOut) {

    int rv = 1;

    fd_set socketReadSet;
    FD_ZERO(&socketReadSet);
    FD_SET(sd,&socketReadSet);
    struct timeval tv;
    usecToTv(&tv, timeOut);

    rv = select(sd + 1, &socketReadSet, NULL, NULL, &tv);

    return rv;
}

/**
 * Wait while socket write available or timeout occurred
 *
 * @param sd File descriptor
 * @param timeOut Timeout in uSec
 *
 * @return -1 - error; 0 - timeout; >0 ready
 */
int isWritable(int sd, uint32_t timeOut) {

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

/**
 * Simple UDP sendto wrapper
 * 
 * @param fd File descriptor
 * @param buffer
 * @param buffer_size
 * @param remaddr
 * @param addrlen
 * @return
 */
inline ssize_t trudpUdpSendto(int fd, void *buffer, size_t buffer_size,
        __CONST_SOCKADDR_ARG remaddr, socklen_t addrlen) {

    ssize_t sendlen = 0;

    //if(waitSocketWriteAvailable(fd, 1000000) > 0) {

    // Write UDP data
    int flags = 0;
    sendlen = sendto(fd, buffer, buffer_size, flags, remaddr, addrlen);
    //}

    return sendlen;
}

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
ssize_t trudpUdpReadEventLoop(int fd, void *buffer, size_t buffer_size,
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
