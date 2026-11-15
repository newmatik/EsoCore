/**
 * @file sensor_codec.h
 * @brief Fixed-layout, little-endian serialisation of sensor readings
 *
 * A sensor module sends each reading as a DATA_RESPONSE payload (protocol.h).
 * The payload is never a struct copied from memory (padding, byte order and
 * the size of bool and enums depend on the compiler) but this byte layout:
 *
 *   offset 0: device type (esocore_device_type_t), 1 byte
 *   offset 1: layout version (ESOCORE_SENSOR_PAYLOAD_VERSION), 1 byte
 *   offset 2: the fields of the module's data struct, in declaration order
 *
 * Field encodings, without padding or alignment:
 * - uint8_t, int8_t, bool (0 or 1), enum (its value): 1 byte
 * - uint16_t, int16_t: 2 bytes, little endian (two's complement)
 * - uint32_t, int32_t: 4 bytes, little endian (two's complement)
 * - float: IEEE 754 binary32, 4 bytes, little endian
 * - char[n]: n bytes, the text NUL-padded; the decoder always terminates it
 * - arrays: their elements one after another; nested structs: their fields
 *
 * Each module's codec (stm32/sensors/<module>_codec.c) walks its data struct
 * field by field with the functions below. The same walk encodes, decodes and,
 * for the tests, fills a struct with deterministic values, so the three cannot
 * drift apart. Layout tables: firmware/README.md, "Sensor Data Payload".
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_SENSOR_CODEC_H
#define ESOCORE_SENSOR_CODEC_H

#include <stdbool.h>
#include <stdint.h>

#include "sensor_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Layout version in byte 1 of every sensor payload; raised on any layout change */
#define ESOCORE_SENSOR_PAYLOAD_VERSION     1
/** Bytes before the first field: device type and layout version */
#define ESOCORE_SENSOR_PAYLOAD_HEADER_SIZE 2
/** Encoded size of esocore_sensor_data_t (the base_data of every module that has one) */
#define ESOCORE_SENSOR_DATA_WIRE_SIZE      (4 + 1 + 1 + 2 + 1 + ESOCORE_SENSOR_UNITS_LENGTH + 1 + 2 + ESOCORE_SENSOR_DATA_BUFFER_SIZE)

typedef enum {
    ESOCORE_CODEC_ENCODE = 0, /**< Struct to bytes */
    ESOCORE_CODEC_DECODE = 1, /**< Bytes to struct */
    ESOCORE_CODEC_FILL   = 2, /**< Deterministic test values into the struct (host tests) */
} esocore_codec_mode_t;

/**
 * @brief State of one walk over a data struct
 *
 * ok turns false at the first error (buffer too short, a bool byte above 1)
 * and stays false; the walk goes on without touching memory.
 */
typedef struct {
    esocore_codec_mode_t mode;
    uint8_t *out;             /**< ENCODE: output buffer */
    const uint8_t *in;        /**< DECODE: input bytes */
    uint32_t capacity;        /**< Bytes in out or in */
    uint32_t position;        /**< Bytes written or read so far */
    uint32_t seed;            /**< FILL: generator state */
    bool ok;
} esocore_codec_t;

void esocore_codec_init_encode(esocore_codec_t *codec, uint8_t *buffer, uint32_t capacity);
void esocore_codec_init_decode(esocore_codec_t *codec, const uint8_t *buffer, uint32_t length);
void esocore_codec_init_fill(esocore_codec_t *codec, uint32_t seed);

void esocore_codec_u8(esocore_codec_t *codec, uint8_t *value);
void esocore_codec_i8(esocore_codec_t *codec, int8_t *value);
void esocore_codec_bool(esocore_codec_t *codec, bool *value);
void esocore_codec_u16(esocore_codec_t *codec, uint16_t *value);
void esocore_codec_i16(esocore_codec_t *codec, int16_t *value);
void esocore_codec_u32(esocore_codec_t *codec, uint32_t *value);
void esocore_codec_i32(esocore_codec_t *codec, int32_t *value);
void esocore_codec_f32(esocore_codec_t *codec, float *value);
/** Text field of n bytes (n >= 1); decoding and filling keep value[n - 1] = '\0' */
void esocore_codec_chars(esocore_codec_t *codec, char *value, uint32_t n);

void esocore_codec_u8_array(esocore_codec_t *codec, uint8_t *values, uint32_t count);
void esocore_codec_bool_array(esocore_codec_t *codec, bool *values, uint32_t count);
void esocore_codec_u16_array(esocore_codec_t *codec, uint16_t *values, uint32_t count);
void esocore_codec_i16_array(esocore_codec_t *codec, int16_t *values, uint32_t count);
void esocore_codec_f32_array(esocore_codec_t *codec, float *values, uint32_t count);

/** An enum field stored as one byte; FILL picks a value from 0 to max_value */
#define ESOCORE_CODEC_ENUM8(codec, field, enum_type, max_value)                      \
    do {                                                                             \
        uint8_t esocore_codec_enum_byte_ = (uint8_t)(field);                         \
        esocore_codec_enum8_value((codec), &esocore_codec_enum_byte_, (max_value));  \
        (field) = (enum_type)esocore_codec_enum_byte_;                               \
    } while (0)

/** Helper of ESOCORE_CODEC_ENUM8 */
void esocore_codec_enum8_value(esocore_codec_t *codec, uint8_t *value, uint8_t max_value);

/** Payload header: device type and layout version; DECODE checks both */
void esocore_codec_header(esocore_codec_t *codec, uint8_t device_type);

/** esocore_sensor_data_t (ESOCORE_SENSOR_DATA_WIRE_SIZE bytes) */
void esocore_codec_sensor_data(esocore_codec_t *codec, esocore_sensor_data_t *data);

/** Walk over one module's data struct (the <module>_sensor_codec functions) */
typedef void (*esocore_codec_walk_t)(esocore_codec_t *codec, void *data);

/**
 * @brief Encode a reading: header, then the fields walk visits
 *
 * @return Bytes written, or 0 if the buffer is too small
 */
uint32_t esocore_codec_encode_payload(uint8_t device_type, esocore_codec_walk_t walk, const void *data,
                                      uint8_t *buffer, uint32_t capacity);

/**
 * @brief Decode a reading encoded by esocore_codec_encode_payload()
 *
 * @return true if the header matches (device type and layout version), every
 *         bool is 0 or 1 and the length is exactly the encoded size; on false
 *         *data is undefined
 */
bool esocore_codec_decode_payload(uint8_t device_type, esocore_codec_walk_t walk, const uint8_t *buffer,
                                  uint32_t length, void *data);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_SENSOR_CODEC_H */
