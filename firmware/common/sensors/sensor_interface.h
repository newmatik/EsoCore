/**
 * @file sensor_interface.h
 * @brief Common Sensor Interface for EsoCore Sensors
 *
 * This file declares the sensor registry implemented in sensor_interface.c
 * and the base types that every EsoCore sensor module embeds in its own
 * configuration, status and data structures.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_SENSOR_INTERFACE_H
#define ESOCORE_SENSOR_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Constants
 * ============================================================================ */

#define ESOCORE_MAX_SENSORS              16    /* Entries in the sensor registry */
#define ESOCORE_SENSOR_DATA_BUFFER_SIZE  256   /* Raw data bytes per reading */
#define ESOCORE_SENSOR_NAME_LENGTH       32    /* Sensor type name, with terminator */
#define ESOCORE_SENSOR_UNITS_LENGTH      8     /* Unit string, with terminator */

/* Data quality flags (esocore_sensor_data_t.quality_flags) */
#define ESOCORE_QUALITY_GOOD             0x00  /* Reading is valid */

/* ============================================================================
 * Enumerations
 * ============================================================================ */

/**
 * @brief Sensor types, numbered like the device types of the bus protocol
 */
typedef enum {
    ESOCORE_SENSOR_VIBRATION   = ESOCORE_DEVICE_TYPE_VIBRATION,
    ESOCORE_SENSOR_ACOUSTIC    = ESOCORE_DEVICE_TYPE_ACOUSTIC,
    ESOCORE_SENSOR_CURRENT     = ESOCORE_DEVICE_TYPE_CURRENT,
    ESOCORE_SENSOR_AIR_QUALITY = ESOCORE_DEVICE_TYPE_AIR_QUALITY,
    ESOCORE_SENSOR_LIGHT       = ESOCORE_DEVICE_TYPE_LIGHT,
    ESOCORE_SENSOR_OIL_QUALITY = ESOCORE_DEVICE_TYPE_OIL_QUALITY,
    ESOCORE_SENSOR_PRESSURE    = ESOCORE_DEVICE_TYPE_PRESSURE,
    ESOCORE_SENSOR_TEMPERATURE = ESOCORE_DEVICE_TYPE_TEMPERATURE,
    ESOCORE_SENSOR_PROXIMITY   = ESOCORE_DEVICE_TYPE_PROXIMITY,
} esocore_sensor_type_t;

/**
 * @brief Sensor operating modes
 */
typedef enum {
    ESOCORE_MODE_STANDBY   = 0,   /* Registered, not acquiring */
    ESOCORE_MODE_ACTIVE    = 1,   /* Acquiring data */
    ESOCORE_MODE_LOW_POWER = 2,   /* Low-power mode */
} esocore_sensor_mode_t;

/**
 * @brief Format of the values in esocore_sensor_data_t.data
 */
typedef enum {
    ESOCORE_DATA_TYPE_FLOAT = 0,  /* 32-bit floating point values */
} esocore_sensor_data_type_t;

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief Base sensor configuration
 */
typedef struct {
    esocore_sensor_type_t sensor_type;   /* Type of sensor */
    uint8_t sensor_id;                   /* Sensor ID in the registry */
    uint32_t sampling_rate_hz;           /* Sampling rate in Hz */
    uint32_t sample_count;               /* Samples per measurement */
    uint32_t measurement_interval_ms;    /* Time between measurements */
    uint16_t filter_cutoff_hz;           /* Filter cutoff frequency */
    bool enable_filter;                  /* Enable the filter */
    bool enable_diagnostics;             /* Enable diagnostic output */
} esocore_sensor_config_t;

/**
 * @brief Base sensor status
 */
typedef struct {
    esocore_sensor_type_t sensor_type;   /* Type of sensor */
    uint8_t sensor_id;                   /* Sensor ID in the registry */
    esocore_sensor_mode_t current_mode;  /* Current operating mode */
    bool is_initialized;                 /* Hardware is initialized */
    bool is_calibrated;                  /* Sensor has been calibrated */
    bool has_error;                      /* An error is pending */
    uint8_t error_code;                  /* Code of the pending error */
    uint32_t measurements_taken;         /* Measurements since acquisition start */
    uint32_t last_measurement_time;      /* Timestamp of the last measurement */
    uint32_t calibration_count;          /* Number of calibrations performed */
    uint32_t uptime_seconds;             /* Sensor uptime in seconds */
} esocore_sensor_status_t;

/**
 * @brief Sensor capabilities
 */
typedef struct {
    esocore_sensor_type_t sensor_type;   /* Type of sensor */
    char sensor_type_name[ESOCORE_SENSOR_NAME_LENGTH]; /* Human-readable type name */
    uint32_t max_sampling_rate_hz;       /* Highest sampling rate in Hz */
    uint32_t min_sampling_rate_hz;       /* Lowest sampling rate in Hz */
    uint8_t max_gain_level;              /* Highest gain level */
    uint16_t max_data_points;            /* Data points per measurement */
    bool supports_calibration;           /* Sensor can be calibrated */
    bool supports_temperature_compensation; /* Readings are temperature compensated */
    bool supports_self_test;             /* Sensor has a self-test */
    bool supports_sleep_mode;            /* Sensor has a low-power mode */
    bool supports_burst_mode;            /* Sensor can acquire in bursts */
    uint32_t power_consumption_ua;       /* Typical supply current in µA */
} esocore_sensor_capabilities_t;

/**
 * @brief Sensor calibration record
 */
typedef struct {
    esocore_sensor_type_t sensor_type;   /* Type of sensor */
    uint8_t sensor_id;                   /* Sensor ID in the registry */
    bool is_valid;                       /* Calibration is valid */
} esocore_sensor_calibration_t;

/**
 * @brief Base sensor reading
 */
typedef struct {
    uint32_t timestamp;                  /* Time of the reading */
    esocore_sensor_type_t sensor_type;   /* Type of sensor */
    uint8_t sensor_id;                   /* Sensor ID in the registry */
    uint16_t data_points;                /* Number of values in data */
    uint8_t data_type;                   /* Value format (esocore_sensor_data_type_t) */
    char units[ESOCORE_SENSOR_UNITS_LENGTH]; /* Unit of the values */
    uint8_t quality_flags;               /* Data quality flags (ESOCORE_QUALITY_*) */
    uint16_t data_size;                  /* Bytes used in data */
    uint8_t data[ESOCORE_SENSOR_DATA_BUFFER_SIZE]; /* Raw values */
} esocore_sensor_data_t;

/* ============================================================================
 * Registry
 * ============================================================================ */

/**
 * @brief Initialize the sensor interface and clear the registry
 */
bool esocore_sensor_init(void);

/**
 * @brief Register a sensor
 *
 * @param sensor_type Type of sensor
 * @param sensor_id ID to register the sensor under; must not be in use
 * @param config Base configuration
 * @return true if the sensor was registered
 */
bool esocore_sensor_register(esocore_sensor_type_t sensor_type, uint8_t sensor_id,
                            const esocore_sensor_config_t *config);

/** @brief Remove a sensor from the registry */
bool esocore_sensor_unregister(uint8_t sensor_id);

/** @brief List the IDs of the registered sensors */
bool esocore_sensor_get_list(uint8_t *sensor_list, uint8_t max_sensors,
                            uint8_t *num_sensors);

/* ============================================================================
 * Configuration
 * ============================================================================ */

/** @brief Replace the base configuration of a sensor */
bool esocore_sensor_configure(uint8_t sensor_id, const esocore_sensor_config_t *config);

/** @brief Set sampling rate and samples per measurement */
bool esocore_sensor_set_sampling_params(uint8_t sensor_id, uint32_t sampling_rate_hz,
                                       uint32_t sample_count);

/** @brief Set the filter cutoff and switch the filter on or off */
bool esocore_sensor_set_filter_params(uint8_t sensor_id, uint16_t cutoff_hz,
                                     bool enable_filter);

/** @brief Set the operating mode */
bool esocore_sensor_set_mode(uint8_t sensor_id, esocore_sensor_mode_t mode);

/** @brief Stop, reinitialize and return the sensor to standby */
bool esocore_sensor_reset(uint8_t sensor_id);

/** @brief Enter low-power mode */
bool esocore_sensor_enter_low_power(uint8_t sensor_id);

/** @brief Leave low-power mode */
bool esocore_sensor_exit_low_power(uint8_t sensor_id);

/* ============================================================================
 * Data Acquisition
 * ============================================================================ */

/** @brief Start data acquisition; initializes the hardware on first use */
bool esocore_sensor_start_acquisition(uint8_t sensor_id);

/** @brief Stop data acquisition */
bool esocore_sensor_stop_acquisition(uint8_t sensor_id);

/**
 * @brief Read one reading from an active sensor
 *
 * @param sensor_id Sensor to read
 * @param data Reading to fill
 * @param timeout_ms Time to wait for data
 * @return true if a reading was stored in data
 */
bool esocore_sensor_read_data(uint8_t sensor_id, esocore_sensor_data_t *data,
                             uint32_t timeout_ms);

/** @brief Whether a reading is waiting */
bool esocore_sensor_data_available(uint8_t sensor_id);

/** @brief Discard buffered readings */
bool esocore_sensor_flush_buffer(uint8_t sensor_id);

/* ============================================================================
 * Status, Calibration and Diagnostics
 * ============================================================================ */

/** @brief Get the base status of a sensor */
bool esocore_sensor_get_status(uint8_t sensor_id, esocore_sensor_status_t *status);

/** @brief Get the capabilities of a sensor */
bool esocore_sensor_get_info(uint8_t sensor_id, esocore_sensor_capabilities_t *info);

/** @brief Run the sensor self-test */
bool esocore_sensor_self_test(uint8_t sensor_id);

/** @brief Calibrate a sensor */
bool esocore_sensor_calibrate(uint8_t sensor_id,
                            const esocore_sensor_calibration_t *calibration_data);

/** @brief Read the calibration record of a sensor */
bool esocore_sensor_get_calibration(uint8_t sensor_id,
                                  esocore_sensor_calibration_t *calibration_data);

/** @brief Get measurement, error and uptime counters */
bool esocore_sensor_get_statistics(uint8_t sensor_id, uint32_t *measurements,
                                  uint32_t *errors, uint32_t *uptime);

/** @brief Reset the counters of a sensor */
bool esocore_sensor_clear_statistics(uint8_t sensor_id);

/** @brief Read the sensor temperature in degrees Celsius */
bool esocore_sensor_get_temperature(uint8_t sensor_id, int8_t *temperature_celsius);

/** @brief Get the typical supply current in µA */
bool esocore_sensor_get_power_consumption(uint8_t sensor_id, uint32_t *current_ua);

/** @brief Switch diagnostic output on or off */
bool esocore_sensor_enable_diagnostics(uint8_t sensor_id, bool enable);

/** @brief Read diagnostic data */
bool esocore_sensor_get_diagnostics(uint8_t sensor_id, uint8_t *diagnostic_data,
                                  uint16_t data_size);

/** @brief Score the sensor health from 0 to 100 and count the issues found */
bool esocore_sensor_health_check(uint8_t sensor_id, uint8_t *health_score,
                                uint8_t *issues);

/** @brief Get the firmware version of a sensor */
bool esocore_sensor_get_firmware_version(uint8_t sensor_id, uint8_t *major_version,
                                        uint8_t *minor_version);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_SENSOR_INTERFACE_H */
