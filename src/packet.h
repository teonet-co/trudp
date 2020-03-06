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
 * File:   packet.h
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 30, 2016, 5:47 PM
 */

#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stddef.h>

#include "trudp_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// TR-UDP Protocol constants
#define TR_UDP_PROTOCOL_VERSION 2
#define MIN_ACK_WAIT 0.000732                  // 000.732 MS
#define MAX_ACK_WAIT 0.500                     // 500 MS
#define MAX_MAX_ACK_WAIT (MAX_ACK_WAIT * 20.0) // 10 sec
#define MAX_ATTEMPT 5 // maximum attempt with MAX_MAX_ACK_WAIT wait value

/**
 * Forward declaration of TR_UDP packet type.
 *
 * This type is never defined and only used as a pointer
 * to memory with correct TR_UDP packet.
 *
 * Please DO NOT cast memory to trudpPacket.
 * Use trudpPacketCheck() to convert received memory to trudpPacket.
 */

typedef struct trudpPacket trudpPacket;

/**
 * TR-UDP message type
 */
typedef enum trudpPacketType {

  TRU_DATA, ///< #0 The DATA messages are carrying payload. (has payload)
  /**
   * #1
   * The ACK messages are used to acknowledge the arrival of the DATA and
   * RESET messages. (has not payload)
   */
  TRU_ACK,
  TRU_RESET,         ///< #2 The RESET messages reset messages counter. (has not
                     ///< payload)
  TRU_ACK_TRU_RESET, ///< #3 = TRU_ACK | TRU_RESET: ACK for RESET. (has not
                     ///< payload)
  TRU_PING, ///< #4 PING The DATA messages can carrying payload, does not sent
            ///< to User level as DATA received. (payload allowed)
  TRU_ACK_PING ///< #5 = TRU_ACK | TRU_PING: ACK for PING (payload allowed)

} trudpPacketType;

TRUDP_API uint32_t trudpGetTimestamp();
TRUDP_API uint32_t trudpPacketGetId(trudpPacket *packet);
TRUDP_API trudpPacketType trudpPacketGetType(trudpPacket *packet);
TRUDP_API size_t trudpPacketGetPacketLength(trudpPacket *packet);

TRUDP_API uint64_t teoGetTimestampFull();
trudpPacket* trudpPacketACKcreateNew(trudpPacket* packet);
size_t trudpPacketACKlength();
trudpPacket* trudpPacketACKtoPINGcreateNew(trudpPacket* packet);
trudpPacket* trudpPacketACKtoRESETcreateNew(trudpPacket* packet);
trudpPacket* trudpPacketCheck(uint8_t* data, size_t packet_length);
void trudpPacketCreatedFree(trudpPacket* packet);
trudpPacket* trudpPacketDATAcreateNew(uint32_t id, unsigned int channel, void *data,
                               size_t data_length, size_t *packetLength);
TRUDP_API void* trudpPacketGetData(trudpPacket *packet);
uint16_t trudpPacketGetDataLength(trudpPacket *packet);
size_t trudpPacketGetHeaderLength(trudpPacket *packet);
uint32_t trudpPacketGetTimestamp(trudpPacket *packet);
void trudpPacketUpdateTimestamp(trudpPacket *packet);

trudpPacket* trudpPacketPINGcreateNew(uint32_t id, unsigned int channel, void *data,
                               size_t data_length, size_t *packetLength);
trudpPacket* trudpPacketRESETcreateNew(uint32_t id, unsigned int channel);
size_t trudpPacketRESETlength();
TRUDP_API void trudpPacketHeaderDump(char *buffer, size_t buffer_len, trudpPacket *packet);

const char *STRING_trudpPacketType(trudpPacketType value);

#ifdef __cplusplus
}
#endif

#endif /* PACKET_H */
