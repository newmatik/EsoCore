/**
 * @file oil_quality_sensor.h
 * @brief Oil Quality Sensor Driver
 *
 * This header defines the oil quality sensor driver for turbidity, viscosity,
 * water content, and dielectric constant analysis.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef OIL_QUALITY_SENSOR_H
#define OIL_QUALITY_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

typedef struct {
    esocore_sensor_config_t base_config;
    uint8_t sensor_type;
    bool enable_turbidity;
    bool enable_viscosity;
    bool enable_water_content;
    bool enable_dielectric;
    bool enable_temperature;
    float measurement_range;
} oil_quality_sensor_config_t;

typedef struct {
    float turbidity_ntu;
    float viscosity_cst;
    float water_content_ppm;
    float dielectric_constant;
    float temperature_celsius;
    uint32_t timestamp;
} oil_quality_sensor_data_t;

typedef struct {
    esocore_sensor_status_t base_status;
    bool sensors_ready;
    uint32_t measurements_taken;
    bool calibration_valid;
} oil_quality_sensor_status_t;

bool oil_quality_sensor_init(const oil_quality_sensor_config_t *config);
bool oil_quality_sensor_read_data(oil_quality_sensor_data_t *data, uint32_t timeout_ms);
bool oil_quality_sensor_get_status(oil_quality_sensor_status_t *status);
const esocore_sensor_capabilities_t *oil_quality_sensor_get_capabilities(void);

#endif /* OIL_QUALITY_SENSOR_H */
