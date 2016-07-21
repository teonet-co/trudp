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

#ifndef SEND_QUEUE_H
#define SEND_QUEUE_H

#include <stdint.h>
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct trudpPacketQueue {

    trudpQueue *q;

} trudpPacketQueue;

typedef struct trudpPacketQueueData {

    uint64_t expected_time;
    uint32_t packet_length;
    uint32_t retrieves;
    uint32_t retrieves_start;
    char packet[];

} trudpPacketQueueData;

trudpPacketQueue *trudpPacketQueueNew();
void trudpPacketQueueDestroy(trudpPacketQueue *tq);
int trudpPacketQueueFree(trudpPacketQueue *tq);

//size_t trudpPacketQueueSize(trudpPacketQueue *tq);
/**
 * Get number of elements in Packet queue
 *
 * @param tq
 *
 * @return Number of elements in TR-UPD queue
 */
static inline size_t trudpPacketQueueSize(trudpPacketQueue *tq) {
    return trudpQueueSize(tq->q);
}

trudpQueueData *trudpPacketQueueDataToQueueData(trudpPacketQueueData *tqd);

trudpPacketQueueData *trudpPacketQueueAdd(trudpPacketQueue *tq, 
        void *packet, size_t packet_length, uint64_t expected_time);
trudpPacketQueueData *trudpPacketQueueAddTime(trudpPacketQueue *tq, 
        void *packet, size_t packet_length, uint64_t expected_time);
trudpPacketQueueData *trudpPacketQueueMoveToEnd(trudpPacketQueue *tq,
        trudpPacketQueueData *tqd);

/**
 * Remove element from Packet queue
 *
 * @param tq Pointer to trudpPacketQueue
 * @param tqd Pointer to trudpPacketQueueData to delete it
 *
 * @return Zero at success
 */
static inline int trudpPacketQueueDelete(trudpPacketQueue *tq, 
        trudpPacketQueueData *tqd) {

    return trudpQueueDelete(tq->q, trudpPacketQueueDataToQueueData(tqd));
}

trudpPacketQueueData *trudpPacketQueueFindById(trudpPacketQueue *tq, uint32_t id);
trudpPacketQueueData *trudpPacketQueueFindByTime(trudpPacketQueue *tq, uint64_t t);
trudpPacketQueueData *trudpPacketQueueGetFirst(trudpPacketQueue *tq);

#ifdef __cplusplus
}
#endif

#endif /* SEND_QUEUE_H */
