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
 * \file   queue.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 30, 2016, 11:56 AM
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
inline int trudpQueueDestroy(trudpQueue *q) {
    
    if(q) {
        trudpQueueFree(q);
        free(q);
    }
    
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
    
    if(!q) return -1;
    
    // Remove all elements
    trudpQueueData *qd_next, *qd = q->first;
    while(qd) {
        qd_next = qd->next;
        free(qd);
        qd = qd_next;
    }    
    memset(q, 0, sizeof(trudpQueue));
    
    return 0;
}

/**
 * Get number of elements in TR-UPD queue
 * 
 * @param q
 * 
 * @return Number of elements in TR-UPD queue
 */
inline size_t trudpQueueSize(trudpQueue *q) {
    
    return q ? q->length : -1;
}

/**
 * Put (add, copy) existing queue record to the end of selected queue
 * 
 * @param q Pointer to trudpQueue
 * @param qd Pointer to trudpQueueData
 * @return Zero at success
 */
trudpQueueData *trudpQueuePut(trudpQueue *q, trudpQueueData *qd) {
   
    if(q) {
        if(qd) {
            // Fill Queue data structure
            qd->prev = q->last;
            qd->next = NULL;

            // Change fields in queue structure
            if(q->last) q->last->next = qd; // Set next in existing last element to this element
            if(!q->first) q->first = qd; // Set this element as first if first does not exists
            q->last = qd; // Set this element as last
            q->length++; // Increment length
        }
    }
    
    return qd;
}

/**
 * Add new element to the end of TR-UPD queue
 * 
 * @param q Pointer to existing TR-UDP Queue
 * @param data Pointer to data of new element
 * @param data_length Length of new element data 
 * 
 * @return Pointer to trudpQueueData of added element
 */
trudpQueueData *trudpQueueAdd(trudpQueue *q, void *data, size_t data_length) {
    
    trudpQueueData *qd = NULL;
    
    if(q) {
        // Create new trudpQueueData
        qd = (trudpQueueData *) malloc(sizeof(trudpQueueData) + data_length);
        if(qd) {
            // Fill Queue data structure
            qd->data_length = data_length;
            if(data && data_length > 0) memcpy(qd->data, data, data_length);
            trudpQueuePut(q, qd);
        }
    }
    return qd;
}

/**
 * Add new element to the top of TR-UPD queue
 * 
 * @param q Pointer to existing TR-UDP Queue
 * @param data Pointer to data of new element
 * @param data_length Length of new element data 
 * 
 * @return Pointer to trudpQueueData of added element
 */
inline trudpQueueData *trudpQueueAddTop(trudpQueue *q, void *data, 
        size_t data_length) {
    
    trudpQueueData *qd = trudpQueueAdd(q, data, data_length);
    return trudpQueueMoveToTop(q, qd);
}

/**
 * Add new element after selected in qd field
 * 
 * @param q Pointer to existing TR-UDP Queue
 * @param data Pointer to data of new element
 * @param data_length Length of new element data 
 * @param qd Pointer to trudpQueueData of existing element
 * @return 
 */
trudpQueueData *trudpQueueAddAfter(trudpQueue *q, void *data, size_t data_length, 
        trudpQueueData *qd) {
    
    trudpQueueData *new_qd = NULL;
            
    if(q) {
        // Add to last position
        if(!q->last || (qd && qd == q->last)) new_qd = trudpQueueAdd(q, data, data_length);
        else {
            // Create new trudpQueueData
            new_qd = (trudpQueueData *) malloc(sizeof(trudpQueueData) + data_length);

            // Fill Queue data structure
            new_qd->data_length = data_length;
            if(data && data_length > 0) memcpy(new_qd->data, data, data_length); 

            // Add inside of queue
            if(qd) {
                // Change fields in queue structure
                if(q->last == qd) q->last = new_qd; 
                new_qd->prev = qd; // Set previous of new element to existing element
                new_qd->next = qd->next; // Set next of new element to next of existing
                qd->next = new_qd; // Set next of existing element to new element
            }
            
            // Add to the first position
            else {
                new_qd->prev = NULL; // Set this element previous to null
                new_qd->next = q->first; // Set this element next to first in queue
                if(q->first) q->first->prev = new_qd; // Set previous of first in queue to this element
                q->first = new_qd; // Set previous of first in queue to this element
            }
            
            q->length++; // Increment length
        }        
    }
    
    return new_qd;
}

/**
 * Update element: remove selected and set new one to it place
 * 
 * @param q
 * @param data
 * @param data_length
 * @param qd
 * @return 
 */
trudpQueueData *trudpQueueUpdate(trudpQueue *q, void *data, size_t data_length, 
        trudpQueueData *qd) {
    
    trudpQueueData *new_qd = NULL;
    
    if(q && qd) {
        
        trudpQueueData *qd_after = qd->prev;
        trudpQueueDelete(q, qd);
        new_qd = trudpQueueAddAfter(q, data, data_length, qd_after);
    }
    
    return new_qd;
}

/**
 * Remove element from queue but not free it
 * 
 * @param q Pointer to trudpQueue
 * @param qd Pointer to trudpQueueData
 * @return Pointer to trudpQueueData
 */
inline trudpQueueData *trudpQueueRemove(trudpQueue *q, trudpQueueData *qd) {
        
    if(q && q->length && qd) {
        
        q->length--;        
        if(!q->length) { q->first = q->last = NULL; } // if this element was one in list
        else {
            if(q->first == qd) { q->first = qd->next; q->first->prev = NULL; }   // if this element is first
            else qd->prev->next = qd->next;           // if this element is not first

            if(q->last == qd) { q->last = qd->prev; q->last->next = NULL; }     // if this element is last
            else qd->next->prev = qd->prev;           // if this element is not last
        }
        qd->prev = qd->next = NULL;
    }
    
    return qd;
}

/**
 * Delete element from queue and free it
 * 
 * @param q Pointer to trudpQueue
 * @param qd Pointer to trudpQueueData
 * @return Zero at success
 */
inline int trudpQueueDelete(trudpQueue *q, trudpQueueData *qd) {
       
    if(q && qd) {
        free(trudpQueueRemove(q, qd));    
        return 0;
    }
    else return -1;
}

/**
 * Delete first element from queue and free it
 * 
 * @param q Pointer to trudpQueue
 * @return Zero at success
 */
inline int trudpQueueDeleteFirst(trudpQueue *q) {
    return q && q->first ? trudpQueueDelete(q, q->first) : -1;
}

/**
 * Delete last element from queue and free it
 * 
 * @param q Pointer to trudpQueue
 * @return Zero at success
 */
inline int trudpQueueDeleteLast(trudpQueue *q) {
    return q && q->last ? trudpQueueDelete(q, q->last) : -1;
}

/**
 * Move element from this queue to the end of queue
 * 
 * @param q Pointer to trudpQueue
 * @param qd Pointer to trudpQueueData
 * @return Zero at success
 */
trudpQueueData *trudpQueueMoveToEnd(trudpQueue *q, trudpQueueData *qd) {
   
    if(q && q->length > 1 && qd && qd->next) {
        
        // Remove element
        trudpQueueRemove(q, qd);

        // Add to the end
        q->last->next = qd;
        qd->prev = q->last;
        qd->next = NULL;
        q->last = qd;            
        q->length++;
    }
    
    return qd;
}

/**
 * Move element from this queue to the top of queue
 * 
 * @param q Pointer to trudpQueue
 * @param qd Pointer to trudpQueueData
 * @return Zero at success
 */
trudpQueueData *trudpQueueMoveToTop(trudpQueue *q, trudpQueueData *qd) {
   
    if(q && q->length > 1 && qd && qd->prev) {

        // Remove element
        trudpQueueRemove(q, qd);

        // Add to the beginning
        q->first->prev = qd;
        qd->prev = NULL;
        qd->next = q->first;
        q->first = qd;
        q->length++;
    }
    
    return qd;
}

/**
 * Create new TR-UPD Queue iterator
 * 
 * @param q
 * @return 
 */
trudpQueueIterator *trudpQueueIteratorNew(trudpQueue *q) {
    
    trudpQueueIterator *it = NULL;
    if(q) {
        it = (trudpQueueIterator *) malloc(sizeof(trudpQueueIterator));
        it->qd = NULL;
        it->q = q;
    }
    
    return it; 
}

/**
 * Reset iterator (or swith to new Queue)
 * 
 * @param it
 * @param q
 * @return 
 */
trudpQueueIterator *trudpQueueIteratorReset(trudpQueueIterator *it, trudpQueue *q) {
    
    if(q) {
        it->qd = NULL;
        it->q = q;
    }
    
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
    
    if(!it) return NULL;
    
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
    
    return it ? it->qd : NULL;
}

/**
 * Free TR-UPD Queue iterator
 * 
 * @param it Pointer to trudpQueueIterator
 * @return Zero at success
 */
int trudpQueueIteratorFree(trudpQueueIterator *it) {
    
    if(it) free(it);
    return 0;
}
