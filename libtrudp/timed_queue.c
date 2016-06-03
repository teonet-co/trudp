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
 * Timed queue: extended queue module used as TR-UDP send and receive queue
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "timed_queue.h"
#include "packet.h"

/**
 * Create new Timed queue
 * 
 * @return Pointer to trudpTimedQueue
 */
inline trudpTimedQueue *trudpTimedQueueNew() {
    
    trudpTimedQueue *tq = (trudpTimedQueue *)malloc(sizeof(trudpTimedQueue));
    tq->q = trudpQueueNew();
    
    return tq;
}

/**
 * Destroy Timed queue
 * 
 * @param tq Pointer to trudpTimedQueue
 */
inline void trudpTimedQueueDestroy(trudpTimedQueue *tq) {

    if(tq) {
        trudpQueueDestroy(tq->q);
        free(tq);
    }
}

/**
 * Remove all elements from Timed queue
 * 
 * @param tq Pointer to trudpTimedQueue
 * @return Zero at success
 */
inline int trudpTimedQueueFree(trudpTimedQueue *tq) {

    return tq && tq->q ? trudpQueueFree(tq->q) : -1;
}

/**
 * Get pointer to trudpQueueData from trudpTimedQueusData pointer
 * @param tqd
 * @return 
 */
inline trudpQueueData *trudpTimedQueueDataToQueueData(trudpTimedQueueData *tqd) {
    return tqd ? (trudpQueueData *)((void*)tqd - sizeof(trudpQueueData)) : NULL;
}

/**
 * Add packet to Timed queue
 * 
 * @param tq Pointer to trudpTimedQueue
 * @param packet Packet to add to queue
 * @param packet_length Packet length
 * @param expected_time Packet expected time
 * 
 * @return Pointer to added trudpTimedQueueData
 */
trudpTimedQueueData *trudpTimedQueueAdd(trudpTimedQueue *tq, void *packet, 
        size_t packet_length, uint32_t expected_time) {
    
    size_t tqd_length = sizeof(trudpTimedQueueData) + packet_length;    
    trudpTimedQueueData *tqd = (trudpTimedQueueData *)((trudpQueueData *)trudpQueueAdd(tq->q, NULL, tqd_length))->data;
    tqd->expected_time = expected_time;
    tqd->retrieves = 0;
    memcpy(tqd->packet, packet, packet_length);
    tqd->packet_length = packet_length;
    
    return tqd;
}

/**
 * Remove element from Timed queue
 * 
 * @param tq Pointer to trudpTimedQueue
 * @param tqd Pointer to trudpTimedQueueData to delete it
 * 
 * @return Zero at success
 */
inline int trudpTimedQueueDelete(trudpTimedQueue *tq, trudpTimedQueueData *tqd) {
    
    return trudpQueueDelete(tq->q, trudpTimedQueueDataToQueueData(tqd));
}

/**
 * Find Timed queue data by Id
 * 
 * @param tq Pointer to trudpTimedQueue
 * @param id Id to find in send queue
 * 
 * @return Pointer to trudpTimedQueueData or NULL if not found
 */
trudpTimedQueueData *trudpTimedQueueFindById(trudpTimedQueue *tq, uint32_t id) {
    
    trudpTimedQueueData *rv = NULL;
            
    trudpQueueIterator *it = trudpQueueIteratorNew(tq->q);
    if(it != NULL) {
        
        while(trudpQueueIteratorNext(it)) {
            
            trudpTimedQueueData *tqd = (trudpTimedQueueData *)((trudpQueueData *)trudpQueueIteratorElement(it))->data;
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
 * Find Timed queue data by time
 * 
 * @param tq Pointer to trudpTimedQueue
 * @param t Time to find (current time usually). This function will find first 
 *          record with expected_time less or equal then this time.
 * 
 * @return  Pointer to trudpTimedQueueData or NULL if not found
 */
trudpTimedQueueData *trudpTimedQueueFindByTime(trudpTimedQueue *tq, uint32_t t) {
    
    trudpTimedQueueData *rv = NULL;
            
    trudpQueueIterator *it = trudpQueueIteratorNew(tq->q);
    if(it != NULL) {
        
        while(trudpQueueIteratorNext(it)) {
            
            trudpTimedQueueData *tqd = (trudpTimedQueueData *)((trudpQueueData *)trudpQueueIteratorElement(it))->data;
            if(tqd->expected_time <= t) {
                rv = tqd;
                break;
            }
        }
        trudpQueueIteratorFree(it);
    }
    
    return rv;
}
