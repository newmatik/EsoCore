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
#include "../../stm32/sensors/acoustic_sensor.h"
#include "../../stm32/sensors/air_quality_sensor.h"
#include "../../stm32/sensors/current_sensor.h"
#include "../../stm32/sensors/light_sensor.h"
#include "../../stm32/sensors/oil_quality_sensor.h"
#include "../../stm32/sensors/pressure_sensor.h"
#include "../../stm32/sensors/proximity_position_sensor.h"
#include "../../stm32/sensors/temperature_spot_sensor.h"
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

    /* Decode into a different, uninitialised-looking struct (bool bytes that
     * are not 0 or 1 must not be read) and encode again: the same bytes */
    memset(m->b, 0xA5, m->struct_size);
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

/* Encoding only reads the reading: a const one in read-only memory works */
static const vibration_sensor_data_t vibration_const;
static const current_sensor_data_t current_const;

static void test_const_readings(void) {
    current_test = "const_readings";
    CHECK(vibration_sensor_encode_data(&vibration_const, bytes_a, BUFFER_SIZE) == VIBRATION_SENSOR_DATA_WIRE_SIZE);
    CHECK(bytes_a[0] == ESOCORE_DEVICE_TYPE_VIBRATION && bytes_a[2] == 0 && bytes_a[VIBRATION_SENSOR_DATA_WIRE_SIZE - 1] == 0);
    CHECK(current_sensor_encode_data(&current_const, bytes_a, BUFFER_SIZE) == CURRENT_SENSOR_DATA_WIRE_SIZE);
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

/* ---------------------------------------------------------------------------
 * Acoustic, air quality, light
 * ------------------------------------------------------------------------- */

#define MODULE_GLUE(prefix, type)                                                             \
    static type prefix##_a;                                                                   \
    static type prefix##_b;                                                                   \
    static void prefix##_fill(esocore_codec_t *codec, void *data) {                          \
        prefix##_sensor_codec(codec, data);                                                   \
    }                                                                                         \
    static uint32_t prefix##_encode(const void *data, uint8_t *buffer, uint32_t capacity) {   \
        return prefix##_sensor_encode_data(data, buffer, capacity);                           \
    }                                                                                         \
    static bool prefix##_decode(const uint8_t *buffer, uint32_t length, void *data) {         \
        return prefix##_sensor_decode_data(buffer, length, data);                             \
    }

MODULE_GLUE(acoustic, acoustic_sensor_data_t)
MODULE_GLUE(air_quality, air_quality_sensor_data_t)
MODULE_GLUE(light, light_sensor_data_t)

static void test_acoustic(void) {
    const module_t m = {"acoustic", ESOCORE_DEVICE_TYPE_ACOUSTIC, ACOUSTIC_SENSOR_DATA_WIRE_SIZE,
                        &acoustic_a, &acoustic_b, sizeof(acoustic_a),
                        acoustic_fill, acoustic_encode, acoustic_decode};
    check_module(&m);

    current_test = "acoustic_layout";
    memset(&acoustic_a, 0, sizeof(acoustic_a));
    acoustic_a.raw_data.audio_samples[0] = -2;
    acoustic_a.processed_data.rms_level_db = 61.5f;
    acoustic_a.fft_data.frequency_bins[0] = 7.25f;
    acoustic_a.frequency_bands.band_31_5_hz = 30.5f;
    acoustic_a.ultrasound_data.ultrasound_detected = true;
    acoustic_a.acoustic_condition = 64;
    strcpy(acoustic_a.condition_description, "quiet");
    CHECK(acoustic_sensor_encode_data(&acoustic_a, bytes_a, BUFFER_SIZE) == 6602);
    CHECK(bytes_a[278] == 0xFE && bytes_a[279] == 0xFF);
    CHECK(read_f32(bytes_a + 4383) == 61.5f);
    CHECK(read_f32(bytes_a + 4415) == 7.25f);
    CHECK(read_f32(bytes_a + 6489) == 30.5f);
    CHECK(bytes_a[6525] == 1);
    CHECK(bytes_a[6537] == 64);
    CHECK(memcmp(bytes_a + 6538, "quiet", 6) == 0);
}

static void test_air_quality(void) {
    const module_t m = {"air_quality", ESOCORE_DEVICE_TYPE_AIR_QUALITY, AIR_QUALITY_SENSOR_DATA_WIRE_SIZE,
                        &air_quality_a, &air_quality_b, sizeof(air_quality_a),
                        air_quality_fill, air_quality_encode, air_quality_decode};
    check_module(&m);

    current_test = "air_quality_layout";
    memset(&air_quality_a, 0, sizeof(air_quality_a));
    air_quality_a.gas_sensors[0].gas_type = 3;
    air_quality_a.gas_sensors[1].gas_type = 4; /* each gas entry is 24 bytes */
    air_quality_a.gas_sensors[AIR_QUALITY_MAX_GASES - 1].timestamp = 0xCAFEF00DU;
    air_quality_a.pm_data.pm1_0 = 12.5f;
    air_quality_a.env_data.temperature_celsius = 21.5f;
    air_quality_a.iaq_data.iaq_index = 0x0203;
    air_quality_a.health_data.overall_risk_level = 9;
    strcpy(air_quality_a.health_data.recommendations, "ventilate");
    air_quality_a.air_quality_score = 77;
    CHECK(air_quality_sensor_encode_data(&air_quality_a, bytes_a, BUFFER_SIZE) == 985);
    CHECK(bytes_a[278] == 3);
    CHECK(bytes_a[302] == 4);
    CHECK(read_u32(bytes_a + 466) == 0xCAFEF00DU);
    CHECK(read_f32(bytes_a + 470) == 12.5f);
    CHECK(read_f32(bytes_a + 485) == 21.5f);
    CHECK(bytes_a[505] == 0x03 && bytes_a[506] == 0x02);
    CHECK(bytes_a[528] == 9);
    CHECK(memcmp(bytes_a + 660, "ventilate", 10) == 0);
    CHECK(bytes_a[920] == 77);
}

static void test_light(void) {
    const module_t m = {"light", ESOCORE_DEVICE_TYPE_LIGHT, LIGHT_SENSOR_DATA_WIRE_SIZE,
                        &light_a, &light_b, sizeof(light_a),
                        light_fill, light_encode, light_decode};
    check_module(&m);

    current_test = "light_layout";
    memset(&light_a, 0, sizeof(light_a));
    light_a.visible_data.lux = 500.0f; /* light_sensor_data_t has no base_data */
    light_a.uv_data.uva_watts_per_m2 = 1.5f;
    light_a.color_data.red_counts = 0x0A0B;
    light_a.spectral_data.channel_data[0] = 2.5f;
    light_a.ir_data.object_temperature_celsius = 36.5f;
    light_a.workplace_compliance.illuminance_compliant = true;
    strcpy(light_a.environmental_analysis.pollution_level, "low");
    light_a.light_quality_score = 90;
    light_a.illuminance_lux = 480.0f;
    light_a.uva_intensity_uwm2 = 3.5f;
    light_a.object_temperature_c = 30.5f;
    light_a.red_raw = 0x0C0D;
    light_a.spectral_channels[0] = 0x0E0F;
    light_a.sensor_status = 0x1F;
    light_a.timestamp = 0x0A0B0C0DU;
    CHECK(light_sensor_encode_data(&light_a, bytes_a, BUFFER_SIZE) == 519);
    CHECK(read_f32(bytes_a + 2) == 500.0f);
    CHECK(read_f32(bytes_a + 18) == 1.5f);
    CHECK(bytes_a[30] == 0x0B && bytes_a[31] == 0x0A);
    CHECK(read_f32(bytes_a + 46) == 2.5f);
    CHECK(read_f32(bytes_a + 87) == 36.5f);
    CHECK(bytes_a[95] == 1);
    CHECK(memcmp(bytes_a + 247, "low", 4) == 0);
    CHECK(bytes_a[367] == 90);
    CHECK(read_f32(bytes_a + 432) == 480.0f);
    CHECK(read_f32(bytes_a + 445) == 3.5f);
    CHECK(read_f32(bytes_a + 458) == 30.5f);
    CHECK(bytes_a[470] == 0x0D && bytes_a[471] == 0x0C);
    CHECK(bytes_a[486] == 0x0F && bytes_a[487] == 0x0E);
    CHECK(bytes_a[514] == 0x1F);
    CHECK(read_u32(bytes_a + 515) == 0x0A0B0C0DU);
}

/* ---------------------------------------------------------------------------
 * Oil quality, pressure, proximity/position, temperature spot
 * ------------------------------------------------------------------------- */

MODULE_GLUE(oil_quality, oil_quality_sensor_data_t)
MODULE_GLUE(pressure, pressure_sensor_data_t)
MODULE_GLUE(proximity_position, proximity_position_sensor_data_t)
MODULE_GLUE(temperature_spot, temperature_spot_sensor_data_t)

static void test_oil_quality(void) {
    const module_t m = {"oil_quality", ESOCORE_DEVICE_TYPE_OIL_QUALITY, OIL_QUALITY_SENSOR_DATA_WIRE_SIZE,
                        &oil_quality_a, &oil_quality_b, sizeof(oil_quality_a),
                        oil_quality_fill, oil_quality_encode, oil_quality_decode};
    check_module(&m);

    current_test = "oil_quality_layout";
    memset(&oil_quality_a, 0, sizeof(oil_quality_a));
    oil_quality_a.turbidity_ntu = 1.5f; /* no base_data */
    oil_quality_a.timestamp = 0x01020304U;
    oil_quality_a.turbidity_data.turbidity_ntu = 2.5f;
    oil_quality_a.viscosity_data.viscosity_cst = 46.5f;
    oil_quality_a.water_data.water_content_ppm = 120.5f;
    oil_quality_a.dielectric_data.dielectric_constant = 2.25f;
    oil_quality_a.temperature_data.temperature_celsius = 55.5f;
    strcpy(oil_quality_a.degradation_analysis.turbidity_assessment, "clear");
    oil_quality_a.degradation_analysis.contamination_level = 2;
    strcpy(oil_quality_a.maintenance_recommendations.immediate_actions, "none");
    oil_quality_a.maintenance_recommendations.change_interval_days = 0x0102;
    oil_quality_a.oil_quality_score = 81;
    CHECK(oil_quality_sensor_encode_data(&oil_quality_a, bytes_a, BUFFER_SIZE) == 791);
    CHECK(read_f32(bytes_a + 2) == 1.5f);
    CHECK(read_u32(bytes_a + 22) == 0x01020304U);
    CHECK(read_f32(bytes_a + 26) == 2.5f);
    CHECK(read_f32(bytes_a + 34) == 46.5f);
    CHECK(read_f32(bytes_a + 42) == 120.5f);
    CHECK(read_f32(bytes_a + 54) == 2.25f);
    CHECK(read_f32(bytes_a + 62) == 55.5f);
    CHECK(memcmp(bytes_a + 68, "clear", 6) == 0);
    CHECK(bytes_a[132] == 2);
    CHECK(memcmp(bytes_a + 336, "none", 5) == 0);
    CHECK(bytes_a[720] == 0x02 && bytes_a[721] == 0x01);
    CHECK(bytes_a[726] == 81);
}

static void test_pressure(void) {
    const module_t m = {"pressure", ESOCORE_DEVICE_TYPE_PRESSURE, PRESSURE_SENSOR_DATA_WIRE_SIZE,
                        &pressure_a, &pressure_b, sizeof(pressure_a),
                        pressure_fill, pressure_encode, pressure_decode};
    check_module(&m);

    current_test = "pressure_layout";
    memset(&pressure_a, 0, sizeof(pressure_a));
    pressure_a.pressure_pa = 101325.0f; /* no base_data */
    pressure_a.raw_adc_value = 0x0FFF;
    strcpy(pressure_a.hydraulic_analysis.pump_performance, "good");
    strcpy(pressure_a.pneumatic_analysis.compressor_performance, "fair");
    pressure_a.anomaly_detection.pressure_surge_detected = true;
    strcpy(pressure_a.leak_detection.leak_status, "tight");
    pressure_a.filter_monitoring.differential_pressure_bar = 0.5f;
    pressure_a.pressure_condition_score = 66;
    strcpy(pressure_a.condition_description, "stable");
    CHECK(pressure_sensor_encode_data(&pressure_a, bytes_a, BUFFER_SIZE) == 565);
    CHECK(read_f32(bytes_a + 2) == 101325.0f);
    CHECK(bytes_a[18] == 0xFF && bytes_a[19] == 0x0F);
    CHECK(memcmp(bytes_a + 28, "good", 5) == 0);
    CHECK(memcmp(bytes_a + 121, "fair", 5) == 0);
    CHECK(bytes_a[233] == 1);
    CHECK(memcmp(bytes_a + 339, "tight", 6) == 0);
    CHECK(read_f32(bytes_a + 400) == 0.5f);
    CHECK(bytes_a[484] == 66);
    CHECK(memcmp(bytes_a + 485, "stable", 7) == 0);
}

static void test_proximity_position(void) {
    const module_t m = {"proximity_position", ESOCORE_DEVICE_TYPE_PROXIMITY, PROXIMITY_POSITION_SENSOR_DATA_WIRE_SIZE,
                        &proximity_position_a, &proximity_position_b, sizeof(proximity_position_a),
                        proximity_position_fill, proximity_position_encode, proximity_position_decode};
    check_module(&m);

    current_test = "proximity_position_layout";
    proximity_position_sensor_data_t *p = &proximity_position_a;
    memset(p, 0, sizeof(*p));
    p->distance_mm = 12.5f; /* no base_data */
    p->target_count = 3;
    p->inductive_data.sensor_states[1] = true;
    p->capacitive_data.distance_mm = 4.5f;
    p->encoder_data.position_counts = -5;
    p->encoder_data.direction = -1;
    p->magnetic_data.sensor_states[0] = true;
    p->laser_data.distance_mm = 250.5f;
    p->accuracy_analysis.encoder_resolution_mm = 0.25f;
    strcpy(p->motion_analysis.speed_profile, "slow");
    strcpy(p->detection_analysis.detection_reliability, "high");
    p->health_monitoring.sensor_health_percentage = 100;
    p->position_condition_score = 93;
    p->timestamp = 0x11223344U;
    CHECK(proximity_position_sensor_encode_data(p, bytes_a, BUFFER_SIZE) == 598);
    CHECK(read_f32(bytes_a + 2) == 12.5f);
    CHECK(bytes_a[18] == 3);
    CHECK(bytes_a[19] == 0 && bytes_a[20] == 1);
    CHECK(read_f32(bytes_a + 23) == 4.5f);
    CHECK(read_u32(bytes_a + 33) == 0xFFFFFFFBU);
    CHECK(bytes_a[49] == 0xFF);
    CHECK(bytes_a[51] == 1);
    CHECK(read_f32(bytes_a + 58) == 250.5f);
    CHECK(read_f32(bytes_a + 68) == 0.25f);
    CHECK(memcmp(bytes_a + 251, "slow", 5) == 0);
    CHECK(memcmp(bytes_a + 349, "high", 5) == 0);
    CHECK(bytes_a[474] == 100);
    CHECK(bytes_a[529] == 93);
    CHECK(read_u32(bytes_a + 594) == 0x11223344U);
}

static void test_temperature_spot(void) {
    const module_t m = {"temperature_spot", ESOCORE_DEVICE_TYPE_TEMPERATURE, TEMPERATURE_SPOT_SENSOR_DATA_WIRE_SIZE,
                        &temperature_spot_a, &temperature_spot_b, sizeof(temperature_spot_a),
                        temperature_spot_fill, temperature_spot_encode, temperature_spot_decode};
    check_module(&m);

    current_test = "temperature_spot_layout";
    temperature_spot_sensor_data_t *t = &temperature_spot_a;
    memset(t, 0, sizeof(*t));
    t->temperature_celsius = 21.5f; /* no base_data */
    t->ntc_temperatures[0].temperature_celsius = 22.5f;
    t->ntc_temperatures[3].valid = true;
    t->rtd_temperatures[0].temperature_celsius = 23.5f;
    t->thermocouple_temperatures[1].valid = true;
    t->ir_temperature.object_temperature_celsius = 24.5f;
    t->statistics.average_temperature_c = 25.5f;
    t->gradient_analysis.max_thermal_gradient_c = 1.5f;
    t->stress_analysis.thermal_stress_level = 0.5f;
    t->limit_monitoring.max_temperature_c = 80.5f;
    t->temperature_condition_score = 70;
    t->timestamp = 0x55667788U;
    CHECK(temperature_spot_sensor_encode_data(t, bytes_a, BUFFER_SIZE) == 439);
    CHECK(read_f32(bytes_a + 2) == 21.5f);
    CHECK(read_f32(bytes_a + 10) == 22.5f);
    CHECK(bytes_a[10 + 3 * 9 + 8] == 1);
    CHECK(read_f32(bytes_a + 46) == 23.5f);
    CHECK(bytes_a[64 + 13 + 12] == 1);
    CHECK(read_f32(bytes_a + 90) == 24.5f);
    CHECK(read_f32(bytes_a + 99) == 25.5f);
    CHECK(read_f32(bytes_a + 155) == 1.5f);
    CHECK(read_f32(bytes_a + 209) == 0.5f);
    CHECK(read_f32(bytes_a + 281) == 80.5f);
    CHECK(bytes_a[370] == 70);
    CHECK(read_u32(bytes_a + 435) == 0x55667788U);
}

int main(void) {
    test_primitives();
    test_base_data_layout();
    test_const_readings();
    test_vibration();
    test_current();
    test_acoustic();
    test_air_quality();
    test_light();
    test_oil_quality();
    test_pressure();
    test_proximity_position();
    test_temperature_spot();

    printf("sensor codec tests: %d checks, %d failed\n", checks_run, checks_failed);
    return checks_failed == 0 ? 0 : 1;
}
