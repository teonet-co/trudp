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

#include "trudp.h"
#include "packet.h"
#include "trudp_stat.h"
#include "packet_queue.h"

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
    tcd->lastReceived = trudpGetTimestampFull();
    tcd->triptimeMiddle = START_MIDDLE_TIME;

    // Initialize statistic
    trudpStatChannelInit(tcd);
}

/**
 * Initialize TR-UDP
 *
 * @param fd File descriptor to read write data
 * @param port Port (optional)
 * @param event_cb Event callback
 * @param user_data User data which will send to most library function
 *
 * @return
 */
trudpData *trudpInit(int fd, int port, trudpEventCb event_cb, void *user_data) {

    trudpData* trudp = (trudpData*) malloc(sizeof(trudpData));
    memset(trudp, 0, sizeof(trudpData));

    trudp->map = trudpMapNew(MAP_SIZE_DEFAULT, 1);
    trudp->process_send_queue_data = NULL;
    trudp->user_data = user_data;
    trudp->port = port;
    trudp->fd = fd;

    // Initialize statistic data
    trudpStatInit(trudp);
    trudp->started = trudpGetTimestampFull();

    // Set event callback
    trudp->evendCb = event_cb;

    // Set trudpData label
    trudp->trudp_data_label[0] = 0x77557755;
    trudp->trudp_data_label[1] = 0x55775577;

    // Send INITIALIZE event
    trudpSendEvent((void*)trudp, INITIALIZE, NULL, 0, NULL);

    return trudp;
}

/**
 * Destroy TR-UDP
 *
 * @param trudp
 */
void trudpDestroy(trudpData* trudp) {

    if(trudp) {
        trudpSendEvent((void*)trudp, DESTROY, NULL, 0, NULL);
        trudpMapDestroy(trudp->map);
        free(trudp);
    }
}

/**
 * Create trudp channel
 *
 * @param td Pointer to trudpData
 * @param remote_address
 * @param remote_port_i
 * @param channel
 * @return
 */
trudpChannelData *trudpNewChannel(trudpData *td, char *remote_address,
        int remote_port_i, int channel) {

    trudpChannelData *tcd = (trudpChannelData *) malloc(sizeof(trudpChannelData));
    memset(tcd, 0, sizeof(trudpChannelData));

    tcd->td = td;
    tcd->sendQueue = trudpPacketQueueNew();
    tcd->writeQueue = trudpWriteQueueNew();
    tcd->receiveQueue = trudpPacketQueueNew();
    tcd->addrlen = sizeof(tcd->remaddr);
    trudpUdpMakeAddr(remote_address, remote_port_i,
            (__SOCKADDR_ARG) &tcd->remaddr, &tcd->addrlen);
    tcd->channel = channel;

    // Set other defaults
    trudpSetDefaults(tcd);

    // Add cannel to map
    size_t key_length;
    char *key = trudpMakeKey(
        trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, NULL),
        remote_port_i,
        channel,
        &key_length
    );
    trudpChannelData *tcd_r = trudpMapAdd(td->map, key, key_length, tcd, sizeof(*tcd));
    free(tcd);

    return tcd_r;
}

/**
 * Free trudp channel
 *
 * @param tcd Pointer to trudpChannelData
 */
static void trudpFreeChannel(trudpChannelData *tcd) {

    TD(tcd)->stat.sendQueue.size_current -= trudpPacketQueueSize(tcd->sendQueue);

    trudpPacketQueueFree(tcd->sendQueue);
    trudpWriteQueueFree(tcd->writeQueue);
    trudpPacketQueueFree(tcd->receiveQueue);
    trudpSetDefaults(tcd);
}

/**
 * Destroy trudp channel
 *
 * @param tcd Pointer to trudpChannelData
 */
void trudpDestroyChannel(trudpChannelData *tcd) {

    trudpSendEvent(tcd, DISCONNECTED, NULL, 0, NULL);
    trudpFreeChannel(tcd);

    int port;
    size_t key_length;
    char *addr = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, &port);
    char *key = trudpMakeKey(addr, port, tcd->channel, &key_length);
    trudpMapDelete(TD(tcd)->map, key, key_length);
}

/**
 * Destroy all trudp channels
 *
 * @param tcd Pointer to trudpData
 */
void trudpDestroyChannelAll(trudpData *td) {

    trudpMapElementData *el;
    trudpMapIterator *it;
    if((it = trudpMapIteratorNew(td->map))) {
        while((el = trudpMapIteratorNext(it))) {
            trudpChannelData *tcd = (trudpChannelData *)
                    trudpMapIteratorElementData(el, NULL);
            trudpDestroyChannel(tcd);
        }
        trudpMapIteratorDestroy(it);
    }
}

/**
 * Destroy trudp channel by Address, port and channel number
 *
 * @param td Pointer to trudpData
 * @param addr String with IP address
 * @param port Port number
 * @param channel Cannel number 0-15
 */
inline void trudpDestroyChannelAddr(trudpData *td, char *addr, int port,
        int channel) {

        size_t key_length;
        char *key = trudpMakeKey(addr, port, channel, &key_length);
        trudpChannelData *tcd = (trudpChannelData *)trudpMapGet(td->map, key, key_length, NULL);
        if(tcd && tcd != (void *)-1) trudpDestroyChannel(tcd);
}

/**
 * Reset trudp channel
 *
 * @param tcd Pointer to trudpChannelData
 */
inline void trudpResetChannel(trudpChannelData *tcd) {
    trudpFreeChannel(tcd);
}

/**
 * Get new send Id
 *
 * @param tcd Pointer to trudpChannelData
 * @return New send Id
 */
static inline uint32_t trudpGetNewId(trudpChannelData *tcd) {

    return tcd->sendId++;
}

/**
 * Get send Id
 *
 * @param tcd Pointer to trudpChannelData
 * @return Send Id
 */
static inline uint32_t trudpGetId(trudpChannelData *tcd) {

    return tcd->sendId;
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
void trudpSendEvent(trudpChannelData *tcd, int event, void *data,
        size_t data_length, void *user_data) {

    trudpData *td = (trudpData *)tcd;

    if(td->trudp_data_label[0] == 0x77557755 &&
       td->trudp_data_label[1] == 0x55775577) {

        trudpEventCb cb = td->evendCb;
        if(cb != NULL) cb((void*)tcd, event, data, data_length, td->user_data);
    }
    else {

        trudpEventCb cb = TD(tcd)->evendCb;
        if(cb != NULL) cb((void*)tcd, event, data, data_length, TD(tcd)->user_data);
    }
}

/**
 * Calculate Expected Time
 *
 * @param td Pointer to trudpChannelData
 * @param current_time Current time (nsec)
 *
 * @return Current time plus
 */
static inline uint64_t trudpCalculateExpectedTime(trudpChannelData *tcd,
        uint64_t current_time) {

    uint64_t expected_time = current_time + tcd->triptimeMiddle + 2 * MAX_RTT;

    if(tcd->sendQueue->q->last) {
        trudpPacketQueueData *pqd = (trudpPacketQueueData*) tcd->sendQueue->q->last->data;
        if(pqd->expected_time > expected_time) expected_time = pqd->expected_time;
    }

    return expected_time;
}

/**
 * Send packet
 *
 * @param td Pointer to trudpChannelData
 * @param data Pointer to send data
 * @param data_length Data length
 * @param save_to_write_queue Save to send queue if true
 *
 * @return Zero on error
 */
static size_t trudpSendPacket(trudpChannelData *tcd, void *packetDATA,
        size_t packetLength, int save_to_write_queue) {

    // Save packet to send queue
    if(save_to_write_queue) {
        /*tpqd = */trudpPacketQueueAdd(tcd->sendQueue,
            packetDATA,
            packetLength,
            trudpCalculateExpectedTime(tcd, trudpGetTimestampFull())
        );
        TD(tcd)->stat.sendQueue.size_current++;
    }

    // Send data (add to write queue)
    #if !USE_WRITE_QUEUE
    trudpSendEvent(tcd, PROCESS_SEND, packetDATA, packetLength, NULL);
    #else
    if(save_to_write_queue) {
        trudpWriteQueueAdd(tcd->writeQueue, NULL, tpqd->packet, packetLength);
    else
        trudpSendEvent(tcd, PROCESS_SEND, packetDATA, packetLength, NULL);
    #endif

    // Statistic
    tcd->stat.packets_send++;

    return packetLength;
}

/**
 * Send data
 *
 * @param tcd Pointer to trudpChannelData
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

    // Send data
    size_t rv = trudpSendPacket(tcd, packetDATA, packetLength, 1);

    // Free created packet
    trudpPacketCreatedFree(packetDATA);

    return rv;
}

/**
 * Send PING packet and send it back to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
static inline size_t trudpSendPING(trudpChannelData *tcd, void *data,
        size_t data_length) {

    // Create DATA package
    size_t packetLength;
    void *packetDATA = trudpPacketPINGcreateNew(trudpGetId(tcd), tcd->channel,
            data, data_length, &packetLength);

    // Send data
    size_t rv = trudpSendPacket(tcd, packetDATA, packetLength, 0);

    // Free created packet
    trudpPacketCreatedFree(packetDATA);

    return rv;
}


/**
 * Process write queue and send first ready packet
 *
 * @param tcd
 *
 * @return Number of send packet or 0 if write queue is empty
 */
static size_t trudpProcessChannelWriteQueue(trudpChannelData *tcd) {

    size_t retval = 0;
    trudpWriteQueueData *wqd = trudpWriteQueueGetFirst(tcd->writeQueue);
    if(wqd) {
        void *packet = wqd->packet_ptr ? wqd->packet_ptr : wqd->packet;
        trudpSendEvent(tcd, PROCESS_SEND, packet, wqd->packet_length, NULL);
        trudpWriteQueueDeleteFirst(tcd->writeQueue);
        retval = wqd->packet_length;
    }

    return retval;
}

/**
 * Check all peers write Queue elements and write one first packet
 *
 * @param td Pointer to trudpData
 *
 * @return Size of send packets
 */
size_t trudpProcessWriteQueue(trudpData *td) {

    int i = 0;
    size_t retval = 0;
    trudpMapElementData *el;
    trudpMapIterator *it;
    if((it = trudpMapIteratorNew(td->map))) {
        while(!retval && (el = trudpMapIteratorNext(it))) {
            if(i++ < td->writeQueueIdx) continue;
            trudpChannelData *tcd = (trudpChannelData *)
                    trudpMapIteratorElementData(el, NULL);
            retval = trudpProcessChannelWriteQueue(tcd);
            td->writeQueueIdx++;
        }
        trudpMapIteratorDestroy(it);
        if(!retval) td->writeQueueIdx = 0;
    }

    return retval;
}

size_t trudpWriteQueueSizeAll(trudpData *td) {

    size_t retval = 0;
    trudpMapElementData *el;
    trudpMapIterator *it;
    if((it = trudpMapIteratorNew(td->map))) {
        while((el = trudpMapIteratorNext(it))) {
            size_t data_lenth;
            trudpChannelData *tcd = (trudpChannelData *)
                    trudpMapIteratorElementData(el, &data_lenth);
            retval += trudpWriteQueueSize(tcd->writeQueue);
        }
        trudpMapIteratorDestroy(it);
    }

    return retval;
}

/**
 * Set last received field to current timestamp
 *
 * @param tcd
 */
static inline void trudpSetLastReceived(trudpChannelData *tcd) {
    tcd->lastReceived = trudpGetTimestampFull();
}

/**
 * Create ACK packet and send it back to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
static inline void trudpSendACK(trudpChannelData *tcd, void *packet) {

    void *packetACK = trudpPacketACKcreateNew(packet);
    #if !USE_WRITE_QUEUE
    trudpSendEvent(tcd, PROCESS_SEND, packetACK, trudpPacketACKlength(), NULL);
    #else
    trudpWriteQueueAdd(tcd->writeQueue, packetACK, NULL, trudpPacketACKlength());
    #endif
    trudpPacketCreatedFree(packetACK);
    trudpSetLastReceived(tcd);
}

/**
 * Create ACK to RESET packet and send it back to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
static inline void trudpSendACKtoRESET(trudpChannelData *tcd, void *packet) {

    void *packetACK = trudpPacketACKtoRESETcreateNew(packet);
    #if !USE_WRITE_QUEUE
    trudpSendEvent(tcd, PROCESS_SEND, packetACK, trudpPacketACKlength(), NULL);
    #else
    trudpWriteQueueAdd(tcd->writeQueue, packetACK, NULL, trudpPacketACKlength());
    #endif
    trudpPacketCreatedFree(packetACK);
    trudpSetLastReceived(tcd);
}

/**
 * Create ACK to PING packet and send it back to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
static inline void trudpSendACKtoPING(trudpChannelData *tcd, void *packet) {

    void *packetACK = trudpPacketACKtoPINGcreateNew(packet);
    #if !USE_WRITE_QUEUE
    trudpSendEvent(tcd, PROCESS_SEND, packetACK, trudpPacketGetPacketLength(packet), NULL);
    #else
    trudpWriteQueueAdd(tcd->writeQueue, NULL, packetACK, trudpPacketGetPacketLength);
    #endif
    trudpPacketCreatedFree(packetACK);
    trudpSetLastReceived(tcd);
}

/**
 * Create RESET packet and send it to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param data NULL
 * @param data_length 0
 */
static inline void trudpSendRESET(trudpChannelData *tcd, void* data, size_t data_length) {

    if(tcd) {
        trudpSendEvent(tcd, SEND_RESET, data, data_length, NULL);

        void *packetRESET = trudpPacketRESETcreateNew(trudpGetNewId(tcd), tcd->channel);
        #if !USE_WRITE_QUEUE
        trudpSendEvent(tcd, PROCESS_SEND, packetRESET, trudpPacketRESETlength(), NULL);
        #else
        trudpWriteQueueAdd(tcd->writeQueue, packetRESET, NULL, trudpPacketRESETlength());
        #endif
        trudpPacketCreatedFree(packetRESET);
    }
}

/**
 * Create RESET packet and send it to sender
 * 
 * @param tcd Pointer to trudpChannelData
 */
inline void trudpSendResetChannel(trudpChannelData *tcd) {
    trudpSendRESET(tcd, NULL, 0);
}

/**
 * Create RESET packet and send it to all channels
 *
 * @param td
 */
void trudpSendResetAll(trudpData *td) {

    trudpMapElementData *el;
    trudpMapIterator *it;
    if((it = trudpMapIteratorNew(td->map))) {
        while((el = trudpMapIteratorNext(it))) {
            trudpChannelData *tcd = (trudpChannelData *)
                    trudpMapIteratorElementData(el, NULL);

            trudpSendRESET(tcd, NULL, 0);
        }
        trudpMapIteratorDestroy(it);
    }
}

/**
 * Calculate Triptime
 *
 * @param tcd
 * @param packet
 * @param send_data_length
 */
static inline void trudpCalculateTriptime(trudpChannelData *tcd, void *packet,
        size_t send_data_length) {

    tcd->triptime = trudpGetTimestamp() - trudpPacketGetTimestamp(packet);

    // Calculate and set Middle Triptime value
    tcd->triptimeMiddle = tcd->triptimeMiddle == START_MIDDLE_TIME ? tcd->triptime * tcd->triptimeFactor : // Set first middle time
        tcd->triptime > tcd->triptimeMiddle ? tcd->triptime * tcd->triptimeFactor : // Set middle time to max triptime
            (tcd->triptimeMiddle * 19 + tcd->triptime) / 20.0; // Calculate middle value
    // Correct triptimeMiddle
    if(tcd->triptimeMiddle < tcd->triptime * tcd->triptimeFactor) tcd->triptimeMiddle = tcd->triptime * tcd->triptimeFactor;
    if(tcd->triptimeMiddle > tcd->triptime * 10) tcd->triptimeMiddle = tcd->triptime * 10;
    if(tcd->triptimeMiddle > MAX_TRIPTIME_MIDDLE) tcd->triptimeMiddle = MAX_TRIPTIME_MIDDLE;

    // Statistic
    tcd->stat.ack_receive++;
    tcd->stat.triptime_last = tcd->triptime;
    tcd->stat.wait = tcd->triptimeMiddle / 1000.0;
    trudpStatProcessLast10Send(tcd, packet, send_data_length);
}

/**
 * Default TR-UDP process read data from UDP
 * h
 * @param td
 * @param data
 * @param data_length
 */
void trudpProcessReceive(trudpData *td, void *data, size_t data_length) {

    struct sockaddr_in remaddr; // remote address
    socklen_t addr_len = sizeof(remaddr);
    ssize_t recvlen = trudpUdpRecvfrom(td->fd, data, data_length,
            (__SOCKADDR_ARG)&remaddr, &addr_len);

    // Process received packet
    if(recvlen > 0) {
        size_t data_length;
        trudpChannelData *tcd = trudpCheckRemoteAddr(td, &remaddr, addr_len, 0);
        if(tcd == (void *)-1 ||
           trudpProcessChannelReceivedPacket(tcd, data, recvlen, &data_length) == (void *)-1) {

            if(tcd == (void *)-1) printf("!!! can't PROCESS_RECEIVE_NO_TRUDP\n");
            else
            trudpSendEvent(tcd, PROCESS_RECEIVE_NO_TRUDP, data, recvlen, NULL);
        }
    }
}

/**
 * Process received packet
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 * @param packet_length Packet length
 * @param data_length Pointer to variable to return packets data length
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
        int type = trudpPacketGetType(packet);
        switch(type) {

            // ACK to DATA packet received
            case TRU_ACK: {

                // Find packet in send queue by id
                size_t send_data_length = 0;
                trudpPacketQueueData *tpqd = trudpPacketQueueFindById(
                    tcd->sendQueue, trudpPacketGetId(packet)
                );
                if(tpqd) {
                                        
                    // Process ACK data callback                    
                    trudpSendEvent(tcd, GOT_ACK, tpqd->packet, tpqd->packet_length, NULL);

                    // Remove packet from send queue
                    send_data_length = trudpPacketGetDataLength(tpqd->packet);
                    trudpPacketQueueDelete(tcd->sendQueue, tpqd);
                    TD(tcd)->stat.sendQueue.size_current--;
                }

                // Calculate triptime
                trudpCalculateTriptime(tcd, packet, send_data_length);
                trudpSetLastReceived(tcd);

                // Reset if id is too big and send queue is empty
                //goto skip_reset_after_id;
                if(tcd->sendId >= RESET_AFTER_ID &&
                   !trudpQueueSize(tcd->sendQueue->q) &&
                   !trudpQueueSize(tcd->receiveQueue->q) ) {

                    // Send event
                    trudpSendRESET(tcd, &tcd->sendId, sizeof(tcd->sendId));
                }
                //skip_reset_after_id: ;

            } break;

            // ACK to RESET packet received
            case TRU_ACK | TRU_RESET: {

                // Send event
                trudpSendEvent(tcd, GOT_ACK_RESET, NULL, 0, NULL);

                // Reset TR-UDP
                trudpResetChannel(tcd);

                // Statistic
                tcd->stat.ack_receive++;

            } break;

            // ACK to PING packet received
            case TRU_ACK | TRU_PING: /*TRU_ACK_PING:*/ {

                // Calculate Triptime
                trudpCalculateTriptime(tcd, packet, packet_length);
                trudpSetLastReceived(tcd);

                // Send event
                trudpSendEvent(tcd, GOT_ACK_PING,
                        trudpPacketGetData(packet),
                        trudpPacketGetDataLength(packet),
                        NULL);

            } break;

            // PING packet received
            case TRU_PING: {

                // Send event
                trudpSendEvent(tcd, GOT_PING,
                        trudpPacketGetData(packet),
                        trudpPacketGetDataLength(packet),
                        NULL);

                // Calculate Triptime
                trudpCalculateTriptime(tcd, packet, packet_length);

                // Create ACK packet and send it back to sender
                trudpSendACKtoPING(tcd, packet);

                // Statistic
                tcd->stat.packets_receive++;
                trudpStatProcessLast10Receive(tcd, packet);

                tcd->outrunning_cnt = 0; // Reset outrunning flag

            } break;

            // DATA packet received
            case TRU_DATA: {

                // Create ACK packet and send it back to sender
                trudpSendACK(tcd, packet);

                // Reset when wait packet with id 0 and receive packet with another id
                if(!tcd->receiveExpectedId && trudpPacketGetId(packet)) {

                    // Send event
                    uint32_t id = trudpPacketGetId(packet);
                    trudpSendEvent(tcd, SEND_RESET, NULL, 0, NULL);
                    trudpSendRESET(tcd, &id, sizeof(id));
                }

                else {
                    // Check expected Id and return data
                    if(trudpPacketGetId(packet) == tcd->receiveExpectedId) {

                        // Send event
                        data = trudpPacketGetData(packet);
                        *data_length = trudpPacketGetDataLength(packet);
                        trudpSendEvent(tcd, GOT_DATA,
                                data,
                                *data_length,
                                NULL);

                        // Check received queue for saved packet with expected id
                        trudpPacketQueueData *tqd;
                        while((tqd = trudpPacketQueueFindById(tcd->receiveQueue,
                                ++tcd->receiveExpectedId)) ) {

                            // Send event
                            data = trudpPacketGetData(tqd->packet);
                            *data_length = trudpPacketGetDataLength(tqd->packet);
                            trudpSendEvent(tcd, GOT_DATA,
                                data,
                                *data_length,
                                NULL);

                            trudpPacketQueueDelete(tcd->receiveQueue, tqd);
                        }

                        // Statistic
                        tcd->stat.packets_receive++;
                        trudpStatProcessLast10Receive(tcd, packet);

                        tcd->outrunning_cnt = 0; // Reset outrunning flag
                    }

                    // Save outrunning packet to receiveQueue
                    else if(trudpPacketGetId(packet) > tcd->receiveExpectedId) {

                        if(!trudpPacketQueueFindById(tcd->receiveQueue,
                                trudpPacketGetId(packet)) ) {

                            trudpPacketQueueAdd(tcd->receiveQueue, packet, packet_length, 0);
                            tcd->outrunning_cnt++; // Increment outrunning count

                            // Statistic
                            tcd->stat.packets_receive++;
                            trudpStatProcessLast10Receive(tcd, packet);

    //                        goto skip_reset_2;
    //                        // Send reset at maximum outrunning
    //                        if(tcd->outrunning_cnt > MAX_OUTRUNNING) {
    //                            // \todo Send event
    //                            fprintf(stderr,
    //                                "To match TR-UDP channel outrunning! "
    //                                "Reset channel ...\n");
    //                            trudpSendRESET(tcd);
    //                        }
    //                        skip_reset_2: ;
                        }
                    }

                    // Reset channel if packet id = 0
                    else if(!trudpPacketGetId(packet)) {
                        // Send event
                        trudpSendRESET(tcd, NULL, 0);
                    }

                    // Skip already processed packet
                    else {

                        // Statistic
                        tcd->stat.packets_receive_dropped++;
                        trudpStatProcessLast10Receive(tcd, packet);
                    }
                }

            } break;

            // RESET packet received
            case TRU_RESET: {

                // Send event
                trudpSendEvent(tcd, GOT_RESET, NULL, 0, NULL);

                // Create ACK to RESET packet and send it back to sender
                trudpSendACKtoRESET(tcd, packet);

                // Reset TR-UDP
                trudpResetChannel(tcd);

            } break;

            // An undefined type of packet (skip it)
            default: {

                // Return error code
                data = (void *)-1;

            } break;
        }
    }
    // Packet is not TR-UDP packet
    else data = (void *)-1;

    return data;
}

/**
 * Check that channel is disconnected and send DISCONNECTED event
 *
 * @param tcd Pointer to trudpChannelData
 * @param ts Current timestamp
 * @return
 */
static int trudpCheckChannelDisconnect(trudpChannelData *tcd, uint64_t ts) {

    // Disconnect channel at long last receive
    if(tcd->lastReceived && ts - tcd->lastReceived > MAX_LAST_RECEIVE) {

        // Send disconnect event
        uint32_t lastReceived = ts - tcd->lastReceived;        
        trudpSendEvent(tcd, DISCONNECTED,
            &lastReceived, sizeof(lastReceived), NULL);
        return -1;
    }
    return 0;
}

/**
 * Check send Queue elements and resend elements with expired time
 *
 * @param tcd Pointer to trudpChannelData
 * @param ts Current timestamp
 * @param next_expected_time [out] Next expected time: return expected time
 *          of next queue record or zero if not found, it may be NULL than not
 *          returned
 *
 * @return Number of resend packets or -1 if the channel was disconnected
 */
int trudpProcessChannelSendQueue(trudpChannelData *tcd, uint64_t ts,
        uint64_t *next_expected_time) {

    int rv = 0;

    trudpPacketQueueData *tqd;
    if((tqd = trudpPacketQueueGetFirst(tcd->sendQueue)) &&
            tqd->expected_time <= ts ) {

        // Move and change records with expected time
        tqd->expected_time = trudpCalculateExpectedTime(tcd, ts) + 100000 * tqd->retrieves;
        //printf("start sq after %.3f\n", (tqd->expected_time - ts) / 1000.0);
        trudpPacketQueueMoveToEnd(tcd->sendQueue, tqd);
        tcd->stat.packets_attempt++; // Attempt(repeat) statistic parameter increment
        if(!tqd->retrieves) tqd->retrieves_start = ts;

        tqd->retrieves++;
        rv++;

        // Resend data
        #if !USE_WRITE_QUEUE
        trudpSendEvent(tcd, PROCESS_SEND, tqd->packet, tqd->packet_length, NULL);
        #else
        trudpWriteQueueAdd(tcd->writeQueue, NULL, tqd->packet, tqd->packet_length);
        #endif

        // Disconnect at max retrieves
//        goto skip_disconnect_on_max_retrives;
//        if(tqd->retrieves > MAX_RETRIEVES ||
//           ts - tqd->retrieves_start > MAX_TRIPTIME_MIDDLE ) {
//
//            char *key = trudpMakeKeyCannel(tcd);
//            // \todo Send event
//            fprintf(stderr, "Disconnect channel %s, wait: %.6f\n", key,
//                (ts - tqd->retrieves_start) / 1000000.0);
//            trudpExecEventCallback(tcd, DISCONNECTED, key, strlen(key) + 1,
//                TD(tcd)->user_data, TD(tcd)->evendCb);
//
//            trudpDestroyChannel(tcd);
//
//            //exit(-1);
//            return -1;
//        }
//        skip_disconnect_on_max_retrives: ;

        // Disconnect channel at long last receive
        if(trudpCheckChannelDisconnect(tcd, ts) == -1) {
            tqd = NULL;
            rv = -1;
        }
        else {
            // Get next value
            tqd = trudpPacketQueueGetFirst(tcd->sendQueue);
        }
    }

    // If record exists
    if(next_expected_time) *next_expected_time = tqd ? tqd->expected_time : 0;

    return rv;
}

/**
 * Check all peers send Queue elements and resend elements with expired time
 *
 * @param td Pointer to trudpData
 * @param next_et [out] Next expected time
 *
 * @return Number of resend packets
 */
int trudpProcessSendQueue(trudpData *td, uint64_t *next_et) {

    int retval, rv = 0;
    uint64_t ts = trudpGetTimestampFull(), min_expected_time, next_expected_time;
    trudpMapElementData *el;
    do {
        retval = 0;
        trudpMapIterator *it;
        //min_expected_time = 0;
        min_expected_time = UINT64_MAX;
        if((it = trudpMapIteratorNew(td->map))) {
            while((el = trudpMapIteratorNext(it))) {
                trudpChannelData *tcd = (trudpChannelData *)trudpMapIteratorElementData(el, NULL);
                retval = trudpProcessChannelSendQueue(tcd, ts, &next_expected_time);
                if(retval < 0) break;
                if(retval > 0) rv += retval;
                if(next_expected_time && next_expected_time < min_expected_time)
                    min_expected_time = next_expected_time;
            }
            trudpMapIteratorDestroy(it);
        }
    } while(retval == -1 || (retval > 0 && /*min_expected_time != UINT64_MAX &&*/ min_expected_time <= ts));

    if(next_et) *next_et = (min_expected_time != UINT64_MAX) ? min_expected_time : 0;

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
            trudpChannelData *tcd = (trudpChannelData *)
                    trudpMapIteratorElementData(el, NULL);

            if(tcd->connected_f) {
                if(tcd->sendQueue->q->length < 100) { // drop packets if send queue > 100 \todo move it to Send Data function or something else
                    if(trudpSendData(tcd, data, data_length) < 0) break;
                    rv++;
                }
            }
        }
        trudpMapIteratorDestroy(it);
    }

    return rv;
}

/**
 * Keep connection at idle line
 *
 * @param td
 * @return
 */
size_t trudpKeepConnection(trudpData *td) {

    int rv = -1;

    trudpMapIterator *it;
    trudpMapElementData *el;
    uint64_t ts = trudpGetTimestampFull();
    while(rv == -1 && (it = trudpMapIteratorNew(td->map))) {
        rv = 0;
        while((el = trudpMapIteratorNext(it))) {
            trudpChannelData *tcd = (trudpChannelData *)
                trudpMapIteratorElementData(el, NULL);

            if(tcd->connected_f && ts - tcd->lastReceived > SEND_PING_AFTER) {
                if(trudpCheckChannelDisconnect(tcd, ts) == -1) {
                    rv = -1;
                    break;
                }
                trudpSendPING(tcd, "PING", 5);
                rv++;
            }
        }
        trudpMapIteratorDestroy(it);
    }

    return rv;
}

/**
 * Get maximum send queue size of all channels
 *
 * @param td Pointer to trudpData
 * @return Maximum send queue size of all channels or zero if all queues is empty
 */
size_t trudpGetSendQueueMax(trudpData *td) {

    int rv = 0;

    trudpMapIterator *it;
    trudpMapElementData *el;
    if((it = trudpMapIteratorNew(td->map))) {
        while((el = trudpMapIteratorNext(it))) {
            trudpChannelData *tcd = (trudpChannelData *)
                    trudpMapIteratorElementData(el, NULL);
            int size = trudpPacketQueueSize(tcd->sendQueue);
            if(size > rv) rv = size;
        }
        trudpMapIteratorDestroy(it);
    }

    return rv;
}

/**
 * Get maximum receive queue size of all channels
 *
 * @param td Pointer to trudpData
 * @return Maximum send queue size of all channels or zero if all queues is empty
 */
size_t trudpGetReceiveQueueMax(trudpData *td) {

    int rv = 0;

    trudpMapIterator *it;
    trudpMapElementData *el;
    if((it = trudpMapIteratorNew(td->map))) {
        while((el = trudpMapIteratorNext(it))) {
            trudpChannelData *tcd = (trudpChannelData *)
                    trudpMapIteratorElementData(el, NULL);
            int size = trudpPacketQueueSize(tcd->receiveQueue);
            if(size > rv) rv = size;
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
 * @param key_length [out] Pointer to keys length (may be NULL)
 *
 * @return Static buffer with key ip:port:channel
 */
char *trudpMakeKey(char *addr, int port, int channel, size_t *key_length) {

    static char buf[MAX_KEY_LENGTH];
    size_t kl = snprintf(buf, MAX_KEY_LENGTH, "%s:%u:%u", addr, port, channel);
    if(key_length) *key_length = kl;

    return buf;
}

/**
 * Make key from channel data
 *
 * @param tcd Pointer to trudpChannelData
 *
 * @return Static buffer with key ip:port:channel
 */
char *trudpMakeKeyChannel(trudpChannelData *tcd) {

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
 * @param channel TR-UDP channel
 *
 * @return Pointer to trudpChannelData or (void*)-1 at error
 */
trudpChannelData *trudpCheckRemoteAddr(trudpData *td,
        struct sockaddr_in *remaddr, socklen_t addr_length, int channel) {

    int port;
    size_t data_length, key_length;
    char *addr_str = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)remaddr, &port);
    char *key = trudpMakeKey(addr_str, port, channel, &key_length);
    trudpChannelData *tcd = (trudpChannelData *)trudpMapGet(td->map, key,
            key_length, &data_length);

    if(tcd == (void*)-1) {
        tcd = trudpNewChannel(td, addr_str, port, channel);
        if(tcd != (void*)-1)
            trudpSendEvent(tcd, CONNECTED, NULL, 0, NULL);
    }

    if(tcd != (void*)-1) tcd->connected_f = 1;

    return tcd;
}

/**
 * Get trudpChannelData by socket address and channel number
 * @param td Pointer to trudpData
 * @param addr Pointer to address structure
 * 
 * @return Pointer to trudpChannelData or (void*)-1 if not found
 */
trudpChannelData *trudpGetChannel(trudpData *td, __CONST_SOCKADDR_ARG addr,
        int channel) {

    int port;
    size_t data_length, key_length;
    char *addr_str = trudpUdpGetAddr(addr, &port);
    char *key = trudpMakeKey(addr_str, port, channel, &key_length);
    trudpChannelData *tcd = (trudpChannelData *)trudpMapGet(td->map, key,
        key_length, &data_length);

    return tcd;
}

/**
 * Get channel send queue timeout
 *
 * @param tcd Pointer to trudpChannelData
 * @return Send queue timeout (may by 0) or UINT32_MAX if send queue is empty
 */
uint32_t trudpGetChannelSendQueueTimeout(trudpChannelData *tcd) {

    // Get sendQueue timeout
    uint32_t timeout_sq = UINT32_MAX;
    if(tcd->sendQueue->q->first) {
        trudpPacketQueueData *pqd = (trudpPacketQueueData *)tcd->sendQueue->q->first->data;
        uint64_t expected_t = pqd->expected_time, current_t = trudpGetTimestampFull();
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
            trudpChannelData *tcd = (trudpChannelData *)trudpMapIteratorElementData(el, NULL);
            uint32_t ts = trudpGetChannelSendQueueTimeout(tcd);
            if(ts < timeout_sq) timeout_sq = ts;
            if(!timeout_sq) break;
        }
        trudpMapIteratorDestroy(it);
    }

    return timeout_sq;
}
