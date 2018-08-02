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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>

#include "trudp_pth.h"

//#if __STDC_VERSION__ >= 201112L
//extern int usleep (__useconds_t __useconds);
//#endif

// Application version
#define APP_VERSION "0.0.1"

// Application options structure
typedef struct options {

    // Integer options
    int debug; // = 0;
    int show_statistic; // = 0;
    int show_statistic_page;
    int show_send_queue; // = 0;
    int show_snake; // = 0;
    int listen; // = 0;
    int numeric; // = 0;
    int dont_send_data; // 0
    int buf_size; // = 4096;

    // String options
    char *local_address; // = NULL;
    char *local_port; // = NULL;
    char *remote_address; // = NULL;
    char *remote_port; // = NULL;

    // Calculated options
    int remote_port_i;
    int local_port_i;

} options;

// Application options
static options o = { 0, 0, 0, 0, 0, 0, 0, 0, 4096, NULL, NULL, NULL, NULL, 0 };

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
	fprintf(stderr, "    -B <size>   Buffer size\n");
        fprintf(stderr, "    -S          Show statistic\n");
        fprintf(stderr, "    -Q          Show queues\n");
        fprintf(stderr, "    -x          Don't send data\n");
	fprintf(stderr, "\n");
	exit(1);
}

static void read_parameters(int argc, char** argv) {
    int i;
    o.local_port = "8000"; // Default local port
    o.local_address = "0.0.0.0"; // Default local address

    // Read parameters
    while(1) {
        int c = getopt (argc, argv, "hdlp:B:s:SQx");
        if (c == -1) break;
        switch(c) {
            case 'h': usage(argv[0]);               break;
            case 'd': o.debug++;		    break;
            case 'l': o.listen++;		    break;
            case 'p': o.local_port = optarg;	    break;
            case 'B': o.buf_size = atoi(optarg);    break;
            case 's': o.local_address = optarg;	    break;
            case 'S': o.show_statistic++;           break;
            case 'Q': o.show_send_queue++;          break;
            case 'x': o.dont_send_data++;           break;
            default:  fprintf(stderr,"Unhandled argument: %c\n", c); exit(0); break;
        }
    }

    // Read arguments
    for(i = optind; i < argc; i++) {
        switch(i - optind) {
            case 0:	o.remote_address = argv[i]; break;
            case 1:	o.remote_port = argv[i];    break;
        }
    }

    // Check necessary arguments
    if(o.listen &&   (o.remote_port || o.remote_address)) usage(argv[0]);
    if(!o.listen && (!o.remote_port || !o.remote_address)) usage(argv[0]);

    // Show execution mode
    if(o.listen) {
        o.local_port_i = atoi(o.local_port);
        fprintf(stderr, "Server started at %s:%d\n", o.local_address,
            o.local_port_i);
    }
    else {
        o.remote_port_i = atoi(o.remote_port);
        fprintf(stderr, "Client start connection to %s:%d\n", o.remote_address,
            o.remote_port_i);
    }
}

void network_select_loop(void *td, int timeout);

/**
 * Main application function
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char** argv) {


    // Show logo
    fprintf(stderr, "TR-UDP p2p sample application ver " APP_VERSION "\n");

    // Read parameters
    read_parameters(argc, argv);
    
    trudp_data_t *tru = trudp_init(o.local_port_i);

    // Server mode
    if(o.listen) {
        while(1) {
            //network_select_loop(tru->td, DELAY);
            
            void *msg;
            void *tcd = NULL;
            size_t msg_length;
            msg = trudp_recv(tru, &tcd, &msg_length);
            if(msg) printf("Got message: %s\n", (char*)msg);            
            else usleep(30000);
        }
    }
    
    // Client mode
    else {
        
        void *tcd = trudp_connect(tru, o.remote_address, o.remote_port_i);
        
        char *msg = "Hello world!";
        size_t msg_length = strlen(msg) + 1;
        while(1) {
            //network_select_loop(tru->td, DELAY);
            
            trudp_send(tru, tcd, msg, msg_length);
            printf("Send message: %s\n", (char*)msg);
            usleep(5000);
        }
        
        trudp_disconnect(tru, tcd);
    }

    trudp_destroy(tru);

    return (EXIT_SUCCESS);
}
