/**
 * @file proximity_position_sensor.h
 * @brief Proximity Position Sensor Driver
 */

#ifndef PROXIMITY_POSITION_SENSOR_H
#define PROXIMITY_POSITION_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

typedef struct {
    esocore_sensor_config_t base_config;
    uint8_t sensor_type;
    float detection_range_mm;
    uint8_t measurement_mode;
    bool enable_multitarget;
    uint8_t output_format;
} proximity_position_sensor_config_t;

typedef struct {
    float distance_mm;
    float position_x_mm;
    float position_y_mm;
    float position_z_mm;
    uint8_t target_count;
    uint32_t timestamp;
} proximity_position_sensor_data_t;

bool proximity_position_sensor_init(const proximity_position_sensor_config_t *config);
bool proximity_position_sensor_read_data(proximity_position_sensor_data_t *data, uint32_t timeout_ms);
const esocore_sensor_capabilities_t *proximity_position_sensor_get_capabilities(void);

#endif /* PROXIMITY_POSITION_SENSOR_H */
