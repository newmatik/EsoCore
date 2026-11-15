/**
 * @file air_quality_codec.c
 * @brief Bus payload of air_quality_sensor_data_t (layout rules: common/sensors/sensor_codec.h)
 *
 * The fields are walked in the order air_quality_sensor.h declares them. Byte
 * offsets of the sections: firmware/README.md, "Sensor Data Payload".
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "air_quality_sensor.h"

void air_quality_sensor_codec(esocore_codec_t *codec, air_quality_sensor_data_t *data) {
    esocore_codec_sensor_data(codec, &data->base_data);

    for (uint32_t i = 0; i < AIR_QUALITY_MAX_GASES; i++) {
        air_quality_gas_data_t *gas = &data->gas_sensors[i];
        esocore_codec_u8(codec, &gas->gas_type);
        esocore_codec_f32(codec, &gas->concentration_ppm);
        esocore_codec_f32(codec, &gas->concentration_ppb);
        esocore_codec_f32(codec, &gas->concentration_ug_m3);
        esocore_codec_u16(codec, &gas->raw_adc_value);
        esocore_codec_f32(codec, &gas->temperature_compensated);
        esocore_codec_bool(codec, &gas->sensor_valid);
        esocore_codec_u32(codec, &gas->timestamp);
    }

    air_quality_pm_data_t *pm = &data->pm_data;
    esocore_codec_f32(codec, &pm->pm1_0);
    esocore_codec_f32(codec, &pm->pm2_5);
    esocore_codec_f32(codec, &pm->pm10);
    esocore_codec_u16(codec, &pm->raw_counts);
    esocore_codec_bool(codec, &pm->sensor_valid);

    air_quality_environmental_data_t *env = &data->env_data;
    esocore_codec_f32(codec, &env->temperature_celsius);
    esocore_codec_f32(codec, &env->humidity_percent);
    esocore_codec_f32(codec, &env->pressure_hpa);
    esocore_codec_f32(codec, &env->dew_point_celsius);
    esocore_codec_f32(codec, &env->absolute_humidity);

    air_quality_iaq_data_t *iaq = &data->iaq_data;
    esocore_codec_u16(codec, &iaq->iaq_index);
    esocore_codec_u8(codec, &iaq->iaq_rating);
    esocore_codec_f32(codec, &iaq->co2_equivalent);
    esocore_codec_f32(codec, &iaq->breath_voc_equivalent);
    esocore_codec_f32(codec, &iaq->gas_resistance);
    esocore_codec_u32(codec, &iaq->accuracy);
    esocore_codec_u32(codec, &iaq->timestamp);

    air_quality_health_data_t *health = &data->health_data;
    esocore_codec_u8(codec, &health->overall_risk_level);
    esocore_codec_u8(codec, &health->respiratory_risk);
    esocore_codec_u8(codec, &health->cardiovascular_risk);
    esocore_codec_u8(codec, &health->neurological_risk);
    esocore_codec_chars(codec, health->risk_description, sizeof(health->risk_description));
    esocore_codec_chars(codec, health->recommendations, sizeof(health->recommendations));
    esocore_codec_u32(codec, &health->assessment_timestamp);

    esocore_codec_u8(codec, &data->air_quality_score);
    esocore_codec_chars(codec, data->quality_description, sizeof(data->quality_description));
}

static void air_quality_walk(esocore_codec_t *codec, void *data) {
    air_quality_sensor_codec(codec, (air_quality_sensor_data_t *)data);
}

uint32_t air_quality_sensor_encode_data(const air_quality_sensor_data_t *data, uint8_t *buffer, uint32_t capacity) {
    return esocore_codec_encode_payload(ESOCORE_DEVICE_TYPE_AIR_QUALITY, air_quality_walk, data, buffer, capacity);
}

bool air_quality_sensor_decode_data(const uint8_t *buffer, uint32_t length, air_quality_sensor_data_t *data) {
    return esocore_codec_decode_payload(ESOCORE_DEVICE_TYPE_AIR_QUALITY, air_quality_walk, buffer, length, data);
}
