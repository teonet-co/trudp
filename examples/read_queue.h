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
 * Created on August 2, 2018, 20:18
 */

#ifndef READ_QUEUE_H
#define READ_QUEUE_H

#include <stdint.h>
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif
    
#define MAX_HEADER_SIZE 64

typedef struct trudpReadQueue {

    teoQueue *q;

} trudpReadQueue;

typedef struct trudpReadQueueData {

    char packet[MAX_HEADER_SIZE];
    uint16_t packet_length;
    void *packet_ptr;

} trudpReadQueueData;

/**
 * Create new Write queue
 * 
 * @return Pointer to trudpReadQueue
 */
static inline 
trudpReadQueue *trudpReadQueueNew() {    
    trudpReadQueue *rq = (trudpReadQueue *)malloc(sizeof(trudpReadQueue));
    rq->q = teoQueueNew();
    return rq;
}
/**
 * Destroy Write queue
 * 
 * @param rq Pointer to trudpReadQueue
 */
static inline 
void trudpReadQueueDestroy(trudpReadQueue *rq) {
    if(rq) {
        teoQueueDestroy(rq->q);
        free(rq);
    }
}
/**
 * Remove all elements from Write queue
 * 
 * @param rq Pointer to trudpReadQueue
 * @return Zero at success
 */
static inline 
int trudpReadQueueFree(trudpReadQueue *rq) {
    return rq && rq->q ? teoQueueFree(rq->q) : -1;
}
/**
 * Get number of elements in Write queue
 * 
 * @param rq
 * 
 * @return Number of elements in Write queue
 */
static inline 
size_t trudpReadQueueSize(trudpReadQueue *rq) {    
    return rq ? teoQueueSize(rq->q) : -1;
}
trudpReadQueueData *trudpReadQueueAdd(trudpReadQueue *rq, void *packet, 
        void *packet_ptr, size_t packet_length);
/**
 * Get pointer to first element data
 * 
 * @param rq Pointer to trudpReadQueue
 * 
 * @return Pointer to trudpReadQueueData data or NULL
 */
static inline 
trudpReadQueueData *trudpReadQueueGetFirst(trudpReadQueue *rq) {    
    return (trudpReadQueueData *) (rq->q->first ? rq->q->first->data : NULL);
}
/**
 * Remove first element from Write queue
 * 
 * @param rq Pointer to trudpReadQueue
 * 
 * @return Zero at success
 */
static inline 
int trudpReadQueueDeleteFirst(trudpReadQueue *rq) {
    return teoQueueDeleteFirst(rq->q);
}

#ifdef __cplusplus
}
#endif

#endif /* READ_QUEUE_H */

