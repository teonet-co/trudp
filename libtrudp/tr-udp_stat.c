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
 * \file   tr-udp_stat.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on June 11, 2016, 4:41 PM
 */

#include <string.h>
#include <stdlib.h>

#include "tr-udp_stat.h"
#include "utils.h"

/**
 * Initialize TR-UDP statistic
 * 
 * @param td
 * @return 
 */
inline trudpStatData *trudpStatInit(trudpData *td) {
    
    return trudpStatReset(td);
}

/**
 * Reset TR-UDP statistic
 * 
 * @param td
 * @return 
 */
inline trudpStatData *trudpStatReset(trudpData *td) {
    
    memset(&td->stat, 0, sizeof(td->stat));
    return &td->stat;
}

/**
 * Reset TR-UDP channel statistic
 * 
 * @param tcd
 */
inline void trudpStatChannelInit(trudpChannelData *tcd) {

    trudpStatChannelReset(tcd);
}

/**
 * Reset TR-UDP channel statistic
 * 
 * @param tcd
 */
inline void trudpStatChannelReset(trudpChannelData *tcd) {

    memset(&tcd->stat, 0, sizeof(tcd->stat));
    tcd->stat.triptime_min = UINT32_MAX;
}

/**
 * TR-UDP statistic data
 */
typedef struct trudpStat {
    
    uint32_t packets_send; ///< Total packets send
    uint32_t ack_receive; ///< Total ACK reseived
    uint32_t packets_receive; ///< Total packet reseived
    uint32_t packets_dropped; ///< Total packet droped
        
    uint32_t cs_num; ///< Number of chanels
    trudpStatChannelData cs[]; ///< Cannels statistic
    
} trudpStat;

/**
 * Get TR-UDP statistic in binary or JSON format
 * 
 * @param td Pointer to trudpData
 * @param type Output type: 0 - binary structure trudp_stat; 1 - JSON string
 * @param stat_len [out] Size of return buffer
 * @return Binary structure or JSON string depend of type parameter or NULL at 
 *         error, should be free after use
 */
void *ksnTRUDPstatGet(trudpData *td, int type, size_t *stat_len) {
            
    if(stat_len != NULL) *stat_len = 0;
    void *retval = NULL;
    
     // Binary output type
    if(!type) {
        
        uint32_t cs_num = trudpMapSize(td->map);
        size_t ts_len = sizeof(trudpStat) + cs_num * sizeof(trudpStatChannelData);

        trudpStat *ts = (trudpStat *)malloc(ts_len);
        if(ts != NULL) {
            
            memset(ts, 0, ts_len);
            ts->cs_num = cs_num;    
            if(cs_num) {

                trudpMapIterator *it = trudpMapIteratorNew(td->map);
                if(it != NULL) {
                    int i = 0;
                    while(trudpMapIteratorNext(it)) {
                        trudpMapElementData *el = trudpMapIteratorElement(it);
                        trudpChannelData *tcd = (trudpChannelData *)
                                    trudpMapIteratorElementData(el, NULL);
                        size_t key_length;
                        void *key = trudpMapIteratorElementKey(el, &key_length);
                        // Common statistic
                        ts->packets_send += tcd->stat.packets_send;
                        ts->ack_receive += tcd->stat.ack_receive;
                        ts->packets_receive += tcd->stat.packets_receive;
                        ts->packets_dropped += tcd->stat.packets_receive_dropped;

                        // Cannel statistic 
                        memcpy(&ts->cs[i], &tcd->stat, sizeof(tcd->stat));
                        memcpy(ts->cs[i].key, key, key_length < CS_KEY_LENGTH ? 
                            key_length : CS_KEY_LENGTH - 1);                                                
                        ts->cs[i].sq = trudpQueueSize(tcd->sendQueue->q);
                        ts->cs[i].rq = trudpQueueSize(tcd->receiveQueue->q);
                        i++;
                    }
                    trudpMapIteratorDestroy(it);
                }
            }
            
            // Set return values
            if(stat_len != NULL) *stat_len = ts_len;            
            retval = ts;
        }
    }
    
    // JSON string output type
    else {
        
        size_t ts_len;
        trudpStat *ts = ksnTRUDPstatGet(td, 0, &ts_len);
        
        if(ts != NULL) {
            
            int i;
            char *cs = strdup("");
            for(i = 0; i < ts->cs_num; i++) {
                cs = sformatMessage(cs,
                    "%s%s"
                    "{ "
                    "\"key\": \"%s\", "    
                    "\"ack_receive\": %d, "
                    "\"packets_attempt\": %d, "
                    "\"packets_receive\": %d, "
                    "\"packets_receive_dropped\": %d, "
                    "\"packets_send\": %d, "
                    "\"receive_speed\": %11.3f, "
                    "\"receive_total\": %10.3f, "
                    "\"send_speed\": %11.3f, "
                    "\"send_total\": %10.3f, " 
                    "\"triptime_avg\": %d, "
                    "\"triptime_last\": %d, "
                    "\"triptime_last_max\": %d, "
                    "\"triptime_max\": %d, "
                    "\"triptime_min\": %d, "
                    "\"wait\": %9.3f, "
                    "\"sq\": %d, "
                    "\"rq\": %d"
                    " }",
                    cs, i ? ", " : "",
                    ts->cs[i].key,
                    ts->cs[i].ack_receive,
                    ts->cs[i].packets_attempt,
                    ts->cs[i].packets_receive,
                    ts->cs[i].packets_receive_dropped,
                    ts->cs[i].packets_send,
                    (double)(1.0 * ts->cs[i].receive_speed / 1024.0),
                    ts->cs[i].receive_total,
                    (double)(1.0 * ts->cs[i].send_speed / 1024.0),
                    ts->cs[i].send_total,
                    ts->cs[i].triptime_avg,
                    ts->cs[i].triptime_last,
                    ts->cs[i].triptime_last_max,
                    ts->cs[i].triptime_max,
                    ts->cs[i].triptime_min,
                    ts->cs[i].wait,
                    ts->cs[i].sq,
                    ts->cs[i].rq
                );
            }
            
            char *json_str = formatMessage(
                "{ "
                "\"packets_send\": %d, "
                "\"ack_receive\": %d, "
                "\"packets_receive\": %d, "
                "\"packets_dropped\": %d, "
                "\"cs_num\": %d, "
                "\"cs\": [ %s ]"
                " }",
                ts->packets_send,
                ts->ack_receive,
                ts->packets_receive,
                ts->packets_dropped,
                ts->cs_num,
                cs    
            );
            free(cs);

            // Set return values and free used memory
            if(stat_len != NULL) *stat_len = strlen(json_str);
            retval = json_str;
            free(ts);            
        }
    }
    
    return retval;
}
