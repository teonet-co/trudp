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
 * Created on May 31, 2016, 1:45 AM
 */

#ifndef TR_UDP_H
#define TR_UDP_H

#include "packet_queue.h"
#include "write_queue.h"
#include "packet.h"
#include "map.h"
#include "udp.h"

#ifdef __cplusplus
extern "C" {
#endif

// TR-UDP constants    
#define MAX_KEY_LENGTH 64 // Maximum key length
#define MAX_OUTRUNNING 500 // Maximum outrunning in receive queue to send reset
#define START_MIDDLE_TIME (MAX_ACK_WAIT/5) * 1000000 // Midle time at start       
#define RESET_AFTER_ID (UINT32_MAX - 1024) // Reset if send id more than this constant
#define MAX_TRIPTIME_MIDDLE 5757575 // Maximum number of Middle triptime
#define MAX_LAST_RECEIVE MAX_TRIPTIME_MIDDLE // Disconnect after last receved packet time older than this constant
#define SEND_PING_AFTER 2500000   
#define MAP_SIZE_DEFAULT 1000 // Default map size; map stored connected channels and can auto resize
#define USE_WRITE_QUEUE 0 // Use write queue instead of direct write to socket
#define MAX_RTT 50000 // 250000; This constant used in send queue expected time calculation

//#define MIN_RETRIEVES_TIME 3 * 1000 * 1000   
//#define SEND_QUEUE_MAX 500
//#define MAX_RETRIEVES 200
    
/**
 * Get pointer to trudpData from trudpChannelData
 */    
#define TD(tcd) ((trudpData*)tcd->td)    
    
/**
 * Data received/send callback
 */
typedef void (*trudpDataCb)(void *tcd, void *data, size_t data_length, void *user_data);

/**
 * Event callback
 */
typedef void (*trudpEventCb)(void *tcd, int event, void *data, size_t data_length, void *user_data);

/**
 * Union of TR-UDP callback
 */
typedef union trudpCb {
    
    trudpDataCb data;
    trudpDataCb send;
    trudpEventCb event;
    void *ptr;
    
} trudpCb;

/**
 * Enumeration of callback types
 */
typedef enum trudpCallbsckType {
    
    PROCESS_DATA,
//    PROCESS_ACK,
    EVENT,
    SEND
            
} trudpCallbsckType;

/**
 * Enumeration of TR-UDP events
 */
typedef enum trudpEvent {
    
    /**
     * TR-UDP channel disconnected event
     * @param data NULL
     * @param data_length 0
     * @param user_data NULL
     */
    CONNECTED,
    /**
     * TR-UDP channel disconnected event
     * @param data Last packet received
     * @param data_length 0
     * @param user_data NULL
     */
    DISCONNECTED,
    /**
     * Got TR-UDP reset packet
     * @param data NULL
     * @param data_length 0
     * @param user_data NULL
     */
    GOT_TRU_RESET,
    /**
     * Send TR-UDP reset packet
     * @param data Pointer to uint32_t send id or NULL if received id = 0
     * @param data_length Size of uint32_t or 0
     * @param user_data NULL
     */
    SEND_TRU_RESET,
    /**
     * Got ACK to reset command
     * @param data NULL
     * @param data_length 0
     * @param user_data NULL
     */
    GOT_ACK_RESET,
    /**
     * Got ACK to ping command
     * @param data Pointer to ping data (usually it is a string)
     * @param data_length Length of data
     * @param user_data NULL
     */
    GOT_ACK_PING,
    /**
     * Got PING command
     * @param data Pointer to ping data (usually it is a string)
     * @param data_length Length of data
     * @param user_data NULL
     */
    GOT_PING,
    /**
     * Got ACK command
     * @param data Pointer to ACK packet
     * @param data_length Length of data
     * @param user_data NULL
     */
    GOT_ACK,
    /**
     * Got DATA 
     * @param data Pointer to data
     * @param data_length Length of data
     * @param user_data NULL
     */
    GOT_DATA
            
} trudpEvent;

/**
 * Last 10 send statistic data
 */
typedef struct last10_data {
    
    uint32_t triptime; ///< Packet triptime
    uint32_t size_b; ///< Size of backet in bites
    uint32_t ts; ///< Packet time
    
} last10_data;  

/**
 * TR-UDP channel statistic data
 */
typedef struct trudpStatChannelData {
    
    #define LAST10_SIZE 10
    char key[MAX_KEY_LENGTH]; ///< Channel key 
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
    last10_data last_send_packets_ar[LAST10_SIZE]; ///< Last 10 send packets
    size_t idx_snd; ///< Index of last_send_packet_ar
    last10_data last_receive_packets_ar[LAST10_SIZE]; ///< Last 10 receive packets
    size_t idx_rcv; ///< Index of last_receive_packets_ar   
    uint32_t sendQueueSize;
    uint32_t receiveQueueSize;
    
} trudpStatChannelData;   

/**
 * Trudp channel Data Structure
 */
typedef struct trudpChannelData {
    
    uint32_t sendId; ///< Send ID
    trudpPacketQueue *sendQueue; ///< Pointer to send queue trudpPacketQueue
    uint32_t triptime; ///< Trip time 
    double triptimeFactor; ///< Triptime factor
    uint32_t triptimeMiddle; ///< Trip time middle
//    uint32_t lastSend; ///< Last send time
    
    trudpWriteQueue *writeQueue; ///< Pointer to write queue trudpWriteQueue
        
    uint32_t receiveExpectedId; ///< Ecpected recive Id
    trudpPacketQueue *receiveQueue; ///< Pointer to recive queue receiveQueue
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
    
} trudpChannelData;

/**
 * TR-UDP Statistic data
 */
typedef struct trudpStatData {
    
    struct sendQueue {
        size_t size_max;
        size_t size_current;
        size_t attempt;
    } sendQueue;
    
    struct receiveQueue {
        size_t size_max;
        size_t size_current;
    } receiveQueue;
    
} trudpStatData;

/**
 * Trudp Data Structure
 */
typedef struct trudpData {

    trudpMapData *map; ///< Channels map (key: ip:port:channel)
    void* user_data; ///< User data
    int port; ///< Port
    int fd; ///< File descriptor
                 
    // Callback
    trudpDataCb processDataCb;
    trudpDataCb processAckCb;
    trudpEventCb evendCb;
    trudpDataCb sendCb;       
    
    // Statistic
    trudpStatData stat;
    unsigned long long started;
    
    size_t writeQueueIdx;
    
} trudpData;

trudpData *trudpInit(int fd, int port, void *user_data);
void trudpDestroy(trudpData* trudp);
trudpCb trudpSetCallback(trudpData *td, trudpCallbsckType type, trudpCb cb);
trudpChannelData *trudpCheckRemoteAddr(trudpData *td, struct sockaddr_in *remaddr, 
        socklen_t addr_length, int channel);
int trudpProcessSendQueue(trudpData *td, uint64_t *next_et);
size_t trudpProcessWriteQueue(trudpData *td);
void trudpSendResetAll(trudpData *td);
size_t trudpKeepConnection(trudpData *td);
uint32_t trudpGetSendQueueTimeout(trudpData *td);

trudpChannelData *trudpNewChannel(trudpData *td, char *remote_address, int remote_port_i, int channel); // void *user_data, trudpDataCb processDataCb, trudpDataCb sendPacketCb);
void trudpDestroyChannel(trudpChannelData *tcd);
void trudpFreeChannel(trudpChannelData *tcd);
void trudpResetChannel(trudpChannelData *tcd);
size_t trudpSendData(trudpChannelData *tcd, void *data, size_t data_length);
size_t trudpSendDataToAll(trudpData *td, void *data, size_t data_length);
void *trudpProcessChannelReceivedPacket(trudpChannelData *tcd, void *packet, 
        size_t packet_length, size_t *data_length);
int trudpProcessChannelSendQueue(trudpChannelData *tcd, uint64_t ts,
        uint64_t *next_expected_time);
char *trudpMakeKeyCannel(trudpChannelData *tcd);

char *trudpMakeKey(char *addr, int port, int channel, size_t *key_length);
size_t trudpGetSendQueueMax(trudpData *td);
size_t trudpGetReceiveQueueMax(trudpData *td);

#ifdef __cplusplus
}
#endif

#endif /* TR_UDP_H */
