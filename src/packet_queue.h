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
 * \file   packet_queue.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 30, 2016, 8:56 PM
 */

#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include <stdlib.h>
#include <stdint.h>

#include "teoccl/queue.h"
#include "teoccl/map.h"

#include "packet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct trudpPacketQueue {

    teoQueue *q;

} trudpPacketQueue;

typedef struct trudpPacketMap {

    teoMap *q;

} trudpPacketMap;

typedef struct trudpPacketQueueData {

    uint64_t expected_time;
    uint32_t packet_length;
    uint32_t retrieves;
    uint32_t retrieves_start;
    char packet[];

} trudpPacketQueueData;

/**
 * Create new Packet queue
 *
 * @return Pointer to trudpPacketQueue
 */
trudpPacketQueue *trudpPacketQueueNew();
trudpPacketMap *trudpPacketMapNew();

/**
 * Destroy Packet queue
 *
 * @param tq Pointer to trudpPacketQueue
 */
void trudpPacketQueueDestroy(trudpPacketQueue *tq);
void trudpPacketMapDestroy(trudpPacketMap *tq);

/**
 * Remove all elements from Packet queue
 *
 * @param tq Pointer to trudpPacketQueue
 * @return Zero at success
 */
int trudpPacketQueueFree(trudpPacketQueue *tq);
int trudpPacketMapFree(trudpPacketMap *tq);


/**
 * Get number of elements in Packet queue
 *
 * @param tq
 *
 * @return Number of elements in TR-UPD queue
 */
size_t trudpPacketQueueSize(trudpPacketQueue *tq);
size_t trudpPacketMapSize(trudpPacketMap *tq);

trudpPacketQueueData *trudpPacketQueueAdd(trudpPacketQueue *tq,
        void *packet, size_t packet_length, uint64_t expected_time);
uint32_t *trudpPacketMapAdd(trudpPacketMap *tq,
        void *packet, size_t packet_length, uint64_t expected_time);
/**
 * Get pointer to trudpQueueData from trudpPacketQueueData pointer
 * @param tqd Pointer to trudpPacketQueueData
 * @return Pointer to trudpQueueData or NULL if tqd is NULL
 */
teoQueueData *trudpPacketQueueDataToQueueData(
    trudpPacketQueueData *tqd);

/**
 * Remove element from Packet queue
 *
 * @param tq Pointer to trudpPacketQueue
 * @param tqd Pointer to trudpPacketQueueData to delete it
 *
 * @return Zero at success
 */
int trudpPacketQueueDelete(trudpPacketQueue *tq, trudpPacketQueueData *tqd);
int trudpPacketMapDelete(trudpPacketMap *tq, uint32_t id);

/**
 * Move element to the end of list
 *
 * @param tq Pointer to trudpPacketQueue
 * @param tqd Pointer to trudpPacketQueueData
 * @return Zero at success
 */
trudpPacketQueueData *trudpPacketQueueMoveToEnd(trudpPacketQueue *tq,
        trudpPacketQueueData *tqd);

trudpPacketQueueData *trudpPacketQueueFindById(trudpPacketQueue *tq, uint32_t id);
trudpPacketQueueData *trudpPacketMapFindById(trudpPacketMap *tq, uint32_t id);
trudpPacketQueueData *trudpPacketQueueGetFirst(trudpPacketQueue *tq);

static inline trudpPacket* trudpPacketQueueDataGetPacket(trudpPacketQueueData* tqd) {
    return (trudpPacket*)(tqd->packet);
}

#ifdef __cplusplus
}
#endif

#endif /* PACKET_QUEUE_H */
