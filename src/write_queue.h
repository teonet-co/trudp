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
 * \file   write_queue.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on June 15, 2016, 12:57 AM
 */

#ifndef WRITE_QUEUE_H
#define WRITE_QUEUE_H

#include "teobase/types.h"

#include "teoccl/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HEADER_SIZE 64

typedef struct trudpWriteQueue {

    teoQueue *q;

} trudpWriteQueue;

typedef struct trudpWriteQueueData {

    char packet[MAX_HEADER_SIZE];
    uint16_t packet_length;
    void *packet_ptr;

} trudpWriteQueueData;

/**
 * Create new Write queue
 *
 * @return Pointer to trudpWriteQueue
 */

trudpWriteQueue *trudpWriteQueueNew();

/**
 * Destroy Write queue
 *
 * @param wq Pointer to trudpWriteQueue
 */

void trudpWriteQueueDestroy(trudpWriteQueue *wq);

/**
 * Remove all elements from Write queue
 *
 * @param wq Pointer to trudpWriteQueue
 * @return Zero at success
 */

int trudpWriteQueueFree(trudpWriteQueue *wq);

/**
 * Get number of elements in Write queue
 *
 * @param wq
 *
 * @return Number of elements in Write queue
 */

size_t trudpWriteQueueSize(trudpWriteQueue *wq);

trudpWriteQueueData *trudpWriteQueueAdd(trudpWriteQueue *wq, void *packet,
        void *packet_ptr, size_t packet_length);
/**
 * Get pointer to first element data
 *
 * @param wq Pointer to trudpWriteQueue
 *
 * @return Pointer to trudpWriteQueueData data or NULL
 */

trudpWriteQueueData *trudpWriteQueueGetFirst(trudpWriteQueue *wq);

/**
 * Remove first element from Write queue
 *
 * @param wq Pointer to trudpWriteQueue
 *
 * @return Zero at success
 */

int trudpWriteQueueDeleteFirst(trudpWriteQueue *wq);
#ifdef __cplusplus
}
#endif

#endif /* WRITE_QUEUE_H */
