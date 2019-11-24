/*
 * The MIT License
 *
 * Copyright 2016-2018 Kirill Scherba <kirill@scherba.ru>.
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

// Local functions

#ifdef RESERVED
static trudpPacketQueueData *_trudpPacketQueueAddTime(trudpPacketQueue *tq,
        void *packet, size_t packet_length, uint64_t expected_time);
trudpPacketQueueData *trudpPacketQueueFindByTime(trudpPacketQueue *tq, uint64_t t);
#endif
/**
 * Create new Packet queue
 *
 * @return Pointer to trudpPacketQueue
 */
trudpPacketQueue *trudpPacketQueueNew() {
    trudpPacketQueue *tq = (trudpPacketQueue *)malloc(sizeof(trudpPacketQueue));
    tq->q = teoQueueNew();
    return tq;
}
/**
 * Destroy Packet queue
 *
 * @param tq Pointer to trudpPacketQueue
 */
void trudpPacketQueueDestroy(trudpPacketQueue *tq) {
    if(tq) {
        teoQueueDestroy(tq->q);
        free(tq);
    }
}
/**
 * Remove all elements from Packet queue
 *
 * @param tq Pointer to trudpPacketQueue
 * @return Zero at success
 */
int trudpPacketQueueFree(trudpPacketQueue *tq) {
    return tq && tq->q ? teoQueueFree(tq->q) : -1;
}

/**
 * Get number of elements in Packet queue
 *
 * @param tq
 *
 * @return Number of elements in TR-UPD queue
 */
size_t trudpPacketQueueSize(trudpPacketQueue *tq) {
    return teoQueueSize(tq->q);
}

trudpPacketQueueData *trudpPacketQueueAdd(trudpPacketQueue *tq,
        void *packet, size_t packet_length, uint64_t expected_time);
/**
 * Get pointer to trudpQueueData from trudpPacketQueueData pointer
 * @param tqd Pointer to trudpPacketQueueData
 * @return Pointer to trudpQueueData or NULL if tqd is NULL
 */
teoQueueData *trudpPacketQueueDataToQueueData(
    trudpPacketQueueData *tqd) {
    return tqd ? (teoQueueData *)((char*)tqd - sizeof(teoQueueData)) : NULL;
}
/**
 * Remove element from Packet queue
 *
 * @param tq Pointer to trudpPacketQueue
 * @param tqd Pointer to trudpPacketQueueData to delete it
 *
 * @return Zero at success
 */
int trudpPacketQueueDelete(trudpPacketQueue *tq,
    trudpPacketQueueData *tqd) {
    return teoQueueDelete(tq->q, trudpPacketQueueDataToQueueData(tqd));
}
/**
 * Move element to the end of list
 *
 * @param tq Pointer to trudpPacketQueue
 * @param tqd Pointer to trudpPacketQueueData
 * @return Zero at success
 */
trudpPacketQueueData *trudpPacketQueueMoveToEnd(trudpPacketQueue *tq,
        trudpPacketQueueData *tqd) {

    return (trudpPacketQueueData *)teoQueueMoveToEnd(tq->q,
                trudpPacketQueueDataToQueueData(tqd))->data;
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
        size_t packet_length, uint64_t expected_time) {

    // Add
    size_t tqd_length = sizeof(trudpPacketQueueData) + packet_length;
    trudpPacketQueueData *tqd = (trudpPacketQueueData *)
            ((teoQueueData *)teoQueueAdd(tq->q, NULL, tqd_length))->data;

    // Fill data
    memcpy(tqd->packet, packet, packet_length);
    tqd->expected_time = expected_time;
    tqd->packet_length = packet_length;
    tqd->retrieves = 0;

    return tqd;
}

/**
 * Find Packet queue data by Id
 *
 * @param tq Pointer to trudpPacketQueue
 * @param id Id to find in send queue
 *
 * @return Pointer to trudpPacketQueueData or NULL if not found
 */
trudpPacketQueueData *trudpPacketQueueFindById(trudpPacketQueue *tq,
        uint32_t id) {

    trudpPacketQueueData *rv = NULL;

    teoQueueIterator *it = teoQueueIteratorNew(tq->q);
    if(it != NULL) {

        while(teoQueueIteratorNext(it)) {

            trudpPacketQueueData *tqd = (trudpPacketQueueData *)
                    ((teoQueueData *)teoQueueIteratorElement(it))->data;

            trudpPacket* trudp_paket = trudpPacketQueueDataGetPacket(tqd);
            if(trudpPacketGetId(trudp_paket) == id) {
                rv = tqd;
                break;
            }
        }
        teoQueueIteratorFree(it);
    }

    return rv;
}

/**
 * Get first element from Packet Queue
 *
 * @param tq Pointer to trudpPacketQueue

 * @return Pointer to trudpPacketQueueData or NULL if not found
 */
trudpPacketQueueData *trudpPacketQueueGetFirst(trudpPacketQueue *tq) {

    trudpPacketQueueData *tqd = NULL;

    teoQueueIterator *it = teoQueueIteratorNew(tq->q);
    if(it != NULL) {
        if(teoQueueIteratorNext(it)) {
            tqd = (trudpPacketQueueData *)
                    ((teoQueueData *)teoQueueIteratorElement(it))->data;
        }
        teoQueueIteratorFree(it);
    }

    return tqd;
}

#ifdef RESERVED
/**
 * Find Packet queue data by time
 *
 * @param tq Pointer to trudpPacketQueue
 * @param t Time to find (current time usually). This function will find first
 *          record with expected_time less or equal to this time.
 *
 * @return  Pointer to trudpPacketQueueData or NULL if not found
 */
trudpPacketQueueData *trudpPacketQueueFindByTime(trudpPacketQueue *tq,
        uint64_t t) {

    trudpPacketQueueData *rv = NULL;

    teoQueueIterator *it = teoQueueIteratorNew(tq->q);
    if(it != NULL) {

        while(teoQueueIteratorNext(it)) {

            trudpPacketQueueData *tqd = (trudpPacketQueueData *)
                    ((teoQueueData *)teoQueueIteratorElement(it))->data;

            if(tqd->expected_time <= t) {
                rv = tqd;
                break;
            }
        }
        teoQueueIteratorFree(it);
    }

    return rv;
}
#endif

#ifdef RESERVED
/**
 * Add packet to Packet queue and sort by expected
 *
 * @param tq
 * @param packet
 * @param packet_length
 * @param expected_time
 * @return
 */
static trudpPacketQueueData *_trudpPacketQueueAddTime(trudpPacketQueue *tq,
        void *packet, size_t packet_length, uint64_t expected_time) {

    if(trudpPacketQueueSize(tq)) {
        // Find expected less or equal then than selected
        trudpPacketQueueData *tpqd = trudpPacketQueueFindByTime(tq, expected_time);
        if(tpqd) {

            // Add after
            size_t tqd_length = sizeof(trudpPacketQueueData) + packet_length;
            trudpPacketQueueData *tqd = (trudpPacketQueueData *)
                ((teoQueueData *)teoQueueAddAfter(tq->q, NULL, tqd_length,
                    _trudpPacketQueueDataToQueueData(tpqd)))->data;

            // Fill data
            memcpy(tqd->packet, packet, packet_length);
            tqd->expected_time = expected_time;
            tqd->packet_length = packet_length;
            tqd->retrieves = 0;

            return tqd;
        }
    }

    return trudpPacketQueueAdd(tq, packet, packet_length, expected_time);
}
#endif
