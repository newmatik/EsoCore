/**
 * @file light_codec.c
 * @brief Bus payload of light_sensor_data_t (layout rules: common/sensors/sensor_codec.h)
 *
 * The fields are walked in the order light_sensor.h declares them. Byte
 * offsets of the sections: firmware/README.md, "Sensor Data Payload".
 * light_sensor_data_t has no base_data.
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "light_sensor.h"

void light_sensor_codec(esocore_codec_t *codec, light_sensor_data_t *data) {
    light_visible_data_t *visible = &data->visible_data;
    esocore_codec_f32(codec, &visible->lux);
    esocore_codec_f32(codec, &visible->infrared_raw);
    esocore_codec_f32(codec, &visible->visible_raw);
    esocore_codec_f32(codec, &visible->full_spectrum_raw);

    light_uv_data_t *uv = &data->uv_data;
    esocore_codec_f32(codec, &uv->uva_watts_per_m2);
    esocore_codec_f32(codec, &uv->uvb_watts_per_m2);
    esocore_codec_f32(codec, &uv->uv_index);

    light_color_data_t *color = &data->color_data;
    esocore_codec_u16(codec, &color->red_counts);
    esocore_codec_u16(codec, &color->green_counts);
    esocore_codec_u16(codec, &color->blue_counts);
    esocore_codec_u16(codec, &color->clear_counts);
    esocore_codec_f32(codec, &color->color_temperature_kelvin);
    esocore_codec_f32(codec, &color->lux_from_color);

    light_spectral_data_t *spectral = &data->spectral_data;
    esocore_codec_f32_array(codec, spectral->channel_data, 10);
    esocore_codec_u8(codec, &spectral->flicker_frequency_hz);

    light_ir_data_t *ir = &data->ir_data;
    esocore_codec_f32(codec, &ir->object_temperature_celsius);
    esocore_codec_f32(codec, &ir->ambient_temperature_celsius);

    light_workplace_compliance_t *compliance = &data->workplace_compliance;
    esocore_codec_bool(codec, &compliance->illuminance_compliant);
    esocore_codec_chars(codec, compliance->illuminance_status, sizeof(compliance->illuminance_status));
    esocore_codec_bool(codec, &compliance->uv_safety_compliant);
    esocore_codec_chars(codec, compliance->uv_safety_status, sizeof(compliance->uv_safety_status));
    esocore_codec_bool(codec, &compliance->color_temperature_compliant);
    esocore_codec_chars(codec, compliance->color_temperature_status, sizeof(compliance->color_temperature_status));
    esocore_codec_bool(codec, &compliance->overall_compliant);
    esocore_codec_u32(codec, &compliance->assessment_timestamp);

    light_environmental_analysis_t *environment = &data->environmental_analysis;
    esocore_codec_chars(codec, environment->pollution_level, sizeof(environment->pollution_level));
    esocore_codec_f32(codec, &environment->sky_brightness_index);
    esocore_codec_chars(codec, environment->ecological_impact, sizeof(environment->ecological_impact));
    esocore_codec_u32(codec, &environment->assessment_timestamp);

    esocore_codec_u8(codec, &data->light_quality_score);
    esocore_codec_chars(codec, data->condition_description, sizeof(data->condition_description));

    /* Visible light sensor (TSL2591) */
    esocore_codec_f32(codec, &data->illuminance_lux);
    esocore_codec_u16(codec, &data->channel0_raw);
    esocore_codec_u16(codec, &data->channel1_raw);
    esocore_codec_f32(codec, &data->infrared_ratio);
    esocore_codec_u8(codec, &data->gain_visible);

    /* UV sensor (VEML6075) */
    esocore_codec_f32(codec, &data->uva_intensity_uwm2);
    esocore_codec_f32(codec, &data->uvb_intensity_uwm2);
    esocore_codec_f32(codec, &data->uv_index);
    esocore_codec_u8(codec, &data->gain_uv);

    /* IR temperature sensor (MLX90614) */
    esocore_codec_f32(codec, &data->object_temperature_c);
    esocore_codec_f32(codec, &data->ambient_temperature_c);
    esocore_codec_f32(codec, &data->emissivity);

    /* Color sensor (TCS34725) */
    esocore_codec_u16(codec, &data->red_raw);
    esocore_codec_u16(codec, &data->green_raw);
    esocore_codec_u16(codec, &data->blue_raw);
    esocore_codec_u16(codec, &data->clear_raw);
    esocore_codec_f32(codec, &data->color_temperature_k);
    esocore_codec_f32(codec, &data->lux_from_color);

    /* Spectral sensor (AS7341) */
    esocore_codec_u16_array(codec, data->spectral_channels, 10);
    esocore_codec_f32(codec, &data->dominant_wavelength_nm);
    esocore_codec_f32(codec, &data->peak_intensity);

    esocore_codec_u8(codec, &data->sensor_status);
    esocore_codec_u32(codec, &data->timestamp);
}

static void light_walk(esocore_codec_t *codec, void *data) {
    light_sensor_codec(codec, (light_sensor_data_t *)data);
}

uint32_t light_sensor_encode_data(const light_sensor_data_t *data, uint8_t *buffer, uint32_t capacity) {
    return esocore_codec_encode_payload(ESOCORE_DEVICE_TYPE_LIGHT, light_walk, data, buffer, capacity);
}

bool light_sensor_decode_data(const uint8_t *buffer, uint32_t length, light_sensor_data_t *data) {
    return esocore_codec_decode_payload(ESOCORE_DEVICE_TYPE_LIGHT, light_walk, buffer, length, data);
}
