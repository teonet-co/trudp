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

#include <stdlib.h>
#include <string.h>

#include "queue.h"

/**
 * Create new TR-UDP Queue
 * 
 * @return Pointer to new TR-UDP Queue or NULL if memory allocate error
 */
trudpQueue *trudpQueueNew() {
    
    trudpQueue *q = (trudpQueue *) malloc(sizeof(trudpQueue));
    memset(q, 0, sizeof(trudpQueue));
    
    return q;    
}

/**
 * Destroy TR-UDP Queue
 * 
 * @param q Pointer to existing TR-UDP Queue
 * 
 * @return Zero at success
 */
int trudpQueueDestroy(trudpQueue *q) {
    
    trudpQueueFree(q);
    free(q);
    
    return 0;    
}

/**
 * Remove all elements from TR-UDP queue
 * 
 * @param q Pointer to existing TR-UDP Queue
 * 
 * @return Zero at success
 */
int trudpQueueFree(trudpQueue *q) {
    
    // Remove all elements
    trudpQueueData *qd = q->first;
    while(qd) {
        free(qd);
        qd = qd->next;
    }
    
    //q->length = 0;
    //q->first = NULL;
    //q->last = NULL;
    memset(q, 0, sizeof(trudpQueue));
    
    return 0;
}

/**
 * Get number of elements in TR-UPD queue
 * @param q
 * 
 * @return Number of elements in TR-UPD queue
 */
inline size_t trudpQueueSize(trudpQueue *q) {
    
    return q->length;
}

/**
 * Add new element to the TR-UPD queue
 * 
 * @param q Pointer to existing TR-UDP Queue
 * @param data
 * @param data_length
 * 
 * @return Pointer to trudpQueueData of added element
 */
trudpQueueData *trudpQueueAdd(trudpQueue *q, void *data, size_t data_length) {
    
    trudpQueueData *qd = (trudpQueueData *) malloc(sizeof(trudpQueueData) + data_length);
    
    // Fill Queue data structure
    qd->prev = q->last;
    qd->next = NULL;
    qd->data_length = data_length;
    if(data && data_length > 0) memcpy(qd->data, data, data_length);

    // Change fields in queue structure
    if(q->last) q->last->next = qd; // Set next in existing last element to this element
    if(!q->first) q->first = qd; // Set this element as first if first does not exists
    q->last = qd; // Set this element as last
    q->length++; // Increment length
    
    return qd;
}

/**
 * Delete element from queue
 * 
 * @param q
 * @param qd
 * @return Zero at success
 */
int trudpQueueDelete(trudpQueue *q, trudpQueueData *qd) {
        
    if(!qd->prev) q->first = qd->next;
    else qd->prev->next = qd->next;
    
    q->length--;
    free(qd);
    
    return 0;
}

/**
 * Create new TR-UPD Queue iterator
 * 
 * @param q
 * @return 
 */
trudpQueueIterator *trudpQueueIteratorNew(trudpQueue *q) {
    
    trudpQueueIterator *it = (trudpQueueIterator *) malloc(sizeof(trudpQueueIterator));
    it->qd = NULL;
    it->q = q;
    
    return it; 
}

/**
 * Get next element from TR-UPD Queue iterator
 * 
 * @param it Pointer to trudpQueueIterator
 * 
 * @return Pointer to the trudpQueueData or NULL if not exists
 */
trudpQueueData *trudpQueueIteratorNext(trudpQueueIterator *it) {
    
    if(!it->qd) it->qd = it->q->first;
    else it->qd = it->qd->next;
    
    return it->qd;
}

/**
 * Get current TR-UPD Queue iterator element
 * @param it Pointer to trudpQueueIterator
 * 
 * @return Pointer to the trudpQueueData or NULL if not exists
 */
inline trudpQueueData *trudpQueueIteratorElement(trudpQueueIterator *it) {
    
    return it->qd;
}

/**
 * Free TR-UPD Queue iterator
 * 
 * @param it Pointer to trudpQueueIterator
 * @return Zero at success
 */
int trudpQueueIteratorFree(trudpQueueIterator *it) {
    
    free(it);
    return 0;
}
