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

/*
 * File:   packet.h
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 30, 2016, 5:47 PM
 */

#ifndef HEADER_H
#define HEADER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TR_UDP_PROTOCOL_VERSION 2
#define MIN_ACK_WAIT 0.000732  // 000.732 MS
#define MAX_ACK_WAIT 0.500  // 500 MS
#define MAX_MAX_ACK_WAIT (MAX_ACK_WAIT * 20.0) // 10 sec
#define MAX_ATTEMPT 5 // maximum attempt with MAX_MAX_ACK_WAIT wait value

/**
 * TR-UDP message type
 */
typedef enum trudpPacketType {

    TRU_DATA, ///< The DATA messages are carrying payload. (has payload)
    /**
     * The ACK messages are used to acknowledge the arrival of the DATA and
     * RESET messages. (has not payload)
     */
    TRU_ACK,
    TRU_RESET ///< The RESET messages reset messages counter. (has not payload)

} trudpPacketType;

inline int trudpPacketCheck(void *th, size_t packetLength);

inline void *trudpPacketACKcreateNew(void *in_th);
inline void *trudpPacketACKtoRESETcreateNew(void *in_th);
inline void *trudpPacketRESETcreateNew(uint32_t id, unsigned int channel);
inline void *trudpPacketDATAcreateNew(uint32_t id, unsigned int channel, void *data, size_t data_length, size_t *packetLength);

inline size_t trudpPacketACKlength();
inline size_t trudpPacketRESETlength();

inline uint32_t trudpPacketGetId(void *packet);
inline int trudpPacketGetChannel(void *packet);
inline int trudpPacketSetChannel(void *packet, int channel);
inline void *trudpPacketGetData(void *packet);
inline uint16_t trudpPacketGetDataLength(void *packet);
inline size_t trudpPacketGetHeaderLength(void *packet);
inline void *trudpPacketGetPacket(void *data);
inline trudpPacketType trudpPacketGetType(void *packet);
inline uint32_t trudpPacketGetTimestamp(void *packet);

inline void trudpPacketCreatedFree(void *in_th);

uint32_t trudpGetTimestamp();

#ifdef __cplusplus
}
#endif

#endif /* HEADER_H */

