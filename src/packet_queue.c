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
        size_t packet_length, uint64_t expected_time, int debug_log_id) {

    // Add
    size_t tqd_length = sizeof(trudpPacketQueueData) + packet_length;
    trudpPacketQueueData *tqd = (trudpPacketQueueData *)
            ((teoQueueData *)teoQueueAdd(tq->q, NULL, tqd_length))->data;

    // Fill data
    memcpy(tqd->packet, packet, packet_length);
    tqd->expected_time = expected_time;
    tqd->packet_length = packet_length;
    tqd->retrieves = 0;
    tqd->debug_log_id = debug_log_id;

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

            if(trudpPacketGetId(tqd->packet) == id) {
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
