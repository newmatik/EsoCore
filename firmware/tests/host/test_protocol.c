/**
 * @file test_protocol.c
 * @brief Host unit tests for the EsoCore protocol frame codec (common/communication/protocol_frame.c)
 *
 * Both ends of the RS-485 sensor bus build and parse frames with this codec:
 * the sensor modules through protocol.c, the Edge through sensor_bus.c. The
 * tests encode frames as one side does and decode them as the other side does,
 * and pin the byte layout documented in protocol.h and firmware/README.md.
 * Built and run with `make test` in firmware/.
 */

#include "protocol.h"

#include <stdio.h>
#include <string.h>

static int checks_run = 0;
static int checks_failed = 0;
static const char *current_test = "";

#define CHECK(condition)                                                              \
    do {                                                                              \
        checks_run++;                                                                 \
        if (!(condition)) {                                                           \
            checks_failed++;                                                          \
            printf("FAIL %s (%s:%d): %s\n", current_test, __FILE__, __LINE__, #condition); \
        }                                                                             \
    } while (0)

static esocore_message_t make_message(uint8_t source, uint8_t destination, uint8_t type,
                                      uint8_t sequence, uint8_t flags,
                                      const uint8_t *payload, uint16_t length) {
    esocore_message_t message;
    memset(&message, 0, sizeof(message));
    message.header.start_byte = ESOCORE_PROTOCOL_START_BYTE;
    message.header.source_address = source;
    message.header.destination_address = destination;
    message.header.message_type = type;
    message.header.sequence_number = sequence;
    message.header.flags = flags;
    message.header.payload_length = length;
    if (length > 0) {
        memcpy(message.payload, payload, length);
    }
    return message;
}

static void test_crc(void) {
    current_test = "crc";
    /* CRC-16/MODBUS check value */
    const uint8_t check[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    CHECK(esocore_protocol_calculate_crc16(check, sizeof(check)) == 0x4B37);
}

static void test_layout(void) {
    current_test = "layout";
    const uint8_t payload[] = {0x11, 0x22, 0x33};
    esocore_message_t message = make_message(0x02, 0x00, ESOCORE_MSG_DATA_RESPONSE, 7,
                                             ESOCORE_FLAG_ACK_REQUIRED, payload, sizeof(payload));
    uint8_t frame[ESOCORE_PROTOCOL_FRAME_SIZE];
    uint16_t length = esocore_protocol_encode_frame(&message, frame, sizeof(frame));

    CHECK(length == 8 + 3 + 2);
    const uint8_t header[] = {0xAA, 0x02, 0x00, 0x11, 0x07, 0x01, 0x03, 0x00};
    CHECK(memcmp(frame, header, sizeof(header)) == 0);
    CHECK(memcmp(&frame[8], payload, sizeof(payload)) == 0);

    /* CRC directly behind the payload, little endian, over header and payload */
    uint16_t crc = esocore_protocol_calculate_crc16(frame, 11);
    CHECK(frame[11] == (uint8_t)(crc & 0xFF));
    CHECK(frame[12] == (uint8_t)(crc >> 8));
    CHECK(esocore_protocol_frame_crc(&message) == crc);

    /* Header size does not depend on the compiler's enum size */
    CHECK(ESOCORE_PROTOCOL_HEADER_SIZE == 8);
    CHECK(sizeof(((esocore_message_header_t *)0)->message_type) == 1);
}

static void test_round_trip(void) {
    current_test = "round trip";
    uint8_t payload[ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE];
    for (uint16_t i = 0; i < sizeof(payload); i++) {
        payload[i] = (uint8_t)(i * 7U);
    }
    const uint16_t lengths[] = {0, 1, 100, ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE};

    for (size_t i = 0; i < sizeof(lengths) / sizeof(lengths[0]); i++) {
        /* Sensor module side (protocol.c) encodes ... */
        esocore_message_t sent = make_message(0x05, ESOCORE_PROTOCOL_MASTER_ADDRESS,
                                              ESOCORE_MSG_DATA_BURST, (uint8_t)i, 0,
                                              payload, lengths[i]);
        uint8_t frame[ESOCORE_PROTOCOL_FRAME_SIZE];
        uint16_t length = esocore_protocol_encode_frame(&sent, frame, sizeof(frame));
        CHECK(length == 8 + lengths[i] + 2);

        /* ... and the Edge side (sensor_bus.c) decodes */
        esocore_message_t received;
        uint32_t consumed = 0;
        CHECK(esocore_protocol_decode_frame(frame, length, &received, &consumed) == ESOCORE_FRAME_OK);
        CHECK(consumed == length);
        CHECK(received.header.source_address == 0x05);
        CHECK(received.header.destination_address == ESOCORE_PROTOCOL_MASTER_ADDRESS);
        CHECK(received.header.message_type == ESOCORE_MSG_DATA_BURST);
        CHECK(received.header.sequence_number == (uint8_t)i);
        CHECK(received.header.payload_length == lengths[i]);
        CHECK(memcmp(received.payload, payload, lengths[i]) == 0);
        CHECK(received.crc == esocore_protocol_frame_crc(&received));
    }
}

static void test_edge_command_to_sensor(void) {
    current_test = "edge command";
    /* Edge (sensor_bus.c) sends a command, the sensor module (protocol.c) decodes it */
    const uint8_t payload[] = {ESOCORE_CMD_SENSOR_START, 0x01};
    esocore_message_t sent = make_message(ESOCORE_PROTOCOL_MASTER_ADDRESS, 0x03, ESOCORE_MSG_COMMAND,
                                          200, ESOCORE_FLAG_ACK_REQUIRED, payload, sizeof(payload));
    uint8_t frame[32];
    uint16_t length = esocore_protocol_encode_frame(&sent, frame, sizeof(frame));
    esocore_message_t received;
    uint32_t consumed = 0;
    CHECK(esocore_protocol_decode_frame(frame, length, &received, &consumed) == ESOCORE_FRAME_OK);
    CHECK(received.header.message_type == ESOCORE_MSG_COMMAND);
    CHECK(received.payload[0] == ESOCORE_CMD_SENSOR_START);
    CHECK(received.header.flags == ESOCORE_FLAG_ACK_REQUIRED);
}

static void test_partial_and_bad_frames(void) {
    current_test = "partial and bad frames";
    const uint8_t payload[] = {1, 2, 3, 4};
    esocore_message_t sent = make_message(0x04, 0x00, ESOCORE_MSG_HEARTBEAT, 1, 0, payload, 4);
    uint8_t frame[64];
    uint16_t length = esocore_protocol_encode_frame(&sent, frame, sizeof(frame));
    esocore_message_t received;
    uint32_t consumed = 99;

    /* Every prefix is incomplete and consumes nothing */
    for (uint16_t n = 1; n < length; n++) {
        CHECK(esocore_protocol_decode_frame(frame, n, &received, &consumed) == ESOCORE_FRAME_INCOMPLETE);
        CHECK(consumed == 0);
    }

    /* A corrupted payload byte fails the CRC; only the start byte is skipped */
    frame[9] ^= 0x40;
    CHECK(esocore_protocol_decode_frame(frame, length, &received, &consumed) == ESOCORE_FRAME_CRC_ERROR);
    CHECK(consumed == 1);
    frame[9] ^= 0x40;

    /* A damaged length must not swallow the next frame: after the CRC error the
     * receiver searches on byte by byte and finds the following frame */
    uint8_t damaged[128];
    memcpy(damaged, frame, length);
    damaged[6] = 12; /* heartbeat now claims 12 payload bytes, covering the next frame's header */
    memcpy(&damaged[length], frame, length);
    uint32_t total = 2U * length;
    uint32_t pos = 0;
    int crc_errors = 0;
    esocore_frame_status_t result;
    while ((result = esocore_protocol_decode_frame(&damaged[pos], total - pos, &received, &consumed)) !=
               ESOCORE_FRAME_OK &&
           result != ESOCORE_FRAME_INCOMPLETE) {
        if (result == ESOCORE_FRAME_CRC_ERROR) {
            crc_errors++;
        }
        pos += consumed;
    }
    CHECK(crc_errors == 1);
    CHECK(result == ESOCORE_FRAME_OK);
    CHECK(pos == length);
    CHECK(received.header.payload_length == 4);

    /* Bytes before a frame are skipped one at a time */
    uint8_t noisy[70];
    noisy[0] = 0x00;
    noisy[1] = 0x55;
    memcpy(&noisy[2], frame, length);
    uint32_t offset = 0;
    esocore_frame_status_t status;
    while ((status = esocore_protocol_decode_frame(&noisy[offset], length + 2 - offset,
                                                   &received, &consumed)) == ESOCORE_FRAME_NO_START) {
        CHECK(consumed == 1);
        offset += consumed;
    }
    CHECK(offset == 2);
    CHECK(status == ESOCORE_FRAME_OK);

    /* A start byte with an impossible length is skipped */
    uint8_t bogus[] = {0xAA, 0, 0, 0, 0, 0, 0xFF, 0xFF};
    CHECK(esocore_protocol_decode_frame(bogus, sizeof(bogus), &received, &consumed) == ESOCORE_FRAME_INVALID);
    CHECK(consumed == 1);

    /* Two frames back to back */
    uint8_t two[128];
    memcpy(two, frame, length);
    memcpy(&two[length], frame, length);
    CHECK(esocore_protocol_decode_frame(two, 2U * length, &received, &consumed) == ESOCORE_FRAME_OK);
    CHECK(consumed == length);
    CHECK(esocore_protocol_decode_frame(&two[consumed], length, &received, &consumed) == ESOCORE_FRAME_OK);
}

static void test_encode_limits(void) {
    current_test = "encode limits";
    esocore_message_t message = make_message(1, 0, ESOCORE_MSG_HEARTBEAT, 0, 0, NULL, 0);
    uint8_t frame[ESOCORE_PROTOCOL_FRAME_SIZE];

    message.header.payload_length = ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE + 1;
    CHECK(esocore_protocol_encode_frame(&message, frame, sizeof(frame)) == 0);

    message.header.payload_length = 10;
    CHECK(esocore_protocol_encode_frame(&message, frame, 19) == 0); /* needs 20 */
    CHECK(esocore_protocol_encode_frame(&message, frame, 20) == 20);
}

static void test_device_info(void) {
    current_test = "device info";
    esocore_device_info_t info;
    memset(&info, 0, sizeof(info));
    info.address = 0x07;
    info.device_type = ESOCORE_DEVICE_TYPE_PRESSURE;
    info.firmware_version_major = 1;
    info.firmware_version_minor = 4;
    info.hardware_version = 2;
    info.serial_number = 0xBEEF;
    info.capabilities = ESOCORE_CAPABILITY_SENSOR;
    info.status_flags = ESOCORE_STATUS_READY;
    info.uptime_seconds = 0x01020304;

    uint8_t payload[ESOCORE_DEVICE_INFO_WIRE_SIZE];
    CHECK(esocore_protocol_encode_device_info(&info, payload) == ESOCORE_DEVICE_INFO_WIRE_SIZE);
    const uint8_t expected[] = {0x07, 1, 4, 2, 0xEF, 0xBE, 0x01, 0x02, 0x04, 0x03, 0x02, 0x01};
    CHECK(memcmp(payload, expected, sizeof(expected)) == 0);

    esocore_device_info_t decoded;
    CHECK(esocore_protocol_decode_device_info(payload, sizeof(payload), 0x07, &decoded));
    CHECK(decoded.address == 0x07);
    CHECK(decoded.device_type == ESOCORE_DEVICE_TYPE_PRESSURE);
    CHECK(decoded.serial_number == 0xBEEF);
    CHECK(decoded.uptime_seconds == 0x01020304);
    CHECK(!esocore_protocol_decode_device_info(payload, ESOCORE_DEVICE_INFO_WIRE_SIZE - 1, 0x07, &decoded));
}

int main(void) {
    test_crc();
    test_layout();
    test_round_trip();
    test_edge_command_to_sensor();
    test_partial_and_bad_frames();
    test_encode_limits();
    test_device_info();

    printf("%d checks, %d failed\n", checks_run, checks_failed);
    return checks_failed == 0 ? 0 : 1;
}
