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
 * \file   write_queue.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on June 15, 2016, 12:56 AM
 */

#include <stdlib.h>
#include <string.h>
#include "write_queue.h"
/**
 * Create new Write queue
 *
 * @return Pointer to trudpWriteQueue
 */

trudpWriteQueue *trudpWriteQueueNew() {
    trudpWriteQueue *wq = (trudpWriteQueue *)malloc(sizeof(trudpWriteQueue));
    wq->q = teoQueueNew();
    return wq;
}
/**
 * Destroy Write queue
 *
 * @param wq Pointer to trudpWriteQueue
 */

void trudpWriteQueueDestroy(trudpWriteQueue *wq) {
    if(wq) {
        teoQueueDestroy(wq->q);
        free(wq);
    }
}
/**
 * Remove all elements from Write queue
 *
 * @param wq Pointer to trudpWriteQueue
 * @return Zero at success
 */

int trudpWriteQueueFree(trudpWriteQueue *wq) {
    return wq && wq->q ? teoQueueFree(wq->q) : -1;
}
/**
 * Get number of elements in Write queue
 *
 * @param wq
 *
 * @return Number of elements in Write queue
 */

size_t trudpWriteQueueSize(trudpWriteQueue *wq) {
    return wq ? teoQueueSize(wq->q) : -1;
}
trudpWriteQueueData *trudpWriteQueueAdd(trudpWriteQueue *wq, void *packet,
        void *packet_ptr, size_t packet_length);
/**
 * Get pointer to first element data
 *
 * @param wq Pointer to trudpWriteQueue
 *
 * @return Pointer to trudpWriteQueueData data or NULL
 */

trudpWriteQueueData *trudpWriteQueueGetFirst(trudpWriteQueue *wq) {
    return (trudpWriteQueueData *) (wq->q->first ? wq->q->first->data : NULL);
}
/**
 * Remove first element from Write queue
 *
 * @param wq Pointer to trudpWriteQueue
 *
 * @return Zero at success
 */

int trudpWriteQueueDeleteFirst(trudpWriteQueue *wq) {
    return teoQueueDeleteFirst(wq->q);
}


/**
 * Add packet to Write queue
 *
 * @param wq Pointer to trudpWriteQueue
 * @param packet Pointer to Packet to add to queue
 * @param packet_ptr Pointer to Packet to add to queue
 * @param packet_length Packet length
 *
 * @return Pointer to added trudpWriteQueueData
 */
trudpWriteQueueData *trudpWriteQueueAdd(trudpWriteQueue *wq, void *packet,
        void *packet_ptr, size_t packet_length) {

    size_t wqd_length = sizeof(trudpWriteQueueData); // + packet_length;
    trudpWriteQueueData *wqd = (trudpWriteQueueData *)(
            (teoQueueData *)teoQueueAdd(wq->q, NULL, wqd_length))->data;

    if(packet != NULL) {
        memcpy(wqd->packet, packet, packet_length < MAX_HEADER_SIZE ? packet_length : MAX_HEADER_SIZE);
        wqd->packet_ptr = NULL;
    }
    else {
        memset(wqd->packet, 0, MAX_HEADER_SIZE);
        wqd->packet_ptr = packet_ptr;
    }
    wqd->packet_length = packet_length;

    return wqd;
}

#ifdef RESERVED
/**
 * Get pointer to trudpQueueData from trudpWriteQueueData pointer
 *
 * @param wqd Pointer to trudpWriteQueueData
 *
 * @return Pointer to trudpQueueData or NULL if wqd is NULL
 */
static  teoQueueData *trudpWriteQueueDataToQueueData(trudpWriteQueueData *wqd) {
    return wqd ? (teoQueueData *)((void*)wqd - sizeof(teoQueueData)) : NULL;
}

/**
 * Remove element from Write queue
 *
 * @param wq Pointer to trudpWriteQueue
 * @param wqd Pointer to trudpWriteQueueData to delete it
 *
 * @return Zero at success
 */
static  int trudpWriteQueueDelete(trudpWriteQueue *wq, trudpWriteQueueData *wqd) {
    return teoQueueDelete(wq->q, trudpWriteQueueDataToQueueData(wqd));
}

#endif
