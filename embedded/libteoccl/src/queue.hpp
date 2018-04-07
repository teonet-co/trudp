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
 * \file   queue
 * \Author Kirill Scherba <kirill@scherba.ru>
 *
 * Teo ccl Queue C++ wrapper
 *
 * Created on March 25, 2018, 2:25 PM
 */

#ifndef QUEUE_HXX
#define QUEUE_HXX

#include <string>
#include <stdlib.h>

#include "queue.h"

namespace teo {

  class Queue {

  private:

    teoQueue *que;

  public:

    Queue() { que = teoQueueNew(); }
    virtual ~Queue() { teoQueueDestroy(que); }

  public:

    // free
    inline int freeAll() { return teoQueueFree(que); }

    // newData
    inline teoQueueData *newData(void *data, size_t data_length) {
      return teoQueueNewData(data, data_length);
    }
    inline teoQueueData *newData(std::string data) {
      return newData((void*)data.c_str(), data.size() + 1);
    }
    template<typename T> teoQueueData *newData(const T &data) {
      return newData((void*)&data, sizeof(T));
    }

    // free data
    inline int freeData(teoQueueData *qd) {
      free((void*)qd);
      return 0;
    }

    // add
    inline teoQueueData *add(void *data, size_t data_length) {
      return teoQueueAdd(que, data, data_length);
    }
    inline teoQueueData *add(std::string data) {
      return add((void*)data.c_str(), data.size() + 1);
    }
    template<typename T> teoQueueData *add(const T &data) {
      return add((void*)&data, sizeof(T));
    }

    // add top
    inline teoQueueData *addTop(void *data, size_t data_length) {
      return teoQueueAddTop(que, data, data_length);
    }
    inline teoQueueData *addTop(std::string data) {
      return addTop((void*)data.c_str(), data.size() + 1);
    }
    template<typename T> teoQueueData *addTop(const T &data) {
      return addTop((void*)&data, sizeof(T));
    }

    // add after
    inline teoQueueData *addAfter(void *data, size_t data_length, teoQueueData *after) {
      return teoQueueAddAfter(que, data, data_length, after);
    }
    inline teoQueueData *addAfter(std::string data, teoQueueData *after) {
      return addAfter((void*)data.c_str(), data.size() + 1, after);
    }
    template<typename T> teoQueueData *addAfter(const T &data, teoQueueData *after) {
      return addAfter((void*)&data, sizeof(T), after);
    }

    // remove (not free)
    inline teoQueueData *remove(teoQueueData *qd) {
      return teoQueueRemove(que, qd);
    }

    // delete (remove and free)
    inline int del(teoQueueData *qd) {
      return teoQueueDelete(que, qd);
    }

    // delete first (remove and free)
    inline int delFirst() {
      return teoQueueDeleteFirst(que);
    }

    // delete last (remove and free)
    inline int delLast() {
      return teoQueueDeleteLast(que);
    }

    // move to top
    inline teoQueueData *moveToTop(teoQueueData *qd) {
      return teoQueueMoveToTop(que, qd);
    }

    // move to end
    inline teoQueueData *moveToEnd(teoQueueData *qd) {
      return teoQueueMoveToEnd(que, qd);
    }

    // put
    inline teoQueueData *put(teoQueueData *qd) {
      return teoQueuePut(que, qd);
    }

    // size
    inline size_t size() {
      return teoQueueSize(que);
    }

    // get iterator new
    inline teoQueueIterator *iterator() {
      return teoQueueIteratorNew(que);
    }

    // iterator next element
    inline teoQueueData *iteratorNext(teoQueueIterator *it) {
      return teoQueueIteratorNext(it);
    }

    // iterator previous element
    inline teoQueueData *iteratorPrev(teoQueueIterator *it) {
      return teoQueueIteratorPrev(it);
    }

    // iterator current element
    inline teoQueueData *iteratorElement(teoQueueIterator *it) {
      return teoQueueIteratorElement(it);
    }

    // iterator reset (or swith to new Queue)
    inline teoQueueIterator *iteratorElement(teoQueueIterator *it, Queue *q = NULL) {
      return teoQueueIteratorReset(it, q ? q->que : NULL);
    }

    // iterator free
    inline int iteratorFree(teoQueueIterator *it) {
      return teoQueueIteratorFree(it);
    }
    
    // foreach    
    inline int foreach(teoQueueForeachFunction callback, void *user_data = NULL) {
      return teoQueueForeach(que, callback, user_data);
    }

  };

}

#endif /* QUEUE_HXX */
