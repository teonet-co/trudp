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
 * \file   queue.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 30, 2016, 11:56 AM
 */

#include <stdlib.h>
#include <string.h>

#include "queue.h"

/**
 * Create new Teo Queue
 * 
 * @return Pointer to new Teo Queue or NULL if memory allocate error
 */
teoQueue *teoQueueNew() {
    
    teoQueue *q = (teoQueue *) malloc(sizeof(teoQueue));
    memset(q, 0, sizeof(teoQueue));
    
    return q;    
}

/**
 * Destroy Teo Queue
 * 
 * @param q Pointer to existing Teo Queue
 * 
 * @return Zero at success
 */
inline int teoQueueDestroy(teoQueue *q) {
    
    if(q) {
        teoQueueFree(q);
        free(q);
    }
    
    return 0;    
}

/**
 * Remove all elements from Teo queue
 * 
 * @param q Pointer to existing Teo Queue
 * 
 * @return Zero at success
 */
int teoQueueFree(teoQueue *q) {
    
    if(!q) return -1;
    
    // Remove all elements
    teoQueueData *qd_next, *qd = q->first;
    while(qd) {
        qd_next = qd->next;
        free(qd);
        qd = qd_next;
    }    
    memset(q, 0, sizeof(teoQueue));
    
    return 0;
}

/**
 * Get number of elements in Teo queue
 * 
 * @param q
 * 
 * @return Number of elements in Teo queue
 */
inline size_t teoQueueSize(teoQueue *q) {
    
    return q ? q->length : -1;
}

/**
 * Put (add, copy) existing queue record to the end of selected queue
 * 
 * @param q Pointer to teoQueue
 * @param qd Pointer to teoQueueData
 * @return Zero at success
 */
teoQueueData *teoQueuePut(teoQueue *q, teoQueueData *qd) {
   
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
 * Create new teoQueueData buffer
 * 
 * @param data Pointer to data
 * @param data_length Data length
 * @return 
 */
teoQueueData *teoQueueNewData(void *data, size_t data_length) {
    
    // Create new teoQueueData
    teoQueueData *qd = (teoQueueData *) malloc(sizeof(teoQueueData) + data_length);
    if(qd) {
        // Fill Queue data structure
        qd->data_length = data_length;
        if(data && data_length > 0) memcpy(qd->data, data, data_length);
    }

    return qd;
}

/**
 * Add new element to the end of Teo queue
 * 
 * @param q Pointer to existing Teo Queue
 * @param data Pointer to data of new element
 * @param data_length Length of new element data 
 * 
 * @return Pointer to teoQueueData of added element
 */
teoQueueData *teoQueueAdd(teoQueue *q, void *data, size_t data_length) {
    
    teoQueueData *qd = NULL;
    
    if(q) {
        // Create new teoQueueData
        qd = teoQueueNewData(data, data_length);
        if(qd) teoQueuePut(q, qd);
    }
    return qd;
}

/**
 * Add new element to the top of Teo queue
 * 
 * @param q Pointer to existing Teo Queue
 * @param data Pointer to data of new element
 * @param data_length Length of new element data 
 * 
 * @return Pointer to teoQueueData of added element
 */
inline teoQueueData *teoQueueAddTop(teoQueue *q, void *data, 
        size_t data_length) {
    
    teoQueueData *qd = teoQueueAdd(q, data, data_length);
    return teoQueueMoveToTop(q, qd);
}

/**
 * Add new element after selected in qd field
 * 
 * @param q Pointer to existing Teo Queue
 * @param data Pointer to data of new element
 * @param data_length Length of new element data 
 * @param qd Pointer to teoQueueData of existing element
 * @return 
 */
teoQueueData *teoQueueAddAfter(teoQueue *q, void *data, size_t data_length, 
        teoQueueData *qd) {
    
    teoQueueData *new_qd = NULL;
            
    if(q) {
        // Add to first position
        if(!qd) new_qd = teoQueueAddTop(q, data, data_length);
        // Add to last position
        else if(!q->last || qd == q->last) new_qd = teoQueueAdd(q, data, data_length);
        // Add after selected element
        else {
            // Create new teoQueueData
            new_qd = teoQueueNewData(data, data_length);

            // Add inside of queue
            if(new_qd) {
                // Change fields in queue structure
                if(q->last == qd) q->last = new_qd; 
                new_qd->prev = qd; // Set previous of new element to existing element
                new_qd->next = qd->next; // Set next of new element to next of existing
                qd->next = new_qd; // Set next of existing element to new element
                q->length++; // Increment length
            }            
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
teoQueueData *teoQueueUpdate(teoQueue *q, void *data, size_t data_length, 
        teoQueueData *qd) {
    
    teoQueueData *new_qd = NULL;
    
    if(q && qd) {
        
//        teoQueueData *qd_after = qd->prev;
//        teoQueueDelete(q, qd);
//        if(qd_after)
//            new_qd = teoQueueAddAfter(q, data, data_length, qd_after);
//        else
//            teoQueueAddTop(q, data, data_length);
        new_qd = teoQueueNewData(data, data_length);
        if(new_qd) {
            new_qd->prev = qd->prev;
            new_qd->next = qd->next;
            if(new_qd->prev) new_qd->prev->next = new_qd;
            if(new_qd->next) new_qd->next->prev = new_qd;
            if(q->first == qd) q->first = new_qd;
            if(q->last == qd) q->last = new_qd;
            
            free(qd);
        }
        
    }
    
    return new_qd;
}

/**
 * Remove element from queue but not free it
 * 
 * @param q Pointer to teoQueue
 * @param qd Pointer to teoQueueData
 * @return Pointer to teoQueueData
 */
inline teoQueueData *teoQueueRemove(teoQueue *q, teoQueueData *qd) {
        
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
 * @param q Pointer to teoQueue
 * @param qd Pointer to teoQueueData
 * @return Zero at success
 */
inline int teoQueueDelete(teoQueue *q, teoQueueData *qd) {
       
    if(q && qd) {
        free(teoQueueRemove(q, qd));    
        return 0;
    }
    else return -1;
}

/**
 * Delete first element from queue and free it
 * 
 * @param q Pointer to teoQueue
 * @return Zero at success
 */
inline int teoQueueDeleteFirst(teoQueue *q) {
    return q && q->first ? teoQueueDelete(q, q->first) : -1;
}

/**
 * Delete last element from queue and free it
 * 
 * @param q Pointer to teoQueue
 * @return Zero at success
 */
inline int teoQueueDeleteLast(teoQueue *q) {
    return q && q->last ? teoQueueDelete(q, q->last) : -1;
}

/**
 * Move element from this queue to the end of queue
 * 
 * @param q Pointer to teoQueue
 * @param qd Pointer to teoQueueData
 * @return Pointer to input teoQueueData
 */
teoQueueData *teoQueueMoveToEnd(teoQueue *q, teoQueueData *qd) {
   
    if(q && q->length > 1 && qd && qd->next) {
        
        // Remove element
        teoQueueRemove(q, qd);

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
 * @param q Pointer to teoQueue
 * @param qd Pointer to teoQueueData
 * @return Pointer to input teoQueueData
 */
teoQueueData *teoQueueMoveToTop(teoQueue *q, teoQueueData *qd) {
   
    if(q && q->length > 1 && qd && qd->prev) {

        // Remove element
        teoQueueRemove(q, qd);

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
 * Create new Teo Queue iterator
 * 
 * @param q Pointer to teoQueue
 * @return 
 */
teoQueueIterator *teoQueueIteratorNew(teoQueue *q) {
    
    teoQueueIterator *it = NULL;
    if(q) {
        it = (teoQueueIterator *) malloc(sizeof(teoQueueIterator));
        it->qd = NULL;
        it->q = q;
    }
    
    return it; 
}

/**
 * Reset iterator (or swith to new Queue)
 * 
 * @param it
 * @param q Pointer to teoQueue
 * @return 
 */
teoQueueIterator *teoQueueIteratorReset(teoQueueIterator *it, teoQueue *q) {
    
    if(q) {
        it->qd = NULL;
        it->q = q;
    }
    
    return it;
}

/**
 * Get next element from Teo Queue iterator
 * 
 * @param it Pointer to teoQueueIterator
 * 
 * @return Pointer to the teoQueueData or NULL if not exists
 */
teoQueueData *teoQueueIteratorNext(teoQueueIterator *it) {
    
    if(!it) return NULL;
    
    if(!it->qd) it->qd = it->q->first;
    else it->qd = it->qd->next;
    
    return it->qd;
}

/**
 * Get previous element from Teo Queue iterator
 * 
 * @param it Pointer to teoQueueIterator
 * 
 * @return Pointer to the teoQueueData or NULL if not exists
 */
teoQueueData *teoQueueIteratorPrev(teoQueueIterator *it) {
    
    if(!it) return NULL;
    
    if(!it->qd) it->qd = it->q->last;
    else it->qd = it->qd->prev;
    
    return it->qd;
}

/**
 * Get current Teo Queue iterator element
 * @param it Pointer to teoQueueIterator
 * 
 * @return Pointer to the teoQueueData or NULL if not exists
 */
inline teoQueueData *teoQueueIteratorElement(teoQueueIterator *it) {
    
    return it ? it->qd : NULL;
}

/**
 * Free Teo Queue iterator
 * 
 * @param it Pointer to teoQueueIterator
 * @return Zero at success
 */
int teoQueueIteratorFree(teoQueueIterator *it) {
    
    if(it) free(it);
    return 0;
}

/**
 * Loop through queue and call callback function with index and data in parameters
 * 
 * @param q Pointer to teoQueue
 * @param callback Pointer to callback function teoQueueForeachFunction
 * 
 * @return Number of elements processed
 */
int teoQueueForeach(teoQueue *q, teoQueueForeachFunction callback, 
        void *user_data) {
    
    int i = 0;
    teoQueueIterator *it = teoQueueIteratorNew(q);
    if(it != NULL) {
        
        while(teoQueueIteratorNext(it)) {
            
            if(callback(q, i, teoQueueIteratorElement(it), user_data)) break;
        }
        teoQueueIteratorFree(it);
    }
    
    return i;
}
