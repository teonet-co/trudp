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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trudp_stat.h"
#include "utils_r.h"
#include "utils.h"

/**
 * Reset TR-UDP statistic
 *
 * @param td
 * @return
 */
static inline trudpStatData *trudpStatReset(trudpData *td) {

    memset(&td->stat, 0, sizeof(td->stat));
    return &td->stat;
}

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
 * Reset TR-UDP channel statistic
 *
 * @param tcd
 */
static inline void trudpStatChannelReset(trudpChannelData *tcd) {

    memset(&tcd->stat, 0, sizeof(tcd->stat));
    tcd->stat.triptime_min = UINT32_MAX;
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
 * Process packet and calculate last 10 send packets array
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to packet
 * @param send_data_length Send packet data size
 */
void trudpStatProcessLast10Send(trudpChannelData *tcd, void *packet,
        size_t send_data_length) {

    // Add triptime to last 10 array
    tcd->stat.last_send_packets_ar[tcd->stat.idx_snd].triptime = tcd->stat.triptime_last;

    // Add last data size to last 10 array
    uint32_t size_b = send_data_length + trudpPacketGetHeaderLength(packet);
    tcd->stat.last_send_packets_ar[tcd->stat.idx_snd].size_b = size_b;
    tcd->stat.send_total += 1.0 * size_b / (1024.0 * 1024.0);
    tcd->stat.last_send_packets_ar[tcd->stat.idx_snd].ts = trudpPacketGetTimestamp(packet);

    // Last 10 array next index
    tcd->stat.idx_snd++;
    if(tcd->stat.idx_snd >= LAST10_SIZE) tcd->stat.idx_snd = 0;

    // Calculate max triptime & speed in bytes in sec in last 10 packet
    {
        int i;
        uint32_t min_ts = UINT32_MAX, max_ts = 0, size_b = 0;
        uint32_t triptime_last_max = 0;
        for(i = 0; i < LAST10_SIZE; i++) {

            // Last maximum triptime
            if(tcd->stat.last_send_packets_ar[i].triptime > triptime_last_max)
                triptime_last_max = tcd->stat.last_send_packets_ar[i].triptime;

            // Calculate size sum & define minimum and maximum timestamp
            size_b += tcd->stat.last_send_packets_ar[i].size_b;
            if(tcd->stat.last_send_packets_ar[i].ts > max_ts) max_ts = tcd->stat.last_send_packets_ar[i].ts;
            else if (tcd->stat.last_send_packets_ar[i].ts > 0 && tcd->stat.last_send_packets_ar[i].ts < min_ts) min_ts = tcd->stat.last_send_packets_ar[i].ts;
        }

        // Last maximal triptime
        tcd->stat.triptime_last_max = (tcd->stat.triptime_last_max + 2 * triptime_last_max) / 3;

        // Send speed \todo use trudpGetTimestamp() or max_ts
        uint32_t dif_ts = trudpGetTimestamp()/*max_ts*/ - min_ts;
        if(dif_ts) tcd->stat.send_speed = 1.0 * size_b / (1.0 * (dif_ts) / 1000000.0);
//        else tcd->stat.send_speed = 0;
    }
}

/**
 * Process packet and calculate last 10 received packets array
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to packet
 */
void trudpStatProcessLast10Receive(trudpChannelData *tcd, void *packet) {

    // Add last data size to last 10 array
    uint32_t size_b = trudpPacketGetDataLength(packet) + trudpPacketGetHeaderLength(packet);
    tcd->stat.last_receive_packets_ar[tcd->stat.idx_rcv].size_b = size_b;
    tcd->stat.last_receive_packets_ar[tcd->stat.idx_rcv].ts = trudpPacketGetTimestamp(packet);
    tcd->stat.receive_total += 1.0 * size_b / (1024.0 * 1024.0);

    // Last 10 array next index
    tcd->stat.idx_rcv++;
    if(tcd->stat.idx_rcv >= LAST10_SIZE) tcd->stat.idx_rcv = 0;

    // Calculate speed in bytes in sec in last 10 packet
    {
        int i;
        uint32_t min_ts = UINT32_MAX, max_ts = 0, size_b = 0;
        for(i = 0; i < LAST10_SIZE; i++) {

            // Calculate size sum & define minimum and maximum timestamp
            size_b += tcd->stat.last_receive_packets_ar[i].size_b;
            if(tcd->stat.last_receive_packets_ar[i].ts > max_ts) {
                
                max_ts = tcd->stat.last_receive_packets_ar[i].ts;
            }
            else if(tcd->stat.last_receive_packets_ar[i].ts > 0 && 
                    tcd->stat.last_receive_packets_ar[i].ts < min_ts) {
                
                min_ts = tcd->stat.last_receive_packets_ar[i].ts;
            }
        }

        // Receive speed \todo use trudpGetTimestamp() or max_ts
        uint32_t dif_ts = trudpGetTimestamp()/*max_ts*/ - min_ts;
        if(dif_ts) tcd->stat.receive_speed = 1.0 * size_b / (1.0 * (dif_ts) / 1000000.0);
        else tcd->stat.receive_speed = 0;
    }
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
void *trudpStatGet(trudpData *td, int type, size_t *stat_len) {

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
                        memcpy(ts->cs[i].key, key, key_length < MAX_KEY_LENGTH ?
                            key_length : MAX_KEY_LENGTH - 1);
                        ts->cs[i].sq = trudpSendQueueSize(tcd->sendQueue);
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
        trudpStat *ts = trudpStatGet(td, 0, &ts_len);

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

/**
 * Remove leading string spaces and zeros
 * 
 * @param str
 * 
 * @return Pointer to part of string without leading spaces and zeros
 */
static char* trimLeadingZerro(char *str) {
    
    char *ret_str = str;
    size_t i = 0, len = strlen(str);
    
    while(i<len) {
        if(str[i] != ' ' && !(str[i] == '0' && str[i+1] != '.') ) { 
            ret_str = str + i; 
            break; 
        }
        i++;
    }
    
    return ret_str;
}

/**
 * Return formated time string
 * 
 * @param t Time
 * 
 * @return Formated time string: days minutes seconds.milliseconds
 */
static char* showTime(double t) {
    
    #define T_STR_LEN 64
    static char t_str[T_STR_LEN];

    double seconds;
    int days, hours, minutes;
    const int minute_sec = 60 /* sec */;
    const int hour_sec = minute_sec * 60 /* min */;
    const int day_sec = hour_sec * 24 /* hours */;
    days = t / day_sec;
    hours = (t - days * day_sec) / hour_sec;
    minutes = (t - days * day_sec - hours * hour_sec) /  minute_sec;
    seconds = t - days * day_sec - hours * hour_sec - minutes * minute_sec;
    
    snprintf((char*)t_str, T_STR_LEN, "%d%s%d%s%d%s%.3f sec"
            , days
            , (days ? " days " : " ")
            , hours
            , (!hours && !days ? " " : " hour ")
            , minutes
            , (!minutes && !hours && !days ? " " : " min ")
            , seconds
    );
            
    return trimLeadingZerro(t_str);
}

/**
 * Show TR-UDP statistics
 *
 * Return string with statistics. It should be free after use.
 *
 * @param td
 *
 * @return Pointer to allocated string with statistics
 */
char *ksnTRUDPstatShowStr(trudpData *td) {

    static uint32_t show_stat_time = 0;
    uint32_t ts = trudpGetTimestamp();
    
    uint32_t packets_send = 0,
             packets_receive = 0,
             ack_receive = 0,
             packets_dropped = 0;

    int i = 0;
    char *tbl_str = strdup(""), *tbl_total = strdup("");
    trudpStatChannelData totalStat;
    trudpMapIterator *it = trudpMapIteratorNew(td->map);
    if(it != NULL) {        
        memset(&totalStat, 0, sizeof(totalStat));
        while(trudpMapIteratorNext(it)) {

            size_t key_len;
            trudpMapElementData *el = trudpMapIteratorElement(it);
            char *key = trudpMapIteratorElementKey(el, &key_len);
            trudpChannelData *tcd = (trudpChannelData *)
                                    trudpMapIteratorElementData(el, NULL);

            packets_send += tcd->stat.packets_send;
            ack_receive += tcd->stat.ack_receive;
            packets_receive += tcd->stat.packets_receive;
            packets_dropped += tcd->stat.packets_receive_dropped;
            
            size_t sendQueueSize = trudpSendQueueSize(tcd->sendQueue);
            size_t receiveQueueSize = trudpQueueSize(tcd->receiveQueue->q);

            tbl_str = sformatMessage(tbl_str,
                "%s%3d "_ANSI_BROWN"%-24.*s"_ANSI_NONE" %8d %11.3f %10.3f  %9.3f /%9.3f %8d %11.3f %10.3f %8d %8d(%d%%) %8d(%d%%) %6d %6d\n",
                tbl_str, i + 1,
                key_len, key,
                tcd->stat.packets_send,
                (double)(1.0 * tcd->stat.send_speed / 1024.0),
                tcd->stat.send_total,
                tcd->stat.triptime_last / 1000.0,
                tcd->stat.wait,
                tcd->stat.packets_receive,
                (double)(1.0 * tcd->stat.receive_speed / 1024.0),
                tcd->stat.receive_total,
                tcd->stat.ack_receive,
                tcd->stat.packets_attempt,
                tcd->stat.packets_send ? 100 * tcd->stat.packets_attempt / tcd->stat.packets_send : 0,     
                tcd->stat.packets_receive_dropped,
                tcd->stat.packets_receive ? 100 * tcd->stat.packets_receive_dropped / tcd->stat.packets_receive : 0,    
                sendQueueSize,
                receiveQueueSize
            );
            totalStat.packets_send += tcd->stat.packets_send;
            totalStat.send_speed += tcd->stat.send_speed;
            totalStat.send_total += tcd->stat.send_total;
            totalStat.triptime_last += tcd->stat.triptime_last;
            totalStat.wait += tcd->stat.wait;
            totalStat.packets_receive += tcd->stat.packets_receive;
            totalStat.receive_speed += tcd->stat.receive_speed;
            totalStat.receive_total += tcd->stat.receive_total;
            totalStat.ack_receive += tcd->stat.ack_receive;
            totalStat.packets_attempt += tcd->stat.packets_attempt;
            totalStat.packets_receive_dropped += tcd->stat.packets_receive_dropped;
            totalStat.sendQueueSize += sendQueueSize;
            totalStat.receiveQueueSize += receiveQueueSize;
            i++;
        }
        if(i > 0) {
            tbl_str = sformatMessage(tbl_str,
            "%s"
            "---------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"
            , tbl_str
            );
        }
        if(i > 1) {
            totalStat.triptime_last /= i;
            totalStat.wait /= i;

            tbl_total = sformatMessage(tbl_total,
            "                             %8d %11.3f %10.3f  %9.3f /%9.3f %8d %11.3f %10.3f %8d %8d(%d%%) %8d(%d%%) %6d %6d\n"
            "---------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"
        
            , totalStat.packets_send
            , (double)(1.0 * totalStat.send_speed / 1024.0)
            , totalStat.send_total
            , totalStat.triptime_last / 1000.0
            , totalStat.wait
            , totalStat.packets_receive
            , (double)(1.0 * totalStat.receive_speed / 1024.0)
            , totalStat.receive_total
            , totalStat.ack_receive
            , totalStat.packets_attempt
            , totalStat.packets_send ? 100 * totalStat.packets_attempt / totalStat.packets_send : 0
            , totalStat.packets_receive_dropped
            , totalStat.packets_receive ? 100 * totalStat.packets_receive_dropped / totalStat.packets_receive : 0
            , totalStat.sendQueueSize
            , totalStat.receiveQueueSize
            );            
        }
        trudpMapIteratorDestroy(it);
    }

    char *ret_str = formatMessage(
//        _ANSI_CLS"\033[0;0H"
        "---------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"
        "TR-UDP statistics, port %d, running time: %s, show statistic time %.3f ms\n"
//        "---------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"
//        "\n"
//        "  Packets sent: %-12d                " "Send list:                      " "Receive Heap:\n"
//        "  ACK receive: %-12d                 " "  size_max: %-12d        "        "  size_max: %-12d\n"
//        "  Packets receive: %-12d             " "  size_current: %-12d    "        "  size_current: %-12d\n"
//        "  Packets receive and dropped: %-12d " "  attempts: %-12d\n"
//        "\n"
        "List of channels:\n"
        "---------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"
        "  # Key                          Send  Speed(kb/s)  Total(mb) Trip time /  Wait(ms) |  Recv  Speed(kb/s)  Total(mb)     ACK |     Repeat         Drop |   SQ     RQ  \n"
        "---------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"
        "%s"
        "%s"
        "  "
        _ANSI_GREEN"send:"_ANSI_NONE" send packets, "
        _ANSI_GREEN"speed:"_ANSI_NONE" send speed(kb/s), "
        _ANSI_GREEN"total:"_ANSI_NONE" send in megabytes, "
        _ANSI_GREEN"wait:"_ANSI_NONE" time to wait ACK, "
        _ANSI_GREEN"recv:"_ANSI_NONE" receive packets   \n"

        "  "
        _ANSI_GREEN"ACK:"_ANSI_NONE" receive ACK,   "
        _ANSI_GREEN"repeat:"_ANSI_NONE" resend packets,  "
        _ANSI_GREEN"drop:"_ANSI_NONE" duplicate received, "
        _ANSI_GREEN"SQ:"_ANSI_NONE" send queue,         "
        _ANSI_GREEN"RQ:"_ANSI_NONE" receive queue       \n"
    
        , td->port
        , showTime((trudpGetTimestampFull() - td->started) / 1000000.0)
        , show_stat_time / 1000000.0

//        , packets_send
//        , ack_receive, td->stat.sendQueue.size_max, td->stat.receiveQueue.size_max
//        , packets_receive, td->stat.sendQueue.size_current, td->stat.receiveQueue.size_current
//        , packets_dropped, td->stat.sendQueue.attempt

        , tbl_str
        , tbl_total
    
    );

    free(tbl_str);
    free(tbl_total);
    
    show_stat_time = trudpGetTimestamp() - ts;

    return ret_str;
}

#define MAX_QUELEN_SHOW 40
char *trudpStatShowQueueStr(trudpChannelData *tcd, int type) {
    
    char *str = strdup("");
    
//    if(trudpPacketQueueSize(tcd->sendQueue) > MAX_QUELEN_SHOW || 
//       trudpPacketQueueSize(tcd->receiveQueue) > MAX_QUELEN_SHOW) 
//    exit(-1); 
    
    trudpQueueIterator *it = !type ? 
        trudpQueueIteratorNew(tcd->sendQueue->q) : 
        trudpQueueIteratorNew(tcd->receiveQueue->q);
    
    if(it != NULL) {        
        
        int i = 0;
        uint64_t current_t = trudpGetTimestampFull();
        str = sformatMessage(str, 
            "--------------------------------------------------------------\n"
            "TR-UDP %s Queue, size: %d, %s %u\n"
            "--------------------------------------------------------------\n"
            "    #   Id          Expected   Retrieves\n"
            "--------------------------------------------------------------\n"
            , !type ? "Send" : "Receive"
            , !type ? trudpSendQueueSize(tcd->sendQueue) : trudpPacketQueueSize(tcd->receiveQueue)
            , !type ? "next id: " : "expected id: "
            , !type ? tcd->sendId : tcd->receiveExpectedId
        );
        while(trudpQueueIteratorNext(it)) {
            
            trudpPacketQueueData *tqd = (trudpPacketQueueData *)
                    ((trudpQueueData *)trudpQueueIteratorElement(it))->data;
                        
            long timeout_sq = current_t < tqd->expected_time ? 
                (long)(tqd->expected_time - current_t) : 
                -1 * (long)(current_t - tqd->expected_time);
                        
            str = sformatMessage(str,             
            "%s"
            "  %3d   %-8d %8.3f ms   %d\n"
            , str
            , i++   
            , trudpPacketGetId(tqd->packet)
            , !type ? timeout_sq / 1000.0 : 0
            , !type ? tqd->retrieves : 0
            );
            if(i > MAX_QUELEN_SHOW) { str = sformatMessage(str, "%s...\n", str); break; }
        }
        if(i) {
            str = sformatMessage(str,
            "%s"
            "--------------------------------------------------------------\n"
            , str
        );

        }
        trudpQueueIteratorFree(it);
    }
    
    return str;
}
