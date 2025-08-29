/**
 * @file light_sensor.h
 * @brief Light Sensor Implementation for EsoCore
 *
 * This file implements the light sensor module for EsoCore with comprehensive
 * environmental light monitoring including visible light, UV radiation,
 * infrared sensing, color analysis, and spectral measurements.
 *
 * Features:
 * - Visible light sensing (TSL2591, 0.002-32k lux range)
 * - UV radiation monitoring (VEML6075 UVA/UVB)
 * - Infrared temperature sensing (MLX90614)
 * - Color analysis (TCS34725 RGB with IR blocking)
 * - Spectral analysis (AS7341 10-channel spectral sensor)
 * - Multi-spectrum environmental monitoring
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Light Sensor Configuration
 * ============================================================================ */

typedef struct {
    uint16_t visible_sample_rate_hz;       /* Visible light sample rate (1-100 Hz) */
    uint16_t uv_sample_rate_hz;            /* UV radiation sample rate (1-50 Hz) */
    uint16_t ir_temp_sample_rate_hz;       /* IR temperature sample rate (1-10 Hz) */
    uint16_t color_sample_rate_hz;         /* Color sensor sample rate (1-10 Hz) */
    uint16_t spectral_sample_rate_hz;      /* Spectral sensor sample rate (1-5 Hz) */
    bool enable_auto_gain_visible;         /* Auto-gain for visible light */
    bool enable_auto_gain_uv;              /* Auto-gain for UV sensor */
    bool enable_color_temperature;         /* Enable color temperature calculation */
    bool enable_lux_calculation;           /* Enable illuminance calculation */
    float uv_alarm_threshold_uwm2;         /* UV radiation alarm threshold (µW/m²) */
    float lux_alarm_low_threshold;         /* Low illuminance alarm (lux) */
    float lux_alarm_high_threshold;        /* High illuminance alarm (lux) */
    float temp_alarm_high_c;               /* High temperature alarm (°C) */
    float temp_alarm_low_c;                /* Low temperature alarm (°C) */
} light_sensor_config_t;

/* ============================================================================
 * Light Sensor Data
 * ============================================================================ */

typedef struct {
    /* Visible Light Sensor (TSL2591) */
    float illuminance_lux;                 /* Illuminance in lux */
    uint16_t channel0_raw;                 /* Full spectrum raw count */
    uint16_t channel1_raw;                 /* IR spectrum raw count */
    float infrared_ratio;                  /* IR to full spectrum ratio */
    uint8_t gain_visible;                  /* Current gain setting */

    /* UV Radiation Sensor (VEML6075) */
    float uva_intensity_uwm2;              /* UVA intensity (µW/m²) */
    float uvb_intensity_uwm2;              /* UVB intensity (µW/m²) */
    float uv_index;                        /* UV Index (0-11+) */
    uint8_t gain_uv;                       /* Current UV gain setting */

    /* IR Temperature Sensor (MLX90614) */
    float object_temperature_c;            /* Object temperature (°C) */
    float ambient_temperature_c;           /* Ambient temperature (°C) */
    float emissivity;                      /* Current emissivity setting */

    /* Color Sensor (TCS34725) */
    uint16_t red_raw;                      /* Red channel raw count */
    uint16_t green_raw;                    /* Green channel raw count */
    uint16_t blue_raw;                     /* Blue channel raw count */
    uint16_t clear_raw;                    /* Clear channel raw count */
    float color_temperature_k;             /* Color temperature (K) */
    float lux_from_color;                  /* Lux calculated from color sensor */

    /* Spectral Sensor (AS7341) */
    uint16_t spectral_channels[10];        /* 10 spectral channel measurements */
    float dominant_wavelength_nm;          /* Dominant wavelength (nm) */
    float peak_intensity;                  /* Peak spectral intensity */

    /* System Status */
    uint8_t sensor_status;                 /* Sensor status flags */
    uint32_t timestamp;                    /* Measurement timestamp */
} light_sensor_data_t;

/* ============================================================================
 * Light Analysis Results
 * ============================================================================ */

typedef struct {
    /* Illuminance Analysis */
    bool adequate_lighting;                /* Lighting meets OSHA/EU standards */
    bool low_light_condition;              /* Low light warning */
    bool excessive_brightness;             /* Excessive brightness warning */
    float recommended_lux;                 /* Recommended illuminance level */

    /* UV Safety Analysis */
    bool uv_exposure_safe;                 /* UV exposure within safe limits */
    bool sunscreen_recommended;            /* Sunscreen recommended */
    bool uv_protection_required;           /* UV protection required */
    uint8_t uv_risk_level;                 /* UV risk level (0-4) */

    /* Color Quality Analysis */
    bool color_rendering_good;             /* Good color rendering */
    bool color_temperature_appropriate;    /* Appropriate color temperature */
    bool light_quality_acceptable;         /* Overall light quality acceptable */

    /* Spectral Analysis */
    bool spectral_distribution_normal;     /* Normal spectral distribution */
    bool led_lighting_detected;            /* LED lighting detected */
    bool incandescent_detected;            /* Incandescent lighting detected */
    bool fluorescent_detected;             /* Fluorescent lighting detected */

    /* Health & Safety */
    bool eye_strain_risk;                 /* Eye strain risk detected */
    bool migraine_trigger;                 /* Potential migraine trigger */
    bool sleep_disruption;                 /* Sleep disruption risk */
    uint8_t comfort_index;                 /* Lighting comfort index (0-100) */

    /* Environmental */
    bool daylight_available;               /* Natural daylight available */
    bool artificial_light_dominant;        /* Artificial light dominant */
    bool light_pollution_detected;         /* Light pollution detected */
} light_analysis_t;

/* ============================================================================
 * Spectral Channels (AS7341)
 * ============================================================================ */

#define SPECTRAL_CHANNEL_415NM   0   /* Violet (415nm) */
#define SPECTRAL_CHANNEL_445NM   1   /* Indigo (445nm) */
#define SPECTRAL_CHANNEL_480NM   2   /* Blue (480nm) */
#define SPECTRAL_CHANNEL_515NM   3   /* Cyan (515nm) */
#define SPECTRAL_CHANNEL_555NM   4   /* Green (555nm) */
#define SPECTRAL_CHANNEL_590NM   5   /* Yellow (590nm) */
#define SPECTRAL_CHANNEL_630NM   6   /* Orange (630nm) */
#define SPECTRAL_CHANNEL_680NM   7   /* Red (680nm) */
#define SPECTRAL_CHANNEL_CLEAR   8   /* Clear channel */
#define SPECTRAL_CHANNEL_NIR     9   /* Near IR (910nm) */

/* ============================================================================
 * Status Flags
 * ============================================================================ */

#define LIGHT_STATUS_VISIBLE_READY     (1 << 0)
#define LIGHT_STATUS_UV_READY         (1 << 1)
#define LIGHT_STATUS_IR_READY         (1 << 2)
#define LIGHT_STATUS_COLOR_READY      (1 << 3)
#define LIGHT_STATUS_SPECTRAL_READY   (1 << 4)
#define LIGHT_STATUS_VISIBLE_SATURATED (1 << 5)
#define LIGHT_STATUS_UV_SATURATED     (1 << 6)
#define LIGHT_STATUS_I2C_ERROR        (1 << 7)

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize light sensor suite
 *
 * @param config Pointer to sensor configuration
 * @return true if initialization successful, false otherwise
 */
bool light_sensor_init(const light_sensor_config_t *config);

/**
 * @brief Deinitialize light sensor suite
 *
 * @return true if deinitialization successful, false otherwise
 */
bool light_sensor_deinit(void);

/**
 * @brief Start light measurement
 *
 * @return true if measurement started successfully, false otherwise
 */
bool light_sensor_start_measurement(void);

/**
 * @brief Stop light measurement
 *
 * @return true if measurement stopped successfully, false otherwise
 */
bool light_sensor_stop_measurement(void);

/**
 * @brief Read light sensor data
 *
 * @param data Pointer to data structure to fill
 * @return true if data read successfully, false otherwise
 */
bool light_sensor_read_data(light_sensor_data_t *data);

/**
 * @brief Perform light quality analysis
 *
 * @param data Pointer to current sensor data
 * @param analysis Pointer to analysis structure to fill
 * @return true if analysis successful, false otherwise
 */
bool light_sensor_analyze_conditions(const light_sensor_data_t *data,
                                   light_analysis_t *analysis);

/**
 * @brief Calculate illuminance from TSL2591 data
 *
 * @param ch0 Raw channel 0 count
 * @param ch1 Raw channel 1 count
 * @param gain Current gain setting
 * @param integration_time Integration time (ms)
 * @return Illuminance in lux
 */
float light_sensor_calculate_lux(uint16_t ch0, uint16_t ch1, uint8_t gain,
                                uint16_t integration_time);

/**
 * @brief Calculate UV index from VEML6075 data
 *
 * @param uva_raw UVA raw count
 * @param uvb_raw UVB raw count
 * @return UV index (0-11+)
 */
float light_sensor_calculate_uv_index(uint16_t uva_raw, uint16_t uvb_raw);

/**
 * @brief Calculate color temperature from TCS34725 data
 *
 * @param red Raw red count
 * @param green Raw green count
 * @param blue Raw blue count
 * @param clear Raw clear count
 * @return Color temperature in Kelvin
 */
float light_sensor_calculate_color_temp(uint16_t red, uint16_t green,
                                       uint16_t blue, uint16_t clear);

/**
 * @brief Perform spectral analysis
 *
 * @param spectral_data Array of 10 spectral channel values
 * @param dominant_wavelength Pointer to store dominant wavelength (nm)
 * @param peak_intensity Pointer to store peak intensity
 * @return true if analysis successful, false otherwise
 */
bool light_sensor_analyze_spectrum(const uint16_t *spectral_data,
                                  float *dominant_wavelength, float *peak_intensity);

/**
 * @brief Calibrate visible light sensor
 *
 * @param reference_lux Reference illuminance value (lux)
 * @return true if calibration successful, false otherwise
 */
bool light_sensor_calibrate_visible(float reference_lux);

/**
 * @brief Calibrate UV sensor
 *
 * @param reference_uva Reference UVA intensity (µW/m²)
 * @param reference_uvb Reference UVB intensity (µW/m²)
 * @return true if calibration successful, false otherwise
 */
bool light_sensor_calibrate_uv(float reference_uva, float reference_uvb);

/**
 * @brief Calibrate IR temperature sensor
 *
 * @param reference_temp Reference temperature (°C)
 * @param emissivity Object emissivity (0.1-1.0)
 * @return true if calibration successful, false otherwise
 */
bool light_sensor_calibrate_ir_temp(float reference_temp, float emissivity);

/**
 * @brief Perform sensor self-test
 *
 * @return true if self-test passed, false otherwise
 */
bool light_sensor_self_test(void);

/**
 * @brief Get sensor status
 *
 * @return Status flags bitmask
 */
uint8_t light_sensor_get_status(void);

/**
 * @brief Set illuminance alarm thresholds
 *
 * @param low_threshold Low illuminance threshold (lux)
 * @param high_threshold High illuminance threshold (lux)
 * @return true if thresholds set successfully, false otherwise
 */
bool light_sensor_set_lux_thresholds(float low_threshold, float high_threshold);

/**
 * @brief Set UV radiation alarm threshold
 *
 * @param threshold UV radiation threshold (µW/m²)
 * @return true if threshold set successfully, false otherwise
 */
bool light_sensor_set_uv_threshold(float threshold);

/**
 * @brief Check if lighting conditions exceed thresholds
 *
 * @param data Pointer to current sensor data
 * @return true if any thresholds exceeded, false otherwise
 */
bool light_sensor_check_thresholds(const light_sensor_data_t *data);

/**
 * @brief Configure sensor sampling rates
 *
 * @param visible_rate Visible light sample rate (Hz)
 * @param uv_rate UV radiation sample rate (Hz)
 * @param ir_rate IR temperature sample rate (Hz)
 * @param color_rate Color sensor sample rate (Hz)
 * @return true if configuration successful, false otherwise
 */
bool light_sensor_configure_rates(uint16_t visible_rate, uint16_t uv_rate,
                                 uint16_t ir_rate, uint16_t color_rate);

/**
 * @brief Enable/disable auto-gain for sensors
 *
 * @param visible_enable Enable auto-gain for visible sensor
 * @param uv_enable Enable auto-gain for UV sensor
 * @return true if configuration successful, false otherwise
 */
bool light_sensor_set_auto_gain(bool visible_enable, bool uv_enable);

/**
 * @brief Get OSHA lighting compliance status
 *
 * @param area_type Type of work area (office, industrial, etc.)
 * @param measured_lux Current measured illuminance
 * @return true if compliant with OSHA standards, false otherwise
 */
bool light_sensor_check_osha_compliance(const char *area_type, float measured_lux);

/**
 * @brief Get EU workplace lighting compliance status
 *
 * @param task_type Type of work task
 * @param measured_lux Current measured illuminance
 * @return true if compliant with EU standards, false otherwise
 */
bool light_sensor_check_eu_compliance(const char *task_type, float measured_lux);

/**
 * @brief Detect light source type
 *
 * @param spectral_data Array of spectral channel values
 * @param source_type_buffer Buffer to store detected source type
 * @param buffer_size Size of buffer
 * @return true if source type detected successfully, false otherwise
 */
bool light_sensor_detect_light_source(const uint16_t *spectral_data,
                                     char *source_type_buffer, uint16_t buffer_size);

/**
 * @brief Calculate eye strain risk
 *
 * @param illuminance Current illuminance (lux)
 * @param color_temp Color temperature (K)
 * @param viewing_distance Viewing distance (cm)
 * @return Eye strain risk level (0-100)
 */
uint8_t light_sensor_calculate_eye_strain_risk(float illuminance, float color_temp,
                                              float viewing_distance);

/**
 * @brief Enter low-power mode
 *
 * @return true if low-power mode entered successfully, false otherwise
 */
bool light_sensor_enter_low_power(void);

/**
 * @brief Exit low-power mode
 *
 * @return true if low-power mode exited successfully, false otherwise
 */
bool light_sensor_exit_low_power(void);

/**
 * @brief Get sensor calibration status
 *
 * @param visible_calibrated Pointer to visible sensor calibration status
 * @param uv_calibrated Pointer to UV sensor calibration status
 * @param ir_calibrated Pointer to IR sensor calibration status
 * @return true if status retrieved successfully, false otherwise
 */
bool light_sensor_get_calibration_status(bool *visible_calibrated, bool *uv_calibrated,
                                        bool *ir_calibrated);

/**
 * @brief Force sensor recalibration
 *
 * @param sensor_type Sensor type to recalibrate ("visible", "uv", "ir")
 * @return true if recalibration started successfully, false otherwise
 */
bool light_sensor_force_recalibration(const char *sensor_type);

#ifdef __cplusplus
}
#endif

#endif /* LIGHT_SENSOR_H */
