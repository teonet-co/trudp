/**
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
 * \file   tr-udp_stat.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on June 11, 2016, 4:40 PM
 */

#ifndef TRU_UDP_STAT_H
#define TRU_UDP_STAT_H

#include <string.h>

#include "trudp.h"
#include "trudp_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_QUELEN_SHOW 40
#define NUMBER_CHANNELS_IN_CLI_PAGE 20

typedef enum trudpStatType {

    BINARY_TYPE,
    JSON_TYPE

} trudpStatType;

TRUDP_API char *ksnTRUDPstatShowStr(trudpData *td, int page);
TRUDP_API char *trudpStatShowQueueStr(trudpChannelData *tcd, int type);

/**
 * Reset TR-UDP statistic
 *
 * @param td
 * @return
 */
static inline 
trudpStatData *trudpStatReset(trudpData *td) {
    memset(&td->stat, 0, sizeof(td->stat));
    return &td->stat;
}
/**
 * Initialize TR-UDP statistic
 *
 * @param td
 * @return
 */
static inline 
trudpStatData *trudpStatInit(trudpData *td) {
    return trudpStatReset(td);
}
/**
 * Reset TR-UDP channel statistic
 *
 * @param tcd
 */
static inline 
void trudpStatChannelReset(trudpChannelData *tcd) {
    memset(&tcd->stat, 0, sizeof(tcd->stat));
    tcd->stat.triptime_min = UINT32_MAX;
}
/**
 * Reset TR-UDP channel statistic
 *
 * @param tcd
 */
static inline 
void trudpStatChannelInit(trudpChannelData *tcd) {
    trudpStatChannelReset(tcd);
}
void trudpStatProcessLast10Send(trudpChannelData *tcd, void *packet, size_t send_data_length);
void trudpStatProcessLast10Receive(trudpChannelData *tcd, void *packet);

void *trudpStatGet(trudpData *td, int type, size_t *stat_len);

#ifdef __cplusplus
}
#endif

#endif /* TRU_UDP_STAT_H */
