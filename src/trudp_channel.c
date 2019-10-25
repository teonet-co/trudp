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
 */

// Channel functions ==========================================================

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "teobase/logging.h"

#include "trudp_channel.h"
#include "trudp_stat.h"
#include "trudp_utils.h"

#include "packet.h"
#include "trudp_receive_queue.h"
#include "trudp_send_queue.h"

// Local function
static trudpChannelData *_trudpChannelAddToMap(void *td, char *key,
                                               size_t key_length,
                                               trudpChannelData *tcd);
static uint64_t _trudpChannelCalculateExpectedTime(trudpChannelData *tcd,
                                                   uint64_t current_time,
                                                   int retransmit);
static void _trudpChannelCalculateTriptime(trudpChannelData *tcd, void *packet,
                                           size_t send_data_length);
static void _trudpChannelFree(trudpChannelData *tcd);
static uint32_t _trudpChannelGetId(trudpChannelData *tcd);
static uint32_t _trudpChannelGetNewId(trudpChannelData *tcd);
static void _trudpChannelIncrementStatSendQueueSize(trudpChannelData *tcd);
static void _trudpChannelIncrementStatWriteQueueSize(trudpChannelData *tcd);
static void _trudpChannelReset(trudpChannelData *tcd);
static void _trudpChannelSendACK(trudpChannelData *tcd, void *packet);
static void _trudpChannelSendACKtoPING(trudpChannelData *tcd, void *packet);
static void _trudpChannelSendACKtoRESET(trudpChannelData *tcd, void *packet);
static size_t _trudpChannelSendPacket(trudpChannelData *tcd, void *packetDATA,
                                      size_t packetLength,
                                      int save_to_send_queue);
static void _trudpChannelSetDefaults(trudpChannelData *tcd);
static void _trudpChannelSetLastReceived(trudpChannelData *tcd);

void trudp_ChannelSendReset(trudpChannelData *tcd) {
  trudpChannelSendRESET(tcd, NULL, 0);
}

/**
 * Add channel to the trudpData map
 *
 * @param td
 * @param key
 * @param key_length
 * @param tcd
 * @return
 */
static trudpChannelData *_trudpChannelAddToMap(void *td, char *key,
                                               size_t key_length,
                                               trudpChannelData *tcd) {

  return teoMapAdd(((trudpData *)td)->map, key, key_length, tcd,
                   sizeof(trudpChannelData));
}

/**
 * Get channel send queue timeout
 *
 * @param tcd Pointer to trudpChannelData
 * @param ts Current time
 *
 * @return Send queue timeout (may by 0) or UINT32_MAX if send queue is empty
 */
uint32_t trudpChannelSendQueueGetTimeout(trudpChannelData *tcd,
                                         uint64_t current_t) {

  return trudpSendQueueGetTimeout(tcd->sendQueue, current_t);
}

/**
 * Set default trudpChannelData values
 *
 * @param tcd
 */
static void _trudpChannelSetDefaults(trudpChannelData *tcd) {

  tcd->sendId = 0;
  tcd->triptime = 0;
  tcd->triptimeFactor = 1.5;
  tcd->outrunning_cnt = 0;
  tcd->receiveExpectedId = 0;
  tcd->lastReceived = teoGetTimestampFull();
  tcd->lastSentPing = 0;  //  Never sent ping before
  tcd->triptimeMiddle = START_MIDDLE_TIME;

  tcd->read_buffer = NULL;
  tcd->read_buffer_ptr = 0;
  tcd->read_buffer_size = 0;
  tcd->last_packet_ptr = 0;

  // Initialize statistic
  trudpStatChannelInit(tcd);
}

/**
 * Free trudp channel
 *
 * @param tcd Pointer to trudpChannelData
 */
static void _trudpChannelFree(trudpChannelData *tcd) {

  TD(tcd)->stat.sendQueue.size_current -= trudpSendQueueSize(tcd->sendQueue);
  TD(tcd)->stat.writeQueue.size_current -= trudpWriteQueueSize(tcd->writeQueue);

  if (tcd->read_buffer) {
    free(tcd->read_buffer);
  }

  trudpSendQueueFree(tcd->sendQueue);
  trudpWriteQueueFree(tcd->writeQueue);
  trudpReceiveQueueFree(tcd->receiveQueue);
  _trudpChannelSetDefaults(tcd);
}

// ============================================================================

/**
 * Create trudp channel
 *
 * @param td Pointer to trudpData
 * @param remote_address
 * @param remote_port_i
 * @param channel
 * @return
 */
trudpChannelData *trudpChannelNew(void *parent, char *remote_address,
                                  int remote_port_i, int channel) {

  trudpChannelData *tcd = (trudpChannelData *)malloc(sizeof(trudpChannelData));
  memset(tcd, 0, sizeof(trudpChannelData));

  tcd->td = parent;
  tcd->sendQueue = trudpSendQueueNew();
  tcd->writeQueue = trudpWriteQueueNew();
  tcd->receiveQueue = trudpReceiveQueueNew();
  tcd->addrlen = sizeof(tcd->remaddr);
  trudpUdpMakeAddr(remote_address, remote_port_i, (__SOCKADDR_ARG)&tcd->remaddr,
                   &tcd->addrlen);
  tcd->channel = channel;

  // Set other defaults
  _trudpChannelSetDefaults(tcd);
  tcd->fd = 0;

  // Add cannel to map
  size_t key_length;
  char *key =
      trudpMakeKey(trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, NULL),
                   remote_port_i, channel, &key_length);
  trudpChannelData *tcd_return =
      _trudpChannelAddToMap(parent, key, key_length, tcd);
  free(tcd);

  return tcd_return;
}

/**
 * Reset trudp channel
 *
 * @param tcd Pointer to trudpChannelData
 */
static void _trudpChannelReset(trudpChannelData *tcd) {
  _trudpChannelFree(tcd);
}

/**
 * Destroy trudp channel
 *
 * @param tcd Pointer to trudpChannelData
 */
void trudpChannelDestroy(trudpChannelData *tcd) {
  //    log_info("TrUdp", "trudpSendEvent DISCONNECTED in trudpChannelDestroy");

  trudpSendEvent(tcd, DISCONNECTED, NULL, 0, NULL);
  _trudpChannelFree(tcd);
  tcd->fd = 0;
  trudpSendQueueDestroy(tcd->sendQueue);
  trudpWriteQueueDestroy(tcd->writeQueue);
  trudpReceiveQueueDestroy(tcd->receiveQueue);

  int port;
  size_t key_length;
  char *addr = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, &port);
  char *key = trudpMakeKey(addr, port, tcd->channel, &key_length);
  teoMapDelete(TD(tcd)->map, key, key_length);
}

// ============================================================================

/**
 * Return next packet id in range 1..(PACKET_ID_LIMIT-1)
 * intentionally avoiding value 0
 *
 * @param packetId value to increment
 */
static  uint32_t _trudpGetNextSeqId(uint32_t packetId) {
    // due to packetId == 0 threaten in special way as initialization one
    // we avoid packetId == 0 in sequential increments
    // i.e. after PACKET_ID_LIMIT-1 will go 1
    packetId = modAddU(packetId, 1, PACKET_ID_LIMIT);
    if (packetId != 0) {
        return packetId;
    }
    return modAddU(packetId, 1, PACKET_ID_LIMIT);
}

/**
 * Return sequential distance between fromId and toId
 * Returns zero if fromId == toId otherwise it returns signed distance (toId - fromId)
 * with respect wrapping around at (PACKET_ID_LIMIT-1) and considering all
 * distances between 0..PACKET_ID_LIMIT/2 as positive and all distances beyond as negative
 * It behaves similar to how signed integer overflow works, but for arbitrary PACKET_ID_LIMIT
 *
 * @param fromId first sequential packet id
 * @param toId second sequential packet id
 */
static int32_t _trudpGetSeqIdDistance(uint32_t fromId, uint32_t toId) {
    uint32_t diff = modSubU(toId, fromId, PACKET_ID_LIMIT);
    if (diff < (PACKET_ID_LIMIT/2)) {
        return diff;
    }
    return diff - PACKET_ID_LIMIT;
}

/**
 * Get new channel send Id
 *
 * @param tcd Pointer to trudpChannelData
 * @return New send Id
 */
static  uint32_t _trudpChannelGetNewId(trudpChannelData *tcd) {
    uint32_t retval = tcd->sendId;
    tcd->sendId = _trudpGetNextSeqId(tcd->sendId);
    return retval;
}

/**
 * Get current channel send Id
 *
 * @param tcd Pointer to trudpChannelData
 * @return Send Id
 */
static uint32_t _trudpChannelGetId(trudpChannelData *tcd) {

  return tcd->sendId;
}

/**
 * Make key from channel data
 *
 * @param tcd Pointer to trudpChannelData
 *
 * @return Static buffer with key ip:port:channel
 */
char *trudpChannelMakeKey(trudpChannelData *tcd) {

  int port;
  size_t key_length;
  char *addr = trudpUdpGetAddr((__CONST_SOCKADDR_ARG)&tcd->remaddr, &port);
  return trudpMakeKey(addr, port, tcd->channel, &key_length);
}

/**
 * Check that channel is disconnected and send DISCONNECTED event
 *
 * @param tcd Pointer to trudpChannelData
 * @param ts Current timestamp
 * @return
 */
int trudpChannelCheckDisconnected(trudpChannelData *tcd, uint64_t ts) {

  // Disconnect channel at long last receive
  if (tcd->lastReceived && ts - tcd->lastReceived > MAX_LAST_RECEIVE) {

    //        log_info("TrUdp", "trudpSendEvent DISCONNECTED in
    //        trudpChannelCheckDisconnected");

    // Send disconnect event
    uint32_t lastReceived = ts - tcd->lastReceived;
    trudpSendEvent(tcd, DISCONNECTED, &lastReceived, sizeof(lastReceived),
                   NULL);
    return -1;
  }
  return 0;
}

// Process received packet ====================================================

/**
 * Calculate Triptime
 *
 * @param tcd
 * @param packet
 * @param send_data_length
 */
static void _trudpChannelCalculateTriptime(trudpChannelData *tcd, void *packet,
                                           size_t send_data_length) {

  tcd->triptime = trudpGetTimestamp() - trudpPacketGetTimestamp(packet);

  // Calculate and set Middle Triptime value
  tcd->triptimeMiddle = tcd->triptimeMiddle == START_MIDDLE_TIME
                            ? tcd->triptime * tcd->triptimeFactor
                            : // Set first middle time
                            tcd->triptime > tcd->triptimeMiddle
                                ? tcd->triptime * tcd->triptimeFactor
                                : // Set middle time to max triptime
                                (tcd->triptimeMiddle * 19 + tcd->triptime) /
                                    20.0; // Calculate middle value

  // Correct triptimeMiddle
  if (tcd->triptimeMiddle < tcd->triptime * tcd->triptimeFactor)
    tcd->triptimeMiddle = tcd->triptime * tcd->triptimeFactor;
  if (tcd->triptimeMiddle > tcd->triptime * 10)
    tcd->triptimeMiddle = tcd->triptime * 10;
  if (tcd->triptimeMiddle > MAX_TRIPTIME_MIDDLE)
    tcd->triptimeMiddle = MAX_TRIPTIME_MIDDLE;

  // tcd->triptimeMiddle *= 5;

  // Statistic
  tcd->stat.ack_receive++;
  tcd->stat.triptime_last = tcd->triptime;
  tcd->stat.wait = tcd->triptimeMiddle / 1000.0;
  trudpStatProcessLast10Send(tcd, packet, send_data_length);
}

/**
 * Set last received field to current timestamp
 *
 * @param tcd
 */
static void _trudpChannelSetLastReceived(trudpChannelData *tcd) {
  tcd->lastReceived = teoGetTimestampFull();
}

/**
 * Create ACK packet and send it back to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
static void _trudpChannelSendACK(trudpChannelData *tcd, void *packet) {

  void *packetACK = trudpPacketACKcreateNew(packet);
#if !USE_WRITE_QUEUE
  trudpSendEvent(tcd, PROCESS_SEND, packetACK, trudpPacketACKlength(), NULL);
#else
  trudpWriteQueueAdd(tcd->writeQueue, packetACK, NULL, trudpPacketACKlength());
#endif
  trudpPacketCreatedFree(packetACK);
  _trudpChannelSetLastReceived(tcd);
}

/**
 * Create ACK to RESET packet and send it back to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
static void _trudpChannelSendACKtoRESET(trudpChannelData *tcd, void *packet) {

  void *packetACK = trudpPacketACKtoRESETcreateNew(packet);
#if !USE_WRITE_QUEUE
  trudpSendEvent(tcd, PROCESS_SEND, packetACK, trudpPacketACKlength(), NULL);
#else
  trudpWriteQueueAdd(tcd->writeQueue, packetACK, NULL, trudpPacketACKlength());
#endif
  trudpPacketCreatedFree(packetACK);
  _trudpChannelSetLastReceived(tcd);
}

/**
 * Create ACK to PING packet and send it back to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
static void _trudpChannelSendACKtoPING(trudpChannelData *tcd, void *packet) {

  void *packetACK = trudpPacketACKtoPINGcreateNew(packet);
#if !USE_WRITE_QUEUE
  trudpSendEvent(tcd, PROCESS_SEND, packetACK,
                 trudpPacketGetPacketLength(packet), NULL);
#else
  trudpWriteQueueAdd(tcd->writeQueue, NULL, packetACK,
                     trudpPacketGetPacketLength);
#endif
  trudpPacketCreatedFree(packetACK);
  _trudpChannelSetLastReceived(tcd);
}

/**
 * Create RESET packet and send it to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param data NULL
 * @param data_length 0
 */
void trudpChannelSendRESET(trudpChannelData *tcd, void *data,
                           size_t data_length) {

  if (tcd) {
    trudpSendEvent(tcd, SEND_RESET, data, data_length, NULL);

    void *packetRESET =
        trudpPacketRESETcreateNew(_trudpChannelGetNewId(tcd), tcd->channel);
#if !USE_WRITE_QUEUE
    trudpSendEvent(tcd, PROCESS_SEND, packetRESET, trudpPacketRESETlength(),
                   NULL);
#else
    trudpWriteQueueAdd(tcd->writeQueue, packetRESET, NULL,
                       trudpPacketRESETlength());
#endif
    trudpPacketCreatedFree(packetRESET);
  }
}

/**
 * Calculate ACK Expected Time
 *
 * @param td Pointer to trudpChannelData
 * @param current_time Current time (nSec)
 * @param retransmit This is retransmitted
 *
 * @return Current time plus
 */
static uint64_t _trudpChannelCalculateExpectedTime(trudpChannelData *tcd,
                                                   uint64_t current_time,
                                                   int retransmit) {

  // int rtt = tcd->triptimeMiddle + RTT * (retransmit);
  // int rtt = tcd->triptimeMiddle + (RTT/10);// * (retransmit?0.5:0);
  int rtt = tcd->triptimeMiddle + RTT;
  if (rtt > MAX_RTT)
    rtt = MAX_RTT;
  uint64_t expected_time = current_time + rtt;

  return expected_time;
}

/**
 * Increment statistics send queue size value
 *
 * @param tcd Pointer to trudpChannelData
 */
static void _trudpChannelIncrementStatSendQueueSize(trudpChannelData *tcd) {
  TD(tcd)->stat.sendQueue.size_current++;
}

static void _trudpChannelIncrementStatWriteQueueSize(trudpChannelData *tcd) {
  TD(tcd)->stat.writeQueue.size_current++;
}

/**
 * Send packet
 *
 * @param td Pointer to trudpChannelData
 * @param data Pointer to send data
 * @param data_length Data length
 * @param save_to_send_queue Save to send queue if true
 *
 * @return Zero on error
 */
static size_t _trudpChannelSendPacket(trudpChannelData *tcd, void *packetDATA,
                                      size_t packetLength,
                                      int save_to_send_queue) {

  size_t size_sq = trudpSendQueueSize(tcd->sendQueue);

  // Save packet to send queue
  if (save_to_send_queue) {
    if (size_sq < NORMAL_S_SIZE) {
      trudpSendQueueAdd(
          tcd->sendQueue, packetDATA, packetLength,
          _trudpChannelCalculateExpectedTime(tcd, teoGetTimestampFull(), 0));
      _trudpChannelIncrementStatSendQueueSize(tcd);
    } else {
      void *packetDATAptr = malloc(packetLength);
      memcpy(packetDATAptr, packetDATA, packetLength);
      trudpWriteQueueAdd(tcd->writeQueue, NULL, packetDATAptr, packetLength);
      _trudpChannelIncrementStatWriteQueueSize(tcd);
    }
  }

  // Send data (add to write queue)
  if (!save_to_send_queue || size_sq < NORMAL_S_SIZE) {
    trudpSendEvent(tcd, PROCESS_SEND, packetDATA, packetLength,
                   NULL);     // Send packet in trudp event loop
    tcd->stat.packets_send++; // Send packets statistic
  }
  //    else if(save_to_send_queue) trudp_start_send_queue_cb(TD(tcd)->psq_data,
  //    0);

  return packetLength;
}

/**
 * Send PING packet and send it back to sender
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 */
size_t trudpChannelSendPING(trudpChannelData *tcd, void *data,
                            size_t data_length) {

  // Create DATA package
  size_t packetLength;
  void *packetDATA = trudpPacketPINGcreateNew(
      _trudpChannelGetId(tcd), tcd->channel, data, data_length, &packetLength);

  // Send data
  size_t rv = _trudpChannelSendPacket(tcd, packetDATA, packetLength, 0);

  // Free created packet
  trudpPacketCreatedFree(packetDATA);

  tcd->lastSentPing = teoGetTimestampFull();
  return rv;
}

/**
 * Send data
 *
 * @param tcd Pointer to trudpChannelData
 * @param data Pointer to send data
 * @param data_length Data length
 *
 * @return Zero on error
 */
size_t trudpChannelSendData(trudpChannelData *tcd, void *data,
                            size_t data_length) {

  size_t rv = 0;

  //    if( trudpSendQueueSize(tcd->sendQueue) <= 50 ||
  //         ( tcd->sendId % 100 != 100 - trudpSendQueueSize(tcd->sendQueue) ) )
  //         {

  // Create DATA package
  size_t packetLength;
  void *packetDATA =
      trudpPacketDATAcreateNew(_trudpChannelGetNewId(tcd), tcd->channel, data,
                               data_length, &packetLength);

  // Send data
  rv = _trudpChannelSendPacket(tcd, packetDATA, packetLength, 1);

  // Free created packet
  trudpPacketCreatedFree(packetDATA);

  //    }

  return rv;
}

/**
 * Process received packet
 *
 * @param tcd Pointer to trudpChannelData
 * @param packet Pointer to received packet
 * @param packet_length Packet length
 * @param data_length Pointer to variable to return packets data length
 *
 * @return Pointer to received data, NULL available, length of data saved to
 *         *data_length, if packet is not TR-UDP packet it's sent to client as
 * is
 */
void *trudpChannelProcessReceivedPacket(trudpChannelData *tcd, void *packet,
                                        size_t packet_length,
                                        size_t *data_length) {

  void *data = NULL;
  *data_length = 0;

  // Check and process TR-UDP packet
  if (trudpPacketCheck(packet, packet_length)) {

    // Check packet type
    int type = trudpPacketGetType(packet);
    switch (type) {

    // ACK to DATA packet received
    case TRU_ACK: {

      // Find packet in send queue by id
      size_t send_data_length = 0;
      trudpSendQueueData *sqd =
          trudpSendQueueFindById(tcd->sendQueue, trudpPacketGetId(packet));
      if (sqd) {

        // Process ACK data callback
        trudpSendEvent(tcd, GOT_ACK, sqd->packet, sqd->packet_length, NULL);

        // Remove packet from send queue
        send_data_length = trudpPacketGetDataLength(sqd->packet);
        trudpSendQueueDelete(tcd->sendQueue, sqd);
        TD(tcd)->stat.sendQueue.size_current--;

        if (trudpWriteQueueSize(tcd->writeQueue) > 0) {
          trudpWriteQueueData *wqd_first =
              trudpWriteQueueGetFirst(tcd->writeQueue);
          _trudpChannelSendPacket(tcd, wqd_first->packet_ptr,
                                  wqd_first->packet_length, 1);
          free(wqd_first->packet_ptr);
          trudpWriteQueueDeleteFirst(tcd->writeQueue);
          TD(tcd)->stat.writeQueue.size_current--;
        }
      }

      // Calculate triptime
      _trudpChannelCalculateTriptime(tcd, packet, send_data_length);
      _trudpChannelSetLastReceived(tcd);
    } break;

    // ACK to RESET packet received
    case TRU_ACK | TRU_RESET: {

      // Send event
      trudpSendEvent(tcd, GOT_ACK_RESET, NULL, 0, NULL);

      // Reset TR-UDP
      _trudpChannelReset(tcd);

      // Statistic
      tcd->stat.ack_receive++;
      _trudpChannelSetLastReceived(tcd);

    } break;

    // ACK to PING packet received
    case TRU_ACK | TRU_PING: /*TRU_ACK_PING:*/ {

      // Calculate Triptime
      _trudpChannelCalculateTriptime(tcd, packet, packet_length);
      _trudpChannelSetLastReceived(tcd);

      // Send event
      trudpSendEvent(tcd, GOT_ACK_PING, trudpPacketGetData(packet),
                     trudpPacketGetDataLength(packet), NULL);

    } break;

    // PING packet received
    case TRU_PING: {

      // Send event
      trudpSendEvent(tcd, GOT_PING, trudpPacketGetData(packet),
                     trudpPacketGetDataLength(packet), NULL);

      // Calculate Triptime
      _trudpChannelCalculateTriptime(tcd, packet, packet_length);

      // Create ACK packet and send it back to sender
      _trudpChannelSendACKtoPING(tcd, packet);

      // Statistic
      tcd->stat.packets_receive++;
      trudpStatProcessLast10Receive(tcd, packet);

      tcd->outrunning_cnt = 0; // Reset outrunning flag

    } break;

    // DATA packet received
    case TRU_DATA: {

      // Create ACK packet and send it back to sender
      _trudpChannelSendACK(tcd, packet);

      // Reset when wait packet with id 0 and receive packet with
      // another id
      if (!tcd->receiveExpectedId && trudpPacketGetId(packet)) {

        // Send event
        uint32_t id = trudpPacketGetId(packet);
        trudpSendEvent(tcd, SEND_RESET, NULL, 0, NULL);
        trudpChannelSendRESET(tcd, &id, sizeof(id));
        break;
      }

      // Check expected Id and return data
      if (trudpPacketGetId(packet) == tcd->receiveExpectedId) {

        // Send Got Data event
        data = trudpSendEventGotData(tcd, packet, data_length);

        // Proceed to next expected id
        tcd->receiveExpectedId = _trudpGetNextSeqId(tcd->receiveExpectedId);
        // Check received queue for saved packet with expected id
        trudpReceiveQueueData *rqd;
        while ((rqd = trudpReceiveQueueFindById(tcd->receiveQueue,
                                                tcd->receiveExpectedId))) {

          // Send Got Data event
          data = trudpSendEventGotData(tcd, rqd->packet, data_length);

          // Delete element from received queue
          trudpReceiveQueueDelete(tcd->receiveQueue, rqd);
          // Proceed to next expected id
          tcd->receiveExpectedId = _trudpGetNextSeqId(tcd->receiveExpectedId);
        }

        // Statistic
        tcd->stat.packets_receive++;
        trudpStatProcessLast10Receive(tcd, packet);

        tcd->outrunning_cnt = 0; // Reset outrunning flag
        break;
      }

      // Save outrunning packet to receiveQueue
      if (_trudpGetSeqIdDistance(tcd->receiveExpectedId, trudpPacketGetId(packet)) > 0 &&
                !trudpReceiveQueueFindById(tcd->receiveQueue,
                                          trudpPacketGetId(packet))) {

        trudpReceiveQueueAdd(tcd->receiveQueue, packet, packet_length, 0);
        tcd->outrunning_cnt++; // Increment outrunning count

        // Statistic
        tcd->stat.packets_receive++;
        trudpStatProcessLast10Receive(tcd, packet);
        break;
      }

      // Reset channel if packet id = 0
      if (!trudpPacketGetId(packet)) {

        // Send Send Reset event
        trudpChannelSendRESET(tcd, NULL, 0);
        break;
      }

      // Skip already processed packet
      // Statistic
      tcd->stat.packets_receive_dropped++;
      trudpStatProcessLast10Receive(tcd, packet);
    } break;

    // RESET packet received
    case TRU_RESET: {

      //                log_info("TrUdp", "trudpSendEvent GOT_RESET in
      //                trudpChannelProcessReceivedPacket");

      // Send Got Reset event
      trudpSendEvent(tcd, GOT_RESET, NULL, 0, NULL);

      // Create ACK to RESET packet and send it back to sender
      _trudpChannelSendACKtoRESET(tcd, packet);

      // Reset TR-UDP
      _trudpChannelReset(tcd);

    } break;

    // An undefined type of packet (skip it)
    default: {
      // Return error code
      data = (void *)-1;

    } break;
    }
  }
  // Packet is not TR-UDP packet
  else {
    trudpSendEvent(tcd, GOT_DATA, packet, packet_length, NULL);
    data = packet;
  }

  return data;
}

// Send queue functions ======================================================

/**
 * Check send Queue elements and resend elements with expired time
 *
 * @param tcd Pointer to trudpChannelData
 * @param ts Current timestamp
 * @param next_expected_time [out] Next expected time: return expected time
 *          of next queue record or zero if not found, it may be NULL than not
 *          returned
 *
 * @return Number of resend packets or -1 if the channel was disconnected
 */
int trudpChannelSendQueueProcess(trudpChannelData *tcd, uint64_t ts,
                                 uint64_t *next_expected_time) {

  int rv = 0;
  trudpSendQueueData *tqd = NULL;

  // Get first element from send queue and check it expected time
  if (trudpSendQueueSize(tcd->sendQueue) &&
      (tqd = trudpSendQueueGetFirst(tcd->sendQueue)) &&
      tqd->expected_time <= ts) {

    // Change records expected time
    tqd->expected_time =
        _trudpChannelCalculateExpectedTime(tcd, ts, tqd->retrieves);

    // Move record to the end of Queue \todo or don't move record to the end of
    // queue because it should be send first
    // trudpPacketQueueMoveToEnd(tcd->sendQueue, tqd);
    tcd->stat.packets_attempt++; // Attempt statistic parameter increment
    if (!tqd->retrieves)
      tqd->retrieves_start = ts;

    tqd->retrieves++;
    rv++;

// Resend data
#if !USE_WRITE_QUEUE
    trudpPacketUpdateTimestamp(tqd->packet);
    trudpSendEvent(tcd, PROCESS_SEND, tqd->packet, tqd->packet_length, NULL);
#else
    trudpWriteQueueAdd(tcd->writeQueue, NULL, tqd->packet, tqd->packet_length);
#endif

    // Disconnect at max retrieves
    //        goto skip_disconnect_on_max_retrieves;
    //        if(tqd->retrieves > MAX_RETRIEVES ||
    //           ts - tqd->retrieves_start > MAX_TRIPTIME_MIDDLE ) {
    //
    //            char *key = trudpMakeKeyCannel(tcd);
    //            // \todo Send event
    //            fprintf(stderr, "Disconnect channel %s, wait: %.6f\n", key,
    //                (ts - tqd->retrieves_start) / 1000000.0);
    //            trudpExecEventCallback(tcd, DISCONNECTED, key, strlen(key) +
    //            1,
    //                TD(tcd)->user_data, TD(tcd)->evendCb);
    //
    //            trudpDestroyChannel(tcd);
    //
    //            //exit(-1);
    //            return -1;
    //        }
    //        skip_disconnect_on_max_retrieves: ;
  }

  // Disconnect channel at long last receive
  if (trudpChannelCheckDisconnected(tcd, ts) == -1) {

    tqd = NULL;
    rv = -1;
  } else {

// \todo Reset this channel at long retransmit
#if RESET_AT_LONG_RETRANSMIT
    if (ts - tqd->retrieves_start > MAX_LAST_RECEIVE) {
      trudpChannelSendRESET(tcd, NULL, 0);
    } else {
#endif
// Get next value \todo if don't move than not need to re-get it
// tqd = trudpPacketQueueGetFirst(tcd->sendQueue);
#if RESET_AT_LONG_RETRANSMIT
    }
#endif
  }

  // If record exists
  if (next_expected_time) {
    if (rv != -1)
      tqd = trudpSendQueueGetFirst(tcd->sendQueue);
    *next_expected_time = tqd ? tqd->expected_time : 0;
  }

  return rv;
}

// Write queue functions ======================================================

/**
 * Process write queue and send first ready packet
 *
 * @param tcd
 *
 * @return Number of send packet or 0 if write queue is empty
 */
size_t trudpChannelWriteQueueProcess(trudpChannelData *tcd) {

  size_t retval = 0;
  trudpWriteQueueData *wqd = trudpWriteQueueGetFirst(tcd->writeQueue);
  if (wqd) {
    void *packet = wqd->packet_ptr ? wqd->packet_ptr : wqd->packet;
    trudpSendEvent(tcd, PROCESS_SEND, packet, wqd->packet_length, NULL);
    trudpWriteQueueDeleteFirst(tcd->writeQueue);
    retval = wqd->packet_length;
  }

  return retval;
}
