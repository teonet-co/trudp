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
typedef struct teoQueueData {
    
    struct teoQueueData *prev;
    struct teoQueueData *next;
    size_t data_length;
    char data[];
    
} teoQueueData;

/**
 * TR-UDP Queue structure
 */
typedef struct teoQueue {
    
    size_t length;
    teoQueueData *first;
    teoQueueData *last;
    
} teoQueue;

/**
 * TR-UDP iterator
 */
typedef struct teoQueueIterator {
    
    teoQueue *q;
    teoQueueData *qd;    
    
} teoQueueIterator;

teoQueue *teoQueueNew();
int teoQueueDestroy(teoQueue *q);
int teoQueueFree(teoQueue *q);

teoQueueData *teoQueueNewData(void *data, size_t data_length);

teoQueueData *teoQueueAdd(teoQueue *q, void *data, size_t data_length);
teoQueueData *teoQueueAddTop(teoQueue *q, void *data, size_t data_length);
teoQueueData *teoQueueAddAfter(teoQueue *q, void *data, size_t data_length, 
        teoQueueData *qd);
teoQueueData *teoQueueUpdate(teoQueue *q, void *data, size_t data_length, 
        teoQueueData *qd);
teoQueueData *teoQueueRemove(teoQueue *q, teoQueueData *qd);
int teoQueueDelete(teoQueue *q, teoQueueData *qd);
int teoQueueDeleteFirst(teoQueue *q);
int teoQueueDeleteLast(teoQueue *q);
teoQueueData *teoQueueMoveToTop(teoQueue *q, teoQueueData *qd);
teoQueueData *teoQueueMoveToEnd(teoQueue *q, teoQueueData *qd);
teoQueueData *teoQueuePut(teoQueue *q, teoQueueData *qd);
size_t teoQueueSize(teoQueue *q);

teoQueueIterator *teoQueueIteratorNew(teoQueue *q);
teoQueueData *teoQueueIteratorNext(teoQueueIterator *it);
teoQueueData *teoQueueIteratorPrev(teoQueueIterator *it);
teoQueueData *teoQueueIteratorElement(teoQueueIterator *it);
teoQueueIterator *teoQueueIteratorReset(teoQueueIterator *it, teoQueue *q);
int teoQueueIteratorFree(teoQueueIterator *it);

typedef int (*teoQueueForeachFunction)(teoQueue *q, int idx, teoQueueData *data, void* user_data);
int teoQueueForeach(teoQueue *q, teoQueueForeachFunction callback, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */

