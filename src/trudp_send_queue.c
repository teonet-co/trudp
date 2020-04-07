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
 * \file   trudp_send_queue.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on July 21, 2016, 03:38
 */

#include "trudp_send_queue.h"

#include "teobase/types.h"

/**
 * Create new Send queue
 *
 * @return Pointer to trudpSendQueue
 */

trudpSendQueue *trudpSendQueueNew() {
    return trudpPacketQueueNew();
}

/**
 * Remove all elements from Send queue
 *
 * @param sq Pointer to Send Queue (trudpSendQueue)
 * @return Zero at success
 */

int trudpSendQueueFree(trudpSendQueue *sq) {
    return trudpPacketQueueFree(sq);
}

/**
 * Destroy Send queue
 *
 * @param sq Pointer to Send Queue (trudpSendQueue)
 */

void trudpSendQueueDestroy(trudpSendQueue *sq) {
    trudpPacketQueueDestroy(sq);
}

/**
 * Get number of elements in Send queue
 *
 * @param sq Pointer to trudpSendQueue
 *
 * @return Number of elements in TR-UPD send queue
 */

size_t trudpSendQueueSize(trudpSendQueue *sq) {
    return trudpPacketQueueSize(sq);
}

/**
 * Add packet to Send queue
 *
 * @param sq Pointer to trudpSendQueue
 * @param packet Packet to add to queue
 * @param packet_length Packet length
 * @param expected_time Packet expected time
 *
 * @return Pointer to added trudpSendQueueData
 */

trudpSendQueueData *trudpSendQueueAdd(trudpSendQueue *sq, void *packet,
        size_t packet_length, uint64_t expected_time) {

    return trudpPacketQueueAdd(sq, packet, packet_length, expected_time);
}

/**
 * Remove element from Send queue
 *
 * @param sq Pointer to trudpSendQueue
 * @param sqd Pointer to trudpSendQueueData to delete it
 *
 * @return Zero at success
 */

int trudpSendQueueDelete(trudpSendQueue *sq, trudpSendQueueData *sqd) {
    return trudpPacketQueueDelete(sq, sqd);
}

/**
 * Find Send queue data by Id
 *
 * @param sq Pointer to trudpSendQueue
 * @param id Id to find in send queue
 *
 * @return Pointer to trudpSendQueueData or NULL if not found
 */

trudpSendQueueData *trudpSendQueueFindById(trudpSendQueue *sq, uint32_t id) {
    return trudpPacketQueueFindById(sq, id);
}

/**
 * Get first element from Send Queue
 *
 * @param tq Pointer to trudpSendQueue

 * @return Pointer to trudpSendQueueData or NULL if not found
 */

trudpSendQueueData *trudpSendQueueGetFirst(trudpSendQueue *sq) {
    return trudpPacketQueueGetFirst(sq);
}


/**
 * Get send queue timeout
 *
 * @param sq Pointer to trudpSendQueue
 * @param current_t Current time
 *
 * @return Send queue timeout (may by 0) or UINT32_MAX if send queue is empty
 */
uint32_t trudpSendQueueGetTimeout(trudpSendQueue *sq, uint64_t current_t) {

    // Get sendQueue timeout
    uint32_t timeout_sq = UINT32_MAX;
    if(sq->q->first) {
        trudpPacketQueueData *pqd = (trudpPacketQueueData *) sq->q->first->data;
        timeout_sq = pqd->expected_time > current_t ? pqd->expected_time - current_t : 0;
    }

    return timeout_sq;
}

uint64_t trudpSendQueueGetExpectedTime(trudpSendQueue *sq) {
        // Get sendQueue timeout
    uint64_t channel_expected_time = UINT64_MAX;
    if(sq->q->first) {
        trudpPacketQueueData *pqd = (trudpPacketQueueData *) sq->q->first->data;
        channel_expected_time = pqd->expected_time;
    }

    return channel_expected_time;
}
