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

/*
 * \file   trudp_channel.h
 * \author Kirill Scherba <kirill@scherba.ru>
 *
 * Created on July 20, 2016, 6:21 PM
 */

#ifndef TRUDP_CHANNEL_H
#define TRUDP_CHANNEL_H

#include <stdint.h>
#include <stddef.h>

#if defined(HAVE_MINGW) || defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
// TODO: Stop using deprecated functions and remove this define.
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
typedef int socklen_t;

# define __SOCKADDR_ARG		struct sockaddr *__restrict
# define __CONST_SOCKADDR_ARG	const struct sockaddr *

#ifndef _SSIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    ssize_t;
#else
typedef _W64 unsigned int   ssize_t;
#endif
#define _SSIZE_T_DEFINED
#endif

#else
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#include "trudp_api.h"
#include "trudp_const.h"
#include "trudp_send_queue.h"
#include "trudp_receive_queue.h"

#include "packet_queue.h"
#include "write_queue.h"

/**
 * Last 10 send statistic data
 */
typedef struct trudpLast10_data {

    uint32_t triptime; ///< Packet triptime
    uint32_t size_b; ///< Size of backet in bites
    uint32_t ts; ///< Packet time

} trudpLast10_data;

/**
 * TR-UDP channel statistic data
 */
typedef struct trudpStatChannelData {

    #define LAST10_SIZE 10
    char key[MAX_KEY_LENGTH]; ///< Channel key
    uint64_t started; ///< Channel created time
    uint32_t triptime_last; ///< Last trip time
    uint32_t triptime_max; ///< Max trip time
    uint32_t triptime_last_max; ///< Max trip time in last 10 packets
    uint32_t triptime_min; ///< Min trip time
    uint32_t triptime_avg; ///< Avr trip time
    uint32_t packets_send; ///< Number of data or reset packets sent
    uint32_t packets_attempt; ///< Number of attempt packets
    uint32_t packets_receive; ///< Number of data or reset packets receive
    uint32_t packets_receive_dropped; ///< Number of dropped received package
    uint32_t ack_receive; ///< Number of ACK packets received
    uint32_t receive_speed; ///< Receive speed in bytes per second
    double receive_total; ///< Receive total in megabytes
    uint32_t send_speed; ///< Send speed in bytes per second
    double send_total; ///< Send total in megabytes
    double wait; ///< Send repeat timer wait time value
    uint32_t sq; ///< Send queue length
    uint32_t rq; ///< Receive queue length
    trudpLast10_data last_send_packets_ar[LAST10_SIZE]; ///< Last 10 send packets
    size_t idx_snd; ///< Index of last_send_packet_ar
    trudpLast10_data last_receive_packets_ar[LAST10_SIZE]; ///< Last 10 receive packets
    size_t idx_rcv; ///< Index of last_receive_packets_ar
    uint32_t sendQueueSize;
    uint32_t receiveQueueSize;
    uint32_t writeQueueSize;

} trudpStatChannelData;

/**
 * TR-UDP channel Data Structure
 */
typedef struct trudpChannelData {

    uint32_t sendId; ///< Send ID
    trudpSendQueue *sendQueue; ///< Pointer to send queue trudpSendQueue
    uint32_t triptime; ///< Trip time
    double triptimeFactor; ///< Triptime factor
    uint32_t triptimeMiddle; ///< Trip time middle
//    uint32_t lastSend; ///< Last send time

    trudpWriteQueue *writeQueue; ///< Pointer to write queue trudpWriteQueue

    uint32_t receiveExpectedId; ///< Ecpected recive Id
    trudpReceiveQueue *receiveQueue; ///< Pointer to recive queue trudpReceiveQueue
    int outrunning_cnt; ///< Receive queue outrunning count
    uint64_t lastReceived; ///< Last received time

    // Link to parent trudpData
    void *td; ///< Pointer to trudpData

    // UDP connection depended variables
    struct sockaddr_in remaddr; ///< Remote address
    socklen_t addrlen;          ///< Remote address length
    int connected_f;            ///< Connected (remote address valid)
    int channel;                ///< TR-UDP channel

    trudpStatChannelData stat;  ///< Channel statistic

    int fd;                     ///< L0 client fd (emulation)


    // Buffer for large packet from client
    void *read_buffer;
    size_t read_buffer_ptr;
    size_t read_buffer_size;
    size_t last_packet_ptr;

} trudpChannelData;

#ifdef __cplusplus
extern "C" {
#endif

TRUDP_API void trudpChannelDestroy(trudpChannelData *tcd);
TRUDP_API char *trudpChannelMakeKey(trudpChannelData *tcd);
TRUDP_API trudpChannelData *trudpChannelNew(void *td, char *remote_address,
        int remote_port_i, int channel);
TRUDP_API size_t trudpChannelSendData(trudpChannelData *tcd, void *data,
  size_t data_length);
TRUDP_API void trudpChannelSendRESET(trudpChannelData *tcd, void* data, size_t data_length);
/**
 * Create RESET packet and send it to sender
 *
 * @param tcd Pointer to trudpChannelData
 */
// TRUDP_API
static
void trudp_ChannelSendReset(trudpChannelData *tcd) {
    trudpChannelSendRESET(tcd, NULL, 0);
}

TRUDP_API void *trudpChannelProcessReceivedPacket(trudpChannelData *tcd, void *packet,
        size_t packet_length, size_t *data_length);
size_t trudpChannelSendPING(trudpChannelData *tcd, void *data, size_t data_length);
uint32_t trudpChannelSendQueueGetTimeout(trudpChannelData *tcd,
        uint64_t current_t);
int trudpChannelSendQueueProcess(trudpChannelData *tcd, uint64_t ts,
        uint64_t *next_expected_time);
int trudpChannelCheckDisconnected(trudpChannelData *tcd, uint64_t ts);
size_t trudpChannelWriteQueueProcess(trudpChannelData *tcd);

#ifdef __cplusplus
}
#endif

#endif /* TRUDP_CHANNEL_H */

