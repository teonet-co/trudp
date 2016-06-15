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
 * \file   write_queue.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on June 15, 2016, 12:57 AM
 */

#ifndef WRITE_QUEUE_H
#define WRITE_QUEUE_H

#include <stdint.h>
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct trudpWriteQueue {

    trudpQueue *q;

} trudpWriteQueue;

typedef struct trudpWriteQueueData {

    uint16_t packet_length; 
    void *packet;

} trudpWriteQueueData;

inline trudpWriteQueue *trudpWriteQueueNew();
inline void trudpWriteQueueDestroy(trudpWriteQueue *wq);
inline int trudpWriteQueueFree(trudpWriteQueue *wq);

trudpWriteQueueData *trudpWriteQueueAdd(trudpWriteQueue *wq, void *packet, 
        size_t packet_length);
inline trudpWriteQueueData *trudpWriteQueueGetFirst(trudpWriteQueue *wq);
inline int trudpWriteQueueDeleteFirst(trudpWriteQueue *wq);


#ifdef __cplusplus
}
#endif

#endif /* WRITE_QUEUE_H */

