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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "send_queue.h"
#include "queue.h"

typedef struct trudpSendQueue {
    
    trudpQueue *q;
    
} trudpSendQueue;

typedef struct trudpSendQueueData {
    
    uint32_t expected_time;
    char packet[];
    
} trudpSendQueueData;


/**
 * Create new send queue
 * 
 * @return Pointer to trudpSendQueue
 */
inline trudpSendQueue *trudpSendQueueNew() {
    
    trudpSendQueue *sq = (trudpSendQueue *)malloc(sizeof(trudpSendQueue));
    sq->q = trudpQueueNew();
    
    return sq;
}

/**
 * Destroy send queue
 * 
 * @param sq Pointer to trudpSendQueue
 */
inline void trudpSendQueueDestroy(trudpSendQueue *sq) {

    trudpQueueDestroy(sq->q);
    free(sq);
}

/**
 * Remove all elements from send queue
 * 
 * @param sq Pointer to trudpSendQueue
 * @return Zero at success
 */
inline int trudpSendQueueFree(trudpSendQueue *sq) {

    return trudpQueueFree(sq->q);
}

/**
 * Add packet to send queue
 * 
 * @param sq Pointer to trudpSendQueue
 * @param packet Packet to add to queue
 * @param packet_length Packet length
 * @param expected_time Packet expected time
 * 
 * @return Pointer to added trudpSendQueueData
 */
trudpSendQueueData *trudpSendQueueAdd(trudpSendQueue *sq, void *packet, size_t packet_length, 
        uint32_t expected_time) {
    
    size_t sqd_length = sizeof(trudpSendQueueData) + packet_length;    
    trudpSendQueueData *sqd = (trudpSendQueueData *) trudpQueueAdd(sq->q, NULL, sqd_length);
    sqd->expected_time = expected_time;
    memcpy(sqd, packet, packet_length);
    
    return sqd;
}

/**
 * Remove element from send queue
 * 
 * @param sq Pointer to trudpSendQueue
 * @param sqd Pointer to trudpSendQueueData to delete it
 * 
 * @return Zero at success
 */
inline int trudpSendQueueDelete(trudpSendQueue *sq, trudpSendQueueData *sqd) {
    
    return trudpQueueDelete(sq->q, (trudpQueueData *)sqd);
}

/**
 * Find send queue data by Id
 * 
 * @param sq Pointer to trudpSendQueue
 * @param id Id to find in send queue
 * 
 * @return Pointer to trudpSendQueueData or NULL if not found
 */
trudpSendQueueData *trudpSendQueueFindById(trudpSendQueue *sq, uint32_t id) {
    
    trudpSendQueueData *rv = NULL;
            
    trudpQueueIterator *it = trudpQueueIteratorNew(sq->q);
    if(it != NULL) {
        
        while(trudpQueueIteratorNext(it)) {
            
            trudpSendQueueData *sqd = (trudpSendQueueData *) trudpQueueIteratorElement(it);
            if(trudpPacketGetId(sqd->packet) == id) {
                rv = sqd;
                break;
            }
        }
        trudpQueueIteratorFree(it);
    }
    
    return rv;
}

/**
 * Find send queue data by time
 * 
 * @param sq Pointer to trudpSendQueue
 * @param t Time to find (current time usually). This function will find first 
 *          record with expected_time less or equal then this time.
 * 
 * @return  Pointer to trudpSendQueueData or NULL if not found
 */
trudpSendQueueData *trudpSendQueueFindByTime(trudpSendQueue *sq, uint32_t t) {
    
    trudpSendQueueData *rv = NULL;
            
    trudpQueueIterator *it = trudpQueueIteratorNew(sq->q);
    if(it != NULL) {
        
        while(trudpQueueIteratorNext(it)) {
            
            trudpSendQueueData *sqd = (trudpSendQueueData *) trudpQueueIteratorElement(it);
            if(sqd->expected_time <= t) {
                rv = sqd;
                break;
            }
        }
        trudpQueueIteratorFree(it);
    }
    
    return rv;
}
