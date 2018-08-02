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
 * Created on August 2, 2018, 20:18
 */

#include <stdlib.h>
#include <string.h>
#include "read_queue.h"

/**
 * Add packet to Write queue
 * 
 * @param rq Pointer to trudpReadQueue
 * @param packet Pointer to Packet to add to queue
 * @param packet_ptr Pointer to Packet to add to queue
 * @param packet_length Packet length
 * 
 * @return Pointer to added trudpReadQueueData
 */
trudpReadQueueData *trudpReadQueueAdd(trudpReadQueue *rq, void *packet, 
        void *packet_ptr, size_t packet_length) {
    
    size_t rqd_length = sizeof(trudpReadQueueData); // + packet_length;    
    trudpReadQueueData *rqd = (trudpReadQueueData *)(
            (teoQueueData *)teoQueueAdd(rq->q, NULL, rqd_length))->data;
    
    if(packet != NULL) {
        memcpy(rqd->packet, packet, packet_length < MAX_HEADER_SIZE ? packet_length : MAX_HEADER_SIZE);
        rqd->packet_ptr = NULL;
    }
    else {
        memset(rqd->packet, 0, MAX_HEADER_SIZE);
        rqd->packet_ptr = packet_ptr;        
    }
    rqd->packet_length = packet_length;
    
    return rqd;
}

#ifdef RESERVED
/**
 * Get pointer to trudpQueueData from trudpReadQueueData pointer
 * 
 * @param rqd Pointer to trudpReadQueueData
 * 
 * @return Pointer to trudpQueueData or NULL if rqd is NULL
 */
static inline teoQueueData *trudpReadQueueDataToQueueData(trudpReadQueueData *rqd) {
    return rqd ? (teoQueueData *)((void*)rqd - sizeof(teoQueueData)) : NULL;
}

/**
 * Remove element from Write queue
 * 
 * @param rq Pointer to trudpReadQueue
 * @param rqd Pointer to trudpReadQueueData to delete it
 * 
 * @return Zero at success
 */
static inline int trudpReadQueueDelete(trudpReadQueue *rq, trudpReadQueueData *rqd) {    
    return teoQueueDelete(rq->q, trudpReadQueueDataToQueueData(rqd));
}

#endif