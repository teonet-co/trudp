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
#include <string.h>
#include <fcntl.h>

#include "udp.h"


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
 * Make address from the IPv4 numbers-and-dots notation and integer port number
 * into binary form
 *
 * @param addr
 * @param port
 * @param remaddr
 * @param addr_len
 * @return
 */
int trudpUdpMakeAddr(const char *addr, int port, __SOCKADDR_ARG remaddr,
        socklen_t *addr_len) {

    if(*addr_len < sizeof(struct sockaddr_in)) return -3;
    *addr_len = sizeof(struct sockaddr_in); // length of addresses
    memset((char *) remaddr, 0, *addr_len);
    ((struct sockaddr_in*)remaddr)->sin_family = AF_INET;
    ((struct sockaddr_in*)remaddr)->sin_port = htons(port);
    #ifndef HAVE_MINGW
    if(inet_aton(addr, &((struct sockaddr_in*)remaddr)->sin_addr) == 0) {
        return(-2);
    }
    #else
    ((struct sockaddr_in*)remaddr)->sin_addr.s_addr = inet_addr(addr);
    #endif

    return 0;
}

/**
 * Create and bind UDP socket for client/server
 * 
 * @param[in][out] port Pointer to Port number
 * @param[in] allow_port_inc_f Allow port increment flag
 * @return File descriptor or error if return value < 0: 
 *         -1 - cannot create socket; -2 - can't bind on port
 */
int trudpUdpBindRaw(int *port, int allow_port_increment_f) {
    
    int i, fd;
    struct sockaddr_in addr;	// Our address 
    
    // Create an UDP socket
    if((fd = ksn_socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
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

            fprintf(stderr, "can't bind on port %d, try next port number ...\n", *port);                    
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
 * @return 
 */
ssize_t trudpUdpRecvfrom(int fd, void *buffer, size_t buffer_size, 
        __SOCKADDR_ARG remaddr, socklen_t *addr_len) {

    int flags = 0;
    
    // Read UDP data
    ssize_t recvlen = recvfrom(fd, buffer, buffer_size, flags, (__SOCKADDR_ARG)remaddr, addr_len);
    
//    if(recvlen > 0) {
//        *addr = strdup(inet_ntoa(((struct sockaddr_in)remaddr).sin_addr)); // IP to string
//        *port = ntohs(((struct sockaddr_in)remaddr).sin_port); // Port to integer
//    }
    
    return recvlen;
}    

/**
 * Simple UDP sendto wrapper
 * @param fd
 * @param buffer
 * @param buffer_size
 * @param addr
 * @param port
 * @return 
 */
ssize_t trudpUdpSendto(int fd, void *buffer, size_t buffer_size, 
        __CONST_SOCKADDR_ARG remaddr, socklen_t addrlen) {
    
    int flags = 0;
//    struct sockaddr_in remaddr;         // remote address
//    socklen_t addrlen = sizeof(remaddr);// length of addresses
//    trudpUdpMakeAddr(addr, port, (__SOCKADDR_ARG) &remaddr, &addrlen);
    
    // Write UDP data
    ssize_t sendlen = sendto(fd, buffer, buffer_size, flags, remaddr, addrlen);
    
    return sendlen;
}    
