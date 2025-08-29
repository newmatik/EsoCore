/**
 * @file sensor_interface.h
 * @brief Common Sensor Interface for EsoCore Sensors
 *
 * This file defines the common interface that all EsoCore sensor modules
 * must implement for standardized operation and communication.
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
 * Sensor Status Flags
 * ============================================================================ */

#define SENSOR_STATUS_READY          (1 << 0)  /* Sensor is ready for operation */
#define SENSOR_STATUS_CALIBRATING    (1 << 1)  /* Sensor is calibrating */
#define SENSOR_STATUS_ERROR          (1 << 2)  /* Sensor has detected an error */
#define SENSOR_STATUS_OVER_RANGE     (1 << 3)  /* Sensor reading is over range */
#define SENSOR_STATUS_UNDER_RANGE    (1 << 4)  /* Sensor reading is under range */
#define SENSOR_STATUS_NEEDS_MAINT    (1 << 5)  /* Sensor needs maintenance */
#define SENSOR_STATUS_LOW_POWER      (1 << 6)  /* Sensor has low power */

/* ============================================================================
 * Sensor Data Types
 * ============================================================================ */

typedef enum {
    DATA_TYPE_RAW             = 0x00,   /* Raw sensor readings */
    DATA_TYPE_PROCESSED       = 0x01,   /* Processed/filtered data */
    DATA_TYPE_FFT             = 0x02,   /* Frequency domain data */
    DATA_TYPE_STATISTICS      = 0x03,   /* Statistical data (RMS, min, max, etc.) */
    DATA_TYPE_ALARM           = 0x04,   /* Alarm/threshold data */
    DATA_TYPE_DIAGNOSTIC      = 0x05,   /* Diagnostic information */
} esocore_data_type_t;

/* ============================================================================
 * Sensor Configuration
 * ============================================================================ */

typedef struct {
    uint32_t sample_rate_hz;            /* Sample rate in Hz */
    uint16_t samples_per_packet;        /* Samples per data packet */
    uint32_t alarm_threshold_high;      /* High alarm threshold */
    uint32_t alarm_threshold_low;       /* Low alarm threshold */
    uint16_t filter_cutoff_hz;          /* Low-pass filter cutoff frequency */
    uint8_t gain_setting;               /* Sensor gain setting */
    uint8_t averaging_samples;          /* Number of samples for averaging */
    bool auto_calibration;              /* Enable automatic calibration */
    bool threshold_enabled;             /* Enable threshold checking */
} esocore_sensor_config_t;

/* ============================================================================
 * Sensor Information
 * ============================================================================ */

typedef struct {
    esocore_device_type_t sensor_type;   /* Type of sensor */
    char sensor_name[32];                /* Human-readable sensor name */
    char manufacturer[16];               /* Manufacturer name */
    char model[16];                      /* Model number */
    uint16_t max_sample_rate;            /* Maximum sample rate in Hz */
    uint16_t min_sample_rate;            /* Minimum sample rate in Hz */
    uint32_t measurement_range_min;      /* Minimum measurement range */
    uint32_t measurement_range_max;      /* Maximum measurement range */
    uint8_t resolution_bits;             /* ADC resolution in bits */
    uint8_t num_channels;                /* Number of measurement channels */
    uint32_t capabilities;               /* Sensor capabilities flags */
} esocore_sensor_info_t;

/* ============================================================================
 * Sensor Data Structure
 * ============================================================================ */

typedef struct {
    uint32_t timestamp;                  /* Unix timestamp */
    esocore_data_type_t data_type;       /* Type of data */
    uint8_t channel_id;                  /* Sensor channel ID */
    uint16_t num_samples;                /* Number of samples in this packet */
    uint8_t data_format_version;         /* Data format version */
    union {
        /* Raw/processed data */
        struct {
            float values[64];           /* Sensor readings */
        } scalar_data;

        /* FFT data */
        struct {
            float magnitudes[32];       /* FFT magnitudes */
            float frequencies[32];      /* Corresponding frequencies */
        } fft_data;

        /* Statistical data */
        struct {
            float rms;                  /* Root mean square */
            float peak;                 /* Peak value */
            float min;                  /* Minimum value */
            float max;                  /* Maximum value */
            float mean;                 /* Mean value */
            float std_dev;              /* Standard deviation */
        } statistics;

        /* Alarm data */
        struct {
            uint8_t alarm_type;         /* Type of alarm triggered */
            float threshold_value;      /* Threshold value that triggered alarm */
            float actual_value;         /* Actual measured value */
            uint32_t duration_ms;       /* Duration alarm condition has persisted */
        } alarm_data;
    } data;
} esocore_sensor_data_t;

/* ============================================================================
 * Sensor Interface Class
 * ============================================================================ */

/**
 * @brief Abstract sensor interface that all sensors must implement
 */
typedef struct esocore_sensor_interface {
    /* Sensor identification and information */
    const esocore_sensor_info_t *info;

    /* Initialization and configuration */
    bool (*init)(void);                                           /* Initialize sensor hardware */
    bool (*deinit)(void);                                         /* Deinitialize sensor */
    bool (*configure)(const esocore_sensor_config_t *config);    /* Configure sensor parameters */
    bool (*calibrate)(void);                                      /* Perform sensor calibration */

    /* Data acquisition */
    bool (*start_measurement)(void);                              /* Start data acquisition */
    bool (*stop_measurement)(void);                               /* Stop data acquisition */
    bool (*read_data)(esocore_sensor_data_t *data);              /* Read sensor data */

    /* Status and diagnostics */
    uint16_t (*get_status)(void);                                /* Get sensor status flags */
    bool (*self_test)(void);                                     /* Perform sensor self-test */
    bool (*get_diagnostics)(uint8_t *buffer, uint16_t *length); /* Get diagnostic information */

    /* Threshold and alarm management */
    bool (*set_thresholds)(uint32_t high, uint32_t low);        /* Set alarm thresholds */
    bool (*check_thresholds)(void);                              /* Check if thresholds exceeded */
    bool (*reset_alarms)(void);                                  /* Reset alarm conditions */

    /* Power management */
    bool (*enter_low_power)(void);                               /* Enter low power mode */
    bool (*exit_low_power)(void);                                /* Exit low power mode */
    uint8_t (*get_power_consumption)(void);                      /* Get current power consumption */

} esocore_sensor_interface_t;

/* ============================================================================
 * Common Sensor Functions
 * ============================================================================ */

/**
 * @brief Register a sensor with the system
 *
 * @param sensor Pointer to sensor interface
 * @return true if registration successful, false otherwise
 */
bool esocore_register_sensor(esocore_sensor_interface_t *sensor);

/**
 * @brief Unregister a sensor from the system
 *
 * @param sensor Pointer to sensor interface
 * @return true if unregistration successful, false otherwise
 */
bool esocore_unregister_sensor(esocore_sensor_interface_t *sensor);

/**
 * @brief Get registered sensor by type
 *
 * @param sensor_type Type of sensor to find
 * @return Pointer to sensor interface, NULL if not found
 */
esocore_sensor_interface_t *esocore_get_sensor(esocore_device_type_t sensor_type);

/**
 * @brief Initialize all registered sensors
 *
 * @return Number of sensors successfully initialized
 */
uint8_t esocore_init_all_sensors(void);

/**
 * @brief Deinitialize all registered sensors
 *
 * @return Number of sensors successfully deinitialized
 */
uint8_t esocore_deinit_all_sensors(void);

/**
 * @brief Start measurements on all sensors
 *
 * @return Number of sensors successfully started
 */
uint8_t esocore_start_all_measurements(void);

/**
 * @brief Stop measurements on all sensors
 *
 * @return Number of sensors successfully stopped
 */
uint8_t esocore_stop_all_measurements(void);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Convert raw ADC value to physical units
 *
 * @param adc_value Raw ADC reading
 * @param adc_resolution ADC resolution in bits
 * @param reference_voltage Reference voltage in mV
 * @param scale_factor Conversion scale factor
 * @param offset Conversion offset
 * @return Converted physical value
 */
float esocore_adc_to_physical(uint32_t adc_value, uint8_t adc_resolution,
                             uint16_t reference_voltage, float scale_factor, float offset);

/**
 * @brief Apply low-pass filter to sensor data
 *
 * @param input Input data point
 * @param previous_output Previous filter output
 * @param alpha Filter coefficient (0.0 to 1.0)
 * @return Filtered output
 */
float esocore_low_pass_filter(float input, float previous_output, float alpha);

/**
 * @brief Calculate RMS (Root Mean Square) value
 *
 * @param data Pointer to data array
 * @param length Number of data points
 * @return RMS value
 */
float esocore_calculate_rms(const float *data, uint16_t length);

/**
 * @brief Perform basic statistical analysis
 *
 * @param data Pointer to data array
 * @param length Number of data points
 * @param stats Pointer to statistics structure to fill
 * @return true if calculation successful, false otherwise
 */
bool esocore_calculate_statistics(const float *data, uint16_t length,
                                 esocore_sensor_data_t *stats);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_SENSOR_INTERFACE_H */
