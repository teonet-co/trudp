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

#include <stdio.h>

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
 * Create new Packet map
 *
 * @return Pointer to trudpPacketMap
 */
trudpPacketMap *trudpPacketMapNew() {
    trudpPacketMap *tq = (trudpPacketMap *)malloc(sizeof(trudpPacketMap));
    tq->q = teoMapNew(100, 1);
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
 * Destroy Packet map
 *
 * @param tq Pointer to trudpPacketMap
 */
void trudpPacketMapDestroy(trudpPacketMap *tq) {
    if(tq) {
        teoMapDestroy(tq->q);
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

/**
 * Get number of elements in Packet map
 *
 * @param tq
 *
 * @return Number of elements in TR-UPD map
 */
size_t trudpPacketMapSize(trudpPacketMap *tq) {
    return teoMapSize(tq->q);
}

trudpPacketQueueData *trudpPacketQueueAdd(trudpPacketQueue *tq,
        void *packet, size_t packet_length, uint64_t expected_time);

/**
 * Get pointer to trudpQueueData from trudpPacketQueueData pointer
 * @param tqd Pointer to trudpPacketQueueData
 * @return Pointer to trudpQueueData or NULL if tqd is NULL
 */
teoQueueData *trudpPacketQueueDataToQueueData(trudpPacketQueueData *tqd) {
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
int trudpPacketQueueDelete(trudpPacketQueue *tq, trudpPacketQueueData *tqd) {
    return teoQueueDelete(tq->q, trudpPacketQueueDataToQueueData(tqd));
}

/**
 * Remove element from Packet map
 *
 * @param tq Pointer to trudpPacketMap
 * @param id Packet id to delete it
 *
 * @return Zero at success
 */
int trudpPacketMapDelete(trudpPacketMap *tq, uint32_t id) {
    return teoMapDelete(tq->q, &id, sizeof(id));
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
 * Add packet to Packet map
 *
 * @param tq Pointer to trudpPacketMap
 * @param packet Packet to add to map
 * @param packet_length Packet length
 * @param expected_time Packet expected time
 *
 * @return Pointer to added packet id or NULL at error
 */
uint32_t *trudpPacketMapAdd(trudpPacketMap *tq, void *packet, 
        size_t packet_length, uint64_t expected_time) {

    // Fill data
    size_t tqd_len = sizeof(trudpPacketQueueData) + packet_length;
    trudpPacketQueueData *tqd = malloc(tqd_len);
    memcpy(tqd->packet, packet, packet_length);
    tqd->expected_time = expected_time;
    tqd->packet_length = packet_length;
    tqd->retrieves = 0;

    // Add
    uint32_t id = trudpPacketGetId((trudpPacket*) packet);
    uint32_t *pid = teoMapAdd(tq->q, &id, sizeof(id), tqd, tqd_len);
    free(tqd);

    return pid; 
}

/**
 * Find Packet queue data by Id
 *
 * @param tq Pointer to trudpPacketQueue
 * @param id Id to find in send queue
 *
 * @return Pointer to trudpPacketQueueData or NULL if not found
 */
#include <inttypes.h>
trudpPacketQueueData *trudpPacketQueueFindById(trudpPacketQueue *tq,
        uint32_t id) {

  int64_t saved_time = teotimeGetCurrentTimeMs();
    trudpPacketQueueData *rv = NULL;

    teoQueueIterator it;
    teoQueueIteratorReset(&it, tq->q);

    while(teoQueueIteratorNext(&it)) {
        trudpPacketQueueData *tqd = (trudpPacketQueueData *)
                ((teoQueueData *)teoQueueIteratorElement(&it))->data;

        trudpPacket* trudp_paket = trudpPacketQueueDataGetPacket(tqd);
        if(trudpPacketGetId(trudp_paket) == id) {
            rv = tqd;
            break;
        }
    }
  int64_t time = teotimeGetTimePassedMs(saved_time);
  if (time >= 2) {
    printf("trudpPacketQueueFindById %" PRId64 " %d\n", teotimeGetTimePassedMs(saved_time), __LINE__);
  }

    return rv;
}

/**
 * Find Packet map data by Id
 *
 * @param tq Pointer to trudpPacketMap
 * @param id Id to find in map
 *
 * @return Pointer to trudpPacketQueueData or NULL if not found
 */
trudpPacketQueueData *trudpPacketMapFindById(trudpPacketMap *tq, uint32_t id) {
    trudpPacketQueueData *rv = teoMapGet(tq->q, &id, sizeof(&id), NULL);
    return rv != (void*)-1 ? rv : NULL;
}

/**
 * Get first element from Packet Queue
 *
 * @param tq Pointer to trudpPacketQueue

 * @return Pointer to trudpPacketQueueData or NULL if not found
 */
trudpPacketQueueData *trudpPacketQueueGetFirst(trudpPacketQueue *tq) {

    trudpPacketQueueData *tqd = NULL;

    teoQueueIterator it;
    teoQueueIteratorReset(&it, tq->q);

    if(teoQueueIteratorNext(&it)) {
        tqd = (trudpPacketQueueData *)
                ((teoQueueData *)teoQueueIteratorElement(&it))->data;
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

    int64_t saved_time = teotimeGetCurrentTimeMs();
    trudpPacketQueueData *rv = NULL;

    teoQueueIterator it;
    teoQueueIteratorReset(&it, tq->q);

    while(teoQueueIteratorNext(&it)) {
        trudpPacketQueueData *tqd = (trudpPacketQueueData *)
                ((teoQueueData *)teoQueueIteratorElement(&it))->data;

        if(tqd->expected_time <= t) {
            rv = tqd;
            break;
        }
    }
  int64_t time = teotimeGetTimePassedMs(saved_time);
  if (time > 3) {
    printf("trudpPacketQueueFindByTime %" PRId64 " %d\n", teotimeGetTimePassedMs(saved_time), __LINE__);
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
