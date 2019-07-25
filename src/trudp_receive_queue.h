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
 *
 * \file   trudp_receive_queue.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on July 21, 2016, 03:44
 */


#ifndef TRUDP_RECEIVE_QUEUE_H
#define TRUDP_RECEIVE_QUEUE_H

#include <stdlib.h>
#include "packet_queue.h"

/**
 * Receive queue type
 */
typedef trudpPacketQueue trudpReceiveQueue;

/**
 * Receive queue data type
 */
typedef trudpPacketQueueData trudpReceiveQueueData;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create new Receive queue
 *
 * @return Pointer to trudpPacketQueue
 */
static
trudpReceiveQueue *trudpReceiveQueueNew() {
    return trudpPacketQueueNew();
}

/**
 * Destroy Receive queue
 *
 * @param sq Pointer to trudpReceiveQueue
 */
static
void trudpReceiveQueueDestroy(trudpReceiveQueue *rq) {
    if(rq) {
        teoQueueDestroy(rq->q);
        free(rq);
    }
}

/**
 * Remove all elements from Receive queue
 *
 * @param sq Pointer to Receive Queue (trudpReceiveQueue)
 * @return Zero at success
 */
static
int trudpReceiveQueueFree(trudpReceiveQueue *rq) {
    return trudpPacketQueueFree(rq);
}

/**
 * Get number of elements in Receive queue
 *
 * @param sq Pointer to trudpReceiveQueue
 *
 * @return Number of elements in TR-UPD send queue
 */
static
size_t trudpReceiveQueueSize(trudpReceiveQueue *sq) {
    return trudpPacketQueueSize(sq);
}

/**
 * Add packet to Receive queue
 *
 * @param sq Pointer to trudpReceiveQueue
 * @param packet Packet to add to queue
 * @param packet_length Packet length
 * @param expected_time Packet expected time
 *
 * @return Pointer to added trudpReceiveQueueData
 */
static
trudpReceiveQueueData *trudpReceiveQueueAdd(trudpReceiveQueue *sq, void *packet,
        size_t packet_length, uint64_t expected_time) {
    return trudpPacketQueueAdd(sq, packet, packet_length, expected_time, 0);
}

/**
 * Remove element from Receive queue
 *
 * @param tq Pointer to trudpReceiveQueue
 * @param tqd Pointer to trudpReceiveQueueData to delete it
 *
 * @return Zero at success
 */
static
int trudpReceiveQueueDelete(trudpReceiveQueue *tq,
        trudpReceiveQueueData *tqd) {
    return trudpPacketQueueDelete(tq, tqd);
}

/**
 * Find Receive queue data by Id
 *
 * @param sq Pointer to trudpReceiveQueue
 * @param id Id to find in send queue
 *
 * @return Pointer to trudpReceiveQueueData or NULL if not found
 */
static
trudpReceiveQueueData *trudpReceiveQueueFindById(trudpReceiveQueue *sq,
        uint32_t id) {
    return trudpPacketQueueFindById(sq, id);
}

#ifdef __cplusplus
}
#endif

#endif /* TRUDP_RECEIVE_QUEUE_H */

