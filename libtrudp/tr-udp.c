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
#include "tr-udp.h"
#include "packet.h"

/**
 * Create trudp chanel
 *
 * @return
 */
trudpData *trudpNew() {

    trudpData *td = (trudpData *) malloc(sizeof(trudpData));

    td->sendId = 0;
    td->triptime = MAX_ACK_WAIT * 1000000; // (500 ms) = 500000; 10000;
    td->sendQueue = trudpTimedQueueNew();

    td->receiveExpectedId = 0;
    td->receiveQueue = trudpTimedQueueNew();

    return td;
}

/**
 * Destroy trudp chanel
 *
 * @param td Pointer to trudpData
 */
void trudpDestroy(trudpData *td) {

    trudpTimedQueueDestroy(td->sendQueue);
    trudpTimedQueueDestroy(td->receiveQueue);
    free(td);
}

/**
 * Free (reset) trudp chanel
 *
 * @param td Pointer to trudpData
 */
void trudpFree(trudpData *td) {

    trudpTimedQueueFree(td->sendQueue);
    trudpTimedQueueFree(td->receiveQueue);

    td->sendId = 0;
    td->receiveExpectedId = 0;
    td->triptime = MAX_ACK_WAIT * 1000000;
}

/**
 * Get new send Id
 *
 * @param td Pointer to trudpData
 * @return New send Id
 */
static inline uint32_t trudpGetNewId(trudpData *td) {

    return ++td->sendId;
}

// \todo Send data (add to write queue)
static size_t trudpWriteQueueAdd(trudpData *td, void *packet, size_t  packetLength) {

    return 0;
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
    void *packetDATA = trudpPacketDATAcreateNew(trudpGetNewId(td), data, data_length, &packetLength);

    // Save packet to send queue
    trudpTimedQueueAdd(td->sendQueue, packetDATA, packetLength, trudpHeaderTimestamp() + td->triptime);

    // Send data (add to write queue)
    trudpWriteQueueAdd(td, packetDATA, packetLength);

    return packetLength;
}

/**
 * Process received packet
 *
 * @param td Pointer to trudpData
 * @param packet Pointer to received packet
 * @param packet_length Packet length
 * @param data_length Pointer to variable to return packets data length
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

        // Get data
        data = trudpPacketGetData(packet);        

        switch(trudpPacketGetDataType(packet)) {

            // ACK packet received
            case TRU_ACK:
                // Remove packet from send queue
                trudpTimedQueueDelete(
                    td->sendQueue,
                    trudpTimedQueueFindById(td->sendQueue, trudpPacketGetId(packet))
                );
                break;

            // DATA packet received
            case TRU_DATA: {
                
                // Create ACK packet and send it
                void *packetACK = trudpPacketACKcreateNew(packet);
                trudpWriteQueueAdd(td, packetACK, trudpPacketACKlength());
                trudpPacketCreatedFree(packetACK);

                // Check expected Id and return data
                if(trudpPacketGetId(packet) == td->receiveExpectedId) {
                    data = trudpPacketGetData(packet);
                    *data_length = (size_t)trudpPacketGetDataLength(packet);
                    // \todo check received queue for saved packet with expected id
                    td->receiveExpectedId++;
                }
                // Save packet to receiveQueue
                else {
                    trudpTimedQueueAdd(td->receiveQueue, packet, packet_length, 0);
                }

            } break;

            // RESET packet received
            case TRU_RESET:
                
                // Reset TR-UDP
                trudpFree(td);
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
