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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "teobase/platform.h"

#if defined(TEONET_OS_WINDOWS)
    #include <sys/types.h>
    #include <sys/timeb.h>
#else
    #include <sys/time.h>
#endif

#include "packet.h"


#pragma pack(push)
#pragma pack(1)

/**
 * TR-UDP message header structure
 */

typedef struct trudpHeader {

    uint8_t checksum; ///< Checksum
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
static void _trudpHeaderACKtoRESETcreate(trudpHeader *out_th, trudpHeader *in_th);
static void _trudpHeaderACKtoPINGcreate(trudpHeader *out_th, trudpHeader *in_th, void *data, size_t data_length);
static uint8_t _trudpHeaderChecksumCalculate(trudpHeader *th);
static int _trudpHeaderChecksumCheck(trudpHeader *th);
static void _trudpHeaderChecksumSet(trudpHeader *th, uint8_t chk);
static void _trudpHeaderCreate(trudpHeader *th, uint32_t id,
      unsigned int message_type, unsigned int channel, uint16_t payload_length,
        uint32_t timestamp);
static void _trudpHeaderDATAcreate(trudpHeader *out_th, uint32_t id,
        unsigned int channel, void *data, size_t data_length);
static void _trudpHeaderPINGcreate(trudpHeader *out_th, uint32_t id,
        unsigned int channel, void *data, size_t data_length);
static void _trudpHeaderRESETcreate(trudpHeader *out_th, uint32_t id,
        unsigned int channel);
static int _trudpPacketGetChannel(void *packet);
static void _trudpPacketSetChannel(void *packet, int channel);
static void _trudpPacketSetType(void *packet, trudpPacketType message_type);

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
    for(i = 1; i < sizeof(trudpHeader); i++) {
        checksum += *((uint8_t*) th + i);
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
static  void _trudpHeaderChecksumSet(trudpHeader *th, uint8_t chk) {

    th->checksum = chk;
}

/**
 * Check TR-UDP header checksum
 *
 * @param th
 * @return
 */
static  int _trudpHeaderChecksumCheck(trudpHeader *th) {

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
    int64_t current_time;

#if defined(TEONET_OS_WINDOWS)
    struct __timeb64 time_value;
    memset(&time_value, 0, sizeof(time_value));

    _ftime64_s(&time_value);

    current_time = time_value.time * 1000000 + time_value.millitm*1000;
#else
    struct timeval time_value;
    memset(&time_value, 0, sizeof(time_value));

    gettimeofday(&time_value, 0);

    // Cast to int64_t is needed on 32-bit unix systems.
    current_time = (int64_t)time_value.tv_sec * 1000000 + time_value.tv_usec;
#endif

    return current_time;

}

/**
 * Get current 32 bit timestamp in microseconds
 *
 * @return
 */
 uint32_t trudpGetTimestamp() {
    return (uint32_t) (teoGetTimestampFull() & 0xFFFFFFFF);
}

/**
 * Create TR-UDP header in buffer
 *
 * @param th Buffer to create in
 * @param id Packet Id
 * @param message_type Message type could be of type DATA(0x0), ACK(0x1) and RESET(0x2)
 * @param payload_length Number of bytes in the package payload
 * @param timestamp Sending time of DATA or RESET messages
 */
static void _trudpHeaderCreate(trudpHeader *th, uint32_t id,
      unsigned int message_type, unsigned int channel, uint16_t payload_length,
        uint32_t timestamp) {

    th->id = id;
    th->message_type = message_type;
    th->channel = channel;
    th->payload_length = payload_length;
    th->timestamp = timestamp; //trudpHeaderTimestamp();
    th->version = TR_UDP_PROTOCOL_VERSION;
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
static  void _trudpHeaderACKtoRESETcreate(trudpHeader *out_th, trudpHeader *in_th) {

    _trudpHeaderCreate(out_th, in_th->id, TRU_ACK | TRU_RESET, in_th->channel,
            0, in_th->timestamp);
}

/**
 * Create ACK to PING package in buffer
 *
 * @param out_th Output buffer to create ACK header
 * @param in_th Input buffer with received TR-UDP package (header)
 */
static  void _trudpHeaderACKtoPINGcreate(trudpHeader *out_th,
        trudpHeader *in_th, void *data, size_t data_length) {

    _trudpHeaderCreate(out_th, in_th->id, TRU_ACK | TRU_PING, in_th->channel,
            in_th->payload_length, in_th->timestamp);

    if(data && data_length)
        memcpy((char *)out_th + sizeof(trudpHeader), data, data_length);
}

/**
 * Create RESET packages header in buffer
 *
 * @param out_th Output buffer to create RESET package (header)
 * @param id Packet serial number
 */
static  void _trudpHeaderRESETcreate(trudpHeader *out_th, uint32_t id,
        unsigned int channel) {

    _trudpHeaderCreate(out_th, id, TRU_RESET, channel, 0, trudpGetTimestamp());
}

/**
 * Create DATA packages header with data in buffer
 *
 * @param out_th Output buffer to create DATA package (header)
 * @param id Packet serial number
 */
static  void _trudpHeaderDATAcreate(trudpHeader *out_th, uint32_t id,
        unsigned int channel, void *data, size_t data_length) {

    _trudpHeaderCreate(out_th, id, TRU_DATA, channel, data_length,
            trudpGetTimestamp());

    if(data && data_length)
        memcpy((char *)out_th + sizeof(trudpHeader), data, data_length);
}

/**
 * Create PING packages header with data in buffer
 *
 * @param out_th Output buffer to create PING package (header)
 * @param id Packet serial number
 */
static  void _trudpHeaderPINGcreate(trudpHeader *out_th, uint32_t id,
        unsigned int channel, void *data, size_t data_length) {

    _trudpHeaderCreate(out_th, id, TRU_PING, channel, data_length,
            trudpGetTimestamp());

    if(data && data_length)
        memcpy((char *)out_th + sizeof(trudpHeader), data, data_length);
}

/*****************************************************************************
 *
 * TR-UDP packet function
 *
 *****************************************************************************/

/**
 * Check TR-UDP packet (header)
 *
 * @param th Pointer to trudpHeader (to packet)
 * @param packetLength Length of packet with trudp header
 *
 * @return Return true if packet is valid
 *
 */
 int trudpPacketCheck(void *th, size_t packetLength) {

    return (packetLength - sizeof(trudpHeader) ==
        ((trudpHeader *)th)->payload_length && _trudpHeaderChecksumCheck(th)
    );
}

/**
 * Create ACK package
 *
 * @param in_th Pointer to received TR-UDP package (header)
 *
 * @return Pointer to allocated ACK package, it should be free after use
 */
 void *trudpPacketACKcreateNew(void *in_th) {

    trudpHeader *out_th = (trudpHeader *) malloc(sizeof(trudpHeader));
    _trudpHeaderACKcreate(out_th, (trudpHeader *)in_th);

    return (void *)out_th;
}

/**
 * Create ACK to RESET package
 *
 * @param in_th Pointer to received TR-UDP package (header)
 *
 * @return Pointer to allocated ACK package, it should be free after use
 */
 void *trudpPacketACKtoRESETcreateNew(void *in_th) {

    trudpHeader *out_th = (trudpHeader *) malloc(sizeof(trudpHeader));
    _trudpHeaderACKtoRESETcreate(out_th, (trudpHeader *)in_th);

    return (void *)out_th;
}

/**
 * Create ACK to PING package
 *
 * @param in_th Pointer to received TR-UDP package (header)
 *
 * @return Pointer to allocated ACK package, it should be free after use
 */
 void *trudpPacketACKtoPINGcreateNew(void *in_th) {

    size_t data_length = trudpPacketGetDataLength(in_th);
    trudpHeader *out_th = (trudpHeader *) malloc(sizeof(trudpHeader) +
            data_length);

    _trudpHeaderACKtoPINGcreate(out_th, (trudpHeader *)in_th,
            trudpPacketGetData(in_th), data_length);

    return (void *)out_th;
}

/**
 * Create RESET package
 *
 * @param id Packet ID
 * @param channel Channel number
 *
 * @return Pointer to allocated RESET package, it should be free after use
 */
 void *trudpPacketRESETcreateNew(uint32_t id, unsigned int channel) {

    trudpHeader *out_th = (trudpHeader *) malloc(sizeof(trudpHeader));
    _trudpHeaderRESETcreate(out_th, id, channel);

    return (void *)out_th;
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
 void *trudpPacketDATAcreateNew(uint32_t id, unsigned int channel,
        void *data, size_t data_length, size_t *packetLength) {

    if(packetLength) *packetLength = sizeof(trudpHeader) + data_length;
    trudpHeader *out_th = (trudpHeader *) malloc(*packetLength);
    _trudpHeaderDATAcreate(out_th, id, channel, data, data_length);

    return (void *)out_th;
}

/**
 * Create PING package
 *
 * @param id Packet ID (last send Id)
 * @param channel TR-UDP cannel
 * @param data Pointer to packet data
 * @param data_length Packet data length
 * @param packetLength [out]
 *
 * @return Pointer to allocated and filled DATA package, it should be free
 *         after use
 */
 void *trudpPacketPINGcreateNew(uint32_t id, unsigned int channel,
        void *data, size_t data_length, size_t *packetLength) {

    if(packetLength) *packetLength = sizeof(trudpHeader) + data_length;
    trudpHeader *out_th = (trudpHeader *) malloc(*packetLength);
    _trudpHeaderPINGcreate(out_th, id, channel, data, data_length);

    return (void *)out_th;
}

/**
 * Get ACK packet length
 *
 * @return ACK packet length
 */
 size_t trudpPacketACKlength() {

    return sizeof(trudpHeader);
}

/**
 * Get RESET packet length
 *
 * @return RESET packet length
 */
 size_t trudpPacketRESETlength() {

    return sizeof(trudpHeader);
}


/**
 * Free packet created with functions trudpHeaderDATAcreateNew,
 * trudpHeaderACKcreateNew or trudpHeaderRESETcreateNew
 *
 * @param in_th
 */
 void trudpPacketCreatedFree(void *in_th) {

    free(in_th);
}

/**
 * Get packet Id
 *
 * @param packet Pointer to packet
 * @return Packet Id
 */
 uint32_t trudpPacketGetId(void *packet) {

    return ((trudpHeader *)packet)->id;
}

/**
 * Get channel number
 *
 * @param packet Pointer to packet
 * @return Packet Id
 */
static  int _trudpPacketGetChannel(void *packet) {

    return ((trudpHeader *)packet)->channel;
}

/**
 * Set channel number
 *
 * @param packet Pointer to packet
 * @param channel Channel number
 * @return Packet Id
 */
static  void _trudpPacketSetChannel(void *packet, int channel) {

    ((trudpHeader *)packet)->channel = channel;
}

/**
 * Get packet data
 *
 * @param packet Pointer to packet
 * @return Pointer to packet data
 */
 void *trudpPacketGetData(void *packet) {

    return (char *)packet + sizeof(trudpHeader);
}

/**
 * Get pointer to packet from it's data
 *
 * @param data Pointer to packet data
 * @return Pointer to packet
 */
 void *trudpPacketGetPacket(void *data) {

    return (char *)data - sizeof(trudpHeader);
}

/**
 * Get packet data length
 *
 * @param packet Pointer to packet
 * @return Payload length defines the number of bytes in the message payload
 */
 uint16_t trudpPacketGetDataLength(void *packet) {

    return ((trudpHeader *)packet)->payload_length;
}

/**
 * Get packet header length
 *
 * @param packet Pointer to packet
 * @return Payload length defines the number of bytes in the message payload
 */
 size_t trudpPacketGetHeaderLength(void *packet) {

    return sizeof(trudpHeader);
}

/**
 * Get packet length
 *
 * @param packet Pointer to packet
 * @return Packet length
 */
 size_t trudpPacketGetPacketLength(void *packet) {

    return trudpPacketGetDataLength(packet)+trudpPacketGetHeaderLength(packet);
}

/**
 * Get packet data type
 *
 * @param packet Pointer to packet
 * @return Message type could be of type:
 *  DATA(0x0), ACK(0x1), RESET(0x2), ACK_RESET(0x3), PING(0x4), ACK_PING(0x5)
 */
 trudpPacketType trudpPacketGetType(void *packet) {

    return ((trudpHeader *)packet)->message_type;
}

/**
 * Set packet data type
 *
 * @param packet Pointer to packet
 * @param message_type
 */
static void _trudpPacketSetType(void *packet, trudpPacketType message_type) {

    ((trudpHeader *)packet)->message_type = message_type;
}

/**
 * Get packet timestamp
 *
 * @param packet Pointer to packet
 * @return Timestamp (32 byte) contains sending time of DATA and RESET messages
 */
 uint32_t trudpPacketGetTimestamp(void *packet) {

    return ((trudpHeader *)packet)->timestamp;
}
