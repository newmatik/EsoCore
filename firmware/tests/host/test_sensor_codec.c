/**
 * @file test_sensor_codec.c
 * @brief Host unit tests for the sensor reading payloads (common/sensors/sensor_codec.c, stm32/sensors/<module>_codec.c)
 *
 * For every module: a reading filled with deterministic values survives
 * encode -> decode -> encode byte for byte, the encoded size is the documented
 * one, bad payloads are refused, and fields sit at the byte offsets that
 * firmware/README.md documents. Built and run with `make test` in firmware/.
 */

#include "sensor_codec.h"
#include "../../stm32/sensors/current_sensor.h"
#include "../../stm32/sensors/vibration_sensor.h"

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

/* Large enough for every module's payload */
#define BUFFER_SIZE 16384U

static uint8_t bytes_a[BUFFER_SIZE];
static uint8_t bytes_b[BUFFER_SIZE];

static uint32_t read_u32(const uint8_t *bytes) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static float read_f32(const uint8_t *bytes) {
    uint32_t bits = read_u32(bytes);
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

/* One module under test: its walk and its public encode/decode functions */
typedef struct {
    const char *name;
    uint8_t device_type;
    uint32_t wire_size;
    void *a;
    void *b;
    size_t struct_size;
    void (*fill)(esocore_codec_t *codec, void *data);
    uint32_t (*encode)(const void *data, uint8_t *buffer, uint32_t capacity);
    bool (*decode)(const uint8_t *buffer, uint32_t length, void *data);
} module_t;

/* Generic checks every module must pass */
static void check_module(const module_t *m) {
    current_test = m->name;
    esocore_codec_t codec;

    /* Fill, encode: documented size and header */
    memset(m->a, 0, m->struct_size);
    esocore_codec_init_fill(&codec, 12345U);
    m->fill(&codec, m->a);
    CHECK(codec.ok);
    memset(bytes_a, 0xEE, sizeof(bytes_a));
    uint32_t length = m->encode(m->a, bytes_a, BUFFER_SIZE);
    CHECK(length == m->wire_size);
    CHECK(bytes_a[0] == m->device_type);
    CHECK(bytes_a[1] == ESOCORE_SENSOR_PAYLOAD_VERSION);
    CHECK(bytes_a[length] == 0xEE); /* nothing written past the payload */

    /* Decode into a different struct and encode again: the same bytes */
    memset(m->b, 0, m->struct_size);
    CHECK(m->decode(bytes_a, length, m->b));
    memset(bytes_b, 0, sizeof(bytes_b));
    CHECK(m->encode(m->b, bytes_b, BUFFER_SIZE) == length);
    CHECK(memcmp(bytes_a, bytes_b, length) == 0);

    /* Another seed gives other bytes: the walk reaches the fields */
    esocore_codec_init_fill(&codec, 999U);
    m->fill(&codec, m->b);
    CHECK(m->encode(m->b, bytes_b, BUFFER_SIZE) == length);
    CHECK(memcmp(bytes_a + ESOCORE_SENSOR_PAYLOAD_HEADER_SIZE, bytes_b + ESOCORE_SENSOR_PAYLOAD_HEADER_SIZE,
                 length - ESOCORE_SENSOR_PAYLOAD_HEADER_SIZE) != 0);

    /* Too small a buffer: nothing encoded */
    CHECK(m->encode(m->a, bytes_b, length - 1) == 0);

    /* Refused payloads */
    CHECK(!m->decode(bytes_a, length - 1, m->b));
    CHECK(!m->decode(bytes_a, length + 1, m->b));
    CHECK(!m->decode(bytes_a, 0, m->b));
    memcpy(bytes_b, bytes_a, length);
    bytes_b[0] = (uint8_t)(m->device_type + 1U);
    CHECK(!m->decode(bytes_b, length, m->b));
    memcpy(bytes_b, bytes_a, length);
    bytes_b[1] = ESOCORE_SENSOR_PAYLOAD_VERSION + 1U;
    CHECK(!m->decode(bytes_b, length, m->b));
}

/* ---------------------------------------------------------------------------
 * Shared encodings
 * ------------------------------------------------------------------------- */

static void test_primitives(void) {
    current_test = "primitives";
    uint8_t buffer[32];
    esocore_codec_t codec;
    esocore_codec_init_encode(&codec, buffer, sizeof(buffer));
    uint32_t u32 = 0x11223344U;
    int16_t i16 = -2;
    float f32 = 1.0f;
    bool flag = true;
    char text[6] = {'a', 'b', '\0', 'z', 'z', 'z'};
    esocore_codec_u32(&codec, &u32);
    esocore_codec_i16(&codec, &i16);
    esocore_codec_f32(&codec, &f32);
    esocore_codec_bool(&codec, &flag);
    esocore_codec_chars(&codec, text, sizeof(text));
    CHECK(codec.ok);
    CHECK(codec.position == 4 + 2 + 4 + 1 + 6);
    const uint8_t expected[] = {0x44, 0x33, 0x22, 0x11, 0xFE, 0xFF, 0x00, 0x00, 0x80, 0x3F,
                                0x01, 'a',  'b',  0x00, 0x00, 0x00, 0x00};
    CHECK(memcmp(buffer, expected, sizeof(expected)) == 0);

    /* Decoding: text always terminated, bool above 1 refused */
    uint8_t in[] = {'x', 'y', 'z', 'w', 0x02};
    char out[4];
    bool value = false;
    esocore_codec_init_decode(&codec, in, sizeof(in));
    esocore_codec_chars(&codec, out, sizeof(out));
    CHECK(codec.ok && strcmp(out, "xyz") == 0);
    esocore_codec_bool(&codec, &value);
    CHECK(!codec.ok);

    /* Overrun: ok turns false and stays false */
    esocore_codec_init_encode(&codec, buffer, 3);
    esocore_codec_u32(&codec, &u32);
    CHECK(!codec.ok);
    esocore_codec_init_decode(&codec, in, 1);
    esocore_codec_u16(&codec, (uint16_t *)&i16);
    CHECK(!codec.ok);
}

static void test_base_data_layout(void) {
    current_test = "base_data_layout";
    static esocore_sensor_data_t data;
    memset(&data, 0, sizeof(data));
    data.timestamp = 0xA1B2C3D4U;
    data.sensor_type = ESOCORE_SENSOR_CURRENT;
    data.sensor_id = 7;
    data.data_points = 0x0102;
    data.data_type = ESOCORE_DATA_TYPE_FLOAT;
    strcpy(data.units, "mm/s");
    data.quality_flags = 0x5A;
    data.data_size = 0x0304;
    data.data[0] = 0x99;
    data.data[ESOCORE_SENSOR_DATA_BUFFER_SIZE - 1] = 0x77;

    static uint8_t buffer[ESOCORE_SENSOR_DATA_WIRE_SIZE];
    esocore_codec_t codec;
    esocore_codec_init_encode(&codec, buffer, sizeof(buffer));
    esocore_codec_sensor_data(&codec, &data);
    CHECK(codec.ok);
    CHECK(codec.position == ESOCORE_SENSOR_DATA_WIRE_SIZE);
    CHECK(ESOCORE_SENSOR_DATA_WIRE_SIZE == 276);
    CHECK(read_u32(buffer + 0) == 0xA1B2C3D4U);
    CHECK(buffer[4] == ESOCORE_SENSOR_CURRENT);
    CHECK(buffer[5] == 7);
    CHECK(buffer[6] == 0x02 && buffer[7] == 0x01);
    CHECK(buffer[8] == ESOCORE_DATA_TYPE_FLOAT);
    CHECK(memcmp(buffer + 9, "mm/s\0\0\0\0", 8) == 0);
    CHECK(buffer[17] == 0x5A);
    CHECK(buffer[18] == 0x04 && buffer[19] == 0x03);
    CHECK(buffer[20] == 0x99);
    CHECK(buffer[275] == 0x77);
}

/* ---------------------------------------------------------------------------
 * Vibration
 * ------------------------------------------------------------------------- */

static vibration_sensor_data_t vibration_a;
static vibration_sensor_data_t vibration_b;

static void vibration_fill(esocore_codec_t *codec, void *data) {
    vibration_sensor_codec(codec, data);
}
static uint32_t vibration_encode(const void *data, uint8_t *buffer, uint32_t capacity) {
    return vibration_sensor_encode_data(data, buffer, capacity);
}
static bool vibration_decode(const uint8_t *buffer, uint32_t length, void *data) {
    return vibration_sensor_decode_data(buffer, length, data);
}

static void test_vibration(void) {
    const module_t m = {"vibration", ESOCORE_DEVICE_TYPE_VIBRATION, VIBRATION_SENSOR_DATA_WIRE_SIZE,
                        &vibration_a, &vibration_b, sizeof(vibration_a),
                        vibration_fill, vibration_encode, vibration_decode};
    check_module(&m);

    /* Section offsets of the README table */
    current_test = "vibration_layout";
    memset(&vibration_a, 0, sizeof(vibration_a));
    vibration_a.base_data.timestamp = 0x01020304U;
    vibration_a.raw_data.x_axis_raw = -1;
    vibration_a.processed_data.x_axis_acceleration = 2.5f;
    vibration_a.fft_data.frequency_bins[0] = 3.5f;
    vibration_a.bearing_analysis.bearing_fault_detected = true;
    vibration_a.gear_analysis.gear_fault_detected = true;
    vibration_a.overall_condition = 88;
    strcpy(vibration_a.condition_description, "ok");
    CHECK(vibration_sensor_encode_data(&vibration_a, bytes_a, BUFFER_SIZE) == 1501);
    CHECK(read_u32(bytes_a + 2) == 0x01020304U);
    CHECK(read_u32(bytes_a + 278) == 0xFFFFFFFFU);
    CHECK(read_f32(bytes_a + 296) == 2.5f);
    CHECK(read_f32(bytes_a + 348) == 3.5f);
    CHECK(bytes_a[1394] == 1);
    CHECK(bytes_a[1418] == 1);
    CHECK(bytes_a[1436] == 88);
    CHECK(memcmp(bytes_a + 1437, "ok", 3) == 0);

    /* A bool byte other than 0 or 1 is refused */
    bytes_a[1394] = 2;
    CHECK(!vibration_sensor_decode_data(bytes_a, 1501, &vibration_b));
}

/* ---------------------------------------------------------------------------
 * Current
 * ------------------------------------------------------------------------- */

static current_sensor_data_t current_a;
static current_sensor_data_t current_b;

static void current_fill(esocore_codec_t *codec, void *data) {
    current_sensor_codec(codec, data);
}
static uint32_t current_encode(const void *data, uint8_t *buffer, uint32_t capacity) {
    return current_sensor_encode_data(data, buffer, capacity);
}
static bool current_decode(const uint8_t *buffer, uint32_t length, void *data) {
    return current_sensor_decode_data(buffer, length, data);
}

static void test_current(void) {
    const module_t m = {"current", ESOCORE_DEVICE_TYPE_CURRENT, CURRENT_SENSOR_DATA_WIRE_SIZE,
                        &current_a, &current_b, sizeof(current_a),
                        current_fill, current_encode, current_decode};
    check_module(&m);

    current_test = "current_layout";
    memset(&current_a, 0, sizeof(current_a));
    current_a.raw_data.current_samples[0] = -3;
    current_a.raw_data.current_samples[CURRENT_SENSOR_MAX_SAMPLES - 1] = 0x1234;
    current_a.processed_data.rms_current_a = 4.25f;
    current_a.harmonic_data.harmonic_magnitudes[0] = 5.5f;
    current_a.power_data.power_factor = 0.75f;
    current_a.motor_data.motor_connected = true;
    current_a.current_condition = 42;
    CHECK(current_sensor_encode_data(&current_a, bytes_a, BUFFER_SIZE) == 2689);
    CHECK(bytes_a[278] == 0xFD && bytes_a[279] == 0xFF);
    CHECK(bytes_a[278 + 2046] == 0x34 && bytes_a[278 + 2047] == 0x12);
    CHECK(read_f32(bytes_a + 2335) == 4.25f);
    CHECK(read_f32(bytes_a + 2367) == 5.5f);
    CHECK(read_f32(bytes_a + 2572) == 0.75f);
    CHECK(bytes_a[2604] == 1);
    CHECK(bytes_a[2624] == 42);
}

int main(void) {
    test_primitives();
    test_base_data_layout();
    test_vibration();
    test_current();

    printf("sensor codec tests: %d checks, %d failed\n", checks_run, checks_failed);
    return checks_failed == 0 ? 0 : 1;
}
