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

/**
 * TR-UDP Header and Packet functions module
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "packet.h"

#include "teobase/time.h"

#pragma pack(push)
#pragma pack(1)

/**
 * TR-UDP message header structure
 */

typedef struct trudpHeader {

  uint8_t checksum;    ///< Checksum
  uint8_t version : 4; ///< Protocol version number
  /**
   * Message type could be of type:
   * DATA(0x0), ACK(0x1), RESET(0x2), ACK_RESET(0x3), PING(0x4), ACK_PING(0x5)
   */
  uint8_t message_type : 4;
  /**
   * TR-UDP channel number
   */
  uint16_t channel : 4;
  /**
   * Payload length defines the number of bytes in the message payload
   */
  uint16_t payload_length : 12;
  /**
   * ID  is a message serial number that sender assigns to DATA and RESET
   * messages. The ACK messages must copy the ID from the corresponding DATA
   * and RESET messages.
   */
  uint32_t id;
  /**
   * Timestamp (32 byte) contains sending time of DATA and RESET messages and
   * filled in by message sender. The ACK messages must copy the timestamp
   * from the corresponding DATA and RESET messages.
   */
  uint32_t timestamp;

} trudpHeader;

#pragma pack(pop)

// Local functions
static void _trudpHeaderACKcreate(trudpHeader *out_th, trudpHeader *in_th);
static void _trudpHeaderACKtoRESETcreate(trudpHeader *out_th,
                                         trudpHeader *in_th);
static void _trudpHeaderACKtoPINGcreate(trudpPacket* packet, trudpHeader *in_th,
                                        void *data, size_t data_length);
static uint8_t _trudpHeaderChecksumCalculate(trudpHeader *th);
static int _trudpHeaderChecksumCheck(trudpHeader *th);
static inline void _trudpHeaderChecksumSet(trudpHeader *th, uint8_t chk);
static void _trudpHeaderCreate(trudpHeader *th, uint32_t id,
                               unsigned int message_type, unsigned int channel,
                               uint16_t payload_length, uint32_t timestamp);
static void _trudpHeaderDATAcreate(trudpPacket *packet, uint32_t id,
                                   unsigned int channel, void *data,
                                   size_t data_length);
static void _trudpHeaderPINGcreate(trudpPacket* packet, uint32_t id,
                                   unsigned int channel, void *data,
                                   size_t data_length);
static void _trudpHeaderRESETcreate(trudpPacket* packet, uint32_t id,
                                    unsigned int channel);
static inline int _trudpPacketGetChannel(trudpPacket *packet);
static inline void _trudpPacketSetChannel(trudpPacket *packet, int channel);
static inline void _trudpPacketSetType(trudpPacket *packet, trudpPacketType message_type);
static inline trudpHeader* _trudpPacketGetHeader(trudpPacket *packet);

/*****************************************************************************
 *
 *  Header checksum functions
 *
 *****************************************************************************/

/**
 * Calculate TR-UDP header checksum
 *
 * @param th
 * @return
 */
static uint8_t _trudpHeaderChecksumCalculate(trudpHeader *th) {

  int i;
  uint8_t checksum = 0;
  for (i = 1; i < sizeof(trudpHeader); i++) {
    checksum += *((uint8_t *)th + i);
  }

  return checksum;
}

/**
 * Set TR-UDP header checksum
 *
 * @param th
 * @param chk
 * @return
 */
static inline void _trudpHeaderChecksumSet(trudpHeader *th, uint8_t chk) {
  th->checksum = chk;
}

/**
 * Check TR-UDP header checksum
 *
 * @param th
 * @return
 */
static int _trudpHeaderChecksumCheck(trudpHeader *th) {
  return th->checksum == _trudpHeaderChecksumCalculate(th);
}

/*****************************************************************************
 *
 * TR-UDP header function
 *
 *****************************************************************************/

/**
 * Get current 32 bit timestamp in microseconds
 *
 * @return
 */
uint64_t teoGetTimestampFull() {
  return teotimeGetCurrentTimeUs();
}

/**
 * Get current 32 bit timestamp in microseconds
 *
 * @return
 */
uint32_t trudpGetTimestamp() {
  return (uint32_t)(teotimeGetCurrentTimeUs() & 0xFFFFFFFF);
}

/**
 * Get header of trudp packet.
 */

static inline trudpHeader* _trudpPacketGetHeader(trudpPacket *packet) {
  return (trudpHeader*)packet;
}

/**
 * Create TR-UDP header in buffer
 *
 * @param th Buffer to create in
 * @param id Packet Id
 * @param message_type Message type could be of type DATA(0x0), ACK(0x1) and
 * RESET(0x2)
 * @param payload_length Number of bytes in the package payload
 * @param timestamp Sending time of DATA or RESET messages
 */
static void _trudpHeaderCreate(trudpHeader *th, uint32_t id,
                               unsigned int message_type, unsigned int channel,
                               uint16_t payload_length, uint32_t timestamp) {

  th->id = id;
  th->message_type = message_type;
  th->channel = channel;
  th->payload_length = payload_length;
  th->timestamp = timestamp; // trudpHeaderTimestamp();
  th->version = TR_UDP_PROTOCOL_VERSION;
  th->checksum = _trudpHeaderChecksumCalculate(th);
}

/**
 * Update packet header to current timestamp
 *
 * @param packet pointer to packet
 *
 */
void trudpPacketUpdateTimestamp(trudpPacket *packet) {
  trudpHeader *th = _trudpPacketGetHeader(packet);
  th->timestamp = trudpGetTimestamp();
  th->checksum = _trudpHeaderChecksumCalculate(th);
}

/**
 * Create ACK package in buffer
 *
 * @param out_th Output buffer to create ACK header
 * @param in_th Input buffer with received TR-UDP package (header)
 */
static void _trudpHeaderACKcreate(trudpHeader *out_th, trudpHeader *in_th) {

  _trudpHeaderCreate(out_th, in_th->id, TRU_ACK, in_th->channel, 0,
                     in_th->timestamp);
}

/**
 * Create ACK to RESET package in buffer
 *
 * @param out_th Output buffer to create ACK header
 * @param in_th Input buffer with received TR-UDP package (header)
 */
static void _trudpHeaderACKtoRESETcreate(trudpHeader *out_th,
                                         trudpHeader *in_th) {

  _trudpHeaderCreate(out_th, in_th->id, TRU_ACK | TRU_RESET, in_th->channel, 0,
                     in_th->timestamp);
}

/**
 * Create ACK to PING package in buffer
 *
 * @param out_th Output buffer to create ACK header
 * @param in_th Input buffer with received TR-UDP package (header)
 */
static void _trudpHeaderACKtoPINGcreate(trudpPacket* packet, trudpHeader *in_th,
                                        void *data, size_t data_length) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);

  _trudpHeaderCreate(packet_header, in_th->id, TRU_ACK | TRU_PING, in_th->channel,
                     in_th->payload_length, in_th->timestamp);

  if (data != NULL && data_length != 0) {
    void* packet_data = trudpPacketGetData(packet);
    memcpy(packet_data, data, data_length);
  }
}

/**
 * Create RESET packages header in buffer
 *
 * @param out_th Output buffer to create RESET package (header)
 * @param id Packet serial number
 */
static void _trudpHeaderRESETcreate(trudpPacket* packet, uint32_t id,
                                    unsigned int channel) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);

  _trudpHeaderCreate(packet_header, id, TRU_RESET, channel, 0, trudpGetTimestamp());
}

/**
 * Create DATA packages header with data in buffer
 *
 * @param packet Output buffer to create DATA package (header)
 * @param id Packet serial number
 */
static void _trudpHeaderDATAcreate(trudpPacket* packet, uint32_t id,
                                   unsigned int channel, void *data,
                                   size_t data_length) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);

  _trudpHeaderCreate(packet_header, id, TRU_DATA, channel, data_length,
                     trudpGetTimestamp());

  if (data != NULL && data_length != 0) {
    void* packet_data = trudpPacketGetData(packet);
    memcpy(packet_data, data, data_length);
  }
}

/**
 * Create PING packages header with data in buffer
 *
 * @param packet Output buffer to create PING package (header)
 * @param id Packet serial number
 */
static void _trudpHeaderPINGcreate(trudpPacket* packet, uint32_t id,
                                   unsigned int channel, void *data,
                                   size_t data_length) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);

  _trudpHeaderCreate(packet_header, id, TRU_PING, channel, data_length,
                     trudpGetTimestamp());

  if (data != NULL && data_length != 0) {
    void* packet_data = trudpPacketGetData(packet);
    memcpy(packet_data, data, data_length);
  }
}

/*****************************************************************************
 *
 * TR-UDP packet function
 *
 *****************************************************************************/

/**
 * Check TR-UDP packet (header)
 *
 * @param data Pointer to trudpHeader (to packet)
 * @param packet_length Length of packet with trudp header
 *
 * @return Return true if packet is valid
 *
 */
trudpPacket* trudpPacketCheck(void *data, size_t packet_length) {
  if (packet_length < sizeof(trudpHeader)) {
    return NULL;
  }

  trudpPacket* packet = (trudpPacket*)data;
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);

  if (packet_length - sizeof(trudpHeader) == packet_header->payload_length &&
      _trudpHeaderChecksumCheck(packet_header)) {
    return packet;
  } else {
    return NULL;
  }
}

/**
 * Create ACK packet
 *
 * @param packet Pointer to received TR-UDP packet
 *
 * @return Pointer to allocated ACK packet, it should be free after use
 */
trudpPacket* trudpPacketACKcreateNew(trudpPacket* packet) {
  trudpHeader* in_th = _trudpPacketGetHeader(packet);

  trudpPacket* ack_packet = (trudpPacket*)malloc(sizeof(trudpHeader));
  trudpHeader* out_th = _trudpPacketGetHeader(ack_packet);

  _trudpHeaderACKcreate(out_th, in_th);

  return ack_packet;
}

/**
 * Create ACK to RESET package
 *
 * @param in_th Pointer to received TR-UDP package (header)
 *
 * @return Pointer to allocated ACK package, it should be free after use
 */
trudpPacket* trudpPacketACKtoRESETcreateNew(trudpPacket* packet) {
  trudpHeader* in_th = _trudpPacketGetHeader(packet);

  trudpPacket* ack_packet = (trudpPacket*)malloc(sizeof(trudpHeader));
  trudpHeader* out_th = _trudpPacketGetHeader(ack_packet);

  _trudpHeaderACKtoRESETcreate(out_th, in_th);

  return ack_packet;
}

/**
 * Create ACK to PING package
 *
 * @param packet Pointer to received TR-UDP package (header)
 *
 * @return Pointer to allocated ACK package, it should be free after use
 */
trudpPacket* trudpPacketACKtoPINGcreateNew(trudpPacket* packet) {
  size_t data_length = trudpPacketGetDataLength(packet);
  size_t new_packet_length = sizeof(trudpHeader) + data_length;

  trudpPacket* ack_packet = (trudpPacket*)malloc(new_packet_length);

  _trudpHeaderACKtoPINGcreate(ack_packet, _trudpPacketGetHeader(packet),
                              trudpPacketGetData(packet), data_length);

  return ack_packet;
}

/**
 * Create RESET package
 *
 * @param id Packet ID
 * @param channel Channel number
 *
 * @return Pointer to allocated RESET package, it should be free after use
 */
trudpPacket* trudpPacketRESETcreateNew(uint32_t id, unsigned int channel) {
  trudpPacket* packet = (trudpPacket*)malloc(sizeof(trudpHeader));

  _trudpHeaderRESETcreate(packet, id, channel);

  return packet;
}

/**
 * Create DATA package
 *
 * @param id Packet ID
 * @param channel TR-UDP channel
 * @param data Pointer to package data
 * @param data_length Package data length
 * @param packetLength
 *
 * @return Pointer to allocated and filled DATA package, it should be free
 *         after use
 */
trudpPacket* trudpPacketDATAcreateNew(uint32_t id, unsigned int channel, void *data,
                               size_t data_length, size_t *packet_length) {
  size_t new_packet_length = sizeof(trudpHeader) + data_length;

  trudpPacket* packet = (trudpPacket*)malloc(new_packet_length);

  _trudpHeaderDATAcreate(packet, id, channel, data, data_length);

  if (packet_length != NULL) {
    *packet_length = new_packet_length;
  }

  return packet;
}

/**
 * Create PING package
 *
 * @param id Packet ID (last send Id)
 * @param channel TR-UDP cannel
 * @param data Pointer to packet data
 * @param data_length [in] Packet data length
 * @param packet_length [out] Length in bytes of created packet.
 *
 * @return Pointer to allocated and filled DATA package, it should be free
 *         after use
 */
trudpPacket* trudpPacketPINGcreateNew(uint32_t id, unsigned int channel, void *data,
                               size_t data_length, size_t *packet_length) {
  size_t new_packet_length = sizeof(trudpHeader) + data_length;

  trudpPacket* packet = (trudpPacket*)malloc(new_packet_length);

  _trudpHeaderPINGcreate(packet, id, channel, data, data_length);

  if (packet_length != NULL) {
    *packet_length = new_packet_length;
  }

  return packet;
}

/**
 * Get ACK packet length
 *
 * @return ACK packet length
 */
size_t trudpPacketACKlength() { return sizeof(trudpHeader); }

/**
 * Get RESET packet length
 *
 * @return RESET packet length
 */
size_t trudpPacketRESETlength() { return sizeof(trudpHeader); }

/**
 * Free packet created with functions trudpHeaderDATAcreateNew,
 * trudpHeaderACKcreateNew or trudpHeaderRESETcreateNew
 *
 * @param in_th
 */
void trudpPacketCreatedFree(trudpPacket* packet) { free(packet); }

/**
 * Get packet Id
 *
 * @param packet Pointer to packet
 * @return Packet Id
 */
uint32_t trudpPacketGetId(trudpPacket *packet) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);
  return packet_header->id;
}

/**
 * Get channel number
 *
 * @param packet Pointer to packet
 * @return Packet Id
 */
static inline int _trudpPacketGetChannel(trudpPacket *packet) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);
  return packet_header->channel;
}

/**
 * Set channel number
 *
 * @param packet Pointer to packet
 * @param channel Channel number
 * @return Packet Id
 */
static inline void _trudpPacketSetChannel(trudpPacket *packet, int channel) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);
  packet_header->channel = channel;
}

/**
 * Get packet data
 *
 * @param packet Pointer to packet
 * @return Pointer to packet data
 */
void *trudpPacketGetData(trudpPacket *packet) {
  return (char *)packet + sizeof(trudpHeader);
}

/**
 * Get pointer to packet from it's data
 *
 * @param data Pointer to packet data
 * @return Pointer to packet
 */
// This is dangerous and should not be used.
/*
void *trudpPacketGetPacket(void *data) {

  return (char *)data - sizeof(trudpHeader);
}
*/

/**
 * Get packet data length
 *
 * @param packet Pointer to packet
 * @return Payload length defines the number of bytes in the message payload
 */
uint16_t trudpPacketGetDataLength(trudpPacket *packet) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);
  return packet_header->payload_length;
}

/**
 * Get packet header length
 *
 * @param packet Pointer to packet
 * @return Payload length defines the number of bytes in the message payload
 */
size_t trudpPacketGetHeaderLength(trudpPacket *packet) { return sizeof(trudpHeader); }

/**
 * Get packet length
 *
 * @param packet Pointer to packet
 * @return Packet length
 */
size_t trudpPacketGetPacketLength(trudpPacket* packet) {
  return trudpPacketGetDataLength(packet) + sizeof(trudpHeader);
}

/**
 * Get packet data type
 *
 * @param packet Pointer to packet
 * @return Message type could be of type:
 *  DATA(0x0), ACK(0x1), RESET(0x2), ACK_RESET(0x3), PING(0x4), ACK_PING(0x5)
 */
trudpPacketType trudpPacketGetType(trudpPacket *packet) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);
  return packet_header->message_type;
}

/**
 * Set packet data type
 *
 * @param packet Pointer to packet
 * @param message_type
 */
static inline void _trudpPacketSetType(trudpPacket *packet, trudpPacketType message_type) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);
  packet_header->message_type = message_type;
}

/**
 * Get packet timestamp
 *
 * @param packet Pointer to packet
 * @return Timestamp (32 byte) contains sending time of DATA and RESET messages
 */
uint32_t trudpPacketGetTimestamp(trudpPacket *packet) {
  trudpHeader* packet_header = _trudpPacketGetHeader(packet);
  return packet_header->timestamp;
}

void trudpPacketHeaderDump(char *buffer, size_t buffer_len, trudpPacket *packet) {
    trudpHeader *header = _trudpPacketGetHeader(packet);
    snprintf(buffer, buffer_len,
             "checksum=%u, version=%u, message_type=%s (%u), channel=%u, "
             "payload_length=%u, id=%u, timestamp=%u\n",
             (uint32_t)header->checksum, (uint32_t)header->version,
             STRING_trudpPacketType((trudpPacketType)header->message_type),
             (uint32_t)header->message_type, (uint32_t)header->channel,
             (uint32_t)header->payload_length, (uint32_t)header->id,
             (uint32_t)header->timestamp
    );
}

const char *STRING_trudpPacketType(trudpPacketType value) {
    switch (value) {
    case TRU_DATA: return "TRU_DATA";
    case TRU_ACK: return "TRU_ACK";
    case TRU_RESET: return "TRU_RESET";
    case TRU_ACK_TRU_RESET: return "TRU_ACK_TRU_RESET";
    case TRU_PING: return "TRU_PING";
    case TRU_ACK_PING: return "TRU_ACK_PING";
    default: break;
    }
    return "INVALID trudpPacketType";
}
