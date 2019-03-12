/**
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
 * \file   trudp_pth.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on August 2, 2018, 12:37 PM
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

#include "trudp.h"
#include "packet.h"
#include "read_queue.h"

#include "trudp_pth.h"

//#if USE_SELECT
// Application exit code and flags
//static int /*exit_code = EXIT_SUCCESS,*/ quit_flag = 0;
//#endif
static int connected_flag = 0;
const int DELAY = 500000; // uSec

// Read buffer
static char *buffer;

enum debug_mode {
    DEBUG = 1,
    DEBUG_VV = 2,
    DEBUG_VVV = 3
};

/**
 * Show debug message
 *
 * @param fmt
 * @param ...
 */
static void debug(const trudp_data_t *tru, int mode, char *fmt, ...)
{
    static unsigned long idx = 0;
    va_list ap;
    if(mode <= tru->o->debug) {
        fflush(stdout);
        fprintf(stderr, "%lu %.3f debug: ", ++idx, trudpGetTimestamp() / 1000.0);
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fflush(stderr);
    }
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

/**
 * TR-UDP event callback
 *
 * @param tcd_pointer
 * @param event
 * @param data
 * @param data_length
 * @param user_data
 */
static void event_cb(void *tcd_pointer, int event, void *data, size_t data_length,
        void *user_data) {

    trudpChannelData *tcd = (trudpChannelData *)tcd_pointer;
    trudp_data_t *tru = user_data;

    switch(event) {

        // CONNECTED event
        // @param data NULL
        // @param user_data NULL
        case CONNECTED: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  "Connect channel %s\n", key);

        } break;

        // DISCONNECTED event
        // @param tcd Pointer to trudpData
        // @param data Last packet received
        // @param user_data NULL
        case DISCONNECTED: {
                        
            char *key = trudpChannelMakeKey(tcd);
            if(data_length == sizeof(uint32_t)) {
                uint32_t last_received = *(uint32_t*)data;
                debug(tru, DEBUG,  
                      "Disconnect channel %s, last received: %.6f sec\n",
                      key, last_received / 1000000.0);
                trudpChannelDestroy(tcd);
            }
            else debug(tru, DEBUG,  "Disconnected channel %s\n", key);

            connected_flag = 0;

        } break;

        // GOT_RESET event
        // @param data NULL
        // @param user_data NULL
        case GOT_RESET: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  "Got TRU_RESET packet from channel %s\n", key);

        } break;

        // SEND_RESET event
        // @param data Pointer to uint32_t id or NULL (data_size == 0)
        // @param user_data NULL
        case SEND_RESET: {


            char *key = trudpChannelMakeKey(tcd);
            
            if(!data) debug(tru, DEBUG,  "Send reset: to channel %s\n", key);
            else {
            
                uint32_t id = (data_length == sizeof(uint32_t)) ? *(uint32_t*)data:0;

                if(!id)
                  debug(tru, DEBUG,  
                    "Send reset: "
                    "Not expected packet with id = 0 received from channel %s\n",
                    key);
                else
                  debug(tru, DEBUG,  
                    "Send reset: "
                    "High send packet number (%d) at channel %s\n",
                    id, key);
                }

        } break;

        // GOT_ACK_RESET event: got ACK to reset command
        // @param data NULL
        // @param user_data NULL
        case GOT_ACK_RESET: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  "Got ACK to RESET packet at channel %s\n", key);

        } break;

        // GOT_ACK_PING event: got ACK to ping command
        // @param data Pointer to ping data (usually it is a string)
        // @param user_data NULL
        case GOT_ACK_PING: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  
              "Got ACK to PING packet at channel %s, data: %s, %.3f(%.3f) ms\n",
              key, (char*)data,
              (tcd->triptime)/1000.0, (tcd->triptimeMiddle)/1000.0);

        } break;

        // GOT_PING event: got PING packet, data
        // @param data Pointer to ping data (usually it is a string)
        // @param user_data NULL
        case GOT_PING: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  
              "Got PING packet at channel %s, data: %s\n",
              key, (char*)data);

        } break;

        // Got ACK event
        // @param data Pointer to ACK packet
        // @param data_length Length of data
        // @param user_data NULL
        case GOT_ACK: {

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  "got ACK id=%u at channel %s, %.3f(%.3f) ms\n",
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

            char *key = trudpChannelMakeKey(tcd);
            debug(tru, DEBUG,  
                "event got %d byte data at channel %s [%.3f(%.3f) ms], id=%u: %s\n", 
                trudpPacketGetPacketLength(trudpPacketGetPacket(data)),
                key,
                (double)tcd->triptime / 1000.0,
                (double)tcd->triptimeMiddle / 1000.0,
                trudpPacketGetId(trudpPacketGetPacket(data)), 
                trudpPacketGetData(trudpPacketGetPacket(data)));
            
            // Add data to read QUEUE
            size_t len = trudpPacketGetPacketLength(trudpPacketGetPacket(data)) + sizeof(tcd);
            void *d = malloc(len);
            memcpy(d, &tcd, sizeof(tcd));
            memcpy(d + sizeof(tcd), trudpPacketGetPacket(data), len - sizeof(tcd));
            trudpReadQueueAdd(tru->rq, NULL, d, len);
            
            //debug(tru, DEBUG,  "len: %d, d: %s\n", len, trudpPacketGetData(d) );
//            if(!tru->o->show_statistic && !tru->o->show_send_queue && !tru->o->show_snake) {
//                if(tru->o->debug) {
//                    printf("#%u at %.3f, cannel %s [%.3f(%.3f) ms] ",
//                           tcd->receiveExpectedId,
//                           (double)trudpGetTimestamp() / 1000.0,
//                           key, 
//                           (double)tcd->triptime / 1000.0,
//                           (double)tcd->triptimeMiddle / 1000.0);
//
//                    printf("%s\n",(char*)data);
//                }
//            }
//            else {
//                // Show statistic window
//                //showStatistic(TD(tcd));
//            }
//            debug(tru, DEBUG,  "\n");

        } break;
        
        // Process received data
        // @param tcd Pointer to trudpData
        // @param data Pointer to receive buffer
        // @param data_length Receive buffer length
        // @param user_data NULL
        case PROCESS_RECEIVE: {
            
            trudpData *td = (trudpData *)tcd;
            trudpProcessReceived(td, data, data_length);
                        
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
            if(/*o.debug*/ 1) {

                int port,type;
                uint32_t id = trudpPacketGetId(data);
                char *addr = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, &port);
                if(!(type = trudpPacketGetType(data))) {
                    debug(tru, DEBUG,  "send %d bytes, id=%u, to %s:%d, %.3f(%.3f) ms\n",
                        (int)data_length, id, addr, port,
                        tcd->triptime / 1000.0, tcd->triptimeMiddle / 1000.0);
                }
                else {
                    debug(tru, DEBUG,  "send %d bytes %s id=%u, to %s:%d\n",
                        (int)data_length, 
                        type == 1 ? "ACK" :
                        type == 2 ? "RESET" :
                        type == 3 ? "ACK to RESET" :
                        type == 4 ? "PING" : "ACK to PING"
                        , id, addr, port);
                }
            }

            #if USE_LIBEV
//            trudpSendQueueCbStart(&psd, 0);
            #endif
            
        } break;

        default: break;
    }
}

/**
 * The TR-UDP cat network loop with select function
 *
 * @param td Pointer to trudpData
 * @param delay Default read data timeout
 */
void network_select_loop(trudp_data_t *tru, int timeout) {

    int rv = 1;
    fd_set rfds, wfds;
    struct timeval tv;
    trudpData *td = tru->td;
    uint64_t ts = teoGetTimestampFull();

    // Watch server_socket to see when it has input.
    FD_ZERO(&wfds);
    FD_ZERO(&rfds);
    FD_SET(td->fd, &rfds);

    pthread_mutex_lock(&tru->mutex);
    
    // Add write queue to processing
    if(trudpGetWriteQueueSize(td)) {
        FD_SET(td->fd, &wfds);
    }

    uint32_t timeout_sq = trudpGetSendQueueTimeout(td, ts);
//    debug(tru, DEBUG,  "set timeout: %.3f ms; default: %.3f ms, send_queue: %.3f ms%s\n",
//            (timeout_sq < timeout ? timeout_sq : timeout) / 1000.0,
//            timeout / 1000.0,
//            timeout_sq / 1000.0,
//            timeout_sq == UINT32_MAX ? "(queue is empty)" : ""
//    );

    // Wait up to ~50 ms. */
    uint32_t t = timeout_sq < timeout ? timeout_sq : timeout;
    usecToTv(&tv, t);

    pthread_mutex_unlock(&tru->mutex);
    
    rv = select((int)td->fd + 1, &rfds, &wfds, NULL, &tv);

    // Error
    if (rv == -1) {
        debug(tru, DEBUG,   "select() handle error\n");
        return;
    }

    // Timeout
    else if(!rv) { // Idle or Timeout event

        // Process send queue
        if(timeout_sq != UINT32_MAX) {
            pthread_mutex_lock(&tru->mutex);
            /*int rv = */trudpProcessSendQueue(td, 0);
            //debug(tru, DEBUG,  "process send queue ... %d\n", rv);
            pthread_mutex_unlock(&tru->mutex);
        }
    }

    // There is a data in fd
    else {

        // Process read fd
        if(FD_ISSET(td->fd, &rfds)) {

            struct sockaddr_in remaddr; // remote address
            socklen_t addr_len = sizeof(remaddr);
            ssize_t recvlen = trudpUdpRecvfrom(td->fd, buffer, /*o.buf_size*/ 4096,
                    (__SOCKADDR_ARG)&remaddr, &addr_len);

            // Process received packet
            if(recvlen > 0) {
                pthread_mutex_lock(&tru->mutex);
                size_t data_length;
                trudpChannelData *tcd = trudpGetChannelCreate(td, (__SOCKADDR_ARG)&remaddr, 0);
                trudpChannelProcessReceivedPacket(tcd, buffer, recvlen, &data_length);
                pthread_mutex_unlock(&tru->mutex);
            }
        }

        // Process write fd
        if(FD_ISSET(td->fd, &wfds)) {
            // Process write queue
            pthread_mutex_lock(&tru->mutex);
            while(trudpProcessWriteQueue(td));
            pthread_mutex_unlock(&tru->mutex);
        }
    }
//    }
}

/**
 * Process thread function
 * 
 * @param tru Pointer to trudp_data_t
 * 
 * @return 
 */
static void* trudp_process_thread(void *tru_p) {
    
    trudp_data_t *tru = tru_p;
    debug(tru, DEBUG,  "Process thread started\n");
    while(tru->running) network_select_loop(tru, 15000);
    debug(tru, DEBUG,  "Process thread stopped\n");
    
    return NULL;
}

// Initialize TR-UDP
trudp_data_t *trudp_init(trudp_options *o) {
    
    // Create read buffer
    buffer = malloc(o->buf_size);

    int fd = trudpUdpBindRaw(&o->local_port_i, 1);
    if(fd <= 0) {
        fprintf(stderr,  "Can't bind %d UDP port ...\n", o->local_port_i);
        return NULL;
    }
    else {
        trudp_data_t *tru = malloc(sizeof(trudp_data_t));
        tru->td = trudpInit(fd, o->local_port_i, event_cb, tru);
        tru->rq = trudpReadQueueNew();
        tru->running = 1;
        tru->fd = fd;
        tru->o = o;

        debug(tru, DEBUG,  "Start listening at port %d\n", o->local_port_i);
        debug(tru, DEBUG,  "Debug print mode: %d\n", o->debug);
        
        //pthread_cond_init(&tru->cv_threshold, NULL);
        //pthread_mutex_init(&tru->cv_mutex, NULL);
        
        // Start module thread
        pthread_mutex_init(&tru->mutex, NULL);
        int err = pthread_create(&tru->tid, NULL, trudp_process_thread, (void*)tru);
        if (err != 0) fprintf(stderr, "Can't create trudp thread :[%s]\n", strerror(err));
        else debug(tru, DEBUG,  "Trudp thread created successfully\n");
    
        return tru;
    }
}

// Destroy TR-UDP
void trudp_destroy(trudp_data_t *tru) {

    tru->running = 0;
    pthread_join(tru->tid, NULL);
    //pthread_cond_destroy(&tru->cv_threshold);
    //pthread_mutex_destroy(&tru->cv_mutex);
    pthread_mutex_destroy(&tru->mutex);
    trudpReadQueueDestroy(tru->rq);
    trudpDestroy(tru->td);
    close(tru->fd);
    free(tru);
}

void *trudp_connect(trudp_data_t *tru, const char *address, int port) {
    return trudpChannelNew(tru->td, (char*)address, port, 0);
}

void trudp_disconnect(trudp_data_t *tru, void *tcd) {
    trudpChannelDestroy((trudpChannelData *)tcd);
}

void *trudp_recv(trudp_data_t *tru, void **tcd_p, size_t *msg_length) {
    // \\ TODO get data from read queue and sleep if queue empty
    pthread_mutex_lock(&tru->mutex);
    void *msg = NULL;
    
    // Add data to read QUEUE
    trudpReadQueueData *rqd = trudpReadQueueGetFirst(tru->rq);
    if(rqd) {
        trudpChannelData *tcd = *(trudpChannelData **) rqd->packet_ptr;
        char *key = trudpChannelMakeKey(tcd);
        void *data = trudpPacketGetData(rqd->packet_ptr + sizeof(tcd));
        debug(tru, DEBUG,  "funct got %d byte data at channel %s, id=%u: %s\n", 
            trudpPacketGetPacketLength(trudpPacketGetPacket(data)),
            key, trudpPacketGetId(trudpPacketGetPacket(data)), trudpPacketGetData(trudpPacketGetPacket(data)));
             
        if(tcd_p) *tcd_p = tcd;
        if(msg_length) *msg_length = trudpPacketGetPacketLength(trudpPacketGetPacket(data));
        msg = trudpPacketGetData(trudpPacketGetPacket(data));
        
        trudpReadQueueDeleteFirst(tru->rq);
    } 
    else { 
        if(msg_length) *msg_length = 0;    
        if(tcd_p) *tcd_p = NULL;
    }
    pthread_mutex_unlock(&tru->mutex);
    return msg;
}

void trudp_free_recv_data(trudp_data_t *tru, void *data) {   
    free(trudpPacketGetPacket(data) - sizeof(trudpChannelData *));
}

int trudp_send(trudp_data_t *tru, void *tcd, void *msg, size_t msg_length) {
    pthread_mutex_lock(&tru->mutex);
    
    int s = 0;
    // Check Send queue size and sleep if too match
    while((s = trudpSendQueueSize(((trudpChannelData *)tcd)->sendQueue))>120) {
        pthread_mutex_unlock(&tru->mutex);
        usleep(1000);
        pthread_mutex_lock(&tru->mutex);
    }
    
    int rv = trudpChannelSendData(tcd, msg, msg_length);
    pthread_mutex_unlock(&tru->mutex);
    //usleep(10);
    return rv;
}
