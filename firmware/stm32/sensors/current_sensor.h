/**
 * @file current_sensor.h
 * @brief Current Sensor Driver
 *
 * This header defines the current sensor driver for ACS723 Hall effect sensor
 * with harmonic analysis, motor load monitoring, and power quality assessment.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

/* ============================================================================
 * Current Sensor Configuration
 * ============================================================================ */

#define CURRENT_SENSOR_MAX_SAMPLES         1024
#define CURRENT_SENSOR_FFT_SIZE            512
#define CURRENT_SENSOR_SAMPLE_RATE_MAX     10000
#define CURRENT_SENSOR_SAMPLE_RATE_MIN     100

/* Current thresholds (Amperes) */
#define CURRENT_THRESHOLD_LOW              0.1f
#define CURRENT_THRESHOLD_NORMAL           2.0f
#define CURRENT_THRESHOLD_HIGH             5.0f
#define CURRENT_THRESHOLD_CRITICAL         10.0f

/* ============================================================================
 * Current Data Types
 * ============================================================================ */

/**
 * @brief Current sensor configuration
 */
typedef struct {
    esocore_sensor_config_t base_config;    /**< Base sensor configuration */
    uint8_t sensor_type;                    /**< Type of current sensor (0=ACS712, 1=ACS723, 2=Custom) */
    float sensitivity_mv_a;                 /**< Sensitivity in mV/A */
    float nominal_current_a;                /**< Nominal current rating (A) */
    bool enable_temperature_compensation;   /**< Enable temperature compensation */
    bool enable_harmonic_analysis;          /**< Enable harmonic analysis */
    uint8_t harmonic_order_max;             /**< Maximum harmonic order to analyze */
    bool enable_power_analysis;             /**< Enable power analysis */
    bool enable_motor_load_detection;       /**< Enable motor load detection */
    uint8_t fft_window_type;                /**< FFT window type */
    float filter_cutoff_hz;                 /**< Low-pass filter cutoff */
} current_sensor_config_t;

/**
 * @brief Raw current data
 */
typedef struct {
    int16_t current_samples[CURRENT_SENSOR_MAX_SAMPLES]; /**< Raw ADC samples */
    uint32_t sample_count;                   /**< Number of valid samples */
    uint32_t timestamp;                      /**< Data timestamp */
    int8_t temperature_celsius;             /**< Sensor temperature */
} current_raw_data_t;

/**
 * @brief Processed current data
 */
typedef struct {
    float rms_current_a;                     /**< RMS current (A) */
    float peak_current_a;                    /**< Peak current (A) */
    float average_current_a;                 /**< Average current (A) */
    float crest_factor;                      /**< Crest factor */
    float form_factor;                       /**< Form factor */
    float fundamental_current_a;             /**< Fundamental current (A) */
    float total_harmonic_distortion;         /**< Total harmonic distortion (%) */
    uint32_t timestamp;                      /**< Processing timestamp */
} current_processed_data_t;

/**
 * @brief Harmonic analysis results
 */
typedef struct {
    float harmonic_magnitudes[16];           /**< Harmonic magnitudes (A) */
    float harmonic_phases[16];               /**< Harmonic phases (degrees) */
    float harmonic_percentages[16];          /**< Harmonic percentages (%) */
    uint8_t significant_harmonics;           /**< Number of significant harmonics */
    float thd_f;                            /**< Total harmonic distortion (fundamental) */
    float thd_r;                            /**< Total harmonic distortion (RMS) */
    float k_factor;                         /**< K-factor for transformer sizing */
} current_harmonic_data_t;

/**
 * @brief Power quality analysis
 */
typedef struct {
    float power_factor;                      /**< Power factor */
    float displacement_power_factor;         /**< Displacement power factor */
    float apparent_power_va;                 /**< Apparent power (VA) */
    float active_power_w;                    /**< Active power (W) */
    float reactive_power_var;                /**< Reactive power (VAR) */
    float voltage_rms_v;                     /**< RMS voltage (V) */
    float frequency_hz;                      /**< Line frequency (Hz) */
    float flicker_severity;                  /**< Voltage flicker severity */
} current_power_data_t;

/**
 * @brief Motor load analysis
 */
typedef struct {
    bool motor_connected;                    /**< Motor detected */
    float motor_load_percentage;             /**< Motor load percentage */
    float motor_efficiency;                  /**< Motor efficiency */
    float motor_power_factor;                /**< Motor power factor */
    uint8_t motor_poles;                     /**< Estimated motor poles */
    float slip_percentage;                   /**< Motor slip percentage */
    bool overload_detected;                  /**< Motor overload detected */
    bool underload_detected;                 /**< Motor underload detected */
} current_motor_data_t;

/**
 * @brief Complete current sensor data
 */
typedef struct {
    esocore_sensor_data_t base_data;         /**< Base sensor data structure */
    current_raw_data_t raw_data;             /**< Raw current data */
    current_processed_data_t processed_data; /**< Processed current data */
    current_harmonic_data_t harmonic_data;   /**< Harmonic analysis */
    current_power_data_t power_data;         /**< Power quality analysis */
    current_motor_data_t motor_data;         /**< Motor load analysis */
    uint8_t current_condition;               /**< Current/power condition (0-100) */
    char condition_description[64];          /**< Condition description */
} current_sensor_data_t;

/**
 * @brief Current sensor status
 */
typedef struct {
    esocore_sensor_status_t base_status;     /**< Base sensor status */
    bool sensor_ready;                       /**< Current sensor is ready */
    bool adc_ready;                          /**< ADC is ready */
    uint32_t samples_acquired;               /**< Number of samples acquired */
    uint32_t harmonic_analyses;              /**< Number of harmonic analyses */
    float current_rms_a;                     /**< Current RMS value */
    uint8_t saturation_flags;                /**< Saturation flags */
    bool calibration_valid;                  /**< Calibration is valid */
    int8_t current_temperature;              /**< Current temperature */
} current_sensor_status_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize current sensor
 */
bool current_sensor_init(const current_sensor_config_t *config);

/**
 * @brief Deinitialize current sensor
 */
bool current_sensor_deinit(void);

/**
 * @brief Configure current sensor parameters
 */
bool current_sensor_configure(const current_sensor_config_t *config);

/**
 * @brief Start current data acquisition
 */
bool current_sensor_start_acquisition(void);

/**
 * @brief Stop current data acquisition
 */
bool current_sensor_stop_acquisition(void);

/**
 * @brief Read current sensor data
 */
bool current_sensor_read_data(current_sensor_data_t *data, uint32_t timeout_ms);

/**
 * @brief Get current sensor status
 */
bool current_sensor_get_status(current_sensor_status_t *status);

/**
 * @brief Perform current sensor self-test
 */
bool current_sensor_self_test(void);

/**
 * @brief Calibrate current sensor
 */
bool current_sensor_calibrate(void);

/**
 * @brief Perform harmonic analysis
 */
bool current_sensor_analyze_harmonics(const int16_t *current_samples, uint32_t sample_count,
                                    uint32_t sampling_rate, current_harmonic_data_t *harmonic_data);

/**
 * @brief Analyze power quality
 */
bool current_sensor_analyze_power(const current_processed_data_t *current_data,
                                current_power_data_t *power_data);

/**
 * @brief Analyze motor load
 */
bool current_sensor_analyze_motor(const current_harmonic_data_t *harmonic_data,
                                const current_processed_data_t *current_data,
                                current_motor_data_t *motor_data);

/**
 * @brief Calculate current condition score
 */
uint8_t current_sensor_calculate_condition(const current_processed_data_t *processed_data,
                                         const current_harmonic_data_t *harmonic_data,
                                         const current_power_data_t *power_data);

/**
 * @brief Convert raw ADC samples to current (Amperes)
 */
bool current_sensor_convert_to_current(const int16_t *adc_samples, float *current_a,
                                     uint32_t sample_count, float sensitivity_mv_a,
                                     float adc_reference_v, uint16_t adc_resolution);

/**
 * @brief Calculate RMS current
 */
bool current_sensor_calculate_rms(const float *current_a, uint32_t sample_count,
                                float *rms_current_a);

/**
 * @brief Calculate total harmonic distortion
 */
bool current_sensor_calculate_thd(const float *harmonic_magnitudes, float fundamental_magnitude,
                                float *thd_percentage);

/**
 * @brief Get current sensor capabilities
 */
const esocore_sensor_capabilities_t *current_sensor_get_capabilities(void);

/**
 * @brief Get current sensor configuration
 */
bool current_sensor_get_configuration(current_sensor_config_t *config);

/**
 * @brief Set current sensor sensitivity
 */
bool current_sensor_set_sensitivity(float sensitivity_mv_a);

/**
 * @brief Enable/disable harmonic analysis
 */
bool current_sensor_enable_harmonic_analysis(bool enable);

/**
 * @brief Set maximum harmonic order
 */
bool current_sensor_set_max_harmonic_order(uint8_t max_order);

/**
 * @brief Get current RMS value
 */
bool current_sensor_get_rms_current(float *rms_current_a);

/**
 * @brief Clear current buffer
 */
bool current_sensor_clear_buffer(void);

/**
 * @brief Get current sensor diagnostics
 */
bool current_sensor_get_diagnostics(uint8_t *diagnostic_data, uint16_t buffer_size);

/**
 * @brief Check current sensor saturation
 */
bool current_sensor_check_saturation(uint8_t *saturation_flags);

/**
 * @brief Export current sensor data
 */
bool current_sensor_export_data(const current_sensor_data_t *data,
                              const char *filename, uint8_t format);

#endif /* CURRENT_SENSOR_H */