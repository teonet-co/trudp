/*
 * File:   main_t.c
 * Author: Kirill Scherba <kirill@scherba.ru>
 *
 * Created on May 30, 2016, 11:53:18 AM
 */

#include "teobase/platform.h"

// This fix CHEAT test framework for MSVC.
#if defined(TEONET_COMPILER_MSVC)
#define _CRT_SECURE_NO_WARNINGS

#if defined(__BASE_FILE__)
#undef __BASE_FILE__
#endif

// This name must match test file name.
#define __BASE_FILE__ "main_t.c"
#endif

#if defined(TEONET_OS_WINDOWS)
#include "teobase/windows.h"
#endif

#include "cheat/cheat.h"

#include <string.h>

#include "teobase/types.h"

#include "packet.h"
#include "packet_queue.h"
#include "trudp.h"
#include "trudp_stat.h"

CHEAT_DECLARE(
    static void CheckPacketIsCorrect(uint8_t *packet_data,
                                               size_t packet_data_length,
                                               uint32_t packet_id) {
        trudpPacket *packet_after_check =
            trudpPacketCheck(packet_data, packet_data_length);

        cheat_assert(packet_after_check != NULL);
        cheat_yield(); // NOTE: This will exit helper function but not the test.

        uint32_t packet_id_from_packet = trudpPacketGetId(packet_after_check);

        cheat_assert(packet_id_from_packet == packet_id);
    }
)

CHEAT_TEST(
    create_data_packet,
    // Packet test data.
    char *packet_payload_string = "Header with Hello!";
    uint8_t *packet_payload = (uint8_t *)packet_payload_string;
    size_t packet_payload_size = strlen(packet_payload_string) + 1;
    uint32_t packet_id = 0; unsigned int channel = 0;

    // Create data packet.
    size_t packet_size;
    trudpPacket *packet = trudpPacketDATAcreateNew(
        packet_id, channel, packet_payload, packet_payload_size, &packet_size);

    // Check packet is valid.
    uint8_t *packet_buffer = (uint8_t *)packet;
    CheckPacketIsCorrect(packet_buffer, packet_size, packet_id);
    cheat_yield(); // Exit test if packet is not correct.

    // Check getter functions.
    cheat_assert(trudpPacketGetId(packet) == packet_id);
    cheat_assert(trudpPacketGetDataLength(packet) == packet_payload_size);
    int memcmp_result = memcmp(packet_payload, trudpPacketGetData(packet),
                               packet_payload_size);
    cheat_assert(memcmp_result == 0);
    cheat_assert(trudpPacketGetType(packet) == TRU_DATA);
    cheat_assert(trudpPacketGetTimestamp(packet) <= trudpGetTimestamp());

    trudpPacketCreatedFree(packet);
)

CHEAT_TEST(create_ack_packet,
    // Packet test data.
    char *packet_payload_string = "Header with Hello!";
    uint8_t *packet_payload = (uint8_t *)packet_payload_string;
    size_t packet_payload_size = strlen(packet_payload_string) + 1;
    uint32_t packet_id = 1; unsigned int channel = 0;

    // Create original packet.
    size_t packet_size;
    trudpPacket *packet = trudpPacketDATAcreateNew(packet_id, channel,
                                                    packet_payload,
                                                    packet_payload_size,
                                                    &packet_size);

    // Create ack packet for original packet.
    size_t ack_packet_size = trudpPacketACKlength();
    trudpPacket *ack_packet = trudpPacketACKcreateNew(packet);

    // Check packet is valid.
    uint8_t *ack_packet_buffer = (uint8_t *)ack_packet;
    CheckPacketIsCorrect(ack_packet_buffer, ack_packet_size, packet_id);
    cheat_yield(); // Exit test if packet is not correct.

    // Check getter functions.
    cheat_assert(trudpPacketGetId(ack_packet) == packet_id);
    cheat_assert(trudpPacketGetDataLength(ack_packet) == 0);
    cheat_assert(trudpPacketGetType(ack_packet) == TRU_ACK);
    cheat_assert(trudpPacketGetTimestamp(ack_packet) ==
                trudpPacketGetTimestamp(packet));

    trudpPacketCreatedFree(packet); trudpPacketCreatedFree(ack_packet);
)

CHEAT_TEST(create_reset_packet,
    // Packet test data.
    uint32_t packet_id = 2;
    unsigned int channel = 0;

    // Create data packet.
    size_t packet_size = trudpPacketRESETlength();
    trudpPacket *packet = trudpPacketRESETcreateNew(packet_id, channel);

    // Check packet is valid.
    uint8_t *packet_buffer = (uint8_t *)packet;
    CheckPacketIsCorrect(packet_buffer, packet_size, packet_id);
    cheat_yield(); // Exit test if packet is not correct.

    // Check getter functions.
    cheat_assert(trudpPacketGetId(packet) == packet_id);
    cheat_assert(trudpPacketGetDataLength(packet) == 0);
    cheat_assert(trudpPacketGetType(packet) == TRU_RESET);
    cheat_assert(trudpPacketGetTimestamp(packet) <= trudpGetTimestamp());

    trudpPacketCreatedFree(packet);
)

CHEAT_TEST(packet_queue,
    // Create packet queue
    trudpPacketQueue *tq = trudpPacketQueueNew();

    // ----------

    // First packet test data.
    char *packet1_payload_string = "Header with Hello!";
    uint8_t *packet1_payload = (uint8_t *)packet1_payload_string;
    size_t packet1_payload_size = strlen(packet1_payload_string) + 1;
    uint32_t packet1_id = 0; unsigned int channel = 0;

    // Second packet test data
    char *packet2_payload_string = "Header with Hello 2!";
    uint8_t *packet2_payload = (uint8_t *)packet2_payload_string;
    size_t packet2_payload_size = strlen(packet2_payload_string) + 1;
    uint32_t packet2_id = 1;

    // ----------

    // Create first data packet
    size_t packet1_size;
    trudpPacket *packet1 = trudpPacketDATAcreateNew(packet1_id, channel,
                                                    packet1_payload,
                                                    packet1_payload_size,
                                                    &packet1_size);

    // Check packet is valid.
    uint8_t *packet1_buffer = (uint8_t *)packet1;
    CheckPacketIsCorrect(packet1_buffer, packet1_size, packet1_id);
    cheat_yield(); // Exit test if packet is not correct.

    // Create second data packet
    size_t packet2_size;
    trudpPacket *packet2 = trudpPacketDATAcreateNew(packet2_id, channel,
                                                    packet2_payload,
                                                    packet2_payload_size,
                                                    &packet2_size);

    // Check packet is valid.
    uint8_t *packet2_buffer = (uint8_t *)packet2;
    CheckPacketIsCorrect(packet2_buffer, packet2_size, packet2_id);
    cheat_yield(); // Exit test if packet is not correct.

    // ----------

    // Add first data packet to timed queue
    trudpPacketQueueData *tqd = trudpPacketQueueAdd(
        tq, packet1_buffer, packet1_size, teoGetTimestampFull() + 10000);
    cheat_assert(tqd != NULL);
    cheat_yield(); // Can't continue test execution if assert failed.

    trudpPacket *tq_packet = trudpPacketQueueDataGetPacket(tqd);
    cheat_assert(trudpPacketGetId(tq_packet) == packet1_id);

    // Add second data packet to timed queue
    tqd = trudpPacketQueueAdd(tq, packet2_buffer, packet2_size,
                                teoGetTimestampFull() + 10000);
    cheat_assert(tqd != NULL);
    cheat_yield(); // Can't continue test execution if assert failed.

    tq_packet = trudpPacketQueueDataGetPacket(tqd);
    cheat_assert(trudpPacketGetId(tq_packet) == packet2_id);

    // ----------

    // Find data packet by Id
    cheat_assert(tqd == trudpPacketQueueFindById(tq, packet2_id));

    // Free timed queue
    trudpPacketQueueFree(tq); cheat_assert(teoQueueSize(tq->q) == 0);

    // Free packets
    trudpPacketCreatedFree(packet1); trudpPacketCreatedFree(packet2);

    // Destroy timed queue
    trudpPacketQueueDestroy(tq);
)

CHEAT_TEST(create_trudp,
    // Create TR-UDP
    trudpData *td = trudpInit(0, 0, NULL, NULL);
    cheat_assert(td != NULL);

    // Destroy TR-UDP
    trudpDestroy(td);
)

CHEAT_TEST(trudp_send_data,
    // Create TR-UDP
    trudpData *td = trudpInit(0, 0, NULL, NULL);
    cheat_assert(td != NULL);
    cheat_yield(); // Exit test if pointer is null.

    trudpChannelData *tcd = trudpChannelNew(td, "0", 8000, 0);
    cheat_assert(tcd != NULL);
    cheat_yield(); // Exit test if pointer is null.

    // Test data payload
    char *data_string = "HelloTR-UDP!";
    uint8_t *data_payload = (uint8_t *)data_string;
    size_t data_payload_size = strlen(data_string) + 1;

    // Send data
    size_t bytes_sent = trudpChannelSendData(tcd, data_payload,
                                            data_payload_size);
    cheat_assert(bytes_sent >= data_payload_size);

    // Send Queue should contain 1 element
    size_t sent_queue_size = trudpSendQueueSize(tcd->sendQueue);
    cheat_assert(sent_queue_size == 1);

    // Send Queue should contain an element with ID 0
    trudpSendQueueData *send_queue_data =
        trudpSendQueueFindById(tcd->sendQueue, 0);
    cheat_assert(send_queue_data != NULL);

    // Destroy TR-UDP
    trudpChannelDestroy(tcd); trudpDestroy(td);
)

#define LAST_RECEIVED_PACKETS_SIZE 10

CHEAT_DECLARE(
    // For tests use only
    static inline uint32_t trudpGetNewId(trudpChannelData *tcd) {
        return tcd->sendId++;
    }

    // For storing received data.
    int received_data_count;
    uint8_t* last_received_data[LAST_RECEIVED_PACKETS_SIZE];
    size_t last_received_data_size[LAST_RECEIVED_PACKETS_SIZE];

    // Use this callback to store and check received trudp packets.
    static void StoreReceivedDataCallback(void *tcd_pointer, int event, void *data, size_t data_length, void *user_data) {
        switch(event) {
            case GOT_DATA: {
            if (received_data_count < LAST_RECEIVED_PACKETS_SIZE) {
                    trudpPacket* packet = (trudpPacket*)data;
                    uint8_t *packet_data = trudpPacketGetData(packet);
                    size_t packet_data_length = trudpPacketGetDataLength(packet);

                    if (packet_data != NULL && packet_data_length > 0) {
                        last_received_data[received_data_count] = (uint8_t*)malloc(packet_data_length);
                        cheat_assert(last_received_data[received_data_count] != NULL);
                        cheat_yield(); // Exit test if pointer is null.

                        memcpy(last_received_data[received_data_count], packet_data, packet_data_length);

                        last_received_data_size[received_data_count] = packet_data_length;
                    } else {
                        last_received_data[received_data_count] = NULL;
                        last_received_data_size[received_data_count] = 0;
                    }

                    ++received_data_count;
                }
            }
        }
    }
)

CHEAT_SET_UP(
    received_data_count = 0;

    memset(last_received_data, 0, sizeof(last_received_data[0]) * LAST_RECEIVED_PACKETS_SIZE);
    memset(last_received_data_size, 0, sizeof(last_received_data_size[0]) * LAST_RECEIVED_PACKETS_SIZE);
)

CHEAT_TEAR_DOWN(
    for (int i = 0; i < received_data_count; ++i) {
        free(last_received_data[i]);
        last_received_data[i] = NULL;
    }

    received_data_count = 0;
)

CHEAT_TEST(
    trudp_process_received_packet,
    // Create TR-UDP
    trudpData *td = trudpInit(0, 0, StoreReceivedDataCallback, NULL);
    cheat_assert(td != NULL);
    cheat_yield(); // Exit test if pointer is null.

    trudpChannelData *tcd = trudpChannelNew(td, "0", 8000, 0);
    cheat_assert(tcd != NULL);
    cheat_yield(); // Exit test if pointer is null.

    char* data_strings[] =
        {
            "Hello TR-UDP test 0!",
            "Hello TR-UDP test 1!",
            "Hello TR-UDP test 2!",
            "Hello TR-UDP test 3!",
        };

    uint8_t* data[4];
    size_t data_length[4];
    trudpPacket* packet[4];
    size_t packet_length[4];

    for (int i = 0; i < 4; i++) {
        data[i] = (uint8_t*)data_strings[i];
        data_length[i] = strlen(data_strings[i]) + 1;

        packet[i] = trudpPacketDATAcreateNew(
            trudpGetNewId(tcd), 0, data[i], data_length[i], &packet_length[i]);

        cheat_assert(packet[i] != NULL);
    }

    // Process received packet (id 0) test
    uint32_t id = 0;
    int process_result = trudpChannelProcessReceivedPacket(tcd, (uint8_t*)packet[id],
                                           packet_length[id]);
    cheat_assert(process_result == 1);

    cheat_assert(received_data_count == 1);
    cheat_assert(last_received_data_size[0] == data_length[id]);
    cheat_yield(); // Exit test if data size is different.

    int memcmp_result = memcmp(data[id], last_received_data[0], data_length[id]);
    cheat_assert(memcmp_result == 0);

    // Process received packet (id 1) test
    id = 1;
    process_result = trudpChannelProcessReceivedPacket(tcd, (uint8_t*)packet[id],
                                           packet_length[id]);
    cheat_assert(process_result == 1);

    cheat_assert(received_data_count == 2);
    cheat_assert(last_received_data_size[1] == data_length[id]);
    cheat_yield(); // Exit test if data size is different.

    memcmp_result = memcmp(data[id], last_received_data[1],
                               data_length[id]);
    cheat_assert(memcmp_result == 0);

    // Process received packet (id 3) out of order test
    id = 3;
    process_result = trudpChannelProcessReceivedPacket(tcd, (uint8_t*)packet[id],
                                           packet_length[id]);
    // The trudpProcessReceivedPacket save this packet to receiveQueue and
    // return NULL
    cheat_assert(process_result == 1);

    // Process received packet (id 2) test
    id = 2;
    process_result = trudpChannelProcessReceivedPacket(tcd, (uint8_t*)packet[id],
                                           packet_length[id]);
    cheat_assert(process_result == 1);

    // The trudpProcessReceivedPacket process this packet, and packet from
    // queue.
    cheat_assert(received_data_count == 4);
    cheat_assert(last_received_data_size[2] == data_length[id]);
    cheat_yield(); // Exit test if data size is different.

    memcmp_result = memcmp(data[id], last_received_data[2],
                               data_length[id]);
    cheat_assert(memcmp_result == 0);

    id = 3;
    cheat_assert(last_received_data_size[3] == data_length[id]);
    cheat_yield(); // Exit test if data size is different.

    memcmp_result = memcmp(data[id], last_received_data[3],
                               data_length[id]);
    cheat_assert(memcmp_result == 0);

    // Process packet with random data.
    char* random_data_string = "Some random data that is not a packet ...";
    uint8_t* random_data_payload = (uint8_t*)random_data_string;
    size_t random_data_payload_size = strlen(random_data_string) + 1;

    // Check that buffer is not a valid trudp packet.
    trudpPacket* packet_after_check = trudpPacketCheck(random_data_payload, random_data_payload_size);
    cheat_assert(packet_after_check == NULL);

    // Process random data.
    process_result = trudpChannelProcessReceivedPacket(tcd, random_data_payload, random_data_payload_size);
    cheat_assert(process_result == 0);

    // Free packets
    for (int i = 0; i < 4; i++) {
        trudpPacketCreatedFree(packet[i]);
    }

    // Destroy TR-UDP
    trudpChannelDestroy(tcd);
    trudpDestroy(td);
)

CHEAT_DECLARE(
    trudpChannelData* tcd_A;
    trudpChannelData* tcd_B;

    static void td_A_eventCb(void *tcd_ptr, int event, void *packet, size_t packet_length, void *user_data) {
        switch(event) {
            case PROCESS_SEND: {
                // Receive data by B TR-UDP
                int process_result = trudpChannelProcessReceivedPacket(tcd_B, packet, packet_length);
                cheat_assert(process_result != -1);
            }
        }
    }

    static void td_B_eventCb(void *tcd_ptr, int event, void *packet, size_t packet_length, void *user_data) {
        switch(event) {
            case PROCESS_SEND: {
                // Receive data by A TR-UDP
                int process_result = trudpChannelProcessReceivedPacket(tcd_A, packet, packet_length);
                cheat_assert(process_result != -1);
            }
        }
    }
)

CHEAT_TEST(trudp_send_process_received_packet,
    // Create sender TR-UDP
    trudpData *td_A = trudpInit(0, 0, td_A_eventCb, "td_A");
    cheat_assert(td_A != NULL);
    cheat_yield(); // Exit test if pointer is null.

    tcd_A = trudpChannelNew(td_A, "0", 8000, 0);
    cheat_assert(tcd_A != NULL);
    cheat_yield(); // Exit test if pointer is null.

    // Create receiver TR-UDP
    trudpData *td_B = trudpInit(0, 0, td_B_eventCb, "td_B");
    cheat_assert(td_B != NULL);
    cheat_yield(); // Exit test if pointer is null.

    tcd_B = trudpChannelNew(td_B, "0", 8001, 0);
    cheat_assert(tcd_B != NULL);
    cheat_yield(); // Exit test if pointer is null.

    // Create DATA packets
    uint32_t idx, num_packets = 4;
    char* data_strings[] =
        {
            "Hello TR-UDP test 0!",
            "Hello TR-UDP test 1!",
            "Hello TR-UDP test 2!",
            "Hello TR-UDP test 3!",
        };

    uint8_t* data[4];
    size_t data_length[4];

    for (int i = 0; i < 4; i++) {
        data[i] = (uint8_t*)data_strings[i];
        data_length[i] = strlen(data_strings[i]) + 1;
    }

    for (int i = 0; i < 125000; i++) {
        // Send data from A to B, packet data idx = 0
        idx = 0;
        size_t send_result = trudpChannelSendData(tcd_A, data[idx], data_length[idx]);
        cheat_assert(send_result > 0);
        send_result = trudpChannelSendData(tcd_B, data[num_packets - idx - 1], data_length[num_packets - idx - 1]);
        cheat_assert(send_result > 0);

        idx = 1;
        // Test send queue retrieves
        send_result = trudpChannelSendData(tcd_A, data[idx], data_length[idx]);
        cheat_assert(send_result > 0);
        trudpChannelSendQueueProcess(tcd_A, trudpGetTimestamp(), NULL);
        // end test send queue retrieves
        send_result = trudpChannelSendData(tcd_B, data[num_packets - idx - 1], data_length[num_packets - idx - 1]);
        cheat_assert(send_result > 0);

        idx = 3;
        send_result = trudpChannelSendData(tcd_A, data[idx], data_length[idx]);
        cheat_assert(send_result > 0);
        send_result = trudpChannelSendData(tcd_B, data[num_packets - idx - 1], data_length[num_packets - idx - 1]);
        cheat_assert(send_result > 0);

        idx = 2;
        send_result = trudpChannelSendData(tcd_A, data[idx], data_length[idx]);
        cheat_assert(send_result > 0);
        send_result = trudpChannelSendData(tcd_B, data[num_packets - idx - 1], data_length[num_packets - idx - 1]);
        cheat_assert(send_result > 0);
    }

    char *stat_str = ksnTRUDPstatShowStr(TD(tcd_A), 0);
    puts(stat_str);
    free(stat_str);

    // Destroy TR-UDP
    trudpChannelDestroy(tcd_A);
    trudpChannelDestroy(tcd_B);
)
