/**
 * @file test_modbus_rtu.c
 * @brief Host unit tests for the Modbus RTU slave (common/communication/modbus_rtu.c)
 *
 * Built and run on the development machine with `make test` in firmware/;
 * needs only a C11 compiler, no target hardware. Request and response bytes
 * of the "spec example" tests are the examples of the Modbus Application
 * Protocol Specification V1.1b3, section 6.
 */

#include "modbus_rtu.h"

#include <stdio.h>
#include <string.h>

/* ----------------------------------------------------------------------------
 * Minimal test harness
 * ------------------------------------------------------------------------- */

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

#define CHECK_BYTES(actual, actual_length, ...)                                       \
    do {                                                                              \
        const uint8_t expected_[] = {__VA_ARGS__};                                    \
        check_bytes(__LINE__, (actual), (actual_length), expected_, (uint16_t)sizeof(expected_)); \
    } while (0)

static void check_bytes(int line, const uint8_t *actual, uint16_t actual_length,
                        const uint8_t *expected, uint16_t expected_length) {
    checks_run++;
    if (actual_length == expected_length && memcmp(actual, expected, expected_length) == 0) {
        return;
    }

    checks_failed++;
    printf("FAIL %s (line %d): got", current_test, line);
    for (uint16_t i = 0; i < actual_length; i++) {
        printf(" %02X", actual[i]);
    }
    printf(", expected");
    for (uint16_t i = 0; i < expected_length; i++) {
        printf(" %02X", expected[i]);
    }
    printf("\n");
}

/* ----------------------------------------------------------------------------
 * Fixtures
 * ------------------------------------------------------------------------- */

#define SLAVE_ADDRESS 0x11
#define NUM_BITS      2000
#define NUM_REGISTERS 200

static uint8_t response[MODBUS_MAX_FRAME_SIZE];
static uint16_t response_length;

static modbus_config_t slave_config(void) {
    modbus_config_t config;
    memset(&config, 0, sizeof(config));
    config.mode = MODBUS_MODE_SLAVE;
    config.slave_address = SLAVE_ADDRESS;
    config.baud_rate = 9600;
    config.data_bits = 8;
    config.stop_bits = 1;
    config.parity = 'E';
    config.enable_broadcast = true;
    config.max_coils = NUM_BITS;
    config.max_registers = NUM_REGISTERS;
    return config;
}

static void begin(const char *name) {
    current_test = name;
    modbus_rtu_deinit();
    modbus_config_t config = slave_config();
    CHECK(modbus_rtu_init(&config));
    memset(response, 0xEE, sizeof(response));
    response_length = 0xFFFF;
}

/**
 * Sends the PDU (function code + data) to `address` through
 * modbus_process_frame() and leaves the answer in `response`.
 */
static bool request_to(uint8_t address, const uint8_t *pdu, uint16_t pdu_length) {
    uint8_t data[MODBUS_MAX_FRAME_SIZE];
    uint8_t raw[MODBUS_MAX_FRAME_SIZE];
    modbus_frame_t frame;

    memset(&frame, 0, sizeof(frame));
    frame.address = address;
    frame.function_code = pdu[0];
    frame.data_length = (uint16_t)(pdu_length - 1);
    memcpy(data, &pdu[1], frame.data_length);
    frame.data = frame.data_length > 0 ? data : NULL;

    raw[0] = address;
    memcpy(&raw[1], pdu, pdu_length);
    frame.crc = modbus_calculate_crc(raw, (uint16_t)(pdu_length + 1));

    return modbus_process_frame(&frame, response, &response_length, sizeof(response));
}

#define REQUEST(...)                                                       \
    do {                                                                   \
        const uint8_t pdu_[] = {__VA_ARGS__};                              \
        CHECK(request_to(SLAVE_ADDRESS, pdu_, (uint16_t)sizeof(pdu_)));    \
    } while (0)

/** Checks that `response` is: slave address, the given PDU bytes, valid CRC. */
#define EXPECT_RESPONSE(...)                                               \
    do {                                                                   \
        const uint8_t pdu_[] = {__VA_ARGS__};                              \
        expect_response(__LINE__, pdu_, (uint16_t)sizeof(pdu_));           \
    } while (0)

static void expect_response(int line, const uint8_t *pdu, uint16_t pdu_length) {
    uint8_t expected[MODBUS_MAX_FRAME_SIZE];

    expected[0] = SLAVE_ADDRESS;
    memcpy(&expected[1], pdu, pdu_length);
    uint16_t crc = modbus_calculate_crc(expected, (uint16_t)(pdu_length + 1));
    expected[pdu_length + 1] = (uint8_t)(crc & 0xFF);
    expected[pdu_length + 2] = (uint8_t)(crc >> 8);

    check_bytes(line, response, response_length, expected, (uint16_t)(pdu_length + 3));
}

static modbus_statistics_t statistics(void) {
    modbus_statistics_t stats;
    memset(&stats, 0, sizeof(stats));
    CHECK(modbus_get_statistics(&stats));
    return stats;
}

/* Sets coils or discrete inputs so that the bytes read back from `start` are `bytes`. */
static void set_coil_bytes(uint16_t start, const uint8_t *bytes, uint16_t count) {
    for (uint16_t i = 0; i < count * 8; i++) {
        bool on = (bytes[i / 8] >> (i % 8)) & 1;
        const uint8_t pdu[] = {0x05, (uint8_t)((start + i) >> 8), (uint8_t)(start + i),
                               on ? 0xFF : 0x00, 0x00};
        CHECK(request_to(SLAVE_ADDRESS, pdu, sizeof(pdu)));
    }
}

static void set_input_bytes(uint16_t start, const uint8_t *bytes, uint16_t count) {
    for (uint16_t i = 0; i < count * 8; i++) {
        CHECK(modbus_set_discrete_input((uint16_t)(start + i), (bytes[i / 8] >> (i % 8)) & 1));
    }
}

/* ----------------------------------------------------------------------------
 * CRC
 * ------------------------------------------------------------------------- */

static void test_crc(void) {
    begin("crc");

    /* Widely published reference frame: 11 03 00 6B 00 03 -> CRC bytes 76 87 */
    const uint8_t frame_a[] = {0x11, 0x03, 0x00, 0x6B, 0x00, 0x03};
    CHECK(modbus_calculate_crc(frame_a, sizeof(frame_a)) == 0x8776);

    /* 01 03 00 00 00 0A -> CRC bytes C5 CD */
    const uint8_t frame_b[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0A};
    CHECK(modbus_calculate_crc(frame_b, sizeof(frame_b)) == 0xCDC5);
}

/* ----------------------------------------------------------------------------
 * Function codes
 * ------------------------------------------------------------------------- */

static void test_read_coils(void) {
    begin("0x01 read coils, spec example");

    /* Coils 20-38 (addresses 19-37) hold CD 6B 05 */
    const uint8_t pattern[] = {0xCD, 0x6B, 0x05};
    set_coil_bytes(19, pattern, 3);

    REQUEST(0x01, 0x00, 0x13, 0x00, 0x13);
    EXPECT_RESPONSE(0x01, 0x03, 0xCD, 0x6B, 0x05);

    begin("0x01 read coils, padding bits are zero");
    const uint8_t all_on[] = {0xFF, 0xFF};
    set_coil_bytes(0, all_on, 2);
    REQUEST(0x01, 0x00, 0x00, 0x00, 0x0A); /* 10 coils */
    EXPECT_RESPONSE(0x01, 0x02, 0xFF, 0x03);

    begin("0x01 read coils, limits");
    REQUEST(0x01, 0x00, 0x00, 0x07, 0xD0); /* 2000 coils: the maximum */
    CHECK(response_length == 5 + 250);
    CHECK(response[1] == 0x01 && response[2] == 250);
    REQUEST(0x01, 0x00, 0x00, 0x07, 0xD1); /* 2001 */
    EXPECT_RESPONSE(0x81, 0x03);
    REQUEST(0x01, 0x00, 0x00, 0x00, 0x00); /* 0 */
    EXPECT_RESPONSE(0x81, 0x03);
    REQUEST(0x01, 0x07, 0xCF, 0x00, 0x01); /* last coil */
    EXPECT_RESPONSE(0x01, 0x01, 0x00);
    REQUEST(0x01, 0x07, 0xCF, 0x00, 0x02); /* one past the end */
    EXPECT_RESPONSE(0x81, 0x02);
    REQUEST(0x01, 0xFF, 0xFF, 0x00, 0x02); /* address + quantity overflows 16 bits */
    EXPECT_RESPONSE(0x81, 0x02);
    REQUEST(0x01, 0x00, 0x00, 0x00); /* truncated request */
    EXPECT_RESPONSE(0x81, 0x03);
}

static void test_read_discrete_inputs(void) {
    begin("0x02 read discrete inputs, spec example");

    /* Inputs 197-218 (addresses 196-217) hold AC DB 35 */
    const uint8_t pattern[] = {0xAC, 0xDB, 0x35};
    set_input_bytes(196, pattern, 3);

    REQUEST(0x02, 0x00, 0xC4, 0x00, 0x16);
    EXPECT_RESPONSE(0x02, 0x03, 0xAC, 0xDB, 0x35);

    begin("0x02 read discrete inputs, separate from coils");
    REQUEST(0x05, 0x00, 0x00, 0xFF, 0x00); /* coil 0 on */
    REQUEST(0x02, 0x00, 0x00, 0x00, 0x08);
    EXPECT_RESPONSE(0x02, 0x01, 0x00);

    begin("0x02 read discrete inputs, limits");
    REQUEST(0x02, 0x00, 0x00, 0x07, 0xD1);
    EXPECT_RESPONSE(0x82, 0x03);
    REQUEST(0x02, 0x00, 0x00, 0x00, 0x00);
    EXPECT_RESPONSE(0x82, 0x03);
    REQUEST(0x02, 0x07, 0xCF, 0x00, 0x02);
    EXPECT_RESPONSE(0x82, 0x02);
    CHECK(!modbus_set_discrete_input(NUM_BITS, true));
}

static void test_read_holding_registers(void) {
    begin("0x03 read holding registers, spec example");

    /* Registers 108-110 (addresses 107-109) hold 022B 0000 0064 */
    REQUEST(0x10, 0x00, 0x6B, 0x00, 0x03, 0x06, 0x02, 0x2B, 0x00, 0x00, 0x00, 0x64);
    REQUEST(0x03, 0x00, 0x6B, 0x00, 0x03);
    EXPECT_RESPONSE(0x03, 0x06, 0x02, 0x2B, 0x00, 0x00, 0x00, 0x64);

    begin("0x03 read holding registers, limits");
    REQUEST(0x03, 0x00, 0x00, 0x00, 0x7D); /* 125: the maximum */
    CHECK(response_length == 5 + 250);
    REQUEST(0x03, 0x00, 0x00, 0x00, 0x7E); /* 126 */
    EXPECT_RESPONSE(0x83, 0x03);
    REQUEST(0x03, 0x00, 0x00, 0x00, 0x00);
    EXPECT_RESPONSE(0x83, 0x03);
    REQUEST(0x03, 0x00, 0xC7, 0x00, 0x01); /* last register (199) */
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);
    REQUEST(0x03, 0x00, 0xC7, 0x00, 0x02);
    EXPECT_RESPONSE(0x83, 0x02);
}

static void test_read_input_registers(void) {
    begin("0x04 read input registers, spec example");

    /* Input register 9 (address 8) holds 000A */
    CHECK(modbus_set_input_register(8, 0x000A));
    REQUEST(0x04, 0x00, 0x08, 0x00, 0x01);
    EXPECT_RESPONSE(0x04, 0x02, 0x00, 0x0A);

    begin("0x04 read input registers, separate from holding registers");
    REQUEST(0x06, 0x00, 0x08, 0x12, 0x34);
    CHECK(modbus_set_input_register(9, 0xBEEF));
    REQUEST(0x04, 0x00, 0x08, 0x00, 0x02);
    EXPECT_RESPONSE(0x04, 0x04, 0x00, 0x00, 0xBE, 0xEF);

    begin("0x04 read input registers, limits");
    REQUEST(0x04, 0x00, 0x00, 0x00, 0x7E);
    EXPECT_RESPONSE(0x84, 0x03);
    REQUEST(0x04, 0x00, 0x00, 0x00, 0x00);
    EXPECT_RESPONSE(0x84, 0x03);
    REQUEST(0x04, 0x00, 0xC7, 0x00, 0x02);
    EXPECT_RESPONSE(0x84, 0x02);
    CHECK(!modbus_set_input_register(NUM_REGISTERS, 1));
}

static void test_write_single_coil(void) {
    begin("0x05 write single coil, spec example");

    /* Coil 173 (address 172) on; the response echoes the request */
    REQUEST(0x05, 0x00, 0xAC, 0xFF, 0x00);
    EXPECT_RESPONSE(0x05, 0x00, 0xAC, 0xFF, 0x00);
    REQUEST(0x01, 0x00, 0xAC, 0x00, 0x01);
    EXPECT_RESPONSE(0x01, 0x01, 0x01);

    REQUEST(0x05, 0x00, 0xAC, 0x00, 0x00);
    EXPECT_RESPONSE(0x05, 0x00, 0xAC, 0x00, 0x00);
    REQUEST(0x01, 0x00, 0xAC, 0x00, 0x01);
    EXPECT_RESPONSE(0x01, 0x01, 0x00);

    begin("0x05 write single coil, errors");
    REQUEST(0x05, 0x00, 0xAC, 0x00, 0x01); /* neither 0000 nor FF00 */
    EXPECT_RESPONSE(0x85, 0x03);
    REQUEST(0x05, 0x00, 0xAC, 0xFF, 0xFF);
    EXPECT_RESPONSE(0x85, 0x03);
    REQUEST(0x05, 0x07, 0xD0, 0xFF, 0x00); /* address 2000 */
    EXPECT_RESPONSE(0x85, 0x02);
    REQUEST(0x01, 0x00, 0xAC, 0x00, 0x01); /* nothing was written */
    EXPECT_RESPONSE(0x01, 0x01, 0x00);
}

static void test_write_single_register(void) {
    begin("0x06 write single register, spec example");

    /* Register 2 (address 1) = 0003; the response echoes the request */
    REQUEST(0x06, 0x00, 0x01, 0x00, 0x03);
    EXPECT_RESPONSE(0x06, 0x00, 0x01, 0x00, 0x03);
    REQUEST(0x03, 0x00, 0x00, 0x00, 0x03);
    EXPECT_RESPONSE(0x03, 0x06, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00);

    begin("0x06 write single register, errors");
    REQUEST(0x06, 0x00, 0xC8, 0x00, 0x03); /* address 200 */
    EXPECT_RESPONSE(0x86, 0x02);
    REQUEST(0x06, 0x00, 0x01, 0x00); /* truncated */
    EXPECT_RESPONSE(0x86, 0x03);
}

static void test_write_multiple_coils(void) {
    begin("0x0F write multiple coils, spec example");

    /* 10 coils from coil 20 (address 19): CD 01 */
    REQUEST(0x0F, 0x00, 0x13, 0x00, 0x0A, 0x02, 0xCD, 0x01);
    EXPECT_RESPONSE(0x0F, 0x00, 0x13, 0x00, 0x0A);
    REQUEST(0x01, 0x00, 0x13, 0x00, 0x0A);
    EXPECT_RESPONSE(0x01, 0x02, 0xCD, 0x01);
    REQUEST(0x01, 0x00, 0x12, 0x00, 0x0C); /* the neighbours stayed off */
    EXPECT_RESPONSE(0x01, 0x02, 0x9A, 0x03);

    begin("0x0F write multiple coils, errors");
    REQUEST(0x0F, 0x00, 0x13, 0x00, 0x0A, 0x01, 0xCD); /* byte count too small for 10 coils */
    EXPECT_RESPONSE(0x8F, 0x03);
    REQUEST(0x0F, 0x00, 0x13, 0x00, 0x0A, 0x02, 0xCD); /* a data byte is missing */
    EXPECT_RESPONSE(0x8F, 0x03);
    REQUEST(0x0F, 0x00, 0x13, 0x00, 0x00, 0x00); /* quantity 0 */
    EXPECT_RESPONSE(0x8F, 0x03);
    REQUEST(0x0F, 0x07, 0xCF, 0x00, 0x02, 0x01, 0x03); /* runs past the last coil */
    EXPECT_RESPONSE(0x8F, 0x02);
    REQUEST(0x01, 0x07, 0xCF, 0x00, 0x01); /* and wrote nothing */
    EXPECT_RESPONSE(0x01, 0x01, 0x00);

    begin("0x0F write multiple coils, maximum quantity");
    uint8_t pdu[6 + 246];
    memset(pdu, 0xFF, sizeof(pdu));
    pdu[0] = 0x0F;
    pdu[1] = 0x00;
    pdu[2] = 0x00;
    pdu[3] = 0x07; /* 1968 coils */
    pdu[4] = 0xB0;
    pdu[5] = 246;
    CHECK(request_to(SLAVE_ADDRESS, pdu, sizeof(pdu)));
    EXPECT_RESPONSE(0x0F, 0x00, 0x00, 0x07, 0xB0);
    REQUEST(0x01, 0x07, 0xA8, 0x00, 0x10); /* coils 1960-1975: first 8 on, rest off */
    EXPECT_RESPONSE(0x01, 0x02, 0xFF, 0x00);
    pdu[4] = 0xB1; /* 1969 coils need 247 bytes: over the limit */
    CHECK(request_to(SLAVE_ADDRESS, pdu, sizeof(pdu)));
    EXPECT_RESPONSE(0x8F, 0x03);
}

static void test_write_multiple_registers(void) {
    begin("0x10 write multiple registers, spec example");

    /* Registers 2-3 (addresses 1-2) = 000A 0102 */
    REQUEST(0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01, 0x02);
    EXPECT_RESPONSE(0x10, 0x00, 0x01, 0x00, 0x02);
    REQUEST(0x03, 0x00, 0x00, 0x00, 0x04);
    EXPECT_RESPONSE(0x03, 0x08, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x02, 0x00, 0x00);

    begin("0x10 write multiple registers, errors");
    REQUEST(0x10, 0x00, 0x01, 0x00, 0x02, 0x03, 0x00, 0x0A, 0x01); /* byte count is not 2 x quantity */
    EXPECT_RESPONSE(0x90, 0x03);
    REQUEST(0x10, 0x00, 0x01, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01); /* a data byte is missing */
    EXPECT_RESPONSE(0x90, 0x03);
    REQUEST(0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00); /* quantity 0 */
    EXPECT_RESPONSE(0x90, 0x03);
    REQUEST(0x10, 0x00, 0xC7, 0x00, 0x02, 0x04, 0x00, 0x0A, 0x01, 0x02); /* past the last register */
    EXPECT_RESPONSE(0x90, 0x02);
    REQUEST(0x03, 0x00, 0xC7, 0x00, 0x01); /* and wrote nothing */
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);

    begin("0x10 write multiple registers, maximum quantity");
    uint8_t pdu[6 + 248];
    memset(pdu, 0x5A, sizeof(pdu));
    pdu[0] = 0x10;
    pdu[1] = 0x00;
    pdu[2] = 0x00;
    pdu[3] = 0x00;
    pdu[4] = 123;
    pdu[5] = 246;
    CHECK(request_to(SLAVE_ADDRESS, pdu, 6 + 246));
    EXPECT_RESPONSE(0x10, 0x00, 0x00, 0x00, 0x7B);
    REQUEST(0x03, 0x00, 0x7A, 0x00, 0x02); /* registers 122 and 123 */
    EXPECT_RESPONSE(0x03, 0x04, 0x5A, 0x5A, 0x00, 0x00);
    pdu[4] = 124; /* over the limit of 123 */
    CHECK(request_to(SLAVE_ADDRESS, pdu, 6 + 246));
    EXPECT_RESPONSE(0x90, 0x03);
    pdu[5] = 248; /* 124 registers of data make the frame longer than an RTU frame can be */
    CHECK(!request_to(SLAVE_ADDRESS, pdu, 6 + 248));
    CHECK(response_length == 0);
}

static void test_unsupported_function(void) {
    begin("unsupported function code");

    REQUEST(0x07);
    EXPECT_RESPONSE(0x87, 0x01);
    REQUEST(0x2B, 0x0E, 0x01, 0x00);
    EXPECT_RESPONSE(0xAB, 0x01);

    modbus_statistics_t stats = statistics();
    CHECK(stats.frames_received == 2);
    CHECK(stats.frames_sent == 2);
    CHECK(stats.exceptions_sent == 2);
}

/* ----------------------------------------------------------------------------
 * Addressing, broadcast, buffers
 * ------------------------------------------------------------------------- */

static void test_addressing_and_broadcast(void) {
    begin("frames for another slave are ignored");
    const uint8_t write_coil[] = {0x05, 0x00, 0x01, 0xFF, 0x00};
    CHECK(!request_to(0x12, write_coil, sizeof(write_coil)));
    CHECK(response_length == 0);
    REQUEST(0x01, 0x00, 0x01, 0x00, 0x01);
    EXPECT_RESPONSE(0x01, 0x01, 0x00);

    begin("broadcast writes are executed without a response");
    CHECK(request_to(MODBUS_BROADCAST_ADDRESS, write_coil, sizeof(write_coil)));
    CHECK(response_length == 0);
    REQUEST(0x01, 0x00, 0x01, 0x00, 0x01);
    EXPECT_RESPONSE(0x01, 0x01, 0x01);

    begin("broadcast errors are not answered either");
    const uint8_t bad_write[] = {0x05, 0x07, 0xD0, 0xFF, 0x00};
    CHECK(request_to(MODBUS_BROADCAST_ADDRESS, bad_write, sizeof(bad_write)));
    CHECK(response_length == 0);
    CHECK(statistics().exceptions_sent == 0);

    begin("broadcast is ignored when disabled");
    CHECK(modbus_enable_broadcast(false));
    CHECK(!request_to(MODBUS_BROADCAST_ADDRESS, write_coil, sizeof(write_coil)));
    REQUEST(0x01, 0x00, 0x01, 0x00, 0x01);
    EXPECT_RESPONSE(0x01, 0x01, 0x00);
}

static void test_response_buffer_limit(void) {
    begin("response that does not fit the buffer is not written");

    const uint8_t pdu[] = {0x03, 0x00, 0x00, 0x00, 0x02}; /* response frame: 9 bytes */
    uint8_t data[4];
    uint8_t small[9];
    uint16_t length = 0xFFFF;
    modbus_frame_t frame;

    memset(&frame, 0, sizeof(frame));
    frame.address = SLAVE_ADDRESS;
    frame.function_code = pdu[0];
    memcpy(data, &pdu[1], 4);
    frame.data = data;
    frame.data_length = 4;

    memset(small, 0xEE, sizeof(small));
    CHECK(!modbus_process_frame(&frame, small, &length, 8));
    CHECK(length == 0);
    CHECK(small[0] == 0xEE && small[8] == 0xEE);

    CHECK(modbus_process_frame(&frame, small, &length, 9));
    CHECK(length == 9);
}

/* ----------------------------------------------------------------------------
 * Callbacks and data map ownership
 * ------------------------------------------------------------------------- */

static uint16_t callback_register_address;
static uint16_t callback_register_value;
static bool callback_fail;

static bool register_callback_stub(uint16_t address, uint16_t *value, bool write) {
    if (callback_fail) {
        return false;
    }
    if (write) {
        callback_register_address = address;
        callback_register_value = *value;
    } else {
        *value = (uint16_t)(0x1000 + address);
    }
    return true;
}

static bool coil_callback_stub(uint16_t address, bool *value, bool write) {
    if (callback_fail) {
        return false;
    }
    if (!write) {
        *value = (address % 2) == 1;
    }
    return true;
}

static void test_callbacks(void) {
    begin("callbacks serve coils and holding registers");
    callback_fail = false;
    CHECK(modbus_register_register_callback(register_callback_stub));
    CHECK(modbus_register_coil_callback(coil_callback_stub));

    REQUEST(0x03, 0x00, 0x05, 0x00, 0x02);
    EXPECT_RESPONSE(0x03, 0x04, 0x10, 0x05, 0x10, 0x06);
    REQUEST(0x06, 0x00, 0x07, 0xAB, 0xCD);
    EXPECT_RESPONSE(0x06, 0x00, 0x07, 0xAB, 0xCD);
    CHECK(callback_register_address == 7 && callback_register_value == 0xABCD);
    REQUEST(0x10, 0x00, 0x08, 0x00, 0x02, 0x04, 0x00, 0x01, 0x00, 0x02);
    CHECK(callback_register_address == 9 && callback_register_value == 2);
    REQUEST(0x01, 0x00, 0x00, 0x00, 0x08);
    EXPECT_RESPONSE(0x01, 0x01, 0xAA);

    begin("a failing callback is answered with exception 04");
    callback_fail = true;
    CHECK(modbus_register_register_callback(register_callback_stub));
    CHECK(modbus_register_coil_callback(coil_callback_stub));
    REQUEST(0x03, 0x00, 0x05, 0x00, 0x02);
    EXPECT_RESPONSE(0x83, 0x04);
    REQUEST(0x06, 0x00, 0x07, 0xAB, 0xCD);
    EXPECT_RESPONSE(0x86, 0x04);
    REQUEST(0x10, 0x00, 0x08, 0x00, 0x01, 0x02, 0x00, 0x01);
    EXPECT_RESPONSE(0x90, 0x04);
    REQUEST(0x01, 0x00, 0x00, 0x00, 0x08);
    EXPECT_RESPONSE(0x81, 0x04);
    REQUEST(0x05, 0x00, 0x00, 0xFF, 0x00);
    EXPECT_RESPONSE(0x85, 0x04);
    REQUEST(0x0F, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01);
    EXPECT_RESPONSE(0x8F, 0x04);
    callback_fail = false;

    begin("callbacks do not survive a re-initialisation");
    REQUEST(0x03, 0x00, 0x05, 0x00, 0x01);
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);
}

static void test_caller_data_map(void) {
    begin("caller-owned data map");

    static bool coils[16];
    static bool inputs[8];
    static uint16_t holding[4];
    static uint16_t input_registers[2];
    modbus_data_map_t map;

    memset(&map, 0, sizeof(map));
    map.coils = coils;
    map.num_coils = 16;
    map.discrete_inputs = inputs;
    map.num_discrete_inputs = 8;
    map.holding_registers = holding;
    map.num_holding_registers = 4;
    map.input_registers = input_registers;
    map.num_input_registers = 2;
    CHECK(modbus_init_data_map(&map));

    inputs[3] = true;
    input_registers[1] = 0x4711;
    REQUEST(0x02, 0x00, 0x00, 0x00, 0x08);
    EXPECT_RESPONSE(0x02, 0x01, 0x08);
    REQUEST(0x04, 0x00, 0x01, 0x00, 0x01);
    EXPECT_RESPONSE(0x04, 0x02, 0x47, 0x11);
    REQUEST(0x06, 0x00, 0x03, 0x00, 0x2A);
    CHECK(holding[3] == 0x002A);
    REQUEST(0x05, 0x00, 0x0F, 0xFF, 0x00);
    CHECK(coils[15]);

    /* The map's own sizes apply, not the configured maximum */
    REQUEST(0x03, 0x00, 0x04, 0x00, 0x01);
    EXPECT_RESPONSE(0x83, 0x02);
    REQUEST(0x02, 0x00, 0x08, 0x00, 0x01);
    EXPECT_RESPONSE(0x82, 0x02);

    /* Deinit must not free the caller's static arrays (a crash here means it did) */
    CHECK(modbus_rtu_deinit());
    CHECK(holding[3] == 0x002A);

    begin("a table that is missing is reported as illegal address");
    memset(&map, 0, sizeof(map));
    CHECK(modbus_init_data_map(&map));
    REQUEST(0x04, 0x00, 0x00, 0x00, 0x01);
    EXPECT_RESPONSE(0x84, 0x02);
    REQUEST(0x01, 0x00, 0x00, 0x00, 0x01);
    EXPECT_RESPONSE(0x81, 0x02);
}

static void test_master_mode_is_not_a_slave(void) {
    current_test = "master mode does not answer requests";
    modbus_rtu_deinit();
    modbus_config_t config = slave_config();
    config.mode = MODBUS_MODE_MASTER;
    CHECK(modbus_rtu_init(&config));

    const uint8_t pdu[] = {0x03, 0x00, 0x00, 0x00, 0x01};
    CHECK(!request_to(SLAVE_ADDRESS, pdu, sizeof(pdu)));

    modbus_rtu_receive_byte(0x11, 0);
    CHECK(!modbus_rtu_poll(1000000, response, &response_length, sizeof(response)));
}

/* ----------------------------------------------------------------------------
 * RTU frame timing: T1.5 and T3.5
 * ------------------------------------------------------------------------- */

static void test_frame_timing_values(void) {
    current_test = "T1.5 and T3.5 values";
    uint32_t t1_5 = 0;
    uint32_t t3_5 = 0;

    /* 9600 Bd, 8E1: 11 bits per character = 1145.83 us */
    CHECK(modbus_rtu_get_frame_timing(9600, 8, 1, 'E', &t1_5, &t3_5));
    CHECK(t1_5 == 1719 && t3_5 == 4011);

    /* 8N2 is 11 bits as well */
    CHECK(modbus_rtu_get_frame_timing(9600, 8, 2, 'N', &t1_5, &t3_5));
    CHECK(t1_5 == 1719 && t3_5 == 4011);

    /* 8N1: 10 bits per character */
    CHECK(modbus_rtu_get_frame_timing(9600, 8, 1, 'N', &t1_5, &t3_5));
    CHECK(t1_5 == 1563 && t3_5 == 3646);

    /* 19200 Bd is the last rate that is calculated */
    CHECK(modbus_rtu_get_frame_timing(19200, 8, 1, 'E', &t1_5, &t3_5));
    CHECK(t1_5 == 860 && t3_5 == 2006);

    /* Above 19200 Bd the fixed values apply */
    CHECK(modbus_rtu_get_frame_timing(38400, 8, 1, 'E', &t1_5, &t3_5));
    CHECK(t1_5 == 750 && t3_5 == 1750);
    CHECK(modbus_rtu_get_frame_timing(115200, 8, 1, 'N', &t1_5, &t3_5));
    CHECK(t1_5 == 750 && t3_5 == 1750);

    CHECK(!modbus_rtu_get_frame_timing(0, 8, 1, 'E', &t1_5, &t3_5));
    CHECK(!modbus_rtu_get_frame_timing(9600, 8, 1, 'E', NULL, &t3_5));

    /* A configuration without a baud rate cannot be initialised */
    modbus_rtu_deinit();
    modbus_config_t config = slave_config();
    config.baud_rate = 0;
    CHECK(!modbus_rtu_init(&config));
}

/* Timing of the fixture (9600 Bd, 8E1) */
#define T1_5_US 1719u
#define T3_5_US 4011u
#define CHAR_US 1146u /* one character back to back */
/* modbus_rtu_poll() closes a frame this long after its last character: T3.5 of
 * silence, plus the time a character that began inside T3.5 needs to be reported. */
#define FRAME_END_US (T3_5_US + CHAR_US)

/** Read holding register 0 from the test slave, with CRC. */
static const uint8_t READ_REGISTER_0[] = {0x11, 0x03, 0x00, 0x00, 0x00, 0x01, 0x86, 0x9A};

/**
 * Feeds bytes back to back. Timestamps mark the end of each character, as a
 * UART receive-complete interrupt reports them; *now_us is left at the last one.
 */
static void feed(const uint8_t *bytes, uint16_t count, uint32_t *now_us) {
    for (uint16_t i = 0; i < count; i++) {
        if (i > 0) {
            *now_us += CHAR_US;
        }
        modbus_rtu_receive_byte(bytes[i], *now_us);
    }
}

static bool poll_at(uint32_t now_us) {
    memset(response, 0xEE, sizeof(response));
    response_length = 0xFFFF;
    return modbus_rtu_poll(now_us, response, &response_length, sizeof(response));
}

static void test_receiver_frame_end(void) {
    begin("receiver: frame ends after T3.5 of silence");
    uint32_t now = 500000;

    /* The CRC of the fixture frame is what the module itself calculates */
    CHECK(modbus_calculate_crc(READ_REGISTER_0, 6) == 0x9A86);

    CHECK(!poll_at(now)); /* nothing received yet */
    feed(READ_REGISTER_0, sizeof(READ_REGISTER_0), &now);

    CHECK(!poll_at(now));           /* right after the last character */
    CHECK(!poll_at(now + T3_5_US)); /* T3.5 passed, but a character could still be arriving */
    CHECK(!poll_at(now + FRAME_END_US - 1));
    CHECK(response_length == 0);
    CHECK(poll_at(now + FRAME_END_US)); /* T3.5 of silence is certain: frame is answered */
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);

    CHECK(!poll_at(now + 2 * FRAME_END_US)); /* and only once */
    CHECK(statistics().frames_received == 1);
}

static void test_receiver_inter_character_gap(void) {
    begin("receiver: a gap of exactly T1.5 inside a frame is accepted");
    uint32_t now = 0;
    feed(READ_REGISTER_0, 4, &now);
    now += T1_5_US + CHAR_US; /* silence, then the next character itself */
    feed(&READ_REGISTER_0[4], 4, &now);
    CHECK(poll_at(now + FRAME_END_US));
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);

    begin("receiver: silence is measured, not the distance between characters");
    /* 1000 us of silence at 9600 Bd is legal (T1.5 = 1719 us), although the
     * characters then end 2146 us apart. */
    now = 0;
    for (uint16_t i = 0; i < sizeof(READ_REGISTER_0); i++) {
        modbus_rtu_receive_byte(READ_REGISTER_0[i], now);
        now += 1000 + CHAR_US;
    }
    now -= 1000 + CHAR_US;
    CHECK(poll_at(now + FRAME_END_US));
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);
    CHECK(statistics().framing_errors == 0);

    begin("receiver: a gap of more than T1.5 inside a frame discards the frame");
    now = 0;
    feed(READ_REGISTER_0, 4, &now);
    now += T1_5_US + 1 + CHAR_US;
    feed(&READ_REGISTER_0[4], 4, &now);
    CHECK(!poll_at(now + FRAME_END_US));
    CHECK(response_length == 0);
    modbus_statistics_t stats = statistics();
    CHECK(stats.framing_errors == 1);
    CHECK(stats.frames_received == 0);
    CHECK(stats.crc_errors == 0);

    /* The receiver recovers with the next frame */
    now += 10 * T3_5_US;
    feed(READ_REGISTER_0, sizeof(READ_REGISTER_0), &now);
    CHECK(poll_at(now + FRAME_END_US));
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);

    begin("receiver: a gap just under T3.5 does not split one frame into two");
    now = 0;
    feed(READ_REGISTER_0, sizeof(READ_REGISTER_0), &now);
    now += T3_5_US - 1 + CHAR_US;
    feed(READ_REGISTER_0, sizeof(READ_REGISTER_0), &now);
    CHECK(!poll_at(now + FRAME_END_US));
    CHECK(statistics().framing_errors == 1);
}

static void test_receiver_bad_frames(void) {
    begin("receiver: CRC error");
    uint32_t now = 0;
    uint8_t corrupted[sizeof(READ_REGISTER_0)];
    memcpy(corrupted, READ_REGISTER_0, sizeof(corrupted));
    corrupted[5] ^= 0x01;
    feed(corrupted, sizeof(corrupted), &now);
    CHECK(!poll_at(now + FRAME_END_US));
    CHECK(response_length == 0);
    CHECK(statistics().crc_errors == 1);

    begin("receiver: frame shorter than 4 characters");
    now = 0;
    feed(READ_REGISTER_0, 3, &now);
    CHECK(!poll_at(now + FRAME_END_US));
    CHECK(statistics().framing_errors == 1);

    begin("receiver: frame longer than 256 characters");
    now = 0;
    for (uint16_t i = 0; i < MODBUS_MAX_FRAME_SIZE + 1; i++) {
        modbus_rtu_receive_byte(0x55, now);
        now += CHAR_US;
    }
    CHECK(!poll_at(now + FRAME_END_US));
    CHECK(statistics().framing_errors == 1);
    now += 10 * T3_5_US;
    feed(READ_REGISTER_0, sizeof(READ_REGISTER_0), &now);
    CHECK(poll_at(now + FRAME_END_US));

    begin("receiver: frame for another slave is dropped silently");
    now = 0;
    uint8_t other[sizeof(READ_REGISTER_0)];
    memcpy(other, READ_REGISTER_0, sizeof(other));
    other[0] = 0x12;
    uint16_t crc = modbus_calculate_crc(other, 6);
    other[6] = (uint8_t)(crc & 0xFF);
    other[7] = (uint8_t)(crc >> 8);
    feed(other, sizeof(other), &now);
    CHECK(!poll_at(now + FRAME_END_US));
    modbus_statistics_t stats = statistics();
    CHECK(stats.crc_errors == 0 && stats.framing_errors == 0 && stats.frames_received == 0);
}

static void test_receiver_back_to_back_frames(void) {
    begin("receiver: a new frame after T3.5 replaces an unpolled one");
    uint32_t now = 0;
    const uint8_t write_register[] = {0x11, 0x06, 0x00, 0x01, 0x00, 0x03};
    uint8_t first[8];
    memcpy(first, write_register, 6);
    uint16_t crc = modbus_calculate_crc(first, 6);
    first[6] = (uint8_t)(crc & 0xFF);
    first[7] = (uint8_t)(crc >> 8);

    feed(first, sizeof(first), &now);
    now += T3_5_US + CHAR_US; /* T3.5 of silence: frame complete, but poll is late */
    feed(READ_REGISTER_0, sizeof(READ_REGISTER_0), &now);
    CHECK(poll_at(now + FRAME_END_US));
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00); /* the answer belongs to the second frame */
    CHECK(statistics().overrun_errors == 1);

    /* The lost write was not executed */
    REQUEST(0x03, 0x00, 0x01, 0x00, 0x01);
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);
}

static void test_receiver_poll_during_a_character(void) {
    begin("receiver: a poll while a late character is arriving does not close the frame");
    uint32_t now = 0;
    const uint8_t write_register[] = {0x11, 0x06, 0x00, 0x01, 0x00, 0x03};
    uint8_t frame[8];
    memcpy(frame, write_register, 6);
    uint16_t crc = modbus_calculate_crc(frame, 6);
    frame[6] = (uint8_t)(crc & 0xFF);
    frame[7] = (uint8_t)(crc >> 8);

    feed(frame, sizeof(frame), &now);

    /* A stray character starts 3500 us after the frame: less than T3.5, so it
     * belongs to the frame and makes it invalid. It is reported when complete,
     * 4646 us after the frame. A poll in between, after T3.5, must wait. */
    CHECK(!poll_at(now + T3_5_US));
    modbus_rtu_receive_byte(0x00, now + 3500 + CHAR_US);
    now += 3500 + CHAR_US;
    CHECK(!poll_at(now + FRAME_END_US));
    CHECK(statistics().framing_errors == 1);

    /* The write of the invalid frame was not executed */
    REQUEST(0x03, 0x00, 0x01, 0x00, 0x01);
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);
}

static void test_receiver_broadcast_and_wraparound(void) {
    begin("receiver: broadcast is executed and not answered");
    uint32_t now = 0;
    uint8_t frame[8] = {0x00, 0x06, 0x00, 0x02, 0x12, 0x34};
    uint16_t crc = modbus_calculate_crc(frame, 6);
    frame[6] = (uint8_t)(crc & 0xFF);
    frame[7] = (uint8_t)(crc >> 8);
    feed(frame, sizeof(frame), &now);
    CHECK(!poll_at(now + FRAME_END_US)); /* nothing to transmit */
    CHECK(response_length == 0);
    REQUEST(0x03, 0x00, 0x02, 0x00, 0x01);
    EXPECT_RESPONSE(0x03, 0x02, 0x12, 0x34);

    begin("receiver: the microsecond counter may wrap around inside a frame");
    now = 0xFFFFFFFFu - 3 * CHAR_US; /* wraps after the fourth character */
    feed(READ_REGISTER_0, sizeof(READ_REGISTER_0), &now);
    CHECK(now < 8 * CHAR_US); /* the counter did wrap */
    CHECK(!poll_at(now + FRAME_END_US - 1));
    CHECK(poll_at(now + FRAME_END_US));
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);
    CHECK(statistics().framing_errors == 0);

    begin("receiver: fixed timing above 19200 Bd");
    modbus_rtu_deinit();
    modbus_config_t config = slave_config();
    config.baud_rate = 115200;
    CHECK(modbus_rtu_init(&config));
    now = 0;
    for (uint16_t i = 0; i < sizeof(READ_REGISTER_0); i++) {
        modbus_rtu_receive_byte(READ_REGISTER_0[i], now);
        now += 96; /* 11 bits at 115200 Bd */
    }
    now -= 96;
    CHECK(!poll_at(now + 1750 + 96 - 1));
    CHECK(poll_at(now + 1750 + 96));
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);

    begin("receiver: timing follows a change of the communication parameters");
    modbus_rtu_deinit();
    config.baud_rate = 115200;
    CHECK(modbus_rtu_init(&config));
    modbus_rtu_receive_byte(0x11, 0); /* a frame in the old format is dropped */
    CHECK(modbus_set_comm_params(9600, 8, 1, 'E'));
    now = 100000;
    feed(READ_REGISTER_0, sizeof(READ_REGISTER_0), &now); /* 1146 us apart: over the old T1.5 of 750 us */
    CHECK(!poll_at(now + 1750 + 96));                     /* the old timing no longer applies */
    CHECK(poll_at(now + FRAME_END_US));
    EXPECT_RESPONSE(0x03, 0x02, 0x00, 0x00);
    CHECK(statistics().framing_errors == 0);
    CHECK(!modbus_set_comm_params(0, 8, 1, 'E'));
}

/* ------------------------------------------------------------------------- */

int main(void) {
    test_crc();
    test_read_coils();
    test_read_discrete_inputs();
    test_read_holding_registers();
    test_read_input_registers();
    test_write_single_coil();
    test_write_single_register();
    test_write_multiple_coils();
    test_write_multiple_registers();
    test_unsupported_function();
    test_addressing_and_broadcast();
    test_response_buffer_limit();
    test_callbacks();
    test_caller_data_map();
    test_master_mode_is_not_a_slave();
    test_frame_timing_values();
    test_receiver_frame_end();
    test_receiver_inter_character_gap();
    test_receiver_bad_frames();
    test_receiver_back_to_back_frames();
    test_receiver_poll_during_a_character();
    test_receiver_broadcast_and_wraparound();

    modbus_rtu_deinit();

    printf("%d checks, %d failed\n", checks_run, checks_failed);
    return checks_failed == 0 ? 0 : 1;
}
