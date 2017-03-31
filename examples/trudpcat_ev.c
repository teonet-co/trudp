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
#include <time.h>

// C11 present
#if __STDC_VERSION__ >= 201112L
extern int usleep (__useconds_t __useconds);
#endif
#include "snake.h"

#include "trudp.h"
#include "utils_r.h"
#include "trudp_stat.h"

// Application version
#define APP_VERSION "0.0.18"

// Application constants
#define SEND_MESSAGE_AFTER_MIN  15000 /* 16667 */ // uSec (mSec * 1000)
#define SEND_MESSAGE_AFTER  SEND_MESSAGE_AFTER_MIN
#define RECONNECT_AFTER 3000000 // uSec (mSec * 1000)
#define SHOW_STATISTIC_AFTER 500000 // uSec (mSec * 1000)

// Application options structure
typedef struct options {

    // Integer options
    int debug; // = 0;
    int show_statistic; // = 0;
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

} options;

//#define USE_LIBEV 1

#if USE_LIBEV

#include <ev.h>
#include "trudp_ev.h"

/**
 * Show statistic data definition
 */
typedef struct show_statistic_data {

    int inited;
    trudpData *td;
    struct ev_loop *loop;
    ev_timer show_statistic_w;
    ev_idle idle_show_statistic_w;
    uint32_t last_show;

} show_statistic_data;

/**
 * Send message callback data
 */
typedef struct send_message_data {

    trudpData *td;
    char *message;
    size_t message_length;
    options *o;

} send_message_data;

// Local static function definition
//static void start_send_queue_cb(process_send_queue_data *psd, uint64_t next_expected_time);
static void start_show_stat_cb(show_statistic_data *ssd);

// Global data
static trudpProcessSendQueueData psd;

#else
#define USE_SELECT 1
#include "utils.h"
#endif

// Application options
static options o = { 0, 0, 0, 0, 0, 0, 0, 4096, NULL, NULL, NULL, NULL, 0 };

#if USE_SELECT
// Application exit code and flags
static int /*exit_code = EXIT_SUCCESS,*/ quit_flag = 0;
#endif
static int connected_flag = 0;

// Read buffer
static char *buffer;

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
 * Show debug message
 *
 * @param fmt
 * @param ...
 */
static void debug(char *fmt, ...)
{
    static unsigned long idx = 0;
    va_list ap;
    if(o.debug) {
        fflush(stdout);
        fprintf(stderr, "%lu %.3f debug: ", ++idx, trudpGetTimestamp() / 1000.0);
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fflush(stderr);
    }
}

/**
 * Show statistic window
 *
 * @param td Pointer to trudpData
 */
static void showStatistic(trudpData *td, options *o, void *user_data) {

    if(o->show_statistic) {
        cls();
        char *stat_str = ksnTRUDPstatShowStr(td);
        if(stat_str) {
            puts(stat_str);
            free(stat_str);
        }
    }

    else if(o->show_send_queue) {
        trudpChannelData *tcd = (trudpChannelData*)trudpMapGetFirst(td->map, 0);
        if(tcd != (void*)-1) {

            char *stat_sq_str = trudpStatShowQueueStr(tcd, 0);
            if(stat_sq_str) {
                cls();
                puts(stat_sq_str);
                free(stat_sq_str);
            }
            else cls();

            char *stat_rq_str = trudpStatShowQueueStr(tcd, 1);
            if(stat_rq_str) {
                puts(stat_rq_str);
                free(stat_rq_str);
            }
        }
        else { cls(); puts("Queues have not been created..."); }
    }

    else if(o->show_snake) {
        if(!run_snake()) o->show_snake = 0;
    }

    // Check key !!!
    int ch = nb_getch();
    if(ch) {
        //printf("key %c pressed\n", ch);

        switch(ch) {
            case 'd': o->debug  = !o->debug; break;
            case 'S': o->show_statistic  = !o->show_statistic;  o->show_send_queue = 0; o->show_snake = 0; break;
            case 'Q': o->show_send_queue = !o->show_send_queue; o->show_statistic = 0; o->show_snake = 0;  break;
            case 's': o->show_snake = !o->show_snake;           o->show_send_queue = 0; o->show_statistic = 0; break;
            #if USE_LIBEV
            case 'q': ev_break(user_data, EVBREAK_ALL);         break;
            #endif
            case 'r': trudpSendResetAll(td);                    break;
            case 'x': o->dont_send_data = !o->dont_send_data;   break;
        }
    }
}

/**
 * TR-UDP event callback
 *
 * @param tcd_pointer
 * @param event
 * @param data
 * @param data_length
 * @param user_data
 */
static void eventCb(void *tcd_pointer, int event, void *data, size_t data_length,
        void *user_data) {

    trudpChannelData *tcd = (trudpChannelData *)tcd_pointer;

    switch(event) {

        // CONNECTED event
        // @param data NULL
        // @param user_data NULL
        case CONNECTED: {

            char *key = trudp_ChannelMakeKey(tcd);
            fprintf(stderr, "Connect channel %s\n", key);

        } break;

        // DISCONNECTED event
        // @param tcd Pointer to trudpData
        // @param data Last packet received
        // @param user_data NULL
        case DISCONNECTED: {
                        
            char *key = trudp_ChannelMakeKey(tcd);
            if(data_length == sizeof(uint32_t)) {
                uint32_t last_received = *(uint32_t*)data;
                fprintf(stderr,
                    "Disconnect channel %s, last received: %.6f sec\n",
                    key, last_received / 1000000.0);
                trudp_ChannelDestroy(tcd);
            }
            else {
                fprintf(stderr,
                    "Disconnected channel %s\n", key);
            }

            connected_flag = 0;

        } break;

        // GOT_RESET event
        // @param data NULL
        // @param user_data NULL
        case GOT_RESET: {

            char *key = trudp_ChannelMakeKey(tcd);
            fprintf(stderr,
              "Got TRU_RESET packet from channel %s\n",
              key);

        } break;

        // SEND_RESET event
        // @param data Pointer to uint32_t id or NULL (data_size == 0)
        // @param user_data NULL
        case SEND_RESET: {


            char *key = trudp_ChannelMakeKey(tcd);
            
            if(!data)
                fprintf(stderr,
                  "Send reset: "
                  "to channel %s\n",
                  key);

            else {
            
                uint32_t id = (data_length == sizeof(uint32_t)) ? *(uint32_t*)data:0;

                if(!id)
                    fprintf(stderr,
                      "Send reset: "
                      "Not expected packet with id = 0 received from channel %s\n",
                      key);
                else
                    fprintf(stderr,
                      "Send reset: "
                      "High send packet number (%d) at channel %s\n",
                      id, key);
                }

        } break;

        // GOT_ACK_RESET event: got ACK to reset command
        // @param data NULL
        // @param user_data NULL
        case GOT_ACK_RESET: {

            char *key = trudp_ChannelMakeKey(tcd);
            fprintf(stderr, "Got ACK to RESET packet at channel %s\n", key);

        } break;

        // GOT_ACK_PING event: got ACK to ping command
        // @param data Pointer to ping data (usually it is a string)
        // @param user_data NULL
        case GOT_ACK_PING: {

            char *key = trudp_ChannelMakeKey(tcd);
            fprintf(stderr,
              "Got ACK to PING packet at channel %s, data: %s, %.3f(%.3f) ms\n",
              key, (char*)data,
              (tcd->triptime)/1000.0, (tcd->triptimeMiddle)/1000.0);

        } break;

        // GOT_PING event: got PING packet, data
        // @param data Pointer to ping data (usually it is a string)
        // @param user_data NULL
        case GOT_PING: {

            char *key = trudp_ChannelMakeKey(tcd);
            fprintf(stderr,
              "Got PING packet at channel %s, data: %s\n",
              key, (char*)data);

        } break;

        // Got ACK event
        // @param data Pointer to ACK packet
        // @param data_length Length of data
        // @param user_data NULL
        case GOT_ACK: {

            char *key = trudp_ChannelMakeKey(tcd);
            debug("got ACK id=%u at channel %s, %.3f(%.3f) ms\n",
                  trudpPacketGetId(data/*trudpPacketGetPacket(data)*/),
                  key, (tcd->triptime)/1000.0, (tcd->triptimeMiddle)/1000.0);

            #if USE_LIBEV
            // trudp_start_send_queue_cb(&psd, 0);
            #endif

        } break;

        // Got DATA event
        // @param data Pointer to data
        // @param data_length Length of data
        // @param user_data NULL
        case GOT_DATA: {

            char *key = trudp_ChannelMakeKey(tcd);
            debug("got %d byte data at channel %s, id=%u: ", 
                trudpPacketGetPacketLength(trudpPacketGetPacket(data))/*(int)data_length*/,
                key, trudpPacketGetId(trudpPacketGetPacket(data)));

            if(!o.show_statistic && !o.show_send_queue && !o.show_snake) {
                if(o.debug) {
                    printf("#%u at %.3f, cannel %s [%.3f(%.3f) ms] ",
                           tcd->receiveExpectedId,
                           (double)trudpGetTimestamp() / 1000.0,
                           key, 
                           (double)tcd->triptime / 1000.0,
                           (double)tcd->triptimeMiddle / 1000.0);

                    printf("%s\n",(char*)data);
                }
            }
            else {
                // Show statistic window
                //showStatistic(TD(tcd));
            }
            debug("\n");

        } break;
        
        // Process received data
        // @param tcd Pointer to trudpData
        // @param data Pointer to receive buffer
        // @param data_length Receive buffer length
        // @param user_data NULL
        case PROCESS_RECEIVE: {
            
            trudpData *td = (trudpData *)tcd;
            trudpProcessReceive(td, data, data_length);
                        
        } break;
        
        // Process send data
        // @param data Pointer to send data
        // @param data_length Length of send
        // @param user_data NULL
        case PROCESS_SEND: {
        
            //if(isWritable(TD(tcd)->fd, timeout) > 0) {
            // Send to UDP
            trudpUdpSendto(TD(tcd)->fd, data, data_length,
                    (__CONST_SOCKADDR_ARG) &tcd->remaddr, sizeof(tcd->remaddr));
            //}

            // Debug message
            if(o.debug) {

                int port,type;
                uint32_t id = trudpPacketGetId(data);
                char *addr = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, &port);
                if(!(type = trudpPacketGetType(data))) {
                    debug("send %d bytes, id=%u, to %s:%d, %.3f(%.3f) ms\n",
                        (int)data_length, id, addr, port,
                        tcd->triptime / 1000.0, tcd->triptimeMiddle / 1000.0);
                }
                else {
                    debug("send %d bytes %s id=%u, to %s:%d\n",
                        (int)data_length, 
                        type == 1 ? "ACK" :
                        type == 2 ? "RESET" :
                        type == 3 ? "ACK to RESET" :
                        type == 4 ? "PING" : "ACK to PING"
                        , id, addr, port);
                }
            }

            #if USE_LIBEV
            trudp_start_send_queue_cb(&psd, 0);
            #endif
            
        } break;

        default: break;
    }
}

/**
 * Connect to peer
 *
 * @param td
 * @return
 */
static trudpChannelData *connectToPeer(trudpData *td) {

    trudpChannelData *tcd = NULL;

    // Start connection and Send "connect" packet
    if(!o.listen) {
        char *connect = "Connect with TR-UDP!";
        size_t connect_length = strlen(connect) + 1;
        tcd = trudp_ChannelNew(td, o.remote_address, o.remote_port_i, 0);
        trudp_ChannelSendData(tcd, connect, connect_length);
        fprintf(stderr, "Connecting to %s:%u:%u\n", o.remote_address, o.remote_port_i, 0);
        connected_flag = 1;
    }

    return tcd;
}

// Libev functions
#if USE_LIBEV

/**
 * Read UDP data libev callback
 *
 * @param loop
 * @param w
 * @param revents
 */
static void host_cb(EV_P_ ev_io *w, int revents) {

    //trudpData *td = (trudpData *)w->data;
    trudpEventSend(w->data, PROCESS_RECEIVE, buffer, o.buf_size, NULL);
}

/**
 * Connect timer libev callback
 *
 * @param loop
 * @param w
 * @param revents
 */
static void connect_cb(EV_P_ ev_timer *w, int revents) {

    static int i = 0;
    trudpData *td = (trudpData *)w->data;

    // Check connections
    if(!o.listen && !connected_flag) {
        if(!(i%4)) connectToPeer(td);
    }
    else {
        // Check all channels line (lastReceived time) and send PING if idle
        trudpProcessKeepConnection(td);
    }

    i++;
}

/**
 * Send message timer libev callback
 *
 * @param loop
 * @param w
 * @param revents
 */
static void send_message_cb(EV_P_ ev_timer *w, int revents) {

    send_message_data *smd = (send_message_data *)w->data;

    if(!smd->o->dont_send_data)
        trudpSendDataToAll(smd->td, smd->message, smd->message_length);
}

/**
 * Show statistic in idle time libev callback
 *
 * @param loop
 * @param w
 * @param revents
 */
static void idle_show_stat_cb(EV_P_ ev_idle *w, int revents) {

    show_statistic_data *ssd = (show_statistic_data *)w->data;

    ev_idle_stop(ssd->loop, &ssd->idle_show_statistic_w);

    trudpData *td = ssd->td;
    showStatistic(td, &o, ssd->loop);
    start_show_stat_cb(ssd);
    ssd->last_show = trudpGetTimestamp();
}

/**
 * Show statistic timer libev callback
 *
 * @param loop
 * @param w
 * @param revents
 */
static void show_stat_cb(EV_P_ ev_timer *w, int revents) {

    show_statistic_data *ssd = (show_statistic_data *)w->data;

    uint32_t tt = trudpGetTimestamp();
    if(tt - ssd->last_show > 3000000) {

        trudpData *td = ssd->td;
        showStatistic(td, &o, ssd->loop);
        start_show_stat_cb(ssd);
        ssd->last_show = tt;
    }
    else {
        // Start idle watcher
        if(!ev_is_active(&ssd->idle_show_statistic_w))
            ev_idle_start(ssd->loop, &ssd->idle_show_statistic_w);
    }
}

/**
 * Start show statistic timer
 *
 * @param ssd
 */
static void start_show_stat_cb(show_statistic_data *ssd) {

    double tt_d = (SHOW_STATISTIC_AFTER/3) / 1000000.0;

    if(!ssd->inited) {

        // Initialize show statistic watcher
        ev_timer_init(&ssd->show_statistic_w, show_stat_cb, tt_d, 0.0);
        ssd->show_statistic_w.data = (void*)ssd;

        // Initialize idle watchers
        ev_idle_init(&ssd->idle_show_statistic_w, idle_show_stat_cb);
        ssd->idle_show_statistic_w.data = (void*)ssd;

        ssd->inited = 1;
    }
    else {
        ev_timer_stop(ssd->loop, &ssd->show_statistic_w);
        ev_timer_set(&ssd->show_statistic_w, tt_d, 0.0);
    }

    ev_timer_start(ssd->loop, &ssd->show_statistic_w);
}

#else

#if USE_SELECT

/**
 * The TR-UDP cat network loop with select function
 *
 * @param td Pointer to trudpData
 * @param delay Default read data timeout
 */
static void network_select_loop(trudpData *td, int timeout) {

    int rv = 1;
    fd_set rfds, wfds;
    struct timeval tv;
    uint64_t /*tt, next_et = UINT64_MAX,*/ ts = trudpGetTimestampFull();

//    while(rv > 0) {
    // Watch server_socket to see when it has input.
    FD_ZERO(&wfds);
    FD_ZERO(&rfds);
    FD_SET(td->fd, &rfds);

    // Process write queue
    if(trudp_WriteQueueSizeAll(td)) {
        FD_SET(td->fd, &wfds);
    }

    uint32_t timeout_sq = trudp_SendQueueGetTimeout(td, ts);
//    debug("set timeout: %.3f ms; default: %.3f ms, send_queue: %.3f ms%s\n",
//            (timeout_sq < timeout ? timeout_sq : timeout) / 1000.0,
//            timeout / 1000.0,
//            timeout_sq / 1000.0,
//            timeout_sq == UINT32_MAX ? "(queue is empty)" : ""
//    );

    // Wait up to ~50 ms. */
    uint32_t t = timeout_sq < timeout ? timeout_sq : timeout;
    usecToTv(&tv, t);

    rv = select((int)td->fd + 1, &rfds, &wfds, NULL, &tv);

    // Error
    if (rv == -1) {
        fprintf(stderr, "select() handle error\n");
        return;
    }

    // Timeout
    else if(!rv) { // Idle or Timeout event

        // Process send queue
        if(timeout_sq != UINT32_MAX) {
            int rv = trudp_SendQueueProcess(td, 0);
            debug("process send queue ... %d\n", rv);
        }
    }

    // There is a data in fd
    else {

        // Process read fd
        if(FD_ISSET(td->fd, &rfds)) {

            struct sockaddr_in remaddr; // remote address
            socklen_t addr_len = sizeof(remaddr);
            ssize_t recvlen = trudpUdpRecvfrom(td->fd, buffer, o.buf_size,
                    (__SOCKADDR_ARG)&remaddr, &addr_len);

            // Process received packet
            if(recvlen > 0) {
                size_t data_length;
                trudpChannelData *tcd = trudpGetChannelCreate(td, (__SOCKADDR_ARG)&remaddr, 0);
                trudp_ChannelProcessReceivedPacket(tcd, buffer, recvlen, &data_length);
            }
        }

        // Process write fd
        if(FD_ISSET(td->fd, &wfds)) {
            // Process write queue
            while(trudp_WriteQueueProcess(td));
            //trudpProcessWriteQueue(td);
        }
    }
//    }
}

#else

/**
 * The TR-UDP cat network loop
 *
 * @param td Pointer to trudpData
 */
static void network_loop(trudpData *td) {

    // Read from UDP
    struct sockaddr_in remaddr; // remote address
    socklen_t addr_len = sizeof(remaddr);
    ssize_t recvlen = trudpUdpRecvfrom(td->fd, buffer, o.buf_size,
            (__SOCKADDR_ARG)&remaddr, &addr_len);

    // Process received packet
    if(recvlen > 0) {
        size_t data_length;
        trudpChannelData *tcd = trudpGetChannelCreate(td, (__SOCKADDR_ARG)&remaddr, 0);
        trudp_ChannelProcessReceivedPacket(tcd, buffer, recvlen, &data_length);
    }

    // Process send queue
    trudp_SendQueueProcess(td, 0);

    // Process write queue
    while(trudp_WriteQueueProcess(td));
}

#endif

#endif

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

    // Show logo
    fprintf(stderr,
            "TR-UDP two node connect sample application ver " APP_VERSION "\n"
    );

    int i/*, connected_f = 0*/;
    o.local_port = "8000"; // Default local port
    o.local_address = "0.0.0.0"; // Default local address

    // Read parameters
    while(1) {
        int c = getopt (argc, argv, "hdlp:B:s:nSQx");
        if (c == -1) break;
        switch(c) {
            case 'h': usage(argv[0]);               break;
            case 'd': o.debug++;		    break;
            case 'l': o.listen++;		    break;
            case 'p': o.local_port = optarg;	    break;
            case 'B': o.buf_size = atoi(optarg);    break;
            case 's': o.local_address = optarg;	    break;
            //case 'n': o.numeric++;		  break;
            //case 'w': break;	// timeout for connects and final net reads
            case 'S': o.show_statistic++;           break;
            case 'Q': o.show_send_queue++;          break;
            case 'x': o.dont_send_data++;           break;
            default:  die("Unhandled argument: %c\n", c); break;
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
        fprintf(stderr, "Server started at %s:%s\n", o.local_address,
            o.local_port);
    }
    else {
        o.remote_port_i = atoi(o.remote_port);
        fprintf(stderr, "Client start connection to %s:%d\n", o.remote_address,
            o.remote_port_i);
    }

    // Create read buffer
    buffer = malloc(o.buf_size);

    // Startup windows socket library
    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    WSADATA wsaData;
    WSAStartup(0x0202, &wsaData);
    #endif

    // 0) Bind UDP port and get FD (start listening at port)
    int port = atoi(o.local_port);
    int fd = trudpUdpBindRaw(&port, 1);
    if(fd <= 0) die("Can't bind UDP port ...\n");
    else fprintf(stderr, "Start listening at port %d\n", port);

    // 1) Initialize TR-UDP
    trudpData *td = trudpInit(fd, port, eventCb, NULL);

    // Create messages
    #define SEND_BUFFER_SIZE 1024 * 1
    char hello_c[SEND_BUFFER_SIZE] =  { "Hello TR-UDP from client!" } ;
    size_t hello_c_length = sizeof(hello_c); // strlen(hello_c) + 1;
    //
    char hello_s[SEND_BUFFER_SIZE] = { "Hello TR-UDP from server!" };
    size_t hello_s_length = sizeof(hello_s); // strlen(hello_s) + 1;
    //
    i = 0;
    char *message;
    size_t message_length;
    if(!o.listen) { message = hello_c; message_length = hello_c_length; }
    else { message = hello_s; message_length = hello_s_length; }

    // 2) Process networking
    #if USE_LIBEV

    // Create event loop
    struct ev_loop *loop = 0 ? ev_loop_new(0) : EV_DEFAULT;
    ev_timer send_message_w;
    show_statistic_data ssd;
    send_message_data smd;
    ev_timer connect_w;
    ev_io w;

    // Initialize process_send_queue_data
    psd.loop = loop;
    psd.inited = 0;
    psd.td = td;

    // Start UDP input output watcher
    ev_io_init(&w, host_cb, fd, EV_READ);
    w.data = (void*)td;
    ev_io_start(loop, &w);

    // Initialize and start reconnect watcher watcher
    ev_timer_init(&connect_w, connect_cb, 0.0, RECONNECT_AFTER / 1000000.0);
    connect_w.data = (void*)td;
    ev_timer_start(loop, &connect_w);

    // Initialize and start send message watcher
    ev_timer_init(&send_message_w, send_message_cb, SEND_MESSAGE_AFTER / 1000000.0, SEND_MESSAGE_AFTER / 1000000.0);
    send_message_w.data = (void*)&smd;
    smd.o = &o;
    smd.td = td;
    smd.message = message;
    smd.message_length = message_length;
    ev_timer_start(loop, &send_message_w);

    // Initialize and start show statistic watcher
    ssd.td = td;
    ssd.inited = 0;
    ssd.loop = loop;
    ssd.last_show = 0;
    start_show_stat_cb(&ssd);

    // Start event loop
    ev_run(loop, 0);

    #else

    const int DELAY = 500000; // uSec
    uint32_t tt, tt_s = 0, tt_c = 0, tt_ss = 0;

    while (!quit_flag) {

        #if !USE_SELECT
        network_loop(td);
        #else
        network_select_loop(td, SEND_MESSAGE_AFTER < DELAY ? SEND_MESSAGE_AFTER : DELAY);
        #endif

        // Current timestamp
        tt = trudpGetTimestamp();

        // Connect
        if(!o.listen && !connected_flag && (tt - tt_c) > RECONNECT_AFTER) {
            connectToPeer(td);
            tt_c = tt;
        }

        // Send message
        // random int between 0 and 500000

        if((tt - tt_s) > SEND_MESSAGE_AFTER) {

            if(td->stat.sendQueue.size_current < 1000)
                if(!o.dont_send_data)
                    trudpSendDataToAll(td, message, message_length);
            //send_message_after = (rand() % (500000 - 1)) + SEND_MESSAGE_AFTER_MIN;
            tt_s = tt;
        }

        // Show statistic
        if(/*o.statistic && */(tt - tt_ss) > SHOW_STATISTIC_AFTER) {

            showStatistic(td, &o, 0);
            tt_ss = tt;
        }

        #if !USE_SELECT
        usleep(DELAY);
        #endif
        i++;
    }

    #endif

    // 4) Destroy TR-UDP
    trudpDestroy(td);
    free(buffer);

    printf("Executed!\n");

    // Cleanup socket library
    #if defined(HAVE_MINGW) || defined(_WIN32) || defined(_WIN64)
    WSACleanup();
    #endif

    return (EXIT_SUCCESS);
}
