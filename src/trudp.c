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
 * \file   tr-udp.c
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 31, 2016, 1:44 AM
 */

#include "trudp.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "teobase/types.h"

#include "teoccl/memory.h"
#include "teobase/logging.h"

#include "trudp_channel.h"
#include "trudp_utils.h"

#include "packet.h"
#include "trudp_stat.h"
#include "packet_queue.h"

// Local functions

#ifdef RESERVED
static size_t trudpGetReceiveQueueMax(trudpData *td);
static size_t trudp_SendQueueSize(trudpData *td);
static size_t trudp_SendQueueGetSizeMax(trudpData *td);
#endif

extern int64_t trudpOpt_CORE_keepaliveFirstPingDelay_us;
extern int64_t trudpOpt_CORE_keepaliveNextPingDelay_us;
extern bool trudpOpt_DBG_echoKeepalivePing;

// Basic module functions ====================================================

/**
 * Initialize TR-UDP
 *
 * @param fd File descriptor to read write data
 * @param port Port (optional)
 * @param event_cb Event callback
 * @param user_data User data which will send to most library function
 *
 * @return New instance of trudpData
 */
trudpData *trudpInit(int fd, int port, trudpEventCb event_cb, void *user_data) {

    trudpData* trudp = (trudpData*)ccl_calloc(sizeof(trudpData));

    trudp->map = teoMapNew(MAP_SIZE_DEFAULT, 1);
    trudp->psq_data = NULL;
    trudp->user_data = user_data;
    trudp->port = port;
    trudp->fd = fd;

    // Initialize statistic data
    trudpStatInit(trudp);
    trudp->started = teoGetTimestampFull();

    // Set event callback
    trudp->evendCb = event_cb;

    // Send INITIALIZE event
    trudpSendGlobalEvent(trudp, INITIALIZE, NULL, 0, NULL);

    return trudp;
}

/**
 * Destroy TR-UDP
 *
 * @param td Pointer to trudpData
 */
void trudpDestroy(trudpData* td) {
    if (td != NULL) {
        trudpSendGlobalEvent(td, DESTROY, NULL, 0, NULL);
        teoMapDestroy(td->map);
        free(td);
    }
}

/**
 * Execute trudpEventCb callback
 *
 * @param tcd Pointer to trudpChannelData
 * @param event
 * @param data
 * @param data_length
 * @param reserved - reserved, not used
 */
void trudpSendEvent(trudpChannelData* tcd, int event, void* data,
        size_t data_length, void* reserved) {
    trudpData *td = (trudpData*)tcd->td;

    trudpEventCb cb = td->evendCb;
    if (cb != NULL) {
        cb((void*)tcd, event, data, data_length, td->user_data);
    }
}

/**
 * Execute trudpEventCb callback
 *
 * @param td Pointer to trudpData
 * @param event
 * @param data
 * @param data_length
 * @param reserved - reserved, not used
 */
void trudpSendGlobalEvent(trudpData* td, int event, void* data,
        size_t data_length, void* reserved) {
    trudpEventCb cb = td->evendCb;
    if (cb != NULL) {
        cb((void*)td, event, data, data_length, td->user_data);
    }
}

/**
 * Execute evetrudpEventCbnt callback with event GOT_DATA when data packet received
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
void trudpSendEventGotData(trudpChannelData* tcd, trudpPacket *packet) {
    void *data = trudpPacketGetData(packet);
    size_t data_len = trudpPacketGetDataLength(packet);
    trudpSendEvent(tcd, GOT_DATA, packet, data_len, NULL);
}

/**
 * Destroy all trudp channels
 *
 * @param td Pointer to trudpData
 */
void trudpChannelDestroyAll(trudpData *td) {
    size_t counter = teoMapSize(td->map);
    while (counter != 0) {
        size_t data_len = 0;
        trudpChannelData *tcd = (trudpChannelData *)teoMapGetFirst(td->map, &data_len);
        trudpChannelDestroy(tcd);
        --counter;
    }
}

/**
 * Destroy trudp channel by Address, port and channel number
 *
 * @param td Pointer to trudpData
 * @param addr String with IP address
 * @param port Port number
 * @param channel Channel number 0-15
 */
void trudpChannelDestroyAddr(trudpData *td, char *addr, int port, int channel) {
        size_t key_length;
        char *key = trudpMakeKey(addr, port, channel, &key_length);

        trudpChannelData *tcd = (trudpChannelData *)teoMapGet(td->map, key, key_length, NULL);

        if (tcd != NULL && tcd != (void *)-1) {
            trudpChannelDestroy(tcd);
        }
}

// ===========================================================================


/**
 * Create RESET packet and send it to all channels
 *
 * @param td Pointer to trudpData
 */
void trudpSendResetAll(trudpData *td) {

    teoMapElementData *el;
    teoMapIterator it;

    teoMapIteratorReset(&it, td->map);

    while((el = teoMapIteratorNext(&it))) {
        trudpChannelData *tcd = (trudpChannelData *)
                teoMapIteratorElementData(el, NULL);

        trudpChannelSendRESET(tcd, NULL, 0);
    }
}

/**
 * Check that buffer contains valid trudp packet and packet is ping packet.
 *
 * @param data Buffer with received data
 * @param packet_length The length of received data in the buffer
 *
 * @return true if buffer contains valid trudp ping packet, false otherwise
 */
bool trudpIsPacketPing(uint8_t* data, size_t packet_length) {
    trudpPacket* packet = trudpPacketCheck(data, packet_length);

    if (packet != NULL) {
        int type = trudpPacketGetType(packet);

        if (type == TRU_PING) {
            return true;
        }
    }

    return false;
}

/**
 * Default TR-UDP process received from UDP data
 *
 * @param td Pointer to trudpData
 * @param data Received data
 * @param data_length The length in bytes of received data
 */
void trudpProcessReceived(trudpData* td, uint8_t* data, size_t data_length) {
    struct sockaddr_in remaddr; // remote address

    socklen_t addr_len = sizeof(remaddr);
    ssize_t recvlen = trudpUdpRecvfrom(td->fd, data, data_length,
            (__SOCKADDR_ARG)&remaddr, &addr_len);

    if (trudpIsPacketPing(data, recvlen) && trudpGetChannel(td, (__CONST_SOCKADDR_ARG) &remaddr, 0) == (void *)-1) {
        return;
    }

    // Process received packet
    if(recvlen > 0) {
        trudpChannelData *tcd = trudpGetChannelCreate(td, (__CONST_SOCKADDR_ARG) &remaddr, 0);
        // FIXME: non trudp data it's return value == 0, not -1. Investigate why
        // it works and fix appropriately
        if(tcd == (void *)-1 || trudpChannelProcessReceivedPacket(tcd, data, recvlen) == -1) {
            if(tcd == (void *)-1) {
                printf("!!! can't PROCESS_RECEIVE_NO_TRUDP\n");
            } else {
                trudpSendEvent(tcd, PROCESS_RECEIVE_NO_TRUDP, data, recvlen, NULL);
            }
        }
    }
}

/**
 * Send the same data to all connected peers
 *
 * @param td Pointer to trudpData
 * @param data Pointer to data
 * @param data_length Data length
 *
 * @return Number of peers
 */
size_t trudpSendDataToAll(trudpData *td, void *data, size_t data_length) {

    int rv = 0;

    teoMapIterator it;
    teoMapIteratorReset(&it, td->map);

    teoMapElementData *el;
    while((el = teoMapIteratorNext(&it))) {
        trudpChannelData *tcd = (trudpChannelData *)
                teoMapIteratorElementData(el, NULL);

        if(tcd->connected_f) {
            // drop packets if send queue > 100 \todo move it to Send Data
            // function or something else
            //if(trudpSendQueueSize(tcd->sendQueue) < 100) {
                trudpChannelSendData(tcd, data, data_length);
                rv++;
            //}
        }
    }

    return rv;
}

/**
 * Keep connection at idle line
 *
 * @param td Pointer to trudpData
 * @return
 */
size_t trudpProcessKeepConnection(trudpData *td) {

    int rv = -1;

    teoMapIterator it;
    teoMapElementData *el;
    uint64_t ts = teoGetTimestampFull();
    while(rv == -1) {
        teoMapIteratorReset(&it, td->map);
        rv = 0;
        while((el = teoMapIteratorNext(&it))) {
            trudpChannelData *tcd = (trudpChannelData *)
                teoMapIteratorElementData(el, NULL);

            if (tcd->connected_f) {
                uint32_t sinceReceived = ts - tcd->lastReceived;
                uint32_t sincePing = ts - tcd->lastSentPing;
                if (sinceReceived > trudpOpt_CORE_keepaliveFirstPingDelay_us) {
                    if(trudpChannelCheckDisconnected(tcd, ts) == -1) {
                        rv = -1;
                        break;
                    }
                    if (sincePing > trudpOpt_CORE_keepaliveNextPingDelay_us) {
                        trudpChannelSendPING(tcd, "PING", 5);
                        CLTRACK_I(trudpOpt_DBG_echoKeepalivePing, "Trudp",
                                  "Sent keepalive ping to %s",
                                  tcd->channel_key);
                    }
                    rv++;
                }
            }
        }
    }

    return rv;
}

#ifdef RESERVED
/**
 * Get maximum receive queue size of all channels
 *
 * @param td Pointer to trudpData
 * @return Maximum send queue size of all channels or zero if all queues is empty
 */
static size_t trudpGetReceiveQueueMax(trudpData *td) {

    int rv = 0;

    teoMapIterator it;
    teoMapIteratorReset(&it, td->map);

    teoMapElementData *el;
    while((el = teoMapIteratorNext(&it))) {
        trudpChannelData *tcd = (trudpChannelData *)
                teoMapIteratorElementData(el, NULL);
        int size = trudpReceiveQueueSize(tcd->receiveQueue);
        if(size > rv) rv = size;
    }

    return rv;
}
#endif

/**
 * Get trudpChannelData by address, port and channel number
 *
 * @param td Pointer to trudpData
 * @param addr Pointer to address string
 * @param port Port number
 * @param channel Channel number
 *
 * @return Pointer to trudpChannelData or (void*)-1 if not found
 */
trudpChannelData *trudpGetChannelAddr(trudpData *td, char *addr,
        int port, int channel) {

    size_t data_length, key_length;
    char *key = trudpMakeKey(addr, port, channel, &key_length);
    trudpChannelData *tcd = (trudpChannelData *)teoMapGet(td->map, key,
        key_length, &data_length);

    return tcd;
}

/**
 * Get trudpChannelData by socket address and channel number
 *
 * @param td Pointer to trudpData
 * @param addr Pointer to address structure
 * @param channel Channel number
 *
 * @return Pointer to trudpChannelData or (void*)-1 if not found
 */
trudpChannelData *trudpGetChannel(trudpData *td, __CONST_SOCKADDR_ARG addr,
        int channel) {

    int port;
    char *addr_str = trudpUdpGetAddr(addr, &port);

    return trudpGetChannelAddr(td, addr_str, port, channel);
}
// \TODO: need channel alive function

/**
 * Get trudpChannelData by socket address and channel number, create channel if
 * not exists
 *
 * @param td Pointer to trudpData
 * @param addr Pointer to sockaddr_in remote address
 * @param channel TR-UDP channel
 *
 * @return Pointer to trudpChannelData or (void*)-1 at error
 */
trudpChannelData *trudpGetChannelCreate(trudpData *td, __CONST_SOCKADDR_ARG addr, int channel) {

    int port;
    char *addr_str = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)addr, &port);
    trudpChannelData *tcd = trudpGetChannelAddr(td, addr_str, port, channel);

    if(tcd == (void*)-1) {
        tcd = trudpChannelNew(td, addr_str, port, channel);
    }

    if (tcd != (void*)-1 && !tcd->connected_f) {
        trudpSendEvent(tcd, CONNECTED, NULL, 0, NULL);
        tcd->connected_f = 1; // MUST BE AFTER EVENT!
    }

    return tcd;
}

// Send queue functions =======================================================

/**
 * Get minimum timeout from all trudp cannel send queue
 *
 * @param td Pointer to trudpData
 * @param current_time Timestamp, usually current time
 *
 * @return Minimum timeout or UINT32_MAX if send queue is empty
 */
uint32_t trudpGetSendQueueTimeout(trudpData *td, uint64_t current_time) {

    teoMapIterator it;
    teoMapElementData *el;
    uint32_t min_timeout_sq = UINT32_MAX;

    teoMapIteratorReset(&it, td->map);
    while((el = teoMapIteratorNext(&it))) {
        trudpChannelData *tcd = (trudpChannelData *)teoMapIteratorElementData(el, NULL);
        uint32_t timeout_sq = trudpChannelSendQueueGetTimeout(tcd, current_time);
        if(timeout_sq < min_timeout_sq) min_timeout_sq = timeout_sq;
        if(!min_timeout_sq) break;
    }

    return min_timeout_sq;
}

#ifdef RESERVED
/**
 * Get sum of all send queues size
 *
 * @param td
 * @return
 */
static size_t trudp_SendQueueSize(trudpData *td) {

    uint32_t sz = 0;
    teoMapIterator it;
    teoMapElementData *el;

    teoMapIteratorReset(&it td->map);
    while((el = teoMapIteratorNext(&it))) {
        trudpChannelData *tcd = (trudpChannelData *)teoMapIteratorElementData(el, NULL);
        sz += trudpSendQueueSize(tcd->sendQueue);
    }

    return sz;
}
#endif

/**
 * Check all peers send Queue elements and resend elements with expired time
 *
 * @param td Pointer to trudpData
 * @param next_et [out] Next expected time
 *
 * @return Number of resend packets
 */
int trudpProcessSendQueue(trudpData *td, uint64_t *next_et) {

    int retval, rv = 0;
    uint64_t ts = teoGetTimestampFull(), min_expected_time, next_expected_time;
    teoMapIterator it;
    do {
        retval = 0;
        teoMapElementData *el;
        min_expected_time = UINT64_MAX;

        teoMapIteratorReset(&it, td->map);

        while((el = teoMapIteratorNext(&it))) {
            trudpChannelData *tcd = (trudpChannelData *)teoMapIteratorElementData(el, NULL);
            retval = trudpChannelSendQueueProcess(tcd, ts, &next_expected_time);
            if(retval < 0) break;
            if(retval > 0) rv += retval;
            if(next_expected_time && next_expected_time < min_expected_time)
                min_expected_time = next_expected_time;
        }
    } while(retval == -1 || (retval > 0 && min_expected_time <= ts));

    if(next_et) *next_et = (min_expected_time != UINT64_MAX) ? min_expected_time : 0;

    return rv;
}

#ifdef RESERVED
/**
 * Get maximum send queue size of all channels
 *
 * @param td Pointer to trudpData
 * @return Maximum send queue size of all channels or zero if all queues is empty
 */
static size_t trudp_SendQueueGetSizeMax(trudpData *td) {

    int rv = 0;

    teoMapIterator it;
    teoMapElementData *el;
    teoMapIteratorReset(&it, td->map);

    while((el = teoMapIteratorNext(&it))) {
        trudpChannelData *tcd = (trudpChannelData *)
                teoMapIteratorElementData(el, NULL);
        int size = trudpPacketQueueSize(tcd->sendQueue);
        if(size > rv) rv = size;
    }

    return rv;
}
#endif

// Write queue functions ======================================================

/**
 * Check all peers write Queue elements and write one first packet
 *
 * @param td Pointer to trudpData
 *
 * @return Size of send packets
 */
size_t trudpProcessWriteQueue(trudpData *td) {

    int i = 0;
    size_t retval = 0;
    teoMapElementData *el;
    teoMapIterator it;

    teoMapIteratorReset(&it, td->map);

    while(!retval && (el = teoMapIteratorNext(&it))) {
        if(i++ < td->writeQueueIdx) continue;
        trudpChannelData *tcd = (trudpChannelData *)
                teoMapIteratorElementData(el, NULL);
        retval = trudpChannelWriteQueueProcess(tcd);
        td->writeQueueIdx++;
    }

    if(!retval) td->writeQueueIdx = 0;

    return retval;
}

/**
 * Get number of elements in all Write queues
 *
 * @param td Pointer to trudpData
 * @return Amount of elements in all write queues
 */
size_t trudpGetWriteQueueSize(trudpData *td) {

    size_t retval = 0;
    teoMapElementData *el;
    teoMapIterator it;
    teoMapIteratorReset(&it, td->map);

    while((el = teoMapIteratorNext(&it))) {
        size_t data_lenth;
        trudpChannelData *tcd = (trudpChannelData *)
                teoMapIteratorElementData(el, &data_lenth);
        retval += trudpWriteQueueSize(tcd->writeQueue);
    }

    return retval;
}

/**
 * Get printable for trudpEvent enum
 *
 * @param val Value of trudpEvent enumeration
 * @return String representation of @a val
 */
const char *STRING_trudpEvent(trudpEvent val) {
  switch (val) {
    case INITIALIZE:
      return "INITIALIZE";
    case DESTROY:
      return "DESTROY";
    case CONNECTED:
      return "CONNECTED";
    case DISCONNECTED:
      return "DISCONNECTED";
    case GOT_RESET:
      return "GOT_RESET";
    case SEND_RESET:
      return "SEND_RESET";
    case GOT_ACK_RESET:
      return "GOT_ACK_RESET";
    case GOT_ACK_PING:
      return "GOT_ACK_PING";
    case GOT_PING:
      return "GOT_PING";
    case GOT_ACK:
      return "GOT_ACK";
    case GOT_DATA:
      return "GOT_DATA";
    case PROCESS_RECEIVE:
      return "PROCESS_RECEIVE";
    case PROCESS_RECEIVE_NO_TRUDP:
      return "PROCESS_RECEIVE_NO_TRUDP";
    case PROCESS_SEND:
      return "PROCESS_SEND";
    case GOT_DATA_NO_TRUDP:
      return "GOT_DATA_NO_TRUDP";
  }
  return "INVALID trudpEvent";
}
