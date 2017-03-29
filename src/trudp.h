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
 *
 * Created on May 31, 2016, 1:45 AM
 */

#ifndef TR_UDP_H
#define TR_UDP_H

#include "packet_queue.h"
#include "write_queue.h"
#include "packet.h"
#include "map.h"
#include "udp.h"

#include "trudp_channel.h"
#include "trudp_const.h"
    
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get pointer to trudpData from trudpChannelData
 */    
#define TD(tcd) ((trudpData*)tcd->td)    
#define TD_P(td) ((trudpData*)td)
    
/**
 * Data received/send callback
 */
typedef void (*trudpDataCb)(void *tcd, void *data, size_t data_length, void *user_data);

/**
 * Event callback
 */
typedef void (*trudpEventCb)(void *tcd, int event, void *data, size_t data_length, void *user_data);

/**
 * Enumeration of TR-UDP events
 */
typedef enum trudpEvent {
    
    /**
     * Initialize TR-UDP event
     * @param td Pointer to trudpData
     */ 
    INITIALIZE,
    
    /**
     * Destroy TR-UDP event
     * @param td Pointer to trudpData
     */ 
    DESTROY,
    
    /**
     * TR-UDP channel disconnected event
     * @param data NULL
     * @param data_length 0
     * @param user_data NULL
     */
    CONNECTED,
            
    /**
     * TR-UDP channel disconnected event
     * @param data Last packet received
     * @param data_length 0
     * @param user_data NULL
     */
    DISCONNECTED,
            
    /**
     * Got TR-UDP reset packet
     * @param data NULL
     * @param data_length 0
     * @param user_data NULL
     */
    GOT_RESET,
            
    /**
     * Send TR-UDP reset packet
     * @param data Pointer to uint32_t send id or NULL if received id = 0
     * @param data_length Size of uint32_t or 0
     * @param user_data NULL
     */
    SEND_RESET,
            
    /**
     * Got ACK to reset command
     * @param data NULL
     * @param data_length 0
     * @param user_data NULL
     */
    GOT_ACK_RESET,
            
    /**
     * Got ACK to ping command
     * @param data Pointer to ping data (usually it is a string)
     * @param data_length Length of data
     * @param user_data NULL
     */
    GOT_ACK_PING,
            
    /**
     * Got PING command
     * @param data Pointer to ping data (usually it is a string)
     * @param data_length Length of data
     * @param user_data NULL
     */
    GOT_PING,
            
    /**
     * Got ACK command
     * @param data Pointer to ACK packet
     * @param data_length Length of data
     * @param user_data NULL
     */
    GOT_ACK,
            
    /**
     * Got DATA 
     * @param data Pointer to data
     * @param data_length Length of data
     * @param user_data NULL
     */
    GOT_DATA,
            
    /**
     * Process received data
     * @param tcd Pointer to trudpData
     * @param data Pointer to receive buffer
     * @param data_length Receive buffer length
     * @param user_data NULL
     */ 
    PROCESS_RECEIVE,

    /** Process received not TR-UDP data
     * @param tcd Pointer to trudpData
     * @param data Pointer to receive buffer
     * @param data_length Receive buffer length
     * @param user_data NULL            
     */ 
    PROCESS_RECEIVE_NO_TRUDP,
            
    /** Process send data
     * @param data Pointer to send data
     * @param data_length Length of send
     * @param user_data NULL            
     */ 
    PROCESS_SEND
            
} trudpEvent;

/**
 * TR-UDP Statistic data
 */
typedef struct trudpStatData {
    
    struct sendQueue {
        size_t size_max;
        size_t size_current;
        size_t attempt;
    } sendQueue;
    
    struct receiveQueue {
        size_t size_max;
        size_t size_current;
    } receiveQueue;
    
} trudpStatData;

/**
 * Trudp Data Structure
 */
typedef struct trudpData {

    uint32_t trudp_data_label[2]; ///< Labele to distinguish trudpData and trudpChannelData
    trudpMapData *map; ///< Channels map (key: ip:port:channel)
    void* psq_data; ///< Send queue process data (used in external event loop)
    void* user_data; ///< User data
    int port; ///< Port
    int fd; ///< File descriptor
                 
    // Callback
    trudpEventCb evendCb;
    
    // Statistic
    trudpStatData stat;
    unsigned long long started;
    
    size_t writeQueueIdx;
    
} trudpData;

trudpData *trudpInit(int fd, int port, trudpEventCb event_cb, void *user_data);
void trudpDestroy(trudpData* td);
trudpChannelData *trudpGetChannel(trudpData *td, __CONST_SOCKADDR_ARG addr, 
        int channel);
trudpChannelData *trudpGetChannelAddr(trudpData *td, char *addr, int port,
        int channel);
trudpChannelData *trudpGetChannelCreate(trudpData *td, __CONST_SOCKADDR_ARG addr, int channel);
void trudpSendResetAll(trudpData *td);
size_t trudpProcessKeepConnection(trudpData *td);
void trudpEventSend(void *t_pointer, int event, void *data,
        size_t data_length, void *user_data);
void *trudpEventGotDataSend(void *t_pointer, void *packet, 
        size_t *data_length);
void trudp_ChannelDestroyAll(trudpData *td);
void trudp_ChannelDestroyAddr(trudpData *td, char *addr, int port, 
        int channel);

void trudpProcessReceive(trudpData *td, void *data, size_t data_length);

size_t trudpSendDataToAll(trudpData *td, void *data, size_t data_length);


size_t trudpGetReceiveQueueMax(trudpData *td);

//size_t trudp_SendQueueGetSizeMax(trudpData *td);
size_t trudp_SendQueueSize(trudpData *td);
uint32_t trudp_SendQueueGetTimeout(trudpData *td, uint64_t ts);
int trudp_SendQueueProcess(trudpData *td, uint64_t *next_et);

size_t trudp_WriteQueueProcess(trudpData *td);

size_t trudp_WriteQueueSizeAll(trudpData *td);


#ifdef __cplusplus
}
#endif

#endif /* TR_UDP_H */
