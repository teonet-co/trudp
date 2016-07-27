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
 * File:   trudp_ev.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 * 
 * Libev module. To use this module set flag -DUSE_LIBEV
 *
 * Created on July 27, 2016, 11:11 AM
 */

#ifdef USE_LIBEV

//#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>

#include <ev.h>

#include "trudp.h"
#include "trudp_ev.h"

/**
 * Send queue processing timer libev callback
 *
 * @param loop
 * @param w
 * @param revents
 */
static void trudp_process_send_queue_cb(EV_P_ ev_timer *w, int revents) {

    trudpProcessSendQueueData *psd = (trudpProcessSendQueueData *) w->data;

    // Process send queue
    //debug("process send queue ... \n");
    uint64_t next_expected_time;
    trudp_SendQueueProcess(psd->td, &next_expected_time);

    // Start new process_send_queue timer
    if(next_expected_time)
        trudp_start_send_queue_cb(psd, next_expected_time);
}

/**
 * Start send queue timer
 *
 * @param psd Pointer to process_send_queue_data
 * @param next_expected_time
 */
void trudp_start_send_queue_cb(trudpProcessSendQueueData *psd,
        uint64_t next_expected_time) {

    uint64_t tt, next_et = UINT64_MAX, ts = trudpGetTimestampFull();

    // If next_expected_time selected (non nil)
    if(next_expected_time) {        
        next_et = next_expected_time > ts ? next_expected_time - ts : 0;
    }

    // If next_expected_time (net) or GetSendQueueTimeout
    if((tt = (next_et != UINT64_MAX) ? next_et : trudp_SendQueueGetTimeout(psd->td, ts)) != UINT32_MAX) {

        double tt_d = tt / 1000000.0;
        if(tt_d == 0.0) tt_d = 0.0001;
        
        if(!psd->inited) {
            ev_timer_init(&psd->process_send_queue_w, trudp_process_send_queue_cb, tt_d, 0.0);
            psd->process_send_queue_w.data = (void*)psd;
            psd->inited = 1;
        }
        else {
            ev_timer_stop(psd->loop, &psd->process_send_queue_w);
            ev_timer_set(&psd->process_send_queue_w, tt_d, 0.0);
        }

        ev_timer_start(psd->loop, &psd->process_send_queue_w);
    }
}

#endif
