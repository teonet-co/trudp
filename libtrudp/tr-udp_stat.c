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

#include "tr-udp_stat.h"

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
