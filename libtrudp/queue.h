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
 * \file   queue.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 30, 2016, 11:56 AM
 */

#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * Queue module, linked list with data in body
 * 
 */

/**
 * TR-UDP queue data structure
 */
typedef struct trudpQueueData {
    
    struct trudpQueueData *prev;
    struct trudpQueueData *next;
    size_t data_length;
    char data[];
    
} trudpQueueData;

/**
 * TR-UDP Queue structure
 */
typedef struct trudpQueue {
    
    size_t length;
    trudpQueueData *first;
    trudpQueueData *last;
    
} trudpQueue;

/**
 * TR-UDP iterator
 */
typedef struct trudpQueueIterator {
    
    trudpQueue *q;
    trudpQueueData *qd;    
    
} trudpQueueIterator;

trudpQueue *trudpQueueNew();
int trudpQueueDestroy(trudpQueue *q);
int trudpQueueFree(trudpQueue *q);

trudpQueueData *trudpQueueAdd(trudpQueue *q, void *data, size_t data_length);
inline trudpQueueData *trudpQueueAddTop(trudpQueue *q, void *data, 
        size_t data_length);
trudpQueueData *trudpQueueAddAfter(trudpQueue *q, void *data, size_t data_length, 
        trudpQueueData *qd);
trudpQueueData *trudpQueueUpdate(trudpQueue *q, void *data, size_t data_length, 
        trudpQueueData *qd);
trudpQueueData *trudpQueueRemove(trudpQueue *q, trudpQueueData *qd);
int trudpQueueDelete(trudpQueue *q, trudpQueueData *qd);
int trudpQueueDeleteFirst(trudpQueue *q);
int trudpQueueDeleteLast(trudpQueue *q);
trudpQueueData *trudpQueueMoveToTop(trudpQueue *q, trudpQueueData *qd);
trudpQueueData *trudpQueueMoveToEnd(trudpQueue *q, trudpQueueData *qd);
trudpQueueData *trudpQueuePut(trudpQueue *q, trudpQueueData *qd);
size_t trudpQueueSize(trudpQueue *q);

trudpQueueIterator *trudpQueueIteratorNew(trudpQueue *q);
trudpQueueData *trudpQueueIteratorNext(trudpQueueIterator *it);
trudpQueueData *trudpQueueIteratorElement(trudpQueueIterator *it);
trudpQueueIterator *trudpQueueIteratorReset(trudpQueueIterator *it, trudpQueue *q);
int trudpQueueIteratorFree(trudpQueueIterator *it);

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */

