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
    td->triptime = 10000;
    td->sendQueue = trudpTimedQueueNew();
    
    td->receiveExpectedId = 0;
    td->receiveQueue = trudpTimedQueueNew();
    
    return td;
}

/**
 * Destroy 
 * @param td
 */
void trudpDestroy(trudpData *td) {
    
    trudpTimedQueueDestroy(td->sendQueue);
    trudpTimedQueueDestroy(td->receiveQueue);    
    free(td);
}

static inline uint32_t trudpGetNewId(trudpData *td) {
    
    return ++td->sendId;
}

size_t trudpSendData(trudpData *td, void *data, size_t data_length) {
    
    // Create DATA package
    size_t packetLength;
    void *packetDATA = trudpPacketDATAcreateNew(trudpGetNewId(td), data, data_length, &packetLength);
    
    // Save packet to send queue
    trudpTimedQueueAdd(td->sendQueue, packetDATA, packetLength, trudpHeaderTimestamp() + td->triptime);
    
    // Send data
    // ...
    
    return 0;
}

void *trudpReceive(trudpData *td, void *packet, size_t packet_length, size_t *data_length) {
    
    void *data = (void *)-1;
    
    if(trudpPacketCheck(packet, packet_length)) {
        
        // Get data
        data = trudpPacketGetData(packet);
        *data_length = (size_t)trudpPacketGetDataLength(packet);
        
        switch(trudpPacketGetDataType(packet)) {
            
            case TRU_ACK:
            case TRU_DATA:    
            case TRU_RESET:    
            default:
                data = (void *)-1;
                break;
        }
    }
    
    return data;
}
