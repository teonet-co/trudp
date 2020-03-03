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

#include "trudp_receive_queue.h"

/**
 * Create new Receive queue
 *
 * @return Pointer to trudpPacketQueue
 */

trudpReceiveQueue *trudpReceiveQueueNew() {
    return trudpPacketMapNew();
}

/**
 * Destroy Receive queue
 *
 * @param sq Pointer to trudpReceiveQueue
 */

void trudpReceiveQueueDestroy(trudpReceiveQueue *rq) {
    if(rq) {
        teoMapDestroy(rq->q);
        free(rq);
    }
}

/**
 * Remove all elements from Receive queue
 *
 * @param sq Pointer to Receive Queue (trudpReceiveQueue)
 * @return Zero at success
 */

int trudpReceiveQueueFree(trudpReceiveQueue *rq) {
    trudpPacketMapDestroy(rq);
    return 0;
}

/**
 * Get number of elements in Receive queue
 *
 * @param sq Pointer to trudpReceiveQueue
 *
 * @return Number of elements in TR-UPD send queue
 */

size_t trudpReceiveQueueSize(trudpReceiveQueue *sq) {
    return trudpPacketMapSize(sq);
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

trudpReceiveQueueData *trudpReceiveQueueAdd(trudpReceiveQueue *sq, void *packet,
        size_t packet_length, uint64_t expected_time) {
    return trudpPacketMapAdd(sq, packet, packet_length, expected_time);
}

/**
 * Remove element from Receive queue
 *
 * @param tq Pointer to trudpReceiveQueue
 * @param tqd Pointer to trudpReceiveQueueData to delete it
 *
 * @return Zero at success
 */

int trudpReceiveQueueDelete(trudpReceiveQueue *tq,
        trudpReceiveQueueData *tqd) {
    return trudpPacketMapDelete(tq, tqd);
}

/**
 * Find Receive queue data by Id
 *
 * @param sq Pointer to trudpReceiveQueue
 * @param id Id to find in send queue
 *
 * @return Pointer to trudpReceiveQueueData or NULL if not found
 */

trudpReceiveQueueData *trudpReceiveQueueFindById(trudpReceiveQueue *sq,
        uint32_t id) {
    return trudpPacketMapFindById(sq, id);
}


