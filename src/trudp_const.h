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
 * \file   trudp_const.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on July 21, 2016, 12:34 AM
 */

#ifndef TRUDP_CONST_H
#define TRUDP_CONST_H

#ifdef __cplusplus
extern "C" {
#endif

// TR-UDP constants
#define MAX_KEY_LENGTH 64 // Maximum key length
#define MAX_OUTRUNNING 500 // Maximum outrunning in receive queue to send reset
#define START_MIDDLE_TIME (MAX_ACK_WAIT/5) * 1000000 // Midle time at start
#define RESET_AFTER_ID (UINT32_MAX - 1024) // Reset if send id more than this constant
#define MAX_TRIPTIME_MIDDLE 5757575/2 // Maximum number of Middle triptime
#define MAX_LAST_RECEIVE MAX_TRIPTIME_MIDDLE*5 // Disconnect after last receved packet time older than this constant (14.39 sec)
#define SEND_PING_AFTER 2500000*4 // Send trudp ping after 10 sec
#define MAP_SIZE_DEFAULT 107 // Default map size; map stored connected channels and can auto resize
#define USE_WRITE_QUEUE 0 // Use write queue instead of direct write to socket
#define RTT 30000 // This constant used in send queue expected time calculation
#define MAX_RTT 500000 // This constant used in send queue expected time calculation
#define RESET_AT_LONG_RETRANSMIT 0 // Send rest at long retransmit retrives time
#define NORMAL_S_SIZE 40 //48 // Normal size of send queue
#define MAX_WRITE_QUEUE_SIZE 1000 //max size of write queue
#define TRIPTIME_MIDDLE_RESET_TIME 100000 //if middle triptime greater than this value, trudp will reset channel


#ifdef __cplusplus
}
#endif

#endif /* TRUDP_CONST_H */
