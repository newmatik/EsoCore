/**
 * @file temperature_spot_sensor.h
 * @brief Temperature Spot Sensor Driver
 */

#ifndef TEMPERATURE_SPOT_SENSOR_H
#define TEMPERATURE_SPOT_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

typedef struct {
    esocore_sensor_config_t base_config;
    uint8_t sensor_type;
    float temp_range_min;
    float temp_range_max;
    uint8_t thermocouple_type;
    bool enable_cold_junction_comp;
} temperature_spot_sensor_config_t;

typedef struct {
    float temperature_celsius;
    float cold_junction_temp;
    uint32_t timestamp;
} temperature_spot_sensor_data_t;

bool temperature_spot_sensor_init(const temperature_spot_sensor_config_t *config);
bool temperature_spot_sensor_read_data(temperature_spot_sensor_data_t *data, uint32_t timeout_ms);
const esocore_sensor_capabilities_t *temperature_spot_sensor_get_capabilities(void);

#endif /* TEMPERATURE_SPOT_SENSOR_H */
