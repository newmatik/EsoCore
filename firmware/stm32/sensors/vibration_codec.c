/**
 * @file vibration_codec.c
 * @brief Bus payload of vibration_sensor_data_t (layout rules: common/sensors/sensor_codec.h)
 *
 * The fields are walked in the order vibration_sensor.h declares them. Byte
 * offsets of the sections: firmware/README.md, "Sensor Data Payload".
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "vibration_sensor.h"

void vibration_sensor_codec(esocore_codec_t *codec, vibration_sensor_data_t *data) {
    esocore_codec_sensor_data(codec, &data->base_data);

    vibration_raw_data_t *raw = &data->raw_data;
    esocore_codec_i32(codec, &raw->x_axis_raw);
    esocore_codec_i32(codec, &raw->y_axis_raw);
    esocore_codec_i32(codec, &raw->z_axis_raw);
    esocore_codec_u32(codec, &raw->timestamp);
    esocore_codec_i16(codec, &raw->temperature_celsius);

    vibration_processed_data_t *processed = &data->processed_data;
    esocore_codec_f32(codec, &processed->x_axis_acceleration);
    esocore_codec_f32(codec, &processed->y_axis_acceleration);
    esocore_codec_f32(codec, &processed->z_axis_acceleration);
    esocore_codec_f32(codec, &processed->x_axis_velocity);
    esocore_codec_f32(codec, &processed->y_axis_velocity);
    esocore_codec_f32(codec, &processed->z_axis_velocity);
    esocore_codec_f32(codec, &processed->x_axis_displacement);
    esocore_codec_f32(codec, &processed->y_axis_displacement);
    esocore_codec_f32(codec, &processed->z_axis_displacement);
    esocore_codec_f32(codec, &processed->rms_acceleration);
    esocore_codec_f32(codec, &processed->peak_acceleration);
    esocore_codec_f32(codec, &processed->crest_factor);
    esocore_codec_u32(codec, &processed->timestamp);

    vibration_fft_data_t *fft = &data->fft_data;
    esocore_codec_f32_array(codec, fft->frequency_bins, VIBRATION_SENSOR_FFT_SIZE / 2);
    esocore_codec_u16(codec, &fft->dominant_frequency_hz);
    esocore_codec_f32(codec, &fft->dominant_amplitude);
    esocore_codec_f32(codec, &fft->total_power);
    esocore_codec_f32(codec, &fft->spectral_centroid);
    esocore_codec_f32(codec, &fft->rms_frequency);
    esocore_codec_f32(codec, &fft->frequency_spread);

    vibration_bearing_analysis_t *bearing = &data->bearing_analysis;
    esocore_codec_bool(codec, &bearing->bearing_fault_detected);
    esocore_codec_f32(codec, &bearing->fault_severity);
    esocore_codec_u16(codec, &bearing->fault_frequency_hz);
    esocore_codec_f32(codec, &bearing->bpfo_amplitude);
    esocore_codec_f32(codec, &bearing->bpfi_amplitude);
    esocore_codec_f32(codec, &bearing->ftf_amplitude);
    esocore_codec_f32(codec, &bearing->bsf_amplitude);
    esocore_codec_u8(codec, &bearing->fault_type);

    vibration_gear_analysis_t *gear = &data->gear_analysis;
    esocore_codec_bool(codec, &gear->gear_fault_detected);
    esocore_codec_f32(codec, &gear->gear_mesh_frequency);
    esocore_codec_f32(codec, &gear->sideband_amplitude);
    esocore_codec_f32(codec, &gear->modulation_index);
    esocore_codec_u8(codec, &gear->missing_teeth);
    esocore_codec_f32(codec, &gear->gear_wear_level);

    esocore_codec_u8(codec, &data->overall_condition);
    esocore_codec_chars(codec, data->condition_description, sizeof(data->condition_description));
}

static void vibration_walk(esocore_codec_t *codec, void *data) {
    vibration_sensor_codec(codec, (vibration_sensor_data_t *)data);
}

uint32_t vibration_sensor_encode_data(const vibration_sensor_data_t *data, uint8_t *buffer, uint32_t capacity) {
    return esocore_codec_encode_payload(ESOCORE_DEVICE_TYPE_VIBRATION, vibration_walk, data, buffer, capacity);
}

bool vibration_sensor_decode_data(const uint8_t *buffer, uint32_t length, vibration_sensor_data_t *data) {
    return esocore_codec_decode_payload(ESOCORE_DEVICE_TYPE_VIBRATION, vibration_walk, buffer, length, data);
}
