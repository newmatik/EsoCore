/**
 * @file sensor_interface.c
 * @brief Unified Sensor Interface Implementation
 *
 * This file contains the implementation of the unified sensor interface for all EsoCore sensor modules,
 * providing a consistent API for sensor initialization, data acquisition, and control.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "sensor_interface.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Private Data Structures
 * ============================================================================ */

static bool sensor_interface_initialized = false;

/* Sensor registry */
static struct {
    esocore_sensor_config_t config;
    esocore_sensor_status_t status;
    esocore_sensor_capabilities_t capabilities;
    bool registered;
    bool initialized;
} sensor_registry[ESOCORE_MAX_SENSORS];

/* Sensor statistics */
static uint32_t total_sensors_registered = 0;
static uint32_t total_sensors_active = 0;

/* ============================================================================
 * Sensor Hardware Abstraction Layer (HAL) - Placeholder Implementations
 * ============================================================================ */

/**
 * @brief Initialize sensor hardware
 */
static bool sensor_hw_init(esocore_sensor_type_t sensor_type, uint8_t sensor_id) {
    /* TODO: Implement hardware-specific sensor initialization */
    return true;
}

/**
 * @brief Deinitialize sensor hardware
 */
static bool sensor_hw_deinit(esocore_sensor_type_t sensor_type, uint8_t sensor_id) {
    /* TODO: Implement hardware-specific sensor deinitialization */
    return true;
}

/**
 * @brief Configure sensor hardware parameters
 */
static bool sensor_hw_configure(esocore_sensor_type_t sensor_type, uint8_t sensor_id,
                               const esocore_sensor_config_t *config) {
    /* TODO: Implement hardware-specific sensor configuration */
    return true;
}

/**
 * @brief Start sensor data acquisition
 */
static bool sensor_hw_start_acquisition(esocore_sensor_type_t sensor_type, uint8_t sensor_id) {
    /* TODO: Implement hardware-specific data acquisition start */
    return true;
}

/**
 * @brief Stop sensor data acquisition
 */
static bool sensor_hw_stop_acquisition(esocore_sensor_type_t sensor_type, uint8_t sensor_id) {
    /* TODO: Implement hardware-specific data acquisition stop */
    return true;
}

/**
 * @brief Read sensor data from hardware
 */
static bool sensor_hw_read_data(esocore_sensor_type_t sensor_type, uint8_t sensor_id,
                               uint8_t *data, uint16_t max_length, uint16_t *data_length) {
    /* TODO: Implement hardware-specific data reading */
    return false;
}

/**
 * @brief Perform sensor self-test
 */
static bool sensor_hw_self_test(esocore_sensor_type_t sensor_type, uint8_t sensor_id) {
    /* TODO: Implement hardware-specific self-test */
    return true;
}

/**
 * @brief Calibrate sensor hardware
 */
static bool sensor_hw_calibrate(esocore_sensor_type_t sensor_type, uint8_t sensor_id,
                               const esocore_sensor_calibration_t *calibration_data) {
    /* TODO: Implement hardware-specific calibration */
    return true;
}

/**
 * @brief Set sensor operating mode
 */
static bool sensor_hw_set_mode(esocore_sensor_type_t sensor_type, uint8_t sensor_id,
                              esocore_sensor_mode_t mode) {
    /* TODO: Implement hardware-specific mode setting */
    return true;
}

/**
 * @brief Get sensor status from hardware
 */
static bool sensor_hw_get_status(esocore_sensor_type_t sensor_type, uint8_t sensor_id,
                                esocore_sensor_status_t *status) {
    /* TODO: Implement hardware-specific status retrieval */
    return true;
}

/**
 * @brief Get sensor temperature (if supported)
 */
static bool sensor_hw_get_temperature(esocore_sensor_type_t sensor_type, uint8_t sensor_id,
                                     int8_t *temperature_celsius) {
    /* TODO: Implement hardware-specific temperature reading */
    return false;
}

/* ============================================================================
 * Sensor Registry Management
 * ============================================================================ */

/**
 * @brief Find free sensor slot in registry
 */
static int8_t find_free_sensor_slot(void) {
    for (uint8_t i = 0; i < ESOCORE_MAX_SENSORS; i++) {
        if (!sensor_registry[i].registered) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Find sensor by ID
 */
static int8_t find_sensor_by_id(uint8_t sensor_id) {
    for (uint8_t i = 0; i < ESOCORE_MAX_SENSORS; i++) {
        if (sensor_registry[i].registered && sensor_registry[i].config.sensor_id == sensor_id) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Initialize sensor capabilities based on type
 */
static bool initialize_sensor_capabilities(esocore_sensor_type_t sensor_type,
                                          esocore_sensor_capabilities_t *capabilities) {
    if (!capabilities) {
        return false;
    }

    memset(capabilities, 0, sizeof(esocore_sensor_capabilities_t));
    capabilities->sensor_type = sensor_type;

    switch (sensor_type) {
        case ESOCORE_SENSOR_VIBRATION:
            strcpy(capabilities->sensor_type_name, "Vibration Sensor");
            capabilities->max_sampling_rate_hz = 10000;
            capabilities->min_sampling_rate_hz = 10;
            capabilities->max_gain_level = 8;
            capabilities->max_data_points = 1024;
            capabilities->supports_calibration = true;
            capabilities->supports_temperature_compensation = true;
            capabilities->supports_self_test = true;
            capabilities->supports_sleep_mode = true;
            capabilities->supports_burst_mode = true;
            capabilities->power_consumption_ua = 1500;
            break;

        case ESOCORE_SENSOR_ACOUSTIC:
            strcpy(capabilities->sensor_type_name, "Acoustic Sensor");
            capabilities->max_sampling_rate_hz = 48000;
            capabilities->min_sampling_rate_hz = 8000;
            capabilities->max_gain_level = 16;
            capabilities->max_data_points = 2048;
            capabilities->supports_calibration = true;
            capabilities->supports_temperature_compensation = true;
            capabilities->supports_self_test = true;
            capabilities->supports_sleep_mode = true;
            capabilities->supports_burst_mode = false;
            capabilities->power_consumption_ua = 2000;
            break;

        case ESOCORE_SENSOR_CURRENT:
            strcpy(capabilities->sensor_type_name, "Current Sensor");
            capabilities->max_sampling_rate_hz = 1000;
            capabilities->min_sampling_rate_hz = 10;
            capabilities->max_gain_level = 4;
            capabilities->max_data_points = 256;
            capabilities->supports_calibration = true;
            capabilities->supports_temperature_compensation = true;
            capabilities->supports_self_test = true;
            capabilities->supports_sleep_mode = true;
            capabilities->supports_burst_mode = false;
            capabilities->power_consumption_ua = 800;
            break;

        case ESOCORE_SENSOR_AIR_QUALITY:
            strcpy(capabilities->sensor_type_name, "Air Quality Sensor");
            capabilities->max_sampling_rate_hz = 10;
            capabilities->min_sampling_rate_hz = 1;
            capabilities->max_gain_level = 2;
            capabilities->max_data_points = 16;
            capabilities->supports_calibration = true;
            capabilities->supports_temperature_compensation = true;
            capabilities->supports_self_test = true;
            capabilities->supports_sleep_mode = true;
            capabilities->supports_burst_mode = false;
            capabilities->power_consumption_ua = 1200;
            break;

        case ESOCORE_SENSOR_LIGHT:
            strcpy(capabilities->sensor_type_name, "Light Sensor");
            capabilities->max_sampling_rate_hz = 100;
            capabilities->min_sampling_rate_hz = 1;
            capabilities->max_gain_level = 8;
            capabilities->max_data_points = 32;
            capabilities->supports_calibration = true;
            capabilities->supports_temperature_compensation = false;
            capabilities->supports_self_test = true;
            capabilities->supports_sleep_mode = true;
            capabilities->supports_burst_mode = false;
            capabilities->power_consumption_ua = 500;
            break;

        case ESOCORE_SENSOR_OIL_QUALITY:
            strcpy(capabilities->sensor_type_name, "Oil Quality Sensor");
            capabilities->max_sampling_rate_hz = 10;
            capabilities->min_sampling_rate_hz = 1;
            capabilities->max_gain_level = 4;
            capabilities->max_data_points = 8;
            capabilities->supports_calibration = true;
            capabilities->supports_temperature_compensation = true;
            capabilities->supports_self_test = true;
            capabilities->supports_sleep_mode = true;
            capabilities->supports_burst_mode = false;
            capabilities->power_consumption_ua = 1000;
            break;

        case ESOCORE_SENSOR_PRESSURE:
            strcpy(capabilities->sensor_type_name, "Pressure Sensor");
            capabilities->max_sampling_rate_hz = 1000;
            capabilities->min_sampling_rate_hz = 10;
            capabilities->max_gain_level = 4;
            capabilities->max_data_points = 128;
            capabilities->supports_calibration = true;
            capabilities->supports_temperature_compensation = true;
            capabilities->supports_self_test = true;
            capabilities->supports_sleep_mode = true;
            capabilities->supports_burst_mode = false;
            capabilities->power_consumption_ua = 600;
            break;

        case ESOCORE_SENSOR_TEMPERATURE:
            strcpy(capabilities->sensor_type_name, "Temperature Sensor");
            capabilities->max_sampling_rate_hz = 100;
            capabilities->min_sampling_rate_hz = 1;
            capabilities->max_gain_level = 2;
            capabilities->max_data_points = 16;
            capabilities->supports_calibration = true;
            capabilities->supports_temperature_compensation = false;
            capabilities->supports_self_test = true;
            capabilities->supports_sleep_mode = true;
            capabilities->supports_burst_mode = false;
            capabilities->power_consumption_ua = 300;
            break;

        case ESOCORE_SENSOR_PROXIMITY:
            strcpy(capabilities->sensor_type_name, "Proximity Sensor");
            capabilities->max_sampling_rate_hz = 1000;
            capabilities->min_sampling_rate_hz = 10;
            capabilities->max_gain_level = 4;
            capabilities->max_data_points = 64;
            capabilities->supports_calibration = true;
            capabilities->supports_temperature_compensation = false;
            capabilities->supports_self_test = true;
            capabilities->supports_sleep_mode = true;
            capabilities->supports_burst_mode = false;
            capabilities->power_consumption_ua = 400;
            break;

        default:
            return false;
    }

    return true;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

/**
 * @brief Initialize sensor interface
 */
bool esocore_sensor_init(void) {
    if (sensor_interface_initialized) {
        return false;
    }

    /* Initialize sensor registry */
    memset(sensor_registry, 0, sizeof(sensor_registry));

    /* Reset statistics */
    total_sensors_registered = 0;
    total_sensors_active = 0;

    sensor_interface_initialized = true;
    return true;
}

/**
 * @brief Register a sensor with the interface
 */
bool esocore_sensor_register(esocore_sensor_type_t sensor_type, uint8_t sensor_id,
                            const esocore_sensor_config_t *config) {
    if (!sensor_interface_initialized || !config) {
        return false;
    }

    /* Check if sensor ID is already registered */
    if (find_sensor_by_id(sensor_id) >= 0) {
        return false;
    }

    /* Find free slot in registry */
    int8_t slot = find_free_sensor_slot();
    if (slot < 0) {
        return false;
    }

    /* Initialize sensor registry entry */
    sensor_registry[slot].config = *config;
    sensor_registry[slot].config.sensor_type = sensor_type;
    sensor_registry[slot].config.sensor_id = sensor_id;

    /* Initialize sensor status */
    memset(&sensor_registry[slot].status, 0, sizeof(esocore_sensor_status_t));
    sensor_registry[slot].status.sensor_type = sensor_type;
    sensor_registry[slot].status.sensor_id = sensor_id;
    sensor_registry[slot].status.current_mode = ESOCORE_MODE_STANDBY;
    sensor_registry[slot].status.is_initialized = false;
    sensor_registry[slot].status.is_calibrated = false;
    sensor_registry[slot].status.has_error = false;

    /* Initialize sensor capabilities */
    if (!initialize_sensor_capabilities(sensor_type, &sensor_registry[slot].capabilities)) {
        return false;
    }

    sensor_registry[slot].registered = true;
    sensor_registry[slot].initialized = false;

    total_sensors_registered++;
    return true;
}

/**
 * @brief Unregister a sensor from the interface
 */
bool esocore_sensor_unregister(uint8_t sensor_id) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Stop acquisition if active */
    if (sensor_registry[slot].status.current_mode == ESOCORE_MODE_ACTIVE) {
        esocore_sensor_stop_acquisition(sensor_id);
    }

    /* Deinitialize hardware */
    sensor_hw_deinit(sensor_registry[slot].config.sensor_type, sensor_id);

    /* Clear registry entry */
    memset(&sensor_registry[slot], 0, sizeof(sensor_registry[slot]));

    total_sensors_registered--;
    if (total_sensors_active > 0) {
        total_sensors_active--;
    }

    return true;
}

/**
 * @brief Configure a registered sensor
 */
bool esocore_sensor_configure(uint8_t sensor_id, const esocore_sensor_config_t *config) {
    if (!sensor_interface_initialized || !config) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Update configuration */
    sensor_registry[slot].config = *config;
    sensor_registry[slot].config.sensor_id = sensor_id; /* Ensure ID doesn't change */

    /* Apply hardware configuration */
    return sensor_hw_configure(sensor_registry[slot].config.sensor_type, sensor_id, config);
}

/**
 * @brief Start sensor data acquisition
 */
bool esocore_sensor_start_acquisition(uint8_t sensor_id) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Initialize sensor if not already done */
    if (!sensor_registry[slot].initialized) {
        if (!sensor_hw_init(sensor_registry[slot].config.sensor_type, sensor_id)) {
            sensor_registry[slot].status.has_error = true;
            sensor_registry[slot].status.error_code = 1; /* Initialization error */
            return false;
        }
        sensor_registry[slot].initialized = true;
    }

    /* Start hardware acquisition */
    if (!sensor_hw_start_acquisition(sensor_registry[slot].config.sensor_type, sensor_id)) {
        sensor_registry[slot].status.has_error = true;
        sensor_registry[slot].status.error_code = 2; /* Acquisition start error */
        return false;
    }

    /* Update status */
    sensor_registry[slot].status.current_mode = ESOCORE_MODE_ACTIVE;
    sensor_registry[slot].status.is_initialized = true;
    sensor_registry[slot].status.has_error = false;
    sensor_registry[slot].status.error_code = 0;
    sensor_registry[slot].status.measurements_taken = 0;

    total_sensors_active++;
    return true;
}

/**
 * @brief Stop sensor data acquisition
 */
bool esocore_sensor_stop_acquisition(uint8_t sensor_id) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Stop hardware acquisition */
    if (!sensor_hw_stop_acquisition(sensor_registry[slot].config.sensor_type, sensor_id)) {
        return false;
    }

    /* Update status */
    sensor_registry[slot].status.current_mode = ESOCORE_MODE_STANDBY;

    if (total_sensors_active > 0) {
        total_sensors_active--;
    }

    return true;
}

/**
 * @brief Read sensor data
 */
bool esocore_sensor_read_data(uint8_t sensor_id, esocore_sensor_data_t *data,
                             uint32_t timeout_ms) {
    if (!sensor_interface_initialized || !data) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Check if sensor is active */
    if (sensor_registry[slot].status.current_mode != ESOCORE_MODE_ACTIVE) {
        return false;
    }

    /* Read data from hardware */
    uint16_t data_length;
    if (!sensor_hw_read_data(sensor_registry[slot].config.sensor_type, sensor_id,
                            data->data, ESOCORE_SENSOR_DATA_BUFFER_SIZE, &data_length)) {
        sensor_registry[slot].status.has_error = true;
        sensor_registry[slot].status.error_code = 3; /* Data read error */
        return false;
    }

    /* Fill data structure */
    data->timestamp = 0; /* TODO: Get current timestamp */
    data->sensor_type = sensor_registry[slot].config.sensor_type;
    data->sensor_id = sensor_id;
    data->data_points = data_length / 4; /* Assuming 4 bytes per data point */
    data->data_type = ESOCORE_DATA_TYPE_FLOAT; /* TODO: Determine from sensor type */
    strcpy(data->units, ""); /* TODO: Set appropriate units */
    data->quality_flags = ESOCORE_QUALITY_GOOD; /* TODO: Determine quality */
    data->data_size = data_length;

    /* Update sensor status */
    sensor_registry[slot].status.measurements_taken++;
    sensor_registry[slot].status.last_measurement_time = data->timestamp;
    sensor_registry[slot].status.has_error = false;
    sensor_registry[slot].status.error_code = 0;

    return true;
}

/**
 * @brief Get sensor status
 */
bool esocore_sensor_get_status(uint8_t sensor_id, esocore_sensor_status_t *status) {
    if (!sensor_interface_initialized || !status) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Get hardware status */
    sensor_hw_get_status(sensor_registry[slot].config.sensor_type, sensor_id,
                        &sensor_registry[slot].status);

    /* Copy status to output */
    *status = sensor_registry[slot].status;

    return true;
}

/**
 * @brief Get sensor information
 */
bool esocore_sensor_get_info(uint8_t sensor_id, esocore_sensor_capabilities_t *info) {
    if (!sensor_interface_initialized || !info) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    *info = sensor_registry[slot].capabilities;
    return true;
}

/**
 * @brief Perform sensor self-test
 */
bool esocore_sensor_self_test(uint8_t sensor_id) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    bool test_result = sensor_hw_self_test(sensor_registry[slot].config.sensor_type, sensor_id);

    /* Update status */
    sensor_registry[slot].status.has_error = !test_result;
    sensor_registry[slot].status.error_code = test_result ? 0 : 4; /* Self-test error */

    return test_result;
}

/**
 * @brief Calibrate sensor
 */
bool esocore_sensor_calibrate(uint8_t sensor_id,
                            const esocore_sensor_calibration_t *calibration_data) {
    if (!sensor_interface_initialized || !calibration_data) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Perform hardware calibration */
    if (!sensor_hw_calibrate(sensor_registry[slot].config.sensor_type, sensor_id,
                            calibration_data)) {
        return false;
    }

    /* Update status */
    sensor_registry[slot].status.is_calibrated = true;
    sensor_registry[slot].status.calibration_count++;
    sensor_registry[slot].status.has_error = false;
    sensor_registry[slot].status.error_code = 0;

    return true;
}

/**
 * @brief Get sensor calibration data
 */
bool esocore_sensor_get_calibration(uint8_t sensor_id,
                                  esocore_sensor_calibration_t *calibration_data) {
    if (!sensor_interface_initialized || !calibration_data) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* TODO: Retrieve calibration data from hardware/storage */
    memset(calibration_data, 0, sizeof(esocore_sensor_calibration_t));
    calibration_data->sensor_type = sensor_registry[slot].config.sensor_type;
    calibration_data->sensor_id = sensor_id;
    calibration_data->is_valid = sensor_registry[slot].status.is_calibrated;

    return true;
}

/**
 * @brief Set sensor operating mode
 */
bool esocore_sensor_set_mode(uint8_t sensor_id, esocore_sensor_mode_t mode) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Set hardware mode */
    if (!sensor_hw_set_mode(sensor_registry[slot].config.sensor_type, sensor_id, mode)) {
        return false;
    }

    /* Update status */
    sensor_registry[slot].status.current_mode = mode;

    return true;
}

/**
 * @brief Reset sensor to default state
 */
bool esocore_sensor_reset(uint8_t sensor_id) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Stop acquisition if active */
    if (sensor_registry[slot].status.current_mode == ESOCORE_MODE_ACTIVE) {
        esocore_sensor_stop_acquisition(sensor_id);
    }

    /* Reset hardware */
    sensor_hw_deinit(sensor_registry[slot].config.sensor_type, sensor_id);
    sensor_registry[slot].initialized = false;

    /* Reset status */
    sensor_registry[slot].status.current_mode = ESOCORE_MODE_STANDBY;
    sensor_registry[slot].status.is_calibrated = false;
    sensor_registry[slot].status.has_error = false;
    sensor_registry[slot].status.error_code = 0;
    sensor_registry[slot].status.measurements_taken = 0;

    return true;
}

/**
 * @brief Put sensor into low-power mode
 */
bool esocore_sensor_enter_low_power(uint8_t sensor_id) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Stop acquisition if active */
    if (sensor_registry[slot].status.current_mode == ESOCORE_MODE_ACTIVE) {
        esocore_sensor_stop_acquisition(sensor_id);
    }

    /* Set low-power mode */
    return esocore_sensor_set_mode(sensor_id, ESOCORE_MODE_LOW_POWER);
}

/**
 * @brief Wake sensor from low-power mode
 */
bool esocore_sensor_exit_low_power(uint8_t sensor_id) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Set standby mode */
    return esocore_sensor_set_mode(sensor_id, ESOCORE_MODE_STANDBY);
}

/**
 * @brief Get list of registered sensors
 */
bool esocore_sensor_get_list(uint8_t *sensor_list, uint8_t max_sensors,
                            uint8_t *num_sensors) {
    if (!sensor_interface_initialized || !sensor_list || !num_sensors) {
        return false;
    }

    uint8_t count = 0;
    for (uint8_t i = 0; i < ESOCORE_MAX_SENSORS && count < max_sensors; i++) {
        if (sensor_registry[i].registered) {
            sensor_list[count++] = sensor_registry[i].config.sensor_id;
        }
    }

    *num_sensors = count;
    return true;
}

/**
 * @brief Get sensor statistics
 */
bool esocore_sensor_get_statistics(uint8_t sensor_id, uint32_t *measurements,
                                  uint32_t *errors, uint32_t *uptime) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    if (measurements) {
        *measurements = sensor_registry[slot].status.measurements_taken;
    }
    if (errors) {
        *errors = sensor_registry[slot].status.has_error ? 1 : 0; /* TODO: Track actual error count */
    }
    if (uptime) {
        *uptime = sensor_registry[slot].status.uptime_seconds;
    }

    return true;
}

/**
 * @brief Clear sensor statistics
 */
bool esocore_sensor_clear_statistics(uint8_t sensor_id) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    sensor_registry[slot].status.measurements_taken = 0;
    sensor_registry[slot].status.uptime_seconds = 0;

    return true;
}

/**
 * @brief Check if sensor data is available
 */
bool esocore_sensor_data_available(uint8_t sensor_id) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* TODO: Check hardware for data availability */
    return (sensor_registry[slot].status.current_mode == ESOCORE_MODE_ACTIVE);
}

/**
 * @brief Flush sensor data buffer
 */
bool esocore_sensor_flush_buffer(uint8_t sensor_id) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* TODO: Flush hardware data buffer */
    return true;
}

/**
 * @brief Set sensor sampling parameters
 */
bool esocore_sensor_set_sampling_params(uint8_t sensor_id, uint32_t sampling_rate_hz,
                                       uint32_t sample_count) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Update configuration */
    sensor_registry[slot].config.sampling_rate_hz = sampling_rate_hz;
    sensor_registry[slot].config.sample_count = sample_count;

    /* Apply to hardware */
    return sensor_hw_configure(sensor_registry[slot].config.sensor_type, sensor_id,
                              &sensor_registry[slot].config);
}

/**
 * @brief Set sensor filter parameters
 */
bool esocore_sensor_set_filter_params(uint8_t sensor_id, uint16_t cutoff_hz,
                                     bool enable_filter) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Update configuration */
    sensor_registry[slot].config.filter_cutoff_hz = cutoff_hz;
    sensor_registry[slot].config.enable_filter = enable_filter;

    /* Apply to hardware */
    return sensor_hw_configure(sensor_registry[slot].config.sensor_type, sensor_id,
                              &sensor_registry[slot].config);
}

/**
 * @brief Get sensor temperature (if supported)
 */
bool esocore_sensor_get_temperature(uint8_t sensor_id, int8_t *temperature_celsius) {
    if (!sensor_interface_initialized || !temperature_celsius) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Get temperature from hardware */
    return sensor_hw_get_temperature(sensor_registry[slot].config.sensor_type,
                                    sensor_id, temperature_celsius);
}

/**
 * @brief Get sensor power consumption
 */
bool esocore_sensor_get_power_consumption(uint8_t sensor_id, uint32_t *current_ua) {
    if (!sensor_interface_initialized || !current_ua) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* Return typical power consumption from capabilities */
    *current_ua = sensor_registry[slot].capabilities.power_consumption_ua;
    return true;
}

/**
 * @brief Enable/disable sensor diagnostics
 */
bool esocore_sensor_enable_diagnostics(uint8_t sensor_id, bool enable) {
    if (!sensor_interface_initialized) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    sensor_registry[slot].config.enable_diagnostics = enable;
    return true;
}

/**
 * @brief Get sensor diagnostic information
 */
bool esocore_sensor_get_diagnostics(uint8_t sensor_id, uint8_t *diagnostic_data,
                                  uint16_t data_size) {
    if (!sensor_interface_initialized || !diagnostic_data) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* TODO: Get diagnostic data from hardware */
    memset(diagnostic_data, 0, data_size);
    return true;
}

/**
 * @brief Perform sensor health check
 */
bool esocore_sensor_health_check(uint8_t sensor_id, uint8_t *health_score,
                                uint8_t *issues) {
    if (!sensor_interface_initialized || !health_score || !issues) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    uint8_t score = 100;
    uint8_t issue_count = 0;

    /* Check initialization status */
    if (!sensor_registry[slot].initialized) {
        score -= 20;
        issue_count++;
    }

    /* Check calibration status */
    if (!sensor_registry[slot].status.is_calibrated) {
        score -= 15;
        issue_count++;
    }

    /* Check error status */
    if (sensor_registry[slot].status.has_error) {
        score -= 30;
        issue_count++;
    }

    /* Check measurement activity */
    if (sensor_registry[slot].status.measurements_taken == 0) {
        score -= 10;
        issue_count++;
    }

    *health_score = score;
    *issues = issue_count;

    return true;
}

/**
 * @brief Get sensor firmware version
 */
bool esocore_sensor_get_firmware_version(uint8_t sensor_id, uint8_t *major_version,
                                        uint8_t *minor_version) {
    if (!sensor_interface_initialized || !major_version || !minor_version) {
        return false;
    }

    int8_t slot = find_sensor_by_id(sensor_id);
    if (slot < 0) {
        return false;
    }

    /* TODO: Get firmware version from hardware */
    *major_version = 1;
    *minor_version = 0;

    return true;
}