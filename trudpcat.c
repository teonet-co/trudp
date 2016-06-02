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

/* 
 * File:   trudpcat.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on June 2, 2016, 1:11 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>

#include "libtrudp/tr-udp.h"

// Integer options
int o_debug = 0,
    o_listen = 0,
    o_numeric = 0;

// String options
char *o_local_address,  
     *o_local_port,
     *o_remote_address = NULL, 
     *o_remote_port;

int o_remote_port_i;

// Application exit code and flags
int exit_code = EXIT_SUCCESS,
    quit_flag = 0;

// Read buffer
const int o_buf_size = 4096;
char buffer[4096];

/**
 * Show error and exit
 * 
 * @param fmt
 * @param ...
 */
static void die(char *fmt, ...)
{
	va_list ap;
	fflush(stdout);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

/**
 * TR-UDP process data callback
 * 
 * @param data
 * @param data_length
 * @param user_data
 */
static void processDataCb(void *td_ptr, void *data, size_t data_length, void *user_data) {
    
    trudpData *td = (trudpData *)td_ptr;
    
    printf("got %d byte data: %s\n\n", (int)data_length, (char*)data);    
}

/**
 * TR-UDP ACK processed callback
 * 
 * @param td
 * @param data
 * @param data_length
 * @param user_data
 */
static void processAckCb(void *td_ptr, void *data, size_t data_length, void *user_data) {
    
    trudpData *td = (trudpData *)td_ptr;
    void *packet = trudpPacketGetPacket(data);
    
    td->triptime = trudpHeaderTimestamp() - trudpPacketGetTimestamp(packet); // Set new triptime
    
    double tt = (td->triptime)/1000.0;

    printf("ACK processed %.3f ms\n", tt );
}

/**
 * TR-UDP write callback
 * 
 * @param data
 * @param data_length
 * @param user_data
 */
static void writeCb(void *td_ptr, void *data, size_t data_length, void *user_data) {
    
    trudpData *td = (trudpData *)td_ptr;
    
    // Write to UDP
    if(td->connected_f) {
        trudpUdpSendto(td->fd, data, data_length, (__CONST_SOCKADDR_ARG) &td->remaddr, sizeof(td->remaddr));
        char *addr = inet_ntoa(td->remaddr.sin_addr); // IP to string
        int port = ntohs(td->remaddr.sin_port); // Port to integer    
        printf("send %d bytes to %s:%d\n", (int)data_length, addr, port);
    }
}

/**
 * The TR-UDP cat network loop
 */
void network_loop(trudpData *td) {
    
    // Read from UDP
    struct sockaddr_in remaddr; // remote address
    socklen_t addr_len = sizeof(remaddr);
    ssize_t recvlen = trudpUdpRecvfrom(td->fd, buffer, o_buf_size, (__SOCKADDR_ARG)&remaddr, &addr_len);    
    
    // Process received packet
    if(recvlen > 0) {
        
        if(!td->connected_f) {
            memcpy(&td->remaddr, &remaddr, addr_len);
            td->addrlen = addr_len;
            td->connected_f = 1;
        }
        
        char *addr = inet_ntoa(remaddr.sin_addr); // IP to string
        int port = ntohs(remaddr.sin_port); // Port to integer       
        printf("got %d bytes from %s:%d\n", (int)recvlen, addr, port);
        
        size_t data_length;
        void *rv = trudpProcessReceivedPacket(td, buffer, recvlen, &data_length);
    }
    
    // Process send queue
    trudpProcessSendQueue(td);
}

/**
 * Show usage screen
 * 
 * @param name Start command to show in usage screen
 */
static void usage(char *name) {

	fprintf(stderr, "\nUsage:\n");
	fprintf(stderr, "    %s [options] <destination-IP> <destination-port>\n", name);
	fprintf(stderr, "    %s [options] -l -p <listening-port>\n", name);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "    -h          Help\n");
	fprintf(stderr, "    -d          Debug mode; use multiple times to increase verbosity.\n");
	fprintf(stderr, "    -l          Listen mode\n");
	fprintf(stderr, "    -p <port>   Local port\n");
	fprintf(stderr, "    -s <IP>     Source IP\n");
//	fprintf(stderr, "    -B <size>   Buffer size\n");
//	fprintf(stderr, "    -n          Don't resolve hostnames\n");
	fprintf(stderr, "\n");
	exit(1);
}

/**
 * Main application function
 * 
 * @param argc
 * @param argv
 * @return 
 */
int main(int argc, char** argv) {

    int i;
    o_local_port = "8000";
    o_local_address = "0.0.0.0";

    // Read parameters
    while(1) {
            int c = getopt (argc, argv, "hdlp:B:s:n");
            if (c == -1) break;
            switch(c) {
                    case 'h': usage(argv[0]);                   break;
                    case 'd': o_debug++;			break;
                    case 'l': o_listen++;			break;
                    case 'p': o_local_port = optarg;		break;
//                    case 'B': o_buf_size = atoi(optarg);	break;
                    case 's': o_local_address = optarg;		break;
//                    case 'n': o_numeric++;			break;
                    //case 'w': break;	// timeout for connects and final net reads
                    default:
                            die("Unhandled argument: %c\n", c);
            }
    }

    // Read arguments
    for(i = optind; i < argc; i++) {
        switch(i - optind) {
            case 0:	o_remote_address = argv[i]; 	break;
            case 1:	o_remote_port = argv[i];	break;
        }
    }
    
    // Check necessary arguments
    if(o_listen && (o_remote_port || o_remote_address)) usage(argv[0]);
    if(!o_listen && (!o_remote_port || !o_remote_address)) usage(argv[0]);

    // Show logo
    fprintf(stderr, "TR-UDP cat example application ver 0.0.1\n");
    
    // Show execution mode
    if(o_listen) 
        fprintf(stderr, "Server at %s:%s\n", o_local_address, o_local_port);    
    else {
        o_remote_port_i = atoi(o_remote_port);
        fprintf(stderr, "Client connected to %s:%d\n", o_remote_address, o_remote_port_i);
    }
    
//    setup();
//    while (!quit_flag)
//            network_loop();
//
//    if(buf_len) {
//            fprintf(stderr, "Warning: send buffer not empty\n");
//            exit_code++;
//    }
//
//    utp_context_stats *stats = utp_get_context_stats(ctx);
//
//    if(stats) {
//            debug("           Bucket size:    <23    <373    <723    <1400    >1400\n");
//            debug("Number of packets sent:  %5d   %5d   %5d    %5d    %5d\n",
//                    stats->_nraw_send[0], stats->_nraw_send[1], stats->_nraw_send[2], stats->_nraw_send[3], stats->_nraw_send[4]);
//            debug("Number of packets recv:  %5d   %5d   %5d    %5d    %5d\n",
//                    stats->_nraw_recv[0], stats->_nraw_recv[1], stats->_nraw_recv[2], stats->_nraw_recv[3], stats->_nraw_recv[4]);
//    }
//    else {
//            debug("utp_get_context_stats() failed?\n");
//    }
//
//    debug("Destorying context\n");
//    utp_destroy(ctx);
    
    // Initialize TR-UDP
    trudpData *td = trudpNew(NULL, processDataCb, writeCb);
    td->processAckCb = processAckCb;
    td->user_data = td;
    
    // Bind UDP port and get FD
    int port = atoi(o_local_port);
    td->fd = trudpUdpBindRaw(&port, 1);
    if(td->fd <= 0) die("Can't bind UDP port ...\n");
    else fprintf(stderr, "Start listening at port %d\n", port);
    
    // Create remote address and Send "connect" packet
    if(!o_listen) {
        char *connect = "Connect with TR-UDP!";
        size_t connect_length = strlen(connect) + 1;
        trudpUdpMakeAddr(o_remote_address, o_remote_port_i, (__SOCKADDR_ARG) &td->remaddr, &td->addrlen);
        trudpSendData(td, connect, connect_length);
        td->connected_f = 1;
    }

    char *hello_c = "Hello TR-UDP from client!";
    size_t hello_c_length = strlen(hello_c) + 1;
    
    char *hello_s = "Hello TR-UDP from server!";
    size_t hello_s_length = strlen(hello_s) + 1;
    
    i = 0;
    while (!quit_flag) {
        
        network_loop(td);
        
        // Client message
        if(!o_listen && !(i%100000)) {
          trudpSendData(td, hello_c, hello_c_length);
        }
        
        // Server message
        if(o_listen && td->connected_f && !(i%100100)) {
          trudpSendData(td, hello_s, hello_s_length);
        }
        
        usleep(10);
        i++;
    }    
    
    // Destroy TR-UDP
    trudpDestroy(td);
    
    printf("Executed!\n");
    
    return (EXIT_SUCCESS);
}
