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
 * Created on May 31, 2016, 1:44 AM
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tr-udp.h"
#include "packet.h"

/**
 * Set default trudpData values
 * 
 * @param td
 */
static void trudpSetDefaults(trudpData *td) {

    td->sendId = 0;
    td->triptime = 0; 
    td->outrunning_cnt = 0;
    td->receiveExpectedId = 0;
    td->triptimeMiddle = START_MIDDLE_TIME;
}

/**
 * Create trudp chanel
 *
 * @param user_data Pointer to user data
 * @return 
 */
trudpData *trudpNew(void *user_data) {

    trudpData *td = (trudpData *) malloc(sizeof(trudpData));
    memset(td, 0, sizeof(trudpData));
    
    td->user_data = user_data;
    td->sendQueue = trudpPacketQueueNew();
    td->receiveQueue = trudpPacketQueueNew();
    
    // Set UDP connection depended defaults
    td->fd = 0;
    td->addrlen = sizeof(td->remaddr);
    
    // Set other defaults
    trudpSetDefaults(td);

    return td;
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
 * Destroy trudp chanel
 *
 * @param td Pointer to trudpData
 */
void trudpDestroy(trudpData *td) {

    trudpPacketQueueDestroy(td->sendQueue);
    trudpPacketQueueDestroy(td->receiveQueue);
    free(td);
}

/**
 * Free trudp chanel
 *
 * @param td Pointer to trudpData
 */
void trudpFree(trudpData *td) {

    trudpPacketQueueFree(td->sendQueue);
    trudpPacketQueueFree(td->receiveQueue);
    trudpSetDefaults(td);    
}

/**
 * Reset trudp chanel
 *
 * @param td Pointer to trudpData
 */
inline void trudpReset(trudpData *td) {
    trudpFree(td);
}

/**
 * Get new send Id
 *
 * @param td Pointer to trudpData
 * @return New send Id
 */
static inline uint32_t trudpGetNewId(trudpData *td) {

    return td->sendId++;
}

/**
 * Call Send data callback
 * 
 * @param td Pointer to trudpData
 * @param packet Pointer to packet
 * @param packetLength Packet length
 * 
 * @return 
 */
static inline size_t trudpExecSendPacketCallback(trudpData *td, void *packet,
        size_t  packetLength) {

    if(td->sendCb) td->sendCb(td, packet, packetLength, td->user_data);
        
    return 0;
}

/**
 * Calculate Expected Time
 * 
 * @param td Pointer to trudpData
 * @param current_time Current time (nsec)
 * 
 * @return Current time plus 
 */
static inline uint32_t trudpCalculateExpectedTime(trudpData *td, 
        uint32_t current_time) {
    
    return current_time + td->triptimeMiddle;
}

/**
 * Send data
 *
 * @param td Pointer to trudpData
 * @param data Pointer to send data
 * @param data_length Data length
 *
 * @return Zero on error
 */
size_t trudpSendData(trudpData *td, void *data, size_t data_length) {

    // Create DATA package
    size_t packetLength;
    void *packetDATA = trudpPacketDATAcreateNew(trudpGetNewId(td), data, 
            data_length, &packetLength);

    // Save packet to send queue
    trudpPacketQueueAdd(td->sendQueue, packetDATA, packetLength,
            trudpCalculateExpectedTime(td, trudpGetTimestamp()));

    // Send data (add to write queue)
    trudpExecSendPacketCallback(td, packetDATA, packetLength);
    
    // Free created packet
    trudpPacketCreatedFree(packetDATA);

    return packetLength;
}

/**
 * Execute trudpProcessReceivedPacket callback
 * 
 * @param packet Pointer to trudpData
 * @param data
 * @param data_length
 * @param user_data
 * @param cb
 */
static void trudpExecProcessDataCallback(trudpData *td, void *packet, void **data, 
        size_t *data_length, void *user_data, trudpDataCb cb) {

    *data = trudpPacketGetData(packet);  
    *data_length = (size_t)trudpPacketGetDataLength(packet);
    if(cb != NULL) cb(td, *data, *data_length, user_data);                
}

/**
 * Create ACK packet and send it back to sender
 * 
 * @param td Pointer to trudpData
 * @param packet Pointer to received packet
 */
static inline void trudpSendACK(trudpData *td, void *packet) {
    
    void *packetACK = trudpPacketACKcreateNew(packet);
    trudpExecSendPacketCallback(td, packetACK, trudpPacketACKlength());
    trudpPacketCreatedFree(packetACK);
}

/**
 * Create ACK to RESET packet and send it back to sender
 * 
 * @param td Pointer to trudpData
 * @param packet Pointer to received packet
 */
static inline void trudpSendACKtoRESET(trudpData *td, void *packet) {
    
    void *packetACK = trudpPacketACKtoRESETcreateNew(packet);
    trudpExecSendPacketCallback(td, packetACK, trudpPacketACKlength());
    trudpPacketCreatedFree(packetACK);
}

/**
 * Create RESET packet and send it to sender
 * 
 * @param td Pointer to trudpData
 * @param packet Pointer to received packet
 */
static inline void trudpSendRESET(trudpData *td) {
    
    void *packetRESET = trudpPacketRESETcreateNew(trudpGetNewId(td));
    trudpExecSendPacketCallback(td, packetRESET, trudpPacketRESETlength());
    trudpPacketCreatedFree(packetRESET);
}

/**
 * Process received packet
 *
 * @param td Pointer to trudpData
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
void *trudpProcessReceivedPacket(trudpData *td, void *packet,
        size_t packet_length, size_t *data_length) {

    void *data = NULL;
    *data_length = 0;

    // Check and process TR-UDP packet
    if(trudpPacketCheck(packet, packet_length)) {

        // Check packet type
        switch(trudpPacketGetType(packet)) {

            // ACK to DATA packet received
            case TRU_ACK:
                
                // Remove packet from send queue
                trudpPacketQueueDelete(
                    td->sendQueue,
                    trudpPacketQueueFindById(
                        td->sendQueue, trudpPacketGetId(packet)
                    )
                );
                
                // Set triptime
                td->triptime = trudpGetTimestamp() - 
                    trudpPacketGetTimestamp(packet);

                // Calculate and set middle triptime value
                td->triptimeMiddle = td->triptimeMiddle == START_MIDDLE_TIME ? td->triptime * 1.5 : // Set first middle time
                    td->triptime > td->triptimeMiddle ? td->triptime : // Set middle time to max triptime
                        (td->triptimeMiddle * 9 + td->triptime) / 10.0; // Calculate middle value

                // Process ACK data callback
                trudpExecProcessDataCallback(td, packet, &data, data_length, 
                        td->user_data, td->processAckCb);
                break;
                
            // ACK to RESET packet received    
            case TRU_ACK | TRU_RESET:

                // \todo Send event
                fprintf(stderr, "Got TRU_ACK of RESET packet\n");

                // Reset TR-UDP
                trudpReset(td);                    
                break;
                
            // DATA packet received
            case TRU_DATA: {
                
                // Create ACK packet and send it back to sender
                trudpSendACK(td, packet);

                // Check expected Id and return data
                if(trudpPacketGetId(packet) == td->receiveExpectedId) {
                    
                    // Execute trudpDataReceivedCb Callback with pointer to data
                    trudpExecProcessDataCallback(td, packet, &data, data_length, 
                            td->user_data, td->processDataCb);
                    
                    // Check received queue for saved packet with expected id
                    trudpPacketQueueData *tqd;
                    while((tqd = trudpPacketQueueFindById(td->receiveQueue, 
                            ++td->receiveExpectedId)) ) {       
                        
                        trudpExecProcessDataCallback(td, tqd->packet, &data, 
                                data_length, td->user_data, td->processDataCb);
                    }
                    
                    td->outrunning_cnt = 0; // Reset outrunning flag
                }
                // Save outrunning packet to receiveQueue
                else if(trudpPacketGetId(packet) > td->receiveExpectedId) {
                    
                  if(!trudpPacketQueueFindById(td->receiveQueue, trudpPacketGetId(packet)) ) 
                    trudpPacketQueueAdd(td->receiveQueue, packet, packet_length, 0);
                  
                  td->outrunning_cnt++; // Increment outrunning count
                  
                  // \todo Send reset at match outrunning
                  if(td->outrunning_cnt > MAX_OUTRUNNING) {
                    fprintf(stderr, "To match TR-UDP channel outrunning! Reset channel ...\n");
                    trudpSendRESET(td);
                    //exit(-12);
                  }
                }
                // Skip already processed packet
                else;

            } break;

            // RESET packet received
            case TRU_RESET:
                
                // \todo Send event
                fprintf(stderr, "Got TRU_RESET packet\n");
                
                // Create ACK to RESET packet and send it back to sender
                trudpSendACKtoRESET(td, packet);
//                fprintf(stderr, "Got TRU_RESET packet - 2\n");
                
                // Reset TR-UDP
                trudpReset(td);
//                fprintf(stderr, "Got TRU_RESET packet - 3\n");
                
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
 * @param td Pointer to trudpData
 * 
 * @return Number of resend packet
 */
int trudpProcessSendQueue(trudpData *td) {
    
    int rv = 0;
    
    trudpPacketQueueData *tqd;
    uint32_t ts = trudpGetTimestamp();
    while((tqd = trudpPacketQueueFindByTime(td->sendQueue, ts))) {
        
        // Resend data and change it expected time
        trudpExecSendPacketCallback(td, tqd->packet, tqd->packet_length);
        trudpPacketQueueMoveToEnd(td->sendQueue, tqd);
        tqd->expected_time = trudpCalculateExpectedTime(td, ts); 
        td->triptimeMiddle *= 2;
        tqd->retrieves++;
        rv++;
        
        // \todo Stop at match retrieves
        if(tqd->retrieves > MAX_RETRIEVES) {
            fprintf(stderr, "To match TR-UDP channel retrieves! Stop executing ...\n");
            exit(-11);
        }
    }
    
    return rv;
}

/**
 * Save remote address to trudpData variable
 * 
 * @param td Pointer to trudpData
 * @param remaddr Pointer to sockaddr_in remote address
 * @param addr_length Remote address length
 */
inline void trudpSaveRemoteAddr(trudpData *td, struct sockaddr_in *remaddr, 
        socklen_t addr_length) {
    
    if(!td->connected_f) {
        memcpy(&td->remaddr, remaddr, addr_length);
        td->addrlen = addr_length;
        td->connected_f = 1;
    }    
}
