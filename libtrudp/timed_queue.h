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
 */

/* 
 * File:   timed_queue.h
 * Author: Kirill Scherba <kirill@scherba.ru>
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

typedef struct trudpTimedQueue {
    
    trudpQueue *q;
    
} trudpTimedQueue;

typedef struct trudpTimedQueueData {
    
    uint32_t expected_time;
    char packet[];
    
} trudpTimedQueueData;

trudpTimedQueue *trudpTimedQueueNew();
void trudpTimedQueueDestroy(trudpTimedQueue *tq);
int trudpTimedQueueFree(trudpTimedQueue *tq);

trudpQueueData *trudpTimedQueueDataToQueueData(trudpTimedQueueData *tqd);

trudpTimedQueueData *trudpTimedQueueAdd(trudpTimedQueue *tq, void *packet, 
        size_t packet_length, uint32_t expected_time);
int trudpTimedQueueDelete(trudpTimedQueue *tq, trudpTimedQueueData *tqd);
trudpTimedQueueData *trudpTimedQueueFindById(trudpTimedQueue *tq, uint32_t id);
trudpTimedQueueData *trudpTimedQueueFindByTime(trudpTimedQueue *tq, uint32_t t);

#ifdef __cplusplus
}
#endif

#endif /* SEND_QUEUE_H */

