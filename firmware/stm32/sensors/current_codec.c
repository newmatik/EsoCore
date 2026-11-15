/**
 * @file current_codec.c
 * @brief Bus payload of current_sensor_data_t (layout rules: common/sensors/sensor_codec.h)
 *
 * The fields are walked in the order current_sensor.h declares them. Byte
 * offsets of the sections: firmware/README.md, "Sensor Data Payload".
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "current_sensor.h"

void current_sensor_codec(esocore_codec_t *codec, current_sensor_data_t *data) {
    esocore_codec_sensor_data(codec, &data->base_data);

    current_raw_data_t *raw = &data->raw_data;
    esocore_codec_i16_array(codec, raw->current_samples, CURRENT_SENSOR_MAX_SAMPLES);
    esocore_codec_u32(codec, &raw->sample_count);
    esocore_codec_u32(codec, &raw->timestamp);
    esocore_codec_i8(codec, &raw->temperature_celsius);

    current_processed_data_t *processed = &data->processed_data;
    esocore_codec_f32(codec, &processed->rms_current_a);
    esocore_codec_f32(codec, &processed->peak_current_a);
    esocore_codec_f32(codec, &processed->average_current_a);
    esocore_codec_f32(codec, &processed->crest_factor);
    esocore_codec_f32(codec, &processed->form_factor);
    esocore_codec_f32(codec, &processed->fundamental_current_a);
    esocore_codec_f32(codec, &processed->total_harmonic_distortion);
    esocore_codec_u32(codec, &processed->timestamp);

    current_harmonic_data_t *harmonic = &data->harmonic_data;
    esocore_codec_f32_array(codec, harmonic->harmonic_magnitudes, 16);
    esocore_codec_f32_array(codec, harmonic->harmonic_phases, 16);
    esocore_codec_f32_array(codec, harmonic->harmonic_percentages, 16);
    esocore_codec_u8(codec, &harmonic->significant_harmonics);
    esocore_codec_f32(codec, &harmonic->thd_f);
    esocore_codec_f32(codec, &harmonic->thd_r);
    esocore_codec_f32(codec, &harmonic->k_factor);

    current_power_data_t *power = &data->power_data;
    esocore_codec_f32(codec, &power->power_factor);
    esocore_codec_f32(codec, &power->displacement_power_factor);
    esocore_codec_f32(codec, &power->apparent_power_va);
    esocore_codec_f32(codec, &power->active_power_w);
    esocore_codec_f32(codec, &power->reactive_power_var);
    esocore_codec_f32(codec, &power->voltage_rms_v);
    esocore_codec_f32(codec, &power->frequency_hz);
    esocore_codec_f32(codec, &power->flicker_severity);

    current_motor_data_t *motor = &data->motor_data;
    esocore_codec_bool(codec, &motor->motor_connected);
    esocore_codec_f32(codec, &motor->motor_load_percentage);
    esocore_codec_f32(codec, &motor->motor_efficiency);
    esocore_codec_f32(codec, &motor->motor_power_factor);
    esocore_codec_u8(codec, &motor->motor_poles);
    esocore_codec_f32(codec, &motor->slip_percentage);
    esocore_codec_bool(codec, &motor->overload_detected);
    esocore_codec_bool(codec, &motor->underload_detected);

    esocore_codec_u8(codec, &data->current_condition);
    esocore_codec_chars(codec, data->condition_description, sizeof(data->condition_description));
}

static void current_walk(esocore_codec_t *codec, void *data) {
    current_sensor_codec(codec, (current_sensor_data_t *)data);
}

uint32_t current_sensor_encode_data(const current_sensor_data_t *data, uint8_t *buffer, uint32_t capacity) {
    return esocore_codec_encode_payload(ESOCORE_DEVICE_TYPE_CURRENT, current_walk, data, buffer, capacity);
}

bool current_sensor_decode_data(const uint8_t *buffer, uint32_t length, current_sensor_data_t *data) {
    return esocore_codec_decode_payload(ESOCORE_DEVICE_TYPE_CURRENT, current_walk, buffer, length, data);
}
