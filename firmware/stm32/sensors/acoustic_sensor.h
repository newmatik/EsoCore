/**
 * @file acoustic_sensor.h
 * @brief Acoustic Sensor Driver
 *
 * This header defines the acoustic sensor driver for digital MEMS microphone
 * with FFT analysis, sound pressure level measurement, and acoustic monitoring.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ACOUSTIC_SENSOR_H
#define ACOUSTIC_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

/* ============================================================================
 * Acoustic Sensor Configuration
 * ============================================================================ */

#define ACOUSTIC_SENSOR_MAX_SAMPLES       2048
#define ACOUSTIC_SENSOR_FFT_SIZE          1024
#define ACOUSTIC_SENSOR_SAMPLE_RATE_MAX   48000
#define ACOUSTIC_SENSOR_SAMPLE_RATE_MIN   8000

/* Acoustic thresholds (dB SPL) */
#define ACOUSTIC_THRESHOLD_QUIET          30.0f
#define ACOUSTIC_THRESHOLD_NORMAL         60.0f
#define ACOUSTIC_THRESHOLD_LOUD           85.0f
#define ACOUSTIC_THRESHOLD_DANGEROUS      100.0f

/* ============================================================================
 * Acoustic Data Types
 * ============================================================================ */

/**
 * @brief Acoustic sensor configuration
 */
typedef struct {
    esocore_sensor_config_t base_config;    /**< Base sensor configuration */
    uint8_t microphone_type;                /**< Type of microphone (0=Analog, 1=Digital MEMS) */
    float sensitivity_mv_pa;                /**< Sensitivity in mV/Pa */
    uint8_t measurement_range;              /**< Measurement range (dB SPL) */
    bool enable_a_weighting;                /**< Enable A-weighting filter */
    bool enable_high_pass_filter;           /**< Enable high-pass filter */
    float high_pass_cutoff_hz;              /**< High-pass filter cutoff frequency */
    bool enable_low_pass_filter;            /**< Enable low-pass filter */
    float low_pass_cutoff_hz;               /**< Low-pass filter cutoff frequency */
    uint8_t fft_window_type;                /**< FFT window type */
    bool enable_ultrasound_detection;       /**< Enable ultrasound detection */
} acoustic_sensor_config_t;

/**
 * @brief Raw acoustic data
 */
typedef struct {
    int16_t audio_samples[ACOUSTIC_SENSOR_MAX_SAMPLES]; /**< Raw audio samples */
    uint32_t sample_count;                   /**< Number of valid samples */
    uint32_t timestamp;                      /**< Data timestamp */
    int8_t temperature_celsius;             /**< Sensor temperature */
} acoustic_raw_data_t;

/**
 * @brief Processed acoustic data
 */
typedef struct {
    float rms_level_db;                      /**< RMS sound pressure level (dB SPL) */
    float peak_level_db;                     /**< Peak sound pressure level (dB SPL) */
    float crest_factor;                      /**< Crest factor */
    float a_weighted_level_db;               /**< A-weighted sound level (dB(A)) */
    float equivalent_level_db;               /**< Equivalent continuous level (dB) */
    float sound_exposure_db;                 /**< Sound exposure level (dB) */
    uint32_t measurement_duration_ms;        /**< Measurement duration */
    uint32_t timestamp;                      /**< Processing timestamp */
} acoustic_processed_data_t;

/**
 * @brief FFT analysis results for acoustic data
 */
typedef struct {
    float frequency_bins[ACOUSTIC_SENSOR_FFT_SIZE / 2]; /**< Frequency domain data */
    uint16_t dominant_frequency_hz;          /**< Dominant frequency */
    float dominant_amplitude;                /**< Dominant amplitude */
    float total_power;                       /**< Total power in frequency domain */
    float spectral_centroid;                 /**< Spectral centroid */
    float spectral_rolloff;                  /**< Spectral rolloff */
    float spectral_flux;                     /**< Spectral flux */
    float tonality_index;                    /**< Tonality index */
} acoustic_fft_data_t;

/**
 * @brief Frequency band analysis
 */
typedef struct {
    float band_31_5_hz;                      /**< 31.5 Hz band level (dB) */
    float band_63_hz;                        /**< 63 Hz band level (dB) */
    float band_125_hz;                       /**< 125 Hz band level (dB) */
    float band_250_hz;                       /**< 250 Hz band level (dB) */
    float band_500_hz;                       /**< 500 Hz band level (dB) */
    float band_1000_hz;                      /**< 1000 Hz band level (dB) */
    float band_2000_hz;                      /**< 2000 Hz band level (dB) */
    float band_4000_hz;                      /**< 4000 Hz band level (dB) */
    float band_8000_hz;                      /**< 8000 Hz band level (dB) */
} acoustic_frequency_bands_t;

/**
 * @brief Ultrasound detection results
 */
typedef struct {
    bool ultrasound_detected;                /**< Ultrasound presence detected */
    uint16_t ultrasound_frequency_hz;        /**< Detected ultrasound frequency */
    float ultrasound_level_db;               /**< Ultrasound level (dB) */
    float ultrasound_bandwidth_hz;           /**< Ultrasound bandwidth */
    uint8_t ultrasound_type;                 /**< Type of ultrasound (0=Aerosol, 1=Leak, 2=PD) */
} acoustic_ultrasound_data_t;

/**
 * @brief Complete acoustic sensor data
 */
typedef struct {
    esocore_sensor_data_t base_data;         /**< Base sensor data structure */
    acoustic_raw_data_t raw_data;            /**< Raw acoustic data */
    acoustic_processed_data_t processed_data; /**< Processed acoustic data */
    acoustic_fft_data_t fft_data;            /**< FFT analysis results */
    acoustic_frequency_bands_t frequency_bands; /**< Frequency band analysis */
    acoustic_ultrasound_data_t ultrasound_data; /**< Ultrasound detection */
    uint8_t acoustic_condition;              /**< Acoustic environment condition (0-100) */
    char condition_description[64];          /**< Condition description */
} acoustic_sensor_data_t;

/**
 * @brief Acoustic sensor status
 */
typedef struct {
    esocore_sensor_status_t base_status;     /**< Base sensor status */
    bool microphone_ready;                   /**< Microphone is ready */
    bool adc_ready;                          /**< ADC is ready */
    uint32_t samples_acquired;               /**< Number of samples acquired */
    uint32_t fft_calculations;               /**< Number of FFT calculations performed */
    float current_rms_level_db;              /**< Current RMS level */
    uint8_t saturation_flags;                /**< Saturation flags */
    bool calibration_valid;                  /**< Calibration is valid */
    int8_t current_temperature;              /**< Current temperature */
} acoustic_sensor_status_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize acoustic sensor
 */
bool acoustic_sensor_init(const acoustic_sensor_config_t *config);

/**
 * @brief Deinitialize acoustic sensor
 */
bool acoustic_sensor_deinit(void);

/**
 * @brief Configure acoustic sensor parameters
 */
bool acoustic_sensor_configure(const acoustic_sensor_config_t *config);

/**
 * @brief Start acoustic data acquisition
 */
bool acoustic_sensor_start_acquisition(void);

/**
 * @brief Stop acoustic data acquisition
 */
bool acoustic_sensor_stop_acquisition(void);

/**
 * @brief Read acoustic sensor data
 */
bool acoustic_sensor_read_data(acoustic_sensor_data_t *data, uint32_t timeout_ms);

/**
 * @brief Get acoustic sensor status
 */
bool acoustic_sensor_get_status(acoustic_sensor_status_t *status);

/**
 * @brief Perform acoustic sensor self-test
 */
bool acoustic_sensor_self_test(void);

/**
 * @brief Calibrate acoustic sensor
 */
bool acoustic_sensor_calibrate(void);

/**
 * @brief Perform FFT analysis on acoustic data
 */
bool acoustic_sensor_perform_fft(const int16_t *audio_data, uint32_t data_length,
                                acoustic_fft_data_t *fft_result);

/**
 * @brief Calculate sound pressure level
 */
bool acoustic_sensor_calculate_spl(const int16_t *audio_data, uint32_t data_length,
                                  uint32_t sampling_rate, acoustic_processed_data_t *processed_data);

/**
 * @brief Apply A-weighting filter
 */
bool acoustic_sensor_apply_a_weighting(const float *frequency_data, float *a_weighted_data,
                                     uint32_t data_length, uint32_t sampling_rate);

/**
 * @brief Analyze frequency bands
 */
bool acoustic_sensor_analyze_frequency_bands(const acoustic_fft_data_t *fft_data,
                                           acoustic_frequency_bands_t *bands);

/**
 * @brief Detect ultrasound
 */
bool acoustic_sensor_detect_ultrasound(const acoustic_fft_data_t *fft_data,
                                     acoustic_ultrasound_data_t *ultrasound_data);

/**
 * @brief Calculate acoustic condition score
 */
uint8_t acoustic_sensor_calculate_condition(const acoustic_processed_data_t *processed_data,
                                          const acoustic_frequency_bands_t *bands);

/**
 * @brief Convert raw samples to pressure (Pa)
 */
bool acoustic_sensor_convert_to_pressure(const int16_t *raw_samples, float *pressure_pa,
                                       uint32_t sample_count, float sensitivity_mv_pa);

/**
 * @brief Calculate equivalent continuous sound level
 */
bool acoustic_sensor_calculate_lequ(const float *pressure_pa, uint32_t sample_count,
                                   uint32_t sampling_rate, float *lequ_db);

/**
 * @brief Get acoustic sensor capabilities
 */
const esocore_sensor_capabilities_t *acoustic_sensor_get_capabilities(void);

/**
 * @brief Get acoustic sensor configuration
 */
bool acoustic_sensor_get_configuration(acoustic_sensor_config_t *config);

/**
 * @brief Set microphone sensitivity
 */
bool acoustic_sensor_set_sensitivity(float sensitivity_mv_pa);

/**
 * @brief Enable/disable A-weighting
 */
bool acoustic_sensor_enable_a_weighting(bool enable);

/**
 * @brief Set measurement range
 */
bool acoustic_sensor_set_range(uint8_t range_db);

/**
 * @brief Get current RMS level
 */
bool acoustic_sensor_get_rms_level(float *rms_level_db);

/**
 * @brief Clear audio buffer
 */
bool acoustic_sensor_clear_buffer(void);

/**
 * @brief Get acoustic diagnostics
 */
bool acoustic_sensor_get_diagnostics(uint8_t *diagnostic_data, uint16_t buffer_size);

/**
 * @brief Check microphone saturation
 */
bool acoustic_sensor_check_saturation(uint8_t *saturation_flags);

/**
 * @brief Export acoustic data
 */
bool acoustic_sensor_export_data(const acoustic_sensor_data_t *data,
                               const char *filename, uint8_t format);

#endif /* ACOUSTIC_SENSOR_H */