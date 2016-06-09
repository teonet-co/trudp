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
#include "packet.h"
#include "map.h"
#include "udp.h"

#ifdef __cplusplus
extern "C" {
#endif
    
#define MAX_RETRIEVES 10
#define MAX_OUTRUNNING 10    
#define START_MIDDLE_TIME (MAX_ACK_WAIT/5) * 1000000    
    
#define TD(tcd) ((trudpData*)tcd->td)    
    
/**
 * Data received/send callback
 */
typedef void (*trudpDataCb)(void *td, void *data, size_t data_length, void *user_data);

/**
 * Event callback
 */
typedef void (*trudpEventCb)(void *td, int event, void *data, size_t data_length, void *user_data);

/**
 * Union of TR-UDP callback
 */
typedef union trudpCb {
    
    trudpDataCb data;
    trudpDataCb send;
    trudpEventCb event;
    void *ptr;
    
} trudpCb;

/**
 * Enumeration of callback types
 */
typedef enum trudpCallbsckType {
    
    PROCESS_DATA,
    PROCESS_ACK,
    EVENT,
    SEND
            
} trudpCallbsckType;

/**
 * Trudp channel Data Structure
 */
typedef struct trudpChannelData {
    
    uint32_t sendId;
    trudpPacketQueue *sendQueue;
    uint32_t triptime;
    uint32_t triptimeMiddle;
        
    uint32_t receiveExpectedId;
    trudpPacketQueue *receiveQueue;
    int outrunning_cnt; ///< Receive queue outrunning count

    // User data
    //void* user_data;
    
    // Link to parent trudpData
    void *td; ///< Pointer to trudpData
    
    // UDP connection depended variables
    struct sockaddr_in remaddr; // remote address
    socklen_t addrlen;          // remote address length
    int connected_f;            // connected (remote address valid)
    int channel;                // TR-UDP channel
    
} trudpChannelData;

/**
 * Trudp Data Structure
 */
typedef struct trudpData {

    trudpMapData *map; ///< Channels map (key: ip:port:channel)
    void* user_data; ///< User data
    int port;
    int fd;
    
    // Callback
    trudpDataCb processDataCb;
    trudpDataCb processAckCb;
    trudpEventCb evendCb;
    trudpDataCb sendCb;       
    
} trudpData;

trudpData *trudpInit(int fd, int port, void *user_data);
void trudpDestroy(trudpData* trudp);
trudpCb trudpSetCallback(trudpData *td, trudpCallbsckType type, trudpCb cb);
trudpChannelData *trudpCheckRemoteAddr(trudpData *td, struct sockaddr_in *remaddr, 
        socklen_t addr_length, int channel);
int trudpProcessSendQueue(trudpData *td);

trudpChannelData *trudpNewChannel(trudpData *td, char *remote_address, int remote_port_i, int channel); // void *user_data, trudpDataCb processDataCb, trudpDataCb sendPacketCb);
void trudpDestroyChannel(trudpChannelData *tcd);
void trudpFree(trudpChannelData *tcd);
void trudpReset(trudpChannelData *tcd);
size_t trudpSendData(trudpChannelData *tcd, void *data, size_t data_length);
void *trudpProcessChannelReceivedPacket(trudpChannelData *tcd, void *packet, 
        size_t packet_length, size_t *data_length);
char *trudpMakeKey(char *addr, int port, int channel, size_t *key_length);

#ifdef __cplusplus
}
#endif

#endif /* TR_UDP_H */

