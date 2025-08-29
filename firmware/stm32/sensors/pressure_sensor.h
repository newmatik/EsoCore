/**
 * @file pressure_sensor.h
 * @brief Pressure Sensor Driver
 */

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

typedef struct {
    esocore_sensor_config_t base_config;
    uint8_t sensor_type;
    float pressure_range_min;
    float pressure_range_max;
    uint8_t measurement_unit;
    bool enable_temperature_comp;
    bool enable_burst_mode;
} pressure_sensor_config_t;

typedef struct {
    float pressure_pa;
    float temperature_celsius;
    uint32_t timestamp;
} pressure_sensor_data_t;

bool pressure_sensor_init(const pressure_sensor_config_t *config);
bool pressure_sensor_read_data(pressure_sensor_data_t *data, uint32_t timeout_ms);
const esocore_sensor_capabilities_t *pressure_sensor_get_capabilities(void);

#endif /* PRESSURE_SENSOR_H */
