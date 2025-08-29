/**
 * @file air_quality_sensor.h
 * @brief Air Quality Sensor Driver
 *
 * This header defines the air quality sensor driver for multi-sensor environmental monitoring
 * with IAQ calculation, health risk assessment, and environmental analysis.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef AIR_QUALITY_SENSOR_H
#define AIR_QUALITY_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

/* ============================================================================
 * Air Quality Sensor Configuration
 * ============================================================================ */

#define AIR_QUALITY_MAX_SAMPLES            16
#define AIR_QUALITY_MAX_GASES              8
#define AIR_QUALITY_CALIBRATION_POINTS     5

/* IAQ Index ranges */
#define IAQ_EXCELLENT                      0
#define IAQ_GOOD                          50
#define IAQ_MODERATE                     100
#define IAQ_POOR                         150
#define IAQ_VERY_POOR                    200
#define IAQ_EXTREMELY_POOR               300

/* ============================================================================
 * Air Quality Data Types
 * ============================================================================ */

/**
 * @brief Air quality sensor configuration
 */
typedef struct {
    esocore_sensor_config_t base_config;    /**< Base sensor configuration */
    uint8_t sensor_suite_type;              /**< Type of sensor suite */
    bool enable_co2_sensor;                 /**< Enable CO2 sensor */
    bool enable_voc_sensor;                 /**< Enable VOC sensor */
    bool enable_pm25_sensor;                /**< Enable PM2.5 sensor */
    bool enable_temperature_sensor;         /**< Enable temperature sensor */
    bool enable_humidity_sensor;            /**< Enable humidity sensor */
    bool enable_pressure_sensor;            /**< Enable pressure sensor */
    bool enable_iaq_calculation;            /**< Enable IAQ calculation */
    bool enable_health_risk_assessment;     /**< Enable health risk assessment */
    uint8_t calibration_method;             /**< Calibration method */
    uint32_t measurement_interval_ms;       /**< Measurement interval */
} air_quality_sensor_config_t;

/**
 * @brief Individual gas sensor data
 */
typedef struct {
    uint8_t gas_type;                       /**< Type of gas (0=CO2, 1=VOC, 2=CO, 3=NO2, etc.) */
    float concentration_ppm;                /**< Concentration in ppm */
    float concentration_ug_m3;              /**< Concentration in µg/m³ */
    uint16_t raw_adc_value;                 /**< Raw ADC value */
    float temperature_compensated;          /**< Temperature compensated value */
    bool sensor_valid;                      /**< Sensor reading is valid */
    uint32_t timestamp;                     /**< Reading timestamp */
} air_quality_gas_data_t;

/**
 * @brief Particulate matter data
 */
typedef struct {
    float pm1_0;                           /**< PM1.0 concentration (µg/m³) */
    float pm2_5;                           /**< PM2.5 concentration (µg/m³) */
    float pm10;                            /**< PM10 concentration (µg/m³) */
    uint16_t raw_counts;                    /**< Raw particle counts */
    bool sensor_valid;                     /**< PM sensor reading is valid */
} air_quality_pm_data_t;

/**
 * @brief Environmental data
 */
typedef struct {
    float temperature_celsius;              /**< Temperature (°C) */
    float humidity_percent;                 /**< Relative humidity (%) */
    float pressure_hpa;                     /**< Atmospheric pressure (hPa) */
    float dew_point_celsius;                /**< Dew point (°C) */
    float absolute_humidity;                /**< Absolute humidity (g/m³) */
} air_quality_environmental_data_t;

/**
 * @brief IAQ (Indoor Air Quality) calculation results
 */
typedef struct {
    uint16_t iaq_index;                     /**< IAQ index (0-500) */
    uint8_t iaq_rating;                     /**< IAQ rating (0=Excellent, 1=Good, etc.) */
    float co2_equivalent;                   /**< CO2 equivalent (ppm) */
    float breath_voc_equivalent;            /**< Breath VOC equivalent (ppm) */
    float gas_resistance;                   /**< Gas sensor resistance */
    uint32_t accuracy;                      /**< IAQ calculation accuracy */
    uint32_t timestamp;                     /**< Calculation timestamp */
} air_quality_iaq_data_t;

/**
 * @brief Health risk assessment
 */
typedef struct {
    uint8_t overall_risk_level;             /**< Overall health risk (0-100) */
    uint8_t respiratory_risk;               /**< Respiratory health risk */
    uint8_t cardiovascular_risk;            /**< Cardiovascular health risk */
    uint8_t neurological_risk;              /**< Neurological health risk */
    char risk_description[128];             /**< Risk description */
    char recommendations[256];              /**< Health recommendations */
    uint32_t assessment_timestamp;          /**< Assessment timestamp */
} air_quality_health_data_t;

/**
 * @brief Complete air quality sensor data
 */
typedef struct {
    esocore_sensor_data_t base_data;        /**< Base sensor data structure */
    air_quality_gas_data_t gas_sensors[AIR_QUALITY_MAX_GASES]; /**< Gas sensor data */
    air_quality_pm_data_t pm_data;          /**< Particulate matter data */
    air_quality_environmental_data_t env_data; /**< Environmental data */
    air_quality_iaq_data_t iaq_data;        /**< IAQ calculation results */
    air_quality_health_data_t health_data;  /**< Health risk assessment */
    uint8_t air_quality_score;              /**< Overall air quality score (0-100) */
    char quality_description[64];           /**< Air quality description */
} air_quality_sensor_data_t;

/**
 * @brief Air quality sensor status
 */
typedef struct {
    esocore_sensor_status_t base_status;    /**< Base sensor status */
    bool co2_sensor_ready;                  /**< CO2 sensor is ready */
    bool voc_sensor_ready;                  /**< VOC sensor is ready */
    bool pm_sensor_ready;                   /**< PM sensor is ready */
    bool temp_sensor_ready;                 /**< Temperature sensor is ready */
    bool humidity_sensor_ready;             /**< Humidity sensor is ready */
    uint32_t measurements_taken;            /**< Number of measurements taken */
    uint8_t active_sensors;                 /**< Number of active sensors */
    bool calibration_valid;                 /**< Calibration is valid */
    uint32_t last_calibration_time;         /**< Last calibration timestamp */
} air_quality_sensor_status_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize air quality sensor
 */
bool air_quality_sensor_init(const air_quality_sensor_config_t *config);

/**
 * @brief Deinitialize air quality sensor
 */
bool air_quality_sensor_deinit(void);

/**
 * @brief Configure air quality sensor
 */
bool air_quality_sensor_configure(const air_quality_sensor_config_t *config);

/**
 * @brief Start air quality measurement
 */
bool air_quality_sensor_start_measurement(void);

/**
 * @brief Stop air quality measurement
 */
bool air_quality_sensor_stop_measurement(void);

/**
 * @brief Read air quality sensor data
 */
bool air_quality_sensor_read_data(air_quality_sensor_data_t *data, uint32_t timeout_ms);

/**
 * @brief Get air quality sensor status
 */
bool air_quality_sensor_get_status(air_quality_sensor_status_t *status);

/**
 * @brief Perform air quality sensor self-test
 */
bool air_quality_sensor_self_test(void);

/**
 * @brief Calibrate air quality sensor
 */
bool air_quality_sensor_calibrate(void);

/**
 * @brief Calculate IAQ index
 */
bool air_quality_calculate_iaq(const air_quality_gas_data_t *gas_data,
                              const air_quality_environmental_data_t *env_data,
                              air_quality_iaq_data_t *iaq_data);

/**
 * @brief Assess health risks
 */
bool air_quality_assess_health_risks(const air_quality_iaq_data_t *iaq_data,
                                    const air_quality_pm_data_t *pm_data,
                                    air_quality_health_data_t *health_data);

/**
 * @brief Analyze environmental conditions
 */
bool air_quality_analyze_environment(const air_quality_environmental_data_t *env_data,
                                    char *analysis_description, uint16_t buffer_size);

/**
 * @brief Calculate overall air quality score
 */
uint8_t air_quality_calculate_score(const air_quality_iaq_data_t *iaq_data,
                                   const air_quality_pm_data_t *pm_data);

/**
 * @brief Get air quality sensor capabilities
 */
const esocore_sensor_capabilities_t *air_quality_sensor_get_capabilities(void);

/**
 * @brief Get air quality sensor configuration
 */
bool air_quality_sensor_get_configuration(air_quality_sensor_config_t *config);

/**
 * @brief Enable/disable specific sensors
 */
bool air_quality_enable_sensor(uint8_t sensor_type, bool enable);

/**
 * @brief Get current IAQ index
 */
bool air_quality_get_iaq_index(uint16_t *iaq_index);

/**
 * @brief Get current CO2 level
 */
bool air_quality_get_co2_level(float *co2_ppm);

/**
 * @brief Get current VOC level
 */
bool air_quality_get_voc_level(float *voc_ppm);

/**
 * @brief Get current PM2.5 level
 */
bool air_quality_get_pm25_level(float *pm25_ug_m3);

/**
 * @brief Clear sensor data buffers
 */
bool air_quality_clear_buffers(void);

/**
 * @brief Get air quality diagnostics
 */
bool air_quality_get_diagnostics(uint8_t *diagnostic_data, uint16_t buffer_size);

/**
 * @brief Export air quality data
 */
bool air_quality_export_data(const air_quality_sensor_data_t *data,
                           const char *filename, uint8_t format);

#endif /* AIR_QUALITY_SENSOR_H */