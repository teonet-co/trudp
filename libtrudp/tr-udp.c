/**
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
 * \file   tr-udp.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 31, 2016, 1:44 AM
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tr-udp.h"
#include "packet.h"
#include "tr-udp_stat.h"

#define MAP_SIZE_DEFAULT 1000

/**
 * Set default trudpChannelData values
 *
 * @param tcd
 */
static void trudpSetDefaults(trudpChannelData *tcd) {

    tcd->sendId = 0;
    tcd->triptime = 0;
    tcd->triptimeFactor = 1.5; 
    tcd->outrunning_cnt = 0;
    tcd->receiveExpectedId = 0;
    tcd->triptimeMiddle = START_MIDDLE_TIME;
    
    // Initialize statistic
    trudpStatChannelInit(tcd);
}

/**
 * Initialize TR-UDP
 *
 * @param fd File descriptor to read write data
 * @param port Port (optional)
 * @param user_data User data which will send to most library function
 *
 * @return
 */
trudpData *trudpInit(int fd, int port, void *user_data) {

    trudpData* trudp = (trudpData*) malloc(sizeof(trudpData));

    trudp->map = trudpMapNew(MAP_SIZE_DEFAULT, 1);
    trudp->user_data = user_data;
    trudp->port = port;
    trudp->fd = fd;
    
    // Initialize statistic data
    trudpStatInit(trudp);
    trudp->started = trudpGetTimestamp();

    return trudp;
}

/**
 * Destroy TR-UDP
 * 
 * @param trudp
 */
void trudpDestroy(trudpData* trudp) {

    if(trudp) {
        free(trudp);
    }
}

/**
 * Set TR-UDP callback
 *
 * @param type
 * @param cb
 * @return
 */
trudpCb trudpSetCallback(trudpData *td, trudpCallbsckType type, trudpCb cb) {

    trudpCb oldCb;

    switch(type) {

        case PROCESS_DATA:
            oldCb.data = td->processDataCb;
            td->processDataCb = cb.data;
            break;

        case PROCESS_ACK:
            oldCb.data = td->processAckCb;
            td->processAckCb = cb.data;
            break;

        case EVENT:
            oldCb.event = td->evendCb;
            td->evendCb = cb.event;
            break;

        case SEND:
            oldCb.send = td->sendCb;
            td->sendCb = cb.send;
            break;

        default:
            oldCb.ptr = NULL;
            break;
    }

    return oldCb;
}

/**
 * Create trudp chanel
 *
 * @param td Pointer to trudpData
 * @param remote_address
 * @param remote_port_i
 * @return
 */
trudpChannelData *trudpNewChannel(trudpData *td, char *remote_address, 
        int remote_port_i, int channel) {

    trudpChannelData *tcd = (trudpChannelData *) malloc(sizeof(trudpChannelData));
    memset(tcd, 0, sizeof(trudpChannelData));

    tcd->td = td;
    tcd->sendQueue = trudpPacketQueueNew();
    tcd->receiveQueue = trudpPacketQueueNew();
    tcd->addrlen = sizeof(tcd->remaddr);
    trudpUdpMakeAddr(remote_address, remote_port_i,
            (__SOCKADDR_ARG) &tcd->remaddr, &tcd->addrlen);
    tcd->channel = channel;

    // Set other defaults
    trudpSetDefaults(tcd);
    
    // Add cannel to map
    size_t key_length;
    char *key = trudpMakeKey(remote_address, remote_port_i, channel, &key_length);
    trudpChannelData *tcd_r = trudpMapAdd(td->map, key, key_length, tcd, sizeof(*tcd));
    free(tcd);
    
    return tcd_r;
}

/**
 * Destroy trudp chanel
 *
 * @param tcd Pointer to trudpChannelData
 */
void trudpDestroyChannel(trudpChannelData *tcd) {

    trudpPacketQueueDestroy(tcd->sendQueue);
    trudpPacketQueueDestroy(tcd->receiveQueue);

    int port;
    size_t key_length;
    char *addr = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, &port);
    char *key = trudpMakeKey(addr, port, tcd->channel, &key_length);
    trudpMapDelete(TD(tcd)->map, key, key_length);
}

/**
 * Free trudp chanel
 *
 * @param tcd Pointer to trudpChannelData
 */
void trudpFreeChannel(trudpChannelData *tcd) {

    trudpPacketQueueFree(tcd->sendQueue);
    trudpPacketQueueFree(tcd->receiveQueue);
    trudpSetDefaults(tcd);
}

/**
 * Reset trudp chanel
 *
 * @param tcd Pointer to trudpChannelData
 */
inline void trudpResetChannel(trudpChannelData *tcd) {
    trudpFreeChannel(tcd);
}

/**
 * Get new send Id
 *
 * @param td Pointer to trudpChannelData
 * @return New send Id
 */
static inline uint32_t trudpGetNewId(trudpChannelData *td) {

    return td->sendId++;
}

/**
 * Call Send data callback
 *
 * @param td Pointer to trudpChannelData
 * @param packet Pointer to packet
 * @param packetLength Packet length
 *
 * @return
 */
static inline size_t trudpExecSendPacketCallback(trudpChannelData *tcd, 
        void *packet, size_t  packetLength) {

    if(TD(tcd)->sendCb) {
        TD(tcd)->sendCb(tcd, packet, packetLength, TD(tcd)->user_data);
    }
    
    return 0;
}

/**
 * Calculate Expected Time
 *
 * @param td Pointer to trudpChannelData
 * @param current_time Current time (nsec)
 *
 * @return Current time plus
 */
static inline uint32_t trudpCalculateExpectedTime(trudpChannelData *td,
        uint32_t current_time) {

    return current_time + td->triptimeMiddle;
}

/**
 * Send data
 *
 * @param td Pointer to trudpChannelData
 * @param data Pointer to send data
 * @param data_length Data length
 *
 * @return Zero on error
 */
size_t trudpSendData(trudpChannelData *tcd, void *data, size_t data_length) {

    // Create DATA package
    size_t packetLength;
    void *packetDATA = trudpPacketDATAcreateNew(trudpGetNewId(tcd), 
            tcd->channel, data, data_length, &packetLength);

    // Save packet to send queue
    trudpPacketQueueAdd(tcd->sendQueue, packetDATA, packetLength,
            trudpCalculateExpectedTime(tcd, trudpGetTimestamp()));

    // Send data (add to write queue)
    trudpExecSendPacketCallback(tcd, packetDATA, packetLength);

    // Statistic
    tcd->stat.packets_send++;

    // Free created packet
    trudpPacketCreatedFree(packetDATA);

    return packetLength;
}

/**
 * Execute trudpDataCb callback
 *
 * @param tcd
 * @param packet
 * @param data
 * @param data_length
 * @param user_data
 * @param cb
 */
static void trudpExecProcessDataCallback(trudpChannelData *tcd, void *packet, 
        void **data, size_t *data_length, void *user_data, trudpDataCb cb) {

    *data = trudpPacketGetData(packet);
    *data_length = (size_t)trudpPacketGetDataLength(packet);
    if(cb != NULL) cb(tcd, *data, *data_length, user_data);
}

/**
 * Execute trudpEventCb callback
 *
 * @param tcd
 * @param event
 * @param data
 * @param data_length
 * @param user_data
 * @param cb
 */
static void trudpExecEventCallback(trudpChannelData *tcd, int event, void *data,
        size_t data_length, void *user_data, trudpEventCb cb) {

    if(cb != NULL) cb((void*)tcd, event, data, data_length, user_data);
}

/**
 * Create ACK packet and send it back to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
static inline void trudpSendACK(trudpChannelData *tcd, void *packet) {

    void *packetACK = trudpPacketACKcreateNew(packet);
    trudpExecSendPacketCallback(tcd, packetACK, trudpPacketACKlength());
    trudpPacketCreatedFree(packetACK);
}

/**
 * Create ACK to RESET packet and send it back to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
static inline void trudpSendACKtoRESET(trudpChannelData *tcd, void *packet) {

    void *packetACK = trudpPacketACKtoRESETcreateNew(packet);
    trudpExecSendPacketCallback(tcd, packetACK, trudpPacketACKlength());
    trudpPacketCreatedFree(packetACK);
}

/**
 * Create RESET packet and send it to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
static inline void trudpSendRESET(trudpChannelData *tcd) {

    void *packetRESET = trudpPacketRESETcreateNew(trudpGetNewId(tcd), tcd->channel);
    trudpExecSendPacketCallback(tcd, packetRESET, trudpPacketRESETlength());
    trudpPacketCreatedFree(packetRESET);
}

/**
 * Process received packet
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 * @param packet_length Packet length
 * @param data_length Pointer to variable to return packets data length
 * @param user_data Pointer to send to callback as third parameter
 * @param cb callback function called when DATA packet received and checked
 *
 * @return Pointer to received data, NULL available, length of data saved to
 *         *data_length, if packet is not TR-UDP packet the (void *)-1 pointer
 *         returned
 */
void *trudpProcessChannelReceivedPacket(trudpChannelData *tcd, void *packet,
        size_t packet_length, size_t *data_length) {

    void *data = NULL;
    *data_length = 0;

    // Check and process TR-UDP packet
    if(trudpPacketCheck(packet, packet_length)) {

        // Check packet type
        switch(trudpPacketGetType(packet)) {

            // ACK to DATA packet received
            case TRU_ACK: {

                // Remove packet from send queue
                size_t send_data_length = 0;
                trudpPacketQueueData *tpqd = trudpPacketQueueFindById(
                    tcd->sendQueue, trudpPacketGetId(packet)
                );
                if(tpqd) {
                    send_data_length = trudpPacketGetDataLength(tpqd->packet);
                    trudpPacketQueueDelete(
                        tcd->sendQueue,
                        trudpPacketQueueFindById(
                            tcd->sendQueue, trudpPacketGetId(packet)
                        )
                    );
                }

                // Set triptime
                tcd->triptime = trudpGetTimestamp() -
                    trudpPacketGetTimestamp(packet);

                // Calculate and set middle triptime value
                tcd->triptimeMiddle = tcd->triptimeMiddle == START_MIDDLE_TIME ? tcd->triptime * tcd->triptimeFactor : // Set first middle time
                    tcd->triptime > tcd->triptimeMiddle ? tcd->triptime * tcd->triptimeFactor : // Set middle time to max triptime
                        (tcd->triptimeMiddle * 19 + tcd->triptime) / 20.0; // Calculate middle value                
                // Correct triptimeMiddle
                if(tcd->triptimeMiddle < tcd->triptime * tcd->triptimeFactor) 
                    tcd->triptimeMiddle = tcd->triptime * tcd->triptimeFactor;
                
                // Process ACK data callback
                trudpExecProcessDataCallback(tcd, packet, &data, data_length,
                        TD(tcd)->user_data, TD(tcd)->processAckCb);

                // Statistic
                tcd->stat.ack_receive++;
                tcd->stat.triptime_last = tcd->triptime;
                tcd->stat.wait = tcd->triptimeMiddle / 1000.0;
                trudpStatProcessLast10Send(tcd, packet, send_data_length);
                
            } break;

            // ACK to RESET packet received
            case TRU_ACK | TRU_RESET:

                // \todo Send event
                fprintf(stderr, "Got TRU_ACK of RESET packet\n");

                // Reset TR-UDP
                trudpResetChannel(tcd);
                
                // Statistic
                tcd->stat.ack_receive++;
                
                break;

            // DATA packet received
            case TRU_DATA: {

                // Create ACK packet and send it back to sender
                trudpSendACK(tcd, packet);
                
               // Check expected Id and return data
                if(trudpPacketGetId(packet) == tcd->receiveExpectedId) {

                    // Execute trudpDataReceivedCb Callback with pointer to data
                    trudpExecProcessDataCallback(tcd, packet, &data, data_length,
                            TD(tcd)->user_data, TD(tcd)->processDataCb);

                    // Check received queue for saved packet with expected id
                    trudpPacketQueueData *tqd;
                    while((tqd = trudpPacketQueueFindById(tcd->receiveQueue,
                            ++tcd->receiveExpectedId)) ) {

                        trudpExecProcessDataCallback(tcd, tqd->packet, &data,
                            data_length, TD(tcd)->user_data,
                            TD(tcd)->processDataCb);
                        
                        trudpPacketQueueDelete(tcd->receiveQueue, tqd);
                    }
                    
                    // Statistic
                    tcd->stat.packets_receive++;
                    trudpStatProcessLast10Receive(tcd, packet);
 
                    tcd->outrunning_cnt = 0; // Reset outrunning flag
                }
                // Save outrunning packet to receiveQueue
                else if(trudpPacketGetId(packet) > tcd->receiveExpectedId) {

                    if(!trudpPacketQueueFindById(tcd->receiveQueue, trudpPacketGetId(packet)) ) {
                        trudpPacketQueueAdd(tcd->receiveQueue, packet, packet_length, 0);
                        tcd->outrunning_cnt++; // Increment outrunning count                    

                        // \todo Send reset at match outrunning
                        if(tcd->outrunning_cnt > MAX_OUTRUNNING) {
                            fprintf(stderr, "To match TR-UDP channel outrunning! Reset channel ...\n");
                            trudpSendRESET(tcd);
                        }
                    }
                }
                // Skip already processed packet
                else {
                    
                    // Statistic
                    tcd->stat.packets_receive_dropped++;
                    trudpStatProcessLast10Receive(tcd, packet);
                }

            } break;

            // RESET packet received
            case TRU_RESET:

                // \todo Send event
                fprintf(stderr, "Got TRU_RESET packet\n");

                // Create ACK to RESET packet and send it back to sender
                trudpSendACKtoRESET(tcd, packet);
                //fprintf(stderr, "Got TRU_RESET packet - 2\n");

                // Reset TR-UDP
                trudpResetChannel(tcd);
                //fprintf(stderr, "Got TRU_RESET packet - 3\n");

                break;

            // An undefined type of packet (skip it)
            default:

                // Return error code
                data = (void *)-1;
                break;
        }
    }
    // Packet is not TR-UDP packet
    else data = (void *)-1;
    
    return data;
}

/**
 * Check send Queue elements and resend elements with expired time
 *
 * @param tcd Pointer to trudpChannelData
 *
 * @return Number of resend packets
 */
int trudpProcessChannelSendQueue(trudpChannelData *tcd) {

    int rv = 0;

    trudpPacketQueueData *tqd;
    uint32_t ts = trudpGetTimestamp();
    while((tqd = trudpPacketQueueFindByTime(tcd->sendQueue, ts))) {

        // Resend data and change it expected time
        trudpExecSendPacketCallback(tcd, tqd->packet, tqd->packet_length);
        trudpPacketQueueMoveToEnd(tcd->sendQueue, tqd);
        tqd->expected_time = trudpCalculateExpectedTime(tcd, ts);
        tcd->stat.packets_attempt++; // Attempt(repeat) statistic parameter increment
//        tcd->stat.send_total += tqd->packet_length / (1024.0 * 1024.0);
        if(!tqd->retrieves) tqd->retrieves_start = ts;
        tqd->retrieves++;
        rv++;

        // Stop at match retrieves
        if(tqd->retrieves > MAX_RETRIEVES || ts - tqd->retrieves_start > MAX_RETRIEVES_TIME) {
            char *key = trudpMakeKeyCannel(tcd);
            fprintf(stderr, "Disconnect channel %s\n", key);
            trudpExecEventCallback(tcd, DISCONNECTED, key, strlen(key) + 1,
                    TD(tcd)->user_data, TD(tcd)->evendCb);
            trudpDestroyChannel(tcd);

            return -1;
        }
    }

    return rv;
}

/**
 * Check all peers send Queue elements and resend elements with expired time
 *
 * @param td Pointer to trudpData
 *
 * @return Number of resend packets
 */

int trudpProcessSendQueue(trudpData *td) {

    int retval, rv = 0;
    trudpMapElementData *el;
    do {
        retval = 0;
        trudpMapIterator *it;
        if((it = trudpMapIteratorNew(td->map))) {
            while((el = trudpMapIteratorNext(it))) {
                size_t data_lenth;
                trudpChannelData *tcd = (
                    trudpChannelData *)trudpMapIteratorElementData(el,
                        &data_lenth);
                retval = trudpProcessChannelSendQueue(tcd);
                if(retval > 0) rv += retval;
                else if(retval < 0) break;
            }
            trudpMapIteratorDestroy(it);
        }
    } while(retval == -1);

    return rv;
}

/**
 * Send the same data to all connected peers
 *
 * @param td Pointer to trudpData
 * @param data Pointer to data
 * @param data_length Data length
 *
 * @return Number of peers
 */
size_t trudpSendDataToAll(trudpData *td, void *data, size_t data_length) {

    int rv = 0;

    trudpMapIterator *it;
    trudpMapElementData *el;
    if((it = trudpMapIteratorNew(td->map))) {
        while((el = trudpMapIteratorNext(it))) {
            size_t data_lenth;
            trudpChannelData *tcd = (trudpChannelData *)
                    trudpMapIteratorElementData(el, &data_lenth
            );
            trudpSendData(tcd, data, data_length);
            rv++;
        }
        trudpMapIteratorDestroy(it);
    }

    return rv;
}

/**
 * Make TR-UDP map key
 *
 * @param addr String with IP address
 * @param port Port number
 * @param channel Cannel number 0-15
 * @param key_length [out] Keys length
 *
 * @return Static buffer with key ip:port:channel
 */
char *trudpMakeKey(char *addr, int port, int channel, size_t *key_length) {

    //#define BUF_SIZE 64
    static char buf[CS_KEY_LENGTH];
    *key_length = snprintf(buf, CS_KEY_LENGTH, "%s:%u:%u", addr, port, channel);

    return buf;
}

/**
 * Make key from channel data
 * @param tcd Pointer to trudpChannelData
 *
 * @return Static buffer with key ip:port:channel
 */
char *trudpMakeKeyCannel(trudpChannelData *tcd) {

    int port;
    size_t key_length;
    char *addr = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, &port);
    return trudpMakeKey(addr, port, tcd->channel, &key_length);
}

/**
 * Save check remote address and select or create new trudpChannelData
 *
 * @param td Pointer to trudpData
 * @param remaddr Pointer to sockaddr_in remote address
 * @param addr_length Remote address length
 */
trudpChannelData *trudpCheckRemoteAddr(trudpData *td, 
        struct sockaddr_in *remaddr, socklen_t addr_length, int channel) {

    int port;
    size_t data_length, key_length;
    char *addr = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)remaddr, &port);
    char *key = trudpMakeKey(addr, port, channel, &key_length);
    trudpChannelData *tcd = (trudpChannelData *)trudpMapGet(td->map, key, 
            key_length, &data_length);
    if(tcd == (void*)-1) {
        tcd = trudpNewChannel(td, addr, port, channel);
        fprintf(stderr, "Connect channel %s\n", trudpMakeKeyCannel(tcd) );
    }
    tcd->connected_f = 1;

    return tcd;
}

/**
 * Get channel send queue timeout
 *
 * @param td Pointer to trudpChannelData
 * @return Send queue timeout (may by 0) or UINT32_MAX if send queue is empty
 */
uint32_t trudpGetChannelSendQueueTimeout(trudpChannelData *tcd) {

    // Get sendQueue timeout
    uint32_t timeout_sq = UINT32_MAX;
    if(tcd->sendQueue->q->first) {

        uint32_t
        expected_t = ((trudpPacketQueueData *)tcd->sendQueue->q->first->data)->expected_time,
        current_t = trudpGetTimestamp();
        timeout_sq = current_t < expected_t ? expected_t - current_t : 0;
    }

    return timeout_sq;
}

/**
 * Get minimum timeout from all trudp cannel send queue
 *
 * @param td
 *
 * @return Minimum timeout or UINT32_MAX if send queue is empty
 */
uint32_t trudpGetSendQueueTimeout(trudpData *td) {

    trudpMapIterator *it; 
    trudpMapElementData *el;
    uint32_t timeout_sq = UINT32_MAX;
    if((it = trudpMapIteratorNew(td->map))) {
        while((el = trudpMapIteratorNext(it))) {
            size_t data_lenth;
            trudpChannelData *tcd = (trudpChannelData *)trudpMapIteratorElementData(el, &data_lenth);
            uint32_t ts = trudpGetChannelSendQueueTimeout(tcd);
            if(ts < timeout_sq) timeout_sq = ts;
            if(!timeout_sq) break;
        }
        trudpMapIteratorDestroy(it);
    }

    return timeout_sq;
}
