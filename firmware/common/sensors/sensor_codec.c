/**
 * @file sensor_codec.c
 * @brief Fixed-layout, little-endian serialisation of sensor readings (see sensor_codec.h)
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "sensor_codec.h"

#include <string.h>

void esocore_codec_init_encode(esocore_codec_t *codec, uint8_t *buffer, uint32_t capacity) {
    memset(codec, 0, sizeof(*codec));
    codec->mode = ESOCORE_CODEC_ENCODE;
    codec->out = buffer;
    codec->capacity = buffer ? capacity : 0;
    codec->ok = true;
}

void esocore_codec_init_decode(esocore_codec_t *codec, const uint8_t *buffer, uint32_t length) {
    memset(codec, 0, sizeof(*codec));
    codec->mode = ESOCORE_CODEC_DECODE;
    codec->in = buffer;
    codec->capacity = buffer ? length : 0;
    codec->ok = true;
}

void esocore_codec_init_fill(esocore_codec_t *codec, uint32_t seed) {
    memset(codec, 0, sizeof(*codec));
    codec->mode = ESOCORE_CODEC_FILL;
    codec->seed = seed ? seed : 1U;
    codec->ok = true;
}

/* xorshift32: deterministic test values for FILL */
static uint32_t codec_random(esocore_codec_t *codec) {
    uint32_t x = codec->seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    codec->seed = x;
    return x;
}

/* Reserve n bytes; false (and ok = false) when they do not fit */
static bool codec_take(esocore_codec_t *codec, uint32_t n) {
    if (!codec->ok || codec->position > codec->capacity || codec->capacity - codec->position < n) {
        codec->ok = false;
        return false;
    }
    return true;
}

/*
 * The primitives below only read the field when encoding and only write it
 * when decoding or filling: an encoded reading may be const, and a decode
 * target may be uninitialised (a bool with any other byte than 0 or 1 must
 * never be read).
 */

/* ENCODE: append value as a little-endian integer of size bytes */
static void codec_put(esocore_codec_t *codec, uint32_t value, uint32_t size) {
    if (codec_take(codec, size)) {
        for (uint32_t i = 0; i < size; i++) {
            codec->out[codec->position++] = (uint8_t)(value >> (8U * i));
        }
    }
}

/* DECODE: read a little-endian integer of size bytes; false on overrun */
static bool codec_get(esocore_codec_t *codec, uint32_t *value, uint32_t size) {
    if (!codec_take(codec, size)) {
        return false;
    }
    uint32_t result = 0;
    for (uint32_t i = 0; i < size; i++) {
        result |= (uint32_t)codec->in[codec->position++] << (8U * i);
    }
    *value = result;
    return true;
}

/* FILL: a random integer of size bytes */
static uint32_t codec_fill(esocore_codec_t *codec, uint32_t size) {
    uint32_t value = codec_random(codec);
    return size < 4 ? value & ((1UL << (8U * size)) - 1U) : value;
}

/* Raw bits of 1, 2 or 4 bytes: *bits is read for ENCODE; returns true when
 * the caller must store *bits back into the field (DECODE, FILL) */
static bool codec_bits(esocore_codec_t *codec, uint32_t *bits, uint32_t size) {
    switch (codec->mode) {
        case ESOCORE_CODEC_ENCODE:
            codec_put(codec, *bits, size);
            return false;
        case ESOCORE_CODEC_DECODE:
            return codec_get(codec, bits, size);
        case ESOCORE_CODEC_FILL:
        default:
            *bits = codec_fill(codec, size);
            return true;
    }
}

void esocore_codec_u8(esocore_codec_t *codec, uint8_t *value) {
    uint32_t bits = codec->mode == ESOCORE_CODEC_ENCODE ? *value : 0U;
    if (codec_bits(codec, &bits, 1)) {
        *value = (uint8_t)bits;
    }
}

void esocore_codec_i8(esocore_codec_t *codec, int8_t *value) {
    uint8_t raw = 0;
    if (codec->mode == ESOCORE_CODEC_ENCODE) {
        memcpy(&raw, value, 1);
    }
    uint32_t bits = raw;
    if (codec_bits(codec, &bits, 1)) {
        raw = (uint8_t)bits;
        memcpy(value, &raw, 1);
    }
}

void esocore_codec_bool(esocore_codec_t *codec, bool *value) {
    switch (codec->mode) {
        case ESOCORE_CODEC_ENCODE:
            codec_put(codec, *value ? 1U : 0U, 1);
            break;
        case ESOCORE_CODEC_DECODE: {
            uint32_t bits = 0;
            if (codec_get(codec, &bits, 1)) {
                if (bits > 1U) {
                    codec->ok = false; /* Not a bool: the bytes do not match this layout */
                } else {
                    *value = bits != 0U;
                }
            }
            break;
        }
        case ESOCORE_CODEC_FILL:
        default:
            *value = (codec_random(codec) & 1U) != 0U;
            break;
    }
}

void esocore_codec_u16(esocore_codec_t *codec, uint16_t *value) {
    uint32_t bits = codec->mode == ESOCORE_CODEC_ENCODE ? *value : 0U;
    if (codec_bits(codec, &bits, 2)) {
        *value = (uint16_t)bits;
    }
}

void esocore_codec_i16(esocore_codec_t *codec, int16_t *value) {
    uint16_t raw = 0;
    if (codec->mode == ESOCORE_CODEC_ENCODE) {
        memcpy(&raw, value, 2);
    }
    uint32_t bits = raw;
    if (codec_bits(codec, &bits, 2)) {
        raw = (uint16_t)bits;
        memcpy(value, &raw, 2);
    }
}

void esocore_codec_u32(esocore_codec_t *codec, uint32_t *value) {
    uint32_t bits = codec->mode == ESOCORE_CODEC_ENCODE ? *value : 0U;
    if (codec_bits(codec, &bits, 4)) {
        *value = bits;
    }
}

void esocore_codec_i32(esocore_codec_t *codec, int32_t *value) {
    uint32_t bits = 0;
    if (codec->mode == ESOCORE_CODEC_ENCODE) {
        memcpy(&bits, value, 4);
    }
    if (codec_bits(codec, &bits, 4)) {
        memcpy(value, &bits, 4);
    }
}

void esocore_codec_f32(esocore_codec_t *codec, float *value) {
    if (codec->mode == ESOCORE_CODEC_FILL) {
        /* Finite values with a fractional part, -1000.00 to 1000.00 */
        *value = (float)((int32_t)(codec_random(codec) % 200001U) - 100000) / 100.0f;
        return;
    }
    uint32_t bits = 0;
    if (codec->mode == ESOCORE_CODEC_ENCODE) {
        memcpy(&bits, value, 4);
    }
    if (codec_bits(codec, &bits, 4)) {
        memcpy(value, &bits, 4);
    }
}

void esocore_codec_chars(esocore_codec_t *codec, char *value, uint32_t n) {
    if (n == 0) {
        return;
    }
    switch (codec->mode) {
        case ESOCORE_CODEC_ENCODE:
            if (codec_take(codec, n)) {
                /* The text up to its terminator, then zeros: bytes after a
                 * terminator never reach the bus */
                uint32_t i = 0;
                for (; i < n - 1 && value[i] != '\0'; i++) {
                    codec->out[codec->position + i] = (uint8_t)value[i];
                }
                memset(codec->out + codec->position + i, 0, n - i);
                codec->position += n;
            }
            break;
        case ESOCORE_CODEC_DECODE:
            if (codec_take(codec, n)) {
                memcpy(value, codec->in + codec->position, n);
                value[n - 1] = '\0';
                codec->position += n;
            }
            break;
        case ESOCORE_CODEC_FILL:
        default: {
            uint32_t length = codec_random(codec) % n; /* 0 to n - 1 characters */
            memset(value, 0, n);
            for (uint32_t i = 0; i < length; i++) {
                value[i] = (char)('a' + (char)(codec_random(codec) % 26U));
            }
            break;
        }
    }
}

void esocore_codec_u8_array(esocore_codec_t *codec, uint8_t *values, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        esocore_codec_u8(codec, &values[i]);
    }
}

void esocore_codec_bool_array(esocore_codec_t *codec, bool *values, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        esocore_codec_bool(codec, &values[i]);
    }
}

void esocore_codec_u16_array(esocore_codec_t *codec, uint16_t *values, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        esocore_codec_u16(codec, &values[i]);
    }
}

void esocore_codec_i16_array(esocore_codec_t *codec, int16_t *values, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        esocore_codec_i16(codec, &values[i]);
    }
}

void esocore_codec_f32_array(esocore_codec_t *codec, float *values, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        esocore_codec_f32(codec, &values[i]);
    }
}

bool esocore_codec_enum8_value(esocore_codec_t *codec, uint8_t *value, uint8_t max_value) {
    if (codec->mode == ESOCORE_CODEC_FILL) {
        *value = (uint8_t)(codec_random(codec) % ((uint32_t)max_value + 1U));
        return true;
    }
    uint32_t bits = *value;
    if (codec_bits(codec, &bits, 1)) {
        *value = (uint8_t)bits;
        return true;
    }
    return false;
}

void esocore_codec_header(esocore_codec_t *codec, uint8_t device_type) {
    uint8_t type = device_type;
    uint8_t version = ESOCORE_SENSOR_PAYLOAD_VERSION;
    if (codec->mode == ESOCORE_CODEC_FILL) {
        return; /* Nothing in the struct to fill */
    }
    esocore_codec_u8(codec, &type);
    esocore_codec_u8(codec, &version);
    if (codec->mode == ESOCORE_CODEC_DECODE &&
        (type != device_type || version != ESOCORE_SENSOR_PAYLOAD_VERSION)) {
        codec->ok = false;
    }
}

void esocore_codec_sensor_data(esocore_codec_t *codec, esocore_sensor_data_t *data) {
    esocore_codec_u32(codec, &data->timestamp);
    ESOCORE_CODEC_ENUM8(codec, data->sensor_type, esocore_sensor_type_t, ESOCORE_SENSOR_PROXIMITY);
    esocore_codec_u8(codec, &data->sensor_id);
    esocore_codec_u16(codec, &data->data_points);
    esocore_codec_u8(codec, &data->data_type);
    esocore_codec_chars(codec, data->units, ESOCORE_SENSOR_UNITS_LENGTH);
    esocore_codec_u8(codec, &data->quality_flags);
    esocore_codec_u16(codec, &data->data_size);
    esocore_codec_u8_array(codec, data->data, ESOCORE_SENSOR_DATA_BUFFER_SIZE);
}

uint32_t esocore_codec_encode_payload(uint8_t device_type, esocore_codec_walk_t walk, const void *data,
                                      uint8_t *buffer, uint32_t capacity) {
    if (!walk || !data || !buffer) {
        return 0;
    }
    esocore_codec_t codec;
    esocore_codec_init_encode(&codec, buffer, capacity);
    esocore_codec_header(&codec, device_type);
    /* ENCODE only reads the struct */
    walk(&codec, (void *)(uintptr_t)data);
    return codec.ok ? codec.position : 0;
}

bool esocore_codec_decode_payload(uint8_t device_type, esocore_codec_walk_t walk, const uint8_t *buffer,
                                  uint32_t length, void *data) {
    if (!walk || !data || !buffer) {
        return false;
    }
    esocore_codec_t codec;
    esocore_codec_init_decode(&codec, buffer, length);
    esocore_codec_header(&codec, device_type);
    if (!codec.ok) {
        return false;
    }
    walk(&codec, data);
    return codec.ok && codec.position == length;
}
