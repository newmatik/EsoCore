/**
 * @file oil_quality_codec.c
 * @brief Bus payload of oil_quality_sensor_data_t (layout rules: common/sensors/sensor_codec.h)
 *
 * The fields are walked in the order oil_quality_sensor.h declares them. Byte
 * offsets of the sections: firmware/README.md, "Sensor Data Payload".
 * oil_quality_sensor_data_t has no base_data.
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "oil_quality_sensor.h"

void oil_quality_sensor_codec(esocore_codec_t *codec, oil_quality_sensor_data_t *data) {
    esocore_codec_f32(codec, &data->turbidity_ntu);
    esocore_codec_f32(codec, &data->viscosity_cst);
    esocore_codec_f32(codec, &data->water_content_ppm);
    esocore_codec_f32(codec, &data->dielectric_constant);
    esocore_codec_f32(codec, &data->temperature_celsius);
    esocore_codec_u32(codec, &data->timestamp);

    oil_quality_turbidity_data_t *turbidity = &data->turbidity_data;
    esocore_codec_f32(codec, &turbidity->turbidity_ntu);
    esocore_codec_u16(codec, &turbidity->raw_scattered_light);
    esocore_codec_u16(codec, &turbidity->raw_incident_light);

    oil_quality_viscosity_data_t *viscosity = &data->viscosity_data;
    esocore_codec_f32(codec, &viscosity->viscosity_cst);
    esocore_codec_u32(codec, &viscosity->oscillation_frequency_hz);

    oil_quality_water_data_t *water = &data->water_data;
    esocore_codec_f32(codec, &water->water_content_ppm);
    esocore_codec_f32(codec, &water->water_content_percent);
    esocore_codec_u32(codec, &water->capacitance_pf);

    oil_quality_dielectric_data_t *dielectric = &data->dielectric_data;
    esocore_codec_f32(codec, &dielectric->dielectric_constant);
    esocore_codec_u32(codec, &dielectric->capacitance_pf);

    oil_quality_temperature_data_t *temperature = &data->temperature_data;
    esocore_codec_f32(codec, &temperature->temperature_celsius);
    esocore_codec_u16(codec, &temperature->raw_adc_value);

    oil_degradation_analysis_t *degradation = &data->degradation_analysis;
    esocore_codec_chars(codec, degradation->turbidity_assessment, sizeof(degradation->turbidity_assessment));
    esocore_codec_u8(codec, &degradation->contamination_level);
    esocore_codec_chars(codec, degradation->viscosity_assessment, sizeof(degradation->viscosity_assessment));
    esocore_codec_u8(codec, &degradation->viscosity_status);
    esocore_codec_chars(codec, degradation->water_assessment, sizeof(degradation->water_assessment));
    esocore_codec_u8(codec, &degradation->water_risk);
    esocore_codec_chars(codec, degradation->dielectric_assessment, sizeof(degradation->dielectric_assessment));
    esocore_codec_u8(codec, &degradation->dielectric_health);
    esocore_codec_f32(codec, &degradation->overall_degradation_level);
    esocore_codec_u32(codec, &degradation->assessment_timestamp);

    oil_maintenance_recommendations_t *maintenance = &data->maintenance_recommendations;
    esocore_codec_chars(codec, maintenance->immediate_actions, sizeof(maintenance->immediate_actions));
    esocore_codec_chars(codec, maintenance->scheduled_maintenance, sizeof(maintenance->scheduled_maintenance));
    esocore_codec_chars(codec, maintenance->long_term_actions, sizeof(maintenance->long_term_actions));
    esocore_codec_u16(codec, &maintenance->change_interval_days);
    esocore_codec_u32(codec, &maintenance->recommendation_timestamp);

    esocore_codec_u8(codec, &data->oil_quality_score);
    esocore_codec_chars(codec, data->condition_description, sizeof(data->condition_description));
}

static void oil_quality_walk(esocore_codec_t *codec, void *data) {
    oil_quality_sensor_codec(codec, (oil_quality_sensor_data_t *)data);
}

uint32_t oil_quality_sensor_encode_data(const oil_quality_sensor_data_t *data, uint8_t *buffer, uint32_t capacity) {
    return esocore_codec_encode_payload(ESOCORE_DEVICE_TYPE_OIL_QUALITY, oil_quality_walk, data, buffer, capacity);
}

bool oil_quality_sensor_decode_data(const uint8_t *buffer, uint32_t length, oil_quality_sensor_data_t *data) {
    return esocore_codec_decode_payload(ESOCORE_DEVICE_TYPE_OIL_QUALITY, oil_quality_walk, buffer, length, data);
}
