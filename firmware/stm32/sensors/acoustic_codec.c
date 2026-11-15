/**
 * @file acoustic_codec.c
 * @brief Bus payload of acoustic_sensor_data_t (layout rules: common/sensors/sensor_codec.h)
 *
 * The fields are walked in the order acoustic_sensor.h declares them. Byte
 * offsets of the sections: firmware/README.md, "Sensor Data Payload".
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "acoustic_sensor.h"

void acoustic_sensor_codec(esocore_codec_t *codec, acoustic_sensor_data_t *data) {
    esocore_codec_sensor_data(codec, &data->base_data);

    acoustic_raw_data_t *raw = &data->raw_data;
    esocore_codec_i16_array(codec, raw->audio_samples, ACOUSTIC_SENSOR_MAX_SAMPLES);
    esocore_codec_u32(codec, &raw->sample_count);
    esocore_codec_u32(codec, &raw->timestamp);
    esocore_codec_i8(codec, &raw->temperature_celsius);

    acoustic_processed_data_t *processed = &data->processed_data;
    esocore_codec_f32(codec, &processed->rms_level_db);
    esocore_codec_f32(codec, &processed->peak_level_db);
    esocore_codec_f32(codec, &processed->crest_factor);
    esocore_codec_f32(codec, &processed->a_weighted_level_db);
    esocore_codec_f32(codec, &processed->equivalent_level_db);
    esocore_codec_f32(codec, &processed->sound_exposure_db);
    esocore_codec_u32(codec, &processed->measurement_duration_ms);
    esocore_codec_u32(codec, &processed->timestamp);

    acoustic_fft_data_t *fft = &data->fft_data;
    esocore_codec_f32_array(codec, fft->frequency_bins, ACOUSTIC_SENSOR_FFT_SIZE / 2);
    esocore_codec_u16(codec, &fft->dominant_frequency_hz);
    esocore_codec_f32(codec, &fft->dominant_amplitude);
    esocore_codec_f32(codec, &fft->total_power);
    esocore_codec_f32(codec, &fft->spectral_centroid);
    esocore_codec_f32(codec, &fft->spectral_rolloff);
    esocore_codec_f32(codec, &fft->spectral_flux);
    esocore_codec_f32(codec, &fft->tonality_index);

    acoustic_frequency_bands_t *bands = &data->frequency_bands;
    esocore_codec_f32(codec, &bands->band_31_5_hz);
    esocore_codec_f32(codec, &bands->band_63_hz);
    esocore_codec_f32(codec, &bands->band_125_hz);
    esocore_codec_f32(codec, &bands->band_250_hz);
    esocore_codec_f32(codec, &bands->band_500_hz);
    esocore_codec_f32(codec, &bands->band_1000_hz);
    esocore_codec_f32(codec, &bands->band_2000_hz);
    esocore_codec_f32(codec, &bands->band_4000_hz);
    esocore_codec_f32(codec, &bands->band_8000_hz);

    acoustic_ultrasound_data_t *ultrasound = &data->ultrasound_data;
    esocore_codec_bool(codec, &ultrasound->ultrasound_detected);
    esocore_codec_u16(codec, &ultrasound->ultrasound_frequency_hz);
    esocore_codec_f32(codec, &ultrasound->ultrasound_level_db);
    esocore_codec_f32(codec, &ultrasound->ultrasound_bandwidth_hz);
    esocore_codec_u8(codec, &ultrasound->ultrasound_type);

    esocore_codec_u8(codec, &data->acoustic_condition);
    esocore_codec_chars(codec, data->condition_description, sizeof(data->condition_description));
}

static void acoustic_walk(esocore_codec_t *codec, void *data) {
    acoustic_sensor_codec(codec, (acoustic_sensor_data_t *)data);
}

uint32_t acoustic_sensor_encode_data(const acoustic_sensor_data_t *data, uint8_t *buffer, uint32_t capacity) {
    return esocore_codec_encode_payload(ESOCORE_DEVICE_TYPE_ACOUSTIC, acoustic_walk, data, buffer, capacity);
}

bool acoustic_sensor_decode_data(const uint8_t *buffer, uint32_t length, acoustic_sensor_data_t *data) {
    return esocore_codec_decode_payload(ESOCORE_DEVICE_TYPE_ACOUSTIC, acoustic_walk, buffer, length, data);
}
