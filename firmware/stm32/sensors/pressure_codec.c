/**
 * @file pressure_codec.c
 * @brief Bus payload of pressure_sensor_data_t (layout rules: common/sensors/sensor_codec.h)
 *
 * The fields are walked in the order pressure_sensor.h declares them. Byte
 * offsets of the sections: firmware/README.md, "Sensor Data Payload".
 * pressure_sensor_data_t has no base_data.
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "pressure_sensor.h"

void pressure_sensor_codec(esocore_codec_t *codec, pressure_sensor_data_t *data) {
    esocore_codec_f32(codec, &data->pressure_pa);
    esocore_codec_f32(codec, &data->pressure_bar);
    esocore_codec_f32(codec, &data->temperature_celsius);
    esocore_codec_u32(codec, &data->timestamp);
    esocore_codec_u16(codec, &data->raw_adc_value);
    esocore_codec_f32(codec, &data->pressure_rate_bar_s);
    esocore_codec_f32(codec, &data->pressure_stability);

    pressure_hydraulic_analysis_t *hydraulic = &data->hydraulic_analysis;
    esocore_codec_chars(codec, hydraulic->pump_performance, sizeof(hydraulic->pump_performance));
    esocore_codec_f32(codec, &hydraulic->pump_efficiency);
    esocore_codec_chars(codec, hydraulic->system_stability, sizeof(hydraulic->system_stability));
    esocore_codec_u8(codec, &hydraulic->stability_rating);
    esocore_codec_f32(codec, &hydraulic->fluid_contamination_indicators);
    esocore_codec_u32(codec, &hydraulic->analysis_timestamp);

    pressure_pneumatic_analysis_t *pneumatic = &data->pneumatic_analysis;
    esocore_codec_chars(codec, pneumatic->compressor_performance, sizeof(pneumatic->compressor_performance));
    esocore_codec_f32(codec, &pneumatic->compressor_efficiency);
    esocore_codec_f32(codec, &pneumatic->air_quality_indicators);
    esocore_codec_chars(codec, pneumatic->system_responsiveness, sizeof(pneumatic->system_responsiveness));
    esocore_codec_u32(codec, &pneumatic->response_time_ms);
    esocore_codec_u32(codec, &pneumatic->analysis_timestamp);

    pressure_anomaly_detection_t *anomaly = &data->anomaly_detection;
    esocore_codec_bool(codec, &anomaly->pressure_surge_detected);
    esocore_codec_bool(codec, &anomaly->pressure_drop_detected);
    esocore_codec_bool(codec, &anomaly->rapid_change_detected);
    esocore_codec_bool(codec, &anomaly->oscillation_detected);
    esocore_codec_bool(codec, &anomaly->anomaly_detected);
    esocore_codec_chars(codec, anomaly->anomaly_description, sizeof(anomaly->anomaly_description));
    esocore_codec_u8(codec, &anomaly->severity_level);
    esocore_codec_u32(codec, &anomaly->detection_timestamp);

    pressure_leak_detection_t *leak = &data->leak_detection;
    esocore_codec_chars(codec, leak->leak_status, sizeof(leak->leak_status));
    esocore_codec_bool(codec, &leak->leak_detected);
    esocore_codec_f32(codec, &leak->leak_rate_bar_min);
    esocore_codec_f32(codec, &leak->leak_severity);
    esocore_codec_u32(codec, &leak->detection_timestamp);

    pressure_filter_monitoring_t *filter = &data->filter_monitoring;
    esocore_codec_f32(codec, &filter->differential_pressure_bar);
    esocore_codec_chars(codec, filter->filter_condition, sizeof(filter->filter_condition));
    esocore_codec_f32(codec, &filter->filter_efficiency);
    esocore_codec_f32(codec, &filter->clogging_level);
    esocore_codec_f32(codec, &filter->estimated_life_remaining_percent);
    esocore_codec_u32(codec, &filter->monitoring_timestamp);

    esocore_codec_u8(codec, &data->pressure_condition_score);
    esocore_codec_chars(codec, data->condition_description, sizeof(data->condition_description));
}

static void pressure_walk(esocore_codec_t *codec, void *data) {
    pressure_sensor_codec(codec, (pressure_sensor_data_t *)data);
}

uint32_t pressure_sensor_encode_data(const pressure_sensor_data_t *data, uint8_t *buffer, uint32_t capacity) {
    return esocore_codec_encode_payload(ESOCORE_DEVICE_TYPE_PRESSURE, pressure_walk, data, buffer, capacity);
}

bool pressure_sensor_decode_data(const uint8_t *buffer, uint32_t length, pressure_sensor_data_t *data) {
    return esocore_codec_decode_payload(ESOCORE_DEVICE_TYPE_PRESSURE, pressure_walk, buffer, length, data);
}
