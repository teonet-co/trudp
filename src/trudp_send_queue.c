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
 * \file   trudp_send_queue.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on July 21, 2016, 03:38
 */

#include "trudp_send_queue.h"

void trudp_ChannelIncrementStatSendQueueSize(trudpPacketQueue *sq);

/**
 * Add packet to Packet queue
 *
 * @param sq Pointer to trudpSendQueue
 * @param packet Packet to add to queue
 * @param packet_length Packet length
 * @param expected_time Packet expected time
 *
 * @return Pointer to added trudpPacketQueueData
 */
trudpPacketQueueData *trudpSendQueueAdd(trudpSendQueue *sq, void *packet,
        size_t packet_length, uint64_t expected_time) {
    
    trudpPacketQueueData *tqd = trudpPacketQueueAdd(sq,
        packet,
        packet_length,
        expected_time
    );
    trudp_ChannelIncrementStatSendQueueSize(sq);
    
    return tqd;
}

/**
 * Get send queue timeout
 *
 * @param sc Pointer to trudpSendQueue
 * @param ts Current time
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
