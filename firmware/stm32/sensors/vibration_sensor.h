/**
 * @file vibration_sensor.h
 * @brief Vibration Sensor Driver
 *
 * This header defines the vibration sensor driver for 3-axis accelerometer
 * with FFT analysis, bearing fault detection, and vibration monitoring.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef VIBRATION_SENSOR_H
#define VIBRATION_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

/* ============================================================================
 * Vibration Sensor Configuration
 * ============================================================================ */

#define VIBRATION_SENSOR_AXES              3
#define VIBRATION_SENSOR_MAX_SAMPLES       1024
#define VIBRATION_SENSOR_FFT_SIZE          512
#define VIBRATION_SENSOR_SAMPLE_RATE_MAX   10000
#define VIBRATION_SENSOR_SAMPLE_RATE_MIN   10

/* Vibration thresholds (in mm/s² RMS) */
#define VIBRATION_THRESHOLD_GOOD           2.5f
#define VIBRATION_THRESHOLD_WARNING        7.1f
#define VIBRATION_THRESHOLD_CRITICAL       18.0f

/* ============================================================================
 * Vibration Data Types
 * ============================================================================ */

/**
 * @brief Vibration sensor configuration
 */
typedef struct {
    esocore_sensor_config_t base_config;    /**< Base sensor configuration */
    uint8_t accelerometer_type;             /**< Type of accelerometer (0=ADXL355, 1=BMI088, etc.) */
    float sensitivity_mg_per_lsb;           /**< Sensitivity in mg/LSB */
    uint8_t measurement_range;              /**< Measurement range (2g, 4g, 8g, 16g) */
    bool enable_high_pass_filter;           /**< Enable high-pass filter */
    float high_pass_cutoff_hz;              /**< High-pass filter cutoff frequency */
    bool enable_low_pass_filter;            /**< Enable low-pass filter */
    float low_pass_cutoff_hz;               /**< Low-pass filter cutoff frequency */
    bool enable_temperature_compensation;   /**< Enable temperature compensation */
    uint8_t fft_window_type;                /**< FFT window type (0=Hanning, 1=Hamming, 2=Blackman) */
} vibration_sensor_config_t;

/**
 * @brief Raw vibration data for 3 axes
 */
typedef struct {
    int32_t x_axis_raw;                     /**< X-axis raw accelerometer data */
    int32_t y_axis_raw;                     /**< Y-axis raw accelerometer data */
    int32_t z_axis_raw;                     /**< Z-axis raw accelerometer data */
    uint32_t timestamp;                     /**< Data timestamp */
    int16_t temperature_celsius;            /**< Sensor temperature */
} vibration_raw_data_t;

/**
 * @brief Processed vibration data
 */
typedef struct {
    float x_axis_acceleration;              /**< X-axis acceleration in m/s² */
    float y_axis_acceleration;              /**< Y-axis acceleration in m/s² */
    float z_axis_acceleration;              /**< Z-axis acceleration in m/s² */
    float x_axis_velocity;                  /**< X-axis velocity in mm/s */
    float y_axis_velocity;                  /**< Y-axis velocity in mm/s */
    float z_axis_velocity;                  /**< Z-axis velocity in mm/s */
    float x_axis_displacement;              /**< X-axis displacement in mm */
    float y_axis_displacement;              /**< Y-axis displacement in mm */
    float z_axis_displacement;              /**< Z-axis displacement in mm */
    float rms_acceleration;                 /**< RMS acceleration magnitude */
    float peak_acceleration;                /**< Peak acceleration magnitude */
    float crest_factor;                     /**< Crest factor */
    uint32_t timestamp;                     /**< Processing timestamp */
} vibration_processed_data_t;

/**
 * @brief FFT analysis results
 */
typedef struct {
    float frequency_bins[VIBRATION_SENSOR_FFT_SIZE / 2]; /**< Frequency domain data */
    uint16_t dominant_frequency_hz;          /**< Dominant frequency */
    float dominant_amplitude;                /**< Dominant amplitude */
    float total_power;                       /**< Total power in frequency domain */
    float spectral_centroid;                 /**< Spectral centroid */
    float rms_frequency;                     /**< RMS frequency */
    float frequency_spread;                  /**< Frequency spread */
} vibration_fft_data_t;

/**
 * @brief Bearing fault detection results
 */
typedef struct {
    bool bearing_fault_detected;            /**< Bearing fault detected */
    float fault_severity;                   /**< Fault severity (0-1) */
    uint16_t fault_frequency_hz;            /**< Fault frequency */
    float bpfo_amplitude;                   /**< Ball Pass Frequency Outer amplitude */
    float bpf_i_amplitude;                  /**< Ball Pass Frequency Inner amplitude */
    float ftf_amplitude;                    /**< Fundamental Train Frequency amplitude */
    float bsf_amplitude;                    /**< Ball Spin Frequency amplitude */
    uint8_t fault_type;                     /**< Fault type (0=Outer race, 1=Inner race, 2=Balls, 3=Cage) */
} vibration_bearing_analysis_t;

/**
 * @brief Gear analysis results
 */
typedef struct {
    bool gear_fault_detected;               /**< Gear fault detected */
    float gear_mesh_frequency;              /**< Gear mesh frequency */
    float sideband_amplitude;               /**< Sideband amplitude */
    float modulation_index;                 /**< Amplitude modulation index */
    uint8_t missing_teeth;                  /**< Number of potentially missing teeth */
    float gear_wear_level;                  /**< Gear wear level (0-1) */
} vibration_gear_analysis_t;

/**
 * @brief Complete vibration sensor data
 */
typedef struct {
    esocore_sensor_data_t base_data;        /**< Base sensor data structure */
    vibration_raw_data_t raw_data;          /**< Raw sensor data */
    vibration_processed_data_t processed_data; /**< Processed vibration data */
    vibration_fft_data_t fft_data;          /**< FFT analysis results */
    vibration_bearing_analysis_t bearing_analysis; /**< Bearing analysis */
    vibration_gear_analysis_t gear_analysis; /**< Gear analysis */
    uint8_t overall_condition;              /**< Overall equipment condition (0-100) */
    char condition_description[64];         /**< Condition description */
} vibration_sensor_data_t;

/**
 * @brief Vibration sensor status
 */
typedef struct {
    esocore_sensor_status_t base_status;    /**< Base sensor status */
    bool accelerometer_ready;               /**< Accelerometer is ready */
    bool data_buffer_full;                  /**< Data buffer is full */
    uint32_t samples_collected;             /**< Number of samples collected */
    uint32_t fft_calculations;              /**< Number of FFT calculations performed */
    float current_temperature;              /**< Current sensor temperature */
    uint8_t saturation_flags;               /**< Saturation flags for each axis */
    bool calibration_valid;                 /**< Calibration is valid */
} vibration_sensor_status_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize vibration sensor
 *
 * @param config Pointer to sensor configuration
 * @return true if initialization successful, false otherwise
 */
bool vibration_sensor_init(const vibration_sensor_config_t *config);

/**
 * @brief Deinitialize vibration sensor
 *
 * @return true if deinitialization successful, false otherwise
 */
bool vibration_sensor_deinit(void);

/**
 * @brief Configure vibration sensor parameters
 *
 * @param config Pointer to new configuration
 * @return true if configuration successful, false otherwise
 */
bool vibration_sensor_configure(const vibration_sensor_config_t *config);

/**
 * @brief Start vibration data acquisition
 *
 * @return true if acquisition started successfully, false otherwise
 */
bool vibration_sensor_start_acquisition(void);

/**
 * @brief Stop vibration data acquisition
 *
 * @return true if acquisition stopped successfully, false otherwise
 */
bool vibration_sensor_stop_acquisition(void);

/**
 * @brief Read vibration sensor data
 *
 * @param data Pointer to data structure to fill
 * @param timeout_ms Timeout in milliseconds
 * @return true if data read successfully, false otherwise
 */
bool vibration_sensor_read_data(vibration_sensor_data_t *data, uint32_t timeout_ms);

/**
 * @brief Get vibration sensor status
 *
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool vibration_sensor_get_status(vibration_sensor_status_t *status);

/**
 * @brief Perform vibration sensor self-test
 *
 * @return true if self-test passed, false otherwise
 */
bool vibration_sensor_self_test(void);

/**
 * @brief Calibrate vibration sensor
 *
 * @return true if calibration successful, false otherwise
 */
bool vibration_sensor_calibrate(void);

/**
 * @brief Perform FFT analysis on vibration data
 *
 * @param input_data Input time-domain data
 * @param data_length Length of input data
 * @param fft_result Pointer to FFT result structure
 * @return true if FFT analysis successful, false otherwise
 */
bool vibration_sensor_perform_fft(const float *input_data, uint32_t data_length,
                                 vibration_fft_data_t *fft_result);

/**
 * @brief Analyze bearing condition from vibration data
 *
 * @param vibration_data Pointer to vibration data
 * @param bearing_analysis Pointer to analysis result structure
 * @param bearing_specs Pointer to bearing specifications
 * @return true if analysis successful, false otherwise
 */
bool vibration_sensor_analyze_bearing(const vibration_processed_data_t *vibration_data,
                                    vibration_bearing_analysis_t *bearing_analysis,
                                    const void *bearing_specs);

/**
 * @brief Analyze gear condition from vibration data
 *
 * @param vibration_data Pointer to vibration data
 * @param gear_analysis Pointer to analysis result structure
 * @param gear_specs Pointer to gear specifications
 * @return true if analysis successful, false otherwise
 */
bool vibration_sensor_analyze_gear(const vibration_processed_data_t *vibration_data,
                                 vibration_gear_analysis_t *gear_analysis,
                                 const void *gear_specs);

/**
 * @brief Calculate overall equipment condition
 *
 * @param vibration_data Pointer to vibration data
 * @param bearing_analysis Pointer to bearing analysis
 * @param gear_analysis Pointer to gear analysis
 * @return Equipment condition score (0-100, 100=perfect)
 */
uint8_t vibration_sensor_calculate_condition(const vibration_processed_data_t *vibration_data,
                                           const vibration_bearing_analysis_t *bearing_analysis,
                                           const vibration_gear_analysis_t *gear_analysis);

/**
 * @brief Convert raw accelerometer data to engineering units
 *
 * @param raw_data Pointer to raw data structure
 * @param processed_data Pointer to processed data structure to fill
 * @param config Pointer to sensor configuration
 * @return true if conversion successful, false otherwise
 */
bool vibration_sensor_convert_units(const vibration_raw_data_t *raw_data,
                                  vibration_processed_data_t *processed_data,
                                  const vibration_sensor_config_t *config);

/**
 * @brief Apply digital filtering to vibration data
 *
 * @param input_data Input data array
 * @param output_data Output data array
 * @param data_length Length of data arrays
 * @param config Pointer to sensor configuration
 * @return true if filtering successful, false otherwise
 */
bool vibration_sensor_apply_filtering(const float *input_data, float *output_data,
                                    uint32_t data_length, const vibration_sensor_config_t *config);

/**
 * @brief Detect vibration anomalies
 *
 * @param vibration_data Pointer to vibration data
 * @param baseline_data Pointer to baseline vibration data
 * @param threshold_multiplier Threshold multiplier for anomaly detection
 * @return true if anomaly detected, false otherwise
 */
bool vibration_sensor_detect_anomaly(const vibration_processed_data_t *vibration_data,
                                   const vibration_processed_data_t *baseline_data,
                                   float threshold_multiplier);

/**
 * @brief Get vibration sensor capabilities
 *
 * @return Pointer to sensor capabilities structure
 */
const esocore_sensor_capabilities_t *vibration_sensor_get_capabilities(void);

/**
 * @brief Get vibration sensor configuration
 *
 * @param config Pointer to configuration structure to fill
 * @return true if configuration retrieved successfully, false otherwise
 */
bool vibration_sensor_get_configuration(vibration_sensor_config_t *config);

/**
 * @brief Set vibration sensor measurement range
 *
 * @param range Measurement range (2g, 4g, 8g, 16g)
 * @return true if range set successfully, false otherwise
 */
bool vibration_sensor_set_range(uint8_t range);

/**
 * @brief Enable/disable temperature compensation
 *
 * @param enable true to enable, false to disable
 * @return true if setting changed successfully, false otherwise
 */
bool vibration_sensor_enable_temperature_compensation(bool enable);

/**
 * @brief Set sampling rate
 *
 * @param sampling_rate_hz Sampling rate in Hz
 * @return true if sampling rate set successfully, false otherwise
 */
bool vibration_sensor_set_sampling_rate(uint32_t sampling_rate_hz);

/**
 * @brief Get current sensor temperature
 *
 * @param temperature_celsius Pointer to store temperature
 * @return true if temperature retrieved successfully, false otherwise
 */
bool vibration_sensor_get_temperature(int8_t *temperature_celsius);

/**
 * @brief Clear sensor data buffer
 *
 * @return true if buffer cleared successfully, false otherwise
 */
bool vibration_sensor_clear_buffer(void);

/**
 * @brief Get sensor diagnostic information
 *
 * @param diagnostic_data Pointer to diagnostic data buffer
 * @param buffer_size Size of buffer
 * @return true if diagnostic data retrieved successfully, false otherwise
 */
bool vibration_sensor_get_diagnostics(uint8_t *diagnostic_data, uint16_t buffer_size);

/**
 * @brief Check if sensor data is saturated
 *
 * @param saturation_flags Pointer to store saturation flags
 * @return true if saturation check completed successfully, false otherwise
 */
bool vibration_sensor_check_saturation(uint8_t *saturation_flags);

/**
 * @brief Calculate vibration velocity from acceleration
 *
 * @param acceleration_data Acceleration data array
 * @param velocity_data Output velocity data array
 * @param data_length Length of data arrays
 * @param sampling_rate_hz Sampling rate
 * @return true if calculation successful, false otherwise
 */
bool vibration_sensor_calculate_velocity(const float *acceleration_data, float *velocity_data,
                                       uint32_t data_length, uint32_t sampling_rate_hz);

/**
 * @brief Calculate vibration displacement from acceleration
 *
 * @param acceleration_data Acceleration data array
 * @param displacement_data Output displacement data array
 * @param data_length Length of data arrays
 * @param sampling_rate_hz Sampling rate
 * @return true if calculation successful, false otherwise
 */
bool vibration_sensor_calculate_displacement(const float *acceleration_data, float *displacement_data,
                                           uint32_t data_length, uint32_t sampling_rate_hz);

/**
 * @brief Export vibration data for analysis
 *
 * @param data Pointer to vibration data
 * @param filename Export filename
 * @param format Export format (0=CSV, 1=Binary)
 * @return true if export successful, false otherwise
 */
bool vibration_sensor_export_data(const vibration_sensor_data_t *data,
                                const char *filename, uint8_t format);

#endif /* VIBRATION_SENSOR_H */