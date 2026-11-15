/**
 * @file proximity_position_codec.c
 * @brief Bus payload of proximity_position_sensor_data_t (layout rules: common/sensors/sensor_codec.h)
 *
 * The fields are walked in the order proximity_position_sensor.h declares them. Byte
 * offsets of the sections: firmware/README.md, "Sensor Data Payload".
 * proximity_position_sensor_data_t has no base_data.
 *
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "proximity_position_sensor.h"

void proximity_position_sensor_codec(esocore_codec_t *codec, proximity_position_sensor_data_t *data) {
    esocore_codec_f32(codec, &data->distance_mm);
    esocore_codec_f32(codec, &data->position_x_mm);
    esocore_codec_f32(codec, &data->position_y_mm);
    esocore_codec_f32(codec, &data->position_z_mm);
    esocore_codec_u8(codec, &data->target_count);

    proximity_inductive_data_t *inductive = &data->inductive_data;
    esocore_codec_bool_array(codec, inductive->sensor_states, 2);
    esocore_codec_u8(codec, &inductive->target_count);
    esocore_codec_bool(codec, &inductive->valid);

    proximity_capacitive_data_t *capacitive = &data->capacitive_data;
    esocore_codec_f32(codec, &capacitive->distance_mm);
    esocore_codec_f32(codec, &capacitive->capacitance_pf);
    esocore_codec_bool(codec, &capacitive->target_detected);
    esocore_codec_bool(codec, &capacitive->valid);

    proximity_encoder_data_t *encoder = &data->encoder_data;
    esocore_codec_i32(codec, &encoder->position_counts);
    esocore_codec_f32(codec, &encoder->position_mm);
    esocore_codec_f32(codec, &encoder->velocity_mm_s);
    esocore_codec_f32(codec, &encoder->acceleration_mm_s2);
    esocore_codec_i8(codec, &encoder->direction);
    esocore_codec_bool(codec, &encoder->valid);

    proximity_magnetic_data_t *magnetic = &data->magnetic_data;
    esocore_codec_bool_array(codec, magnetic->sensor_states, 2);
    esocore_codec_f32(codec, &magnetic->magnetic_field_mt);
    esocore_codec_bool(codec, &magnetic->valid);

    proximity_laser_data_t *laser = &data->laser_data;
    esocore_codec_f32(codec, &laser->distance_mm);
    esocore_codec_u16(codec, &laser->signal_rate_mcps);
    esocore_codec_u16(codec, &laser->ambient_rate_mcps);
    esocore_codec_u8(codec, &laser->range_status);
    esocore_codec_bool(codec, &laser->valid);

    position_accuracy_analysis_t *accuracy = &data->accuracy_analysis;
    esocore_codec_f32(codec, &accuracy->encoder_resolution_mm);
    esocore_codec_f32(codec, &accuracy->encoder_accuracy_mm);
    esocore_codec_u8(codec, &accuracy->encoder_precision_bits);
    esocore_codec_chars(codec, accuracy->encoder_accuracy_assessment, sizeof(accuracy->encoder_accuracy_assessment));
    esocore_codec_u8(codec, &accuracy->encoder_accuracy_rating);
    esocore_codec_f32(codec, &accuracy->laser_accuracy_mm);
    esocore_codec_f32(codec, &accuracy->laser_precision_mm);
    esocore_codec_chars(codec, accuracy->laser_accuracy_assessment, sizeof(accuracy->laser_accuracy_assessment));
    esocore_codec_u8(codec, &accuracy->laser_accuracy_rating);
    esocore_codec_f32(codec, &accuracy->overall_accuracy_mm);
    esocore_codec_chars(codec, accuracy->redundancy_benefits, sizeof(accuracy->redundancy_benefits));

    motion_analysis_t *motion = &data->motion_analysis;
    esocore_codec_chars(codec, motion->speed_profile, sizeof(motion->speed_profile));
    esocore_codec_u8(codec, &motion->motion_stability);
    esocore_codec_f32(codec, &motion->acceleration_mm_s2);
    esocore_codec_chars(codec, motion->acceleration_profile, sizeof(motion->acceleration_profile));
    esocore_codec_f32(codec, &motion->vibration_level);
    esocore_codec_chars(codec, motion->direction_consistency, sizeof(motion->direction_consistency));
    esocore_codec_u8(codec, &motion->direction_stability);

    proximity_detection_analysis_t *detection = &data->detection_analysis;
    esocore_codec_chars(codec, detection->detection_reliability, sizeof(detection->detection_reliability));
    esocore_codec_f32(codec, &detection->false_positive_rate);
    esocore_codec_u8(codec, &detection->detection_confidence);
    esocore_codec_f32(codec, &detection->effective_range_mm);
    esocore_codec_f32(codec, &detection->blind_zone_mm);
    esocore_codec_chars(codec, detection->coverage_assessment, sizeof(detection->coverage_assessment));

    system_health_monitoring_t *health = &data->health_monitoring;
    esocore_codec_u8(codec, &health->sensor_health_percentage);
    esocore_codec_chars(codec, health->system_health_status, sizeof(health->system_health_status));
    esocore_codec_u8(codec, &health->system_reliability_rating);
    esocore_codec_bool(codec, &health->calibration_drift_detected);
    esocore_codec_f32(codec, &health->environmental_interference_level);

    esocore_codec_u8(codec, &data->position_condition_score);
    esocore_codec_chars(codec, data->condition_description, sizeof(data->condition_description));
    esocore_codec_u32(codec, &data->timestamp);
}

static void proximity_position_walk(esocore_codec_t *codec, void *data) {
    proximity_position_sensor_codec(codec, (proximity_position_sensor_data_t *)data);
}

uint32_t proximity_position_sensor_encode_data(const proximity_position_sensor_data_t *data, uint8_t *buffer, uint32_t capacity) {
    return esocore_codec_encode_payload(ESOCORE_DEVICE_TYPE_PROXIMITY, proximity_position_walk, data, buffer, capacity);
}

bool proximity_position_sensor_decode_data(const uint8_t *buffer, uint32_t length, proximity_position_sensor_data_t *data) {
    return esocore_codec_decode_payload(ESOCORE_DEVICE_TYPE_PROXIMITY, proximity_position_walk, buffer, length, data);
}
