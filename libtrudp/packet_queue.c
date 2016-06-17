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

/**
 * Packet queue: extended queue module used as TR-UDP send and receive queue
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "packet_queue.h"
#include "packet.h"

/**
 * Create new Packet queue
 * 
 * @return Pointer to trudpPacketQueue
 */
inline trudpPacketQueue *trudpPacketQueueNew() {
    
    trudpPacketQueue *tq = (trudpPacketQueue *)malloc(sizeof(trudpPacketQueue));
    tq->q = trudpQueueNew();
    
    return tq;
}

/**
 * Destroy Packet queue
 * 
 * @param tq Pointer to trudpPacketQueue
 */
inline void trudpPacketQueueDestroy(trudpPacketQueue *tq) {

    if(tq) {
        trudpQueueDestroy(tq->q);
        free(tq);
    }
}

/**
 * Remove all elements from Packet queue
 * 
 * @param tq Pointer to trudpPacketQueue
 * @return Zero at success
 */
inline int trudpPacketQueueFree(trudpPacketQueue *tq) {

    return tq && tq->q ? trudpQueueFree(tq->q) : -1;
}

/**
 * Get pointer to trudpQueueData from trudpPacketQueueData pointer
 * @param tqd Pointer to trudpPacketQueueData
 * @return Pointer to trudpQueueData or NULL if tqd is NULL
 */
inline trudpQueueData *trudpPacketQueueDataToQueueData(trudpPacketQueueData *tqd) {
    return tqd ? (trudpQueueData *)((void*)tqd - sizeof(trudpQueueData)) : NULL;
}

/**
 * Add packet to Packet queue
 * 
 * @param tq Pointer to trudpPacketQueue
 * @param packet Packet to add to queue
 * @param packet_length Packet length
 * @param expected_time Packet expected time
 * 
 * @return Pointer to added trudpPacketQueueData
 */
trudpPacketQueueData *trudpPacketQueueAdd(trudpPacketQueue *tq, void *packet, 
        size_t packet_length, uint32_t expected_time) {
    
    size_t tqd_length = sizeof(trudpPacketQueueData) + packet_length;    
    trudpPacketQueueData *tqd = (trudpPacketQueueData *)((trudpQueueData *)trudpQueueAdd(tq->q, NULL, tqd_length))->data;
    tqd->expected_time = expected_time;
    tqd->retrieves = 0;
    memcpy(tqd->packet, packet, packet_length);
    tqd->packet_length = packet_length;
    
    return tqd;
}

/**
 * Remove element from Packet queue
 * 
 * @param tq Pointer to trudpPacketQueue
 * @param tqd Pointer to trudpPacketQueueData to delete it
 * 
 * @return Zero at success
 */
inline int trudpPacketQueueDelete(trudpPacketQueue *tq, trudpPacketQueueData *tqd) {
    
    return trudpQueueDelete(tq->q, trudpPacketQueueDataToQueueData(tqd));
}

/**
 * Find Packet queue data by Id
 * 
 * @param tq Pointer to trudpPacketQueue
 * @param id Id to find in send queue
 * 
 * @return Pointer to trudpPacketQueueData or NULL if not found
 */
trudpPacketQueueData *trudpPacketQueueFindById(trudpPacketQueue *tq, uint32_t id) {
    
    trudpPacketQueueData *rv = NULL;
            
    trudpQueueIterator *it = trudpQueueIteratorNew(tq->q);
    if(it != NULL) {
        
        while(trudpQueueIteratorNext(it)) {
            
            trudpPacketQueueData *tqd = (trudpPacketQueueData *)((trudpQueueData *)trudpQueueIteratorElement(it))->data;
            if(trudpPacketGetId(tqd->packet) == id) {
                rv = tqd;
                break;
            }
        }
        trudpQueueIteratorFree(it);
    }
    
    return rv;
}

/**
 * Find Packet queue data by time
 * 
 * @param tq Pointer to trudpPacketQueue
 * @param t Time to find (current time usually). This function will find first 
 *          record with expected_time less or equal then this time.
 * 
 * @return  Pointer to trudpPacketQueueData or NULL if not found
 */
trudpPacketQueueData *trudpPacketQueueFindByTime(trudpPacketQueue *tq, uint32_t t) {
    
    trudpPacketQueueData *rv = NULL;
            
    trudpQueueIterator *it = trudpQueueIteratorNew(tq->q);
    if(it != NULL) {
        
        while(trudpQueueIteratorNext(it)) {
            
            trudpPacketQueueData *tqd = (trudpPacketQueueData *)((trudpQueueData *)trudpQueueIteratorElement(it))->data;
            if(tqd->expected_time <= t) {
                rv = tqd;
                break;
            }
        }
        trudpQueueIteratorFree(it);
    }
    
    return rv;
}

/**
 * Move element to the end of list
 * 
 * @param q Pointer to trudpPacketQueue
 * @param qd Pointer to trudpPacketQueueData
 * @return Zero at success
 */
inline trudpPacketQueueData *trudpPacketQueueMoveToEnd(trudpPacketQueue *tq, trudpPacketQueueData *tqd) {

    return (trudpPacketQueueData *)trudpQueueMoveToEnd(tq->q, trudpPacketQueueDataToQueueData(tqd))->data;
}

/**
 * Get number of elements in Packet queue
 * 
 * @param q
 * 
 * @return Number of elements in TR-UPD queue
 */
inline size_t trudpPacketQueueSize(trudpPacketQueue *tq) {
    return trudpQueueSize(tq->q);
}
