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
 */

/**
 * TR-UDP Header and Packet functions module
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

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
     * Message type could be of type DATA(0x0), ACK(0x1) and RESET(0x2).
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
static uint8_t trudpHeaderChecksumCalculate(trudpHeader *th) {

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
static inline void trudpHeaderChecksumSet(trudpHeader *th, uint8_t chk) {

    th->checksum = chk;
}

/**
 * Check TR-UDP header checksum
 *
 * @param th
 * @return
 */
static inline int trudpHeaderChecksumCheck(trudpHeader *th) {

    return th->checksum == trudpHeaderChecksumCalculate(th);
}


/*****************************************************************************
 *
 * TR-UDP header function
 *
 *****************************************************************************/

/**
 * Get current 32 bit timestamp in thousands of milliseconds (uSec)
 *
 * @return
 */
inline unsigned long long trudpGetTimestampFull() {

// C11 present
#if __STDC_VERSION__ >= 201112L

    struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    unsigned long long tmilliseconds = ts.tv_sec*1000000LL + ts.tv_nsec/1000; // calculate thousands of milliseconds
    //return (uint32_t) (tmilliseconds & 0xFFFFFFFF);

#else

    struct timeval te;
    gettimeofday(&te, NULL); // get current time

    unsigned long long tmilliseconds = te.tv_sec*1000000LL + te.tv_usec; // calculate thousands of milliseconds
    //return (uint32_t) (tmilliseconds & 0xFFFFFFFF);

//    struct timespec tp;
//    clock_gettime(CLOCK_REALTIME, &tp);
//
//    unsigned long long tmilliseconds = tp.tv_sec*1000000LL + tp.tv_nsec/1000; // calculate thousands of milliseconds
//    return (uint32_t) (tmilliseconds & 0xFFFFFFFF);

#endif

    return tmilliseconds & 0xFFFFFFFF;
}

/**
 * Get current 32 bit timestamp in thousands of milliseconds (uSec)
 *
 * @return
 */
inline uint32_t trudpGetTimestamp() {
    return (uint32_t) (trudpGetTimestampFull() & 0xFFFFFFFF);
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
static void trudpHeaderCreate(trudpHeader *th, uint32_t id,
      unsigned int message_type, unsigned int channel, uint16_t payload_length,
        uint32_t timestamp) {

    th->id = id;
    th->message_type = message_type;
    th->channel = channel;
    th->payload_length = payload_length;
    th->timestamp = timestamp; //trudpHeaderTimestamp();
    th->version = TR_UDP_PROTOCOL_VERSION;
    th->checksum = trudpHeaderChecksumCalculate(th);
}

/**
 * Create ACK package in buffer
 *
 * @param out_th Output buffer to create ACK header
 * @param in_th Input buffer with received TR-UDP package (header)
 */
static inline void *trudpHeaderACKcreate(trudpHeader *out_th, trudpHeader *in_th) {

    trudpHeaderCreate(out_th, in_th->id, TRU_ACK, in_th->channel, 0, in_th->timestamp);
}

/**
 * Create ACK to RESET package in buffer
 *
 * @param out_th Output buffer to create ACK header
 * @param in_th Input buffer with received TR-UDP package (header)
 */
static inline void *trudpHeaderACKtoRESETcreate(trudpHeader *out_th, trudpHeader *in_th) {

    trudpHeaderCreate(out_th, in_th->id, TRU_ACK | TRU_RESET, in_th->channel, 0, in_th->timestamp);
}

/**
 * Create RESET packages header in buffer
 *
 * @param out_th Output buffer to create RESET package (header)
 * @param id Packet serial number
 */
static inline void trudpHeaderRESETcreate(trudpHeader *out_th, uint32_t id,
        unsigned int channel) {

    trudpHeaderCreate(out_th, id, TRU_RESET, channel, 0, trudpGetTimestamp());
}

/**
 * Create DATA packages header with data in buffer
 *
 * @param out_th Output buffer to create DATA package (header)
 * @param id Packet serial number
 */
static inline void trudpHeaderDATAcreate(trudpHeader *out_th, uint32_t id,
        unsigned int channel, void *data, size_t data_length) {

    trudpHeaderCreate(out_th, id, TRU_DATA, channel, data_length, trudpGetTimestamp());
    if(data && data_length)
        memcpy((void *)out_th + sizeof(trudpHeader), data, data_length);
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
inline int trudpPacketCheck(void *th, size_t packetLength) {

    return (packetLength - sizeof(trudpHeader) == ((trudpHeader *)th)->payload_length &&
           trudpHeaderChecksumCheck(th));
}

/**
 * Create ACK package
 *
 * @param in_th Pointer to received TR-UDP package (header)
 *
 * @return Pointer to allocated ACK package, it should be free after use
 */
inline void *trudpPacketACKcreateNew(void *in_th) {

    trudpHeader *out_th = (trudpHeader *) malloc(sizeof(trudpHeader));
    trudpHeaderACKcreate(out_th, (trudpHeader *)in_th);

    return (void *)out_th;
}

/**
 * Create ACK to RESET package
 *
 * @param in_th Pointer to received TR-UDP package (header)
 *
 * @return Pointer to allocated ACK package, it should be free after use
 */
inline void *trudpPacketACKtoRESETcreateNew(void *in_th) {

    trudpHeader *out_th = (trudpHeader *) malloc(sizeof(trudpHeader));
    trudpHeaderACKtoRESETcreate(out_th, (trudpHeader *)in_th);

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
inline void *trudpPacketRESETcreateNew(uint32_t id, unsigned int channel) {

    trudpHeader *out_th = (trudpHeader *) malloc(sizeof(trudpHeader));
    trudpHeaderRESETcreate(out_th, id, channel);

    return (void *)out_th;
}

/**
 * Create DATA package
 *
 * @param id Packet ID
 * @param data Pointer to package data
 * @param data_length Package data length
 * @param packetLength
 *
 * @return Pointer to allocated and filled DATA package, it should be free after use
 */
inline void *trudpPacketDATAcreateNew(uint32_t id, unsigned int channel,
        void *data, size_t data_length, size_t *packetLength) {

    *packetLength = sizeof(trudpHeader) + data_length;
    trudpHeader *out_th = (trudpHeader *) malloc(*packetLength);
    trudpHeaderDATAcreate(out_th, id, channel, data, data_length);

    return (void *)out_th;
}

/**
 * Get ACK packet length
 *
 * @return ACK packet length
 */
inline size_t trudpPacketACKlength() {

    return sizeof(trudpHeader);
}

/**
 * Get RESET packet length
 *
 * @return RESET packet length
 */
inline size_t trudpPacketRESETlength() {

    return sizeof(trudpHeader);
}


/**
 * Free packet created with functions trudpHeaderDATAcreateNew, trudpHeaderACKcreateNew or trudpHeaderRESETcreateNew
 *
 * @param in_th
 */
inline void trudpPacketCreatedFree(void *in_th) {

    free(in_th);
}

/**
 * Get packet Id
 *
 * @param packet Pointer to packet
 * @return Packet Id
 */
inline uint32_t trudpPacketGetId(void *packet) {

    return ((trudpHeader *)packet)->id;
}

/**
 * Get channel number
 *
 * @param packet Pointer to packet
 * @return Packet Id
 */
inline int trudpPacketGetChannel(void *packet) {

    return ((trudpHeader *)packet)->channel;
}

/**
 * Set channel number
 *
 * @param packet Pointer to packet
 * @param channel Channel number
 * @return Packet Id
 */
inline int trudpPacketSetChannel(void *packet, int channel) {

    return ((trudpHeader *)packet)->channel;
}

/**
 * Get packet data
 *
 * @param packet Pointer to packet
 * @return Pointer to packet data
 */
inline void *trudpPacketGetData(void *packet) {

    return packet + sizeof(trudpHeader);
}

/**
 * Get pointer to packet from it's data
 *
 * @param data Pointer to packet data
 * @return Pointer to packet
 */
inline void *trudpPacketGetPacket(void *data) {

    return data - sizeof(trudpHeader);
}

/**
 * Get packet data length
 *
 * @param packet Pointer to packet
 * @return Payload length defines the number of bytes in the message payload
 */
inline uint16_t trudpPacketGetDataLength(void *packet) {

    return ((trudpHeader *)packet)->payload_length;
}

/**
 * Get packet header length
 *
 * @param packet Pointer to packet
 * @return Payload length defines the number of bytes in the message payload
 */
inline size_t trudpPacketGetHeaderLength(void *packet) {

    return sizeof(trudpHeader);
}

/**
 * Get packet data type
 *
 * @param packet Pointer to packet
 * @return Message type could be of type DATA(0x0), ACK(0x1) and RESET(0x2)
 */
inline trudpPacketType trudpPacketGetType(void *packet) {

    return ((trudpHeader *)packet)->message_type;
}

/**
 * Get packet timestamp
 *
 * @param packet Pointer to packet
 * @return Timestamp (32 byte) contains sending time of DATA and RESET messages
 */
inline uint32_t trudpPacketGetTimestamp(void *packet) {

    return ((trudpHeader *)packet)->timestamp;
}
