/**
 * @file temperature_spot_codec.c
 * @brief Bus payload of temperature_spot_sensor_data_t (layout rules: common/sensors/sensor_codec.h)
 *
 * The fields are walked in the order temperature_spot_sensor.h declares them. Byte
 * offsets of the sections: firmware/README.md, "Sensor Data Payload".
 * temperature_spot_sensor_data_t has no base_data.
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "temperature_spot_sensor.h"

static void resistive_reading(esocore_codec_t *codec, temperature_resistive_reading_t *reading) {
    esocore_codec_f32(codec, &reading->temperature_celsius);
    esocore_codec_f32(codec, &reading->resistance_ohms);
    esocore_codec_bool(codec, &reading->valid);
}

void temperature_spot_sensor_codec(esocore_codec_t *codec, temperature_spot_sensor_data_t *data) {
    esocore_codec_f32(codec, &data->temperature_celsius);
    esocore_codec_f32(codec, &data->cold_junction_temp);

    for (uint32_t i = 0; i < 4; i++) {
        resistive_reading(codec, &data->ntc_temperatures[i]);
    }
    for (uint32_t i = 0; i < 2; i++) {
        resistive_reading(codec, &data->rtd_temperatures[i]);
    }
    for (uint32_t i = 0; i < 2; i++) {
        temperature_thermocouple_reading_t *thermocouple = &data->thermocouple_temperatures[i];
        esocore_codec_f32(codec, &thermocouple->temperature_celsius);
        esocore_codec_f32(codec, &thermocouple->voltage_mv);
        esocore_codec_f32(codec, &thermocouple->cold_junction_temp_c);
        esocore_codec_bool(codec, &thermocouple->valid);
    }

    temperature_ir_reading_t *ir = &data->ir_temperature;
    esocore_codec_f32(codec, &ir->object_temperature_celsius);
    esocore_codec_f32(codec, &ir->ambient_temperature_celsius);
    esocore_codec_bool(codec, &ir->valid);

    temperature_statistics_t *statistics = &data->statistics;
    esocore_codec_f32(codec, &statistics->average_temperature_c);
    esocore_codec_f32(codec, &statistics->min_temperature_c);
    esocore_codec_f32(codec, &statistics->max_temperature_c);
    esocore_codec_f32(codec, &statistics->temperature_range_c);
    esocore_codec_f32(codec, &statistics->temperature_std_dev);
    esocore_codec_f32(codec, &statistics->thermal_stability);
    esocore_codec_chars(codec, statistics->stability_assessment, sizeof(statistics->stability_assessment));

    temperature_gradient_analysis_t *gradient = &data->gradient_analysis;
    esocore_codec_f32(codec, &gradient->max_thermal_gradient_c);
    esocore_codec_bool(codec, &gradient->hotspot_detected);
    esocore_codec_u8(codec, &gradient->hotspot_sensor_index);
    esocore_codec_chars(codec, gradient->thermal_distribution, sizeof(gradient->thermal_distribution));

    temperature_stress_analysis_t *stress = &data->stress_analysis;
    esocore_codec_f32(codec, &stress->thermal_stress_level);
    esocore_codec_f32(codec, &stress->thermal_fatigue_accumulation);
    esocore_codec_chars(codec, stress->stress_assessment, sizeof(stress->stress_assessment));

    temperature_limit_monitoring_t *limits = &data->limit_monitoring;
    esocore_codec_f32(codec, &limits->max_temperature_c);
    esocore_codec_f32(codec, &limits->min_temperature_c);
    esocore_codec_u8(codec, &limits->limit_violation_severity);
    esocore_codec_chars(codec, limits->limit_status, sizeof(limits->limit_status));

    esocore_codec_u8(codec, &data->temperature_condition_score);
    esocore_codec_chars(codec, data->condition_description, sizeof(data->condition_description));
    esocore_codec_u32(codec, &data->timestamp);
}

static void temperature_spot_walk(esocore_codec_t *codec, void *data) {
    temperature_spot_sensor_codec(codec, (temperature_spot_sensor_data_t *)data);
}

uint32_t temperature_spot_sensor_encode_data(const temperature_spot_sensor_data_t *data, uint8_t *buffer, uint32_t capacity) {
    return esocore_codec_encode_payload(ESOCORE_DEVICE_TYPE_TEMPERATURE, temperature_spot_walk, data, buffer, capacity);
}

bool temperature_spot_sensor_decode_data(const uint8_t *buffer, uint32_t length, temperature_spot_sensor_data_t *data) {
    return esocore_codec_decode_payload(ESOCORE_DEVICE_TYPE_TEMPERATURE, temperature_spot_walk, buffer, length, data);
}
