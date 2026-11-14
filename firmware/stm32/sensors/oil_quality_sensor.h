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

/* ============================================================================
 * Oil Quality Sensor Configuration
 * ============================================================================ */

#define OIL_QUALITY_ASSESSMENT_TEXT_LEN    64   /**< Length of one assessment text (incl. NUL) */
#define OIL_QUALITY_ACTION_TEXT_LEN        128  /**< Length of one recommendation text (incl. NUL) */

/* ============================================================================
 * Oil Quality Data Types
 * ============================================================================ */

/**
 * @brief Oil quality sensor configuration
 */
typedef struct {
    esocore_sensor_config_t base_config;    /**< Base sensor configuration */
    uint8_t sensor_type;                    /**< Type of oil quality sensor */
    bool enable_turbidity;                  /**< Enable turbidity measurement */
    bool enable_viscosity;                  /**< Enable viscosity measurement */
    bool enable_water_content;              /**< Enable water content measurement */
    bool enable_dielectric;                 /**< Enable dielectric constant measurement */
    bool enable_temperature;                /**< Enable oil temperature measurement */
    float measurement_range;                /**< Measurement range */
} oil_quality_sensor_config_t;

/**
 * @brief Turbidity measurement (optical scattering)
 */
typedef struct {
    float turbidity_ntu;                    /**< Turbidity (NTU) */
    uint16_t raw_scattered_light;           /**< Raw scattered light intensity */
    uint16_t raw_incident_light;            /**< Raw incident light intensity */
} oil_quality_turbidity_data_t;

/**
 * @brief Viscosity measurement (piezoelectric)
 */
typedef struct {
    float viscosity_cst;                    /**< Temperature-compensated viscosity (cSt) */
    uint32_t oscillation_frequency_hz;      /**< Crystal oscillation frequency (Hz) */
} oil_quality_viscosity_data_t;

/**
 * @brief Water content measurement (capacitive)
 */
typedef struct {
    float water_content_ppm;                /**< Water content (ppm) */
    float water_content_percent;            /**< Water content (%) */
    uint32_t capacitance_pf;                /**< Measured capacitance (pF) */
} oil_quality_water_data_t;

/**
 * @brief Dielectric constant measurement
 */
typedef struct {
    float dielectric_constant;              /**< Relative dielectric constant */
    uint32_t capacitance_pf;                /**< Measured capacitance (pF) */
} oil_quality_dielectric_data_t;

/**
 * @brief Oil temperature measurement
 */
typedef struct {
    float temperature_celsius;              /**< Oil temperature (°C) */
    uint16_t raw_adc_value;                 /**< Raw ADC value */
} oil_quality_temperature_data_t;

/**
 * @brief Oil degradation analysis
 */
typedef struct {
    char turbidity_assessment[OIL_QUALITY_ASSESSMENT_TEXT_LEN];  /**< Turbidity assessment text */
    uint8_t contamination_level;            /**< Contamination level (0=clean .. 3=severe) */
    char viscosity_assessment[OIL_QUALITY_ASSESSMENT_TEXT_LEN];  /**< Viscosity assessment text */
    uint8_t viscosity_status;               /**< Viscosity status (0=optimal, 1=low, 2=high) */
    char water_assessment[OIL_QUALITY_ASSESSMENT_TEXT_LEN];      /**< Water content assessment text */
    uint8_t water_risk;                     /**< Water risk (0=dry .. 3=critical) */
    char dielectric_assessment[OIL_QUALITY_ASSESSMENT_TEXT_LEN]; /**< Dielectric assessment text */
    uint8_t dielectric_health;              /**< Dielectric health (0=normal .. 2=degraded) */
    float overall_degradation_level;        /**< Mean of the four levels above */
    uint32_t assessment_timestamp;          /**< Assessment timestamp */
} oil_degradation_analysis_t;

/**
 * @brief Oil maintenance recommendations
 */
typedef struct {
    char immediate_actions[OIL_QUALITY_ACTION_TEXT_LEN];     /**< Immediate actions */
    char scheduled_maintenance[OIL_QUALITY_ACTION_TEXT_LEN]; /**< Scheduled maintenance */
    char long_term_actions[OIL_QUALITY_ACTION_TEXT_LEN];     /**< Long-term actions */
    uint16_t change_interval_days;          /**< Recommended oil change interval (days) */
    uint32_t recommendation_timestamp;      /**< Recommendation timestamp */
} oil_maintenance_recommendations_t;

/**
 * @brief Oil quality sensor data
 */
typedef struct {
    float turbidity_ntu;                    /**< Turbidity (NTU) */
    float viscosity_cst;                    /**< Viscosity (cSt) */
    float water_content_ppm;                /**< Water content (ppm) */
    float dielectric_constant;              /**< Dielectric constant */
    float temperature_celsius;              /**< Oil temperature (°C) */
    uint32_t timestamp;                     /**< Data timestamp */
    oil_quality_turbidity_data_t turbidity_data;       /**< Turbidity measurement */
    oil_quality_viscosity_data_t viscosity_data;       /**< Viscosity measurement */
    oil_quality_water_data_t water_data;               /**< Water content measurement */
    oil_quality_dielectric_data_t dielectric_data;     /**< Dielectric measurement */
    oil_quality_temperature_data_t temperature_data;   /**< Temperature measurement */
    oil_degradation_analysis_t degradation_analysis;   /**< Degradation analysis */
    oil_maintenance_recommendations_t maintenance_recommendations; /**< Maintenance recommendations */
    uint8_t oil_quality_score;              /**< Overall oil quality score (0-100) */
    char condition_description[OIL_QUALITY_ASSESSMENT_TEXT_LEN]; /**< Oil condition description */
} oil_quality_sensor_data_t;

/**
 * @brief Oil quality sensor status
 */
typedef struct {
    esocore_sensor_status_t base_status;    /**< Base sensor status */
    bool sensors_ready;                     /**< All enabled sensors ready */
    uint32_t measurements_taken;            /**< Measurements taken since start */
    bool calibration_valid;                 /**< Calibration valid */
    uint32_t last_measurement_time;         /**< Last measurement timestamp */
} oil_quality_sensor_status_t;

/* ============================================================================
 * Oil Quality Sensor API
 * ============================================================================ */

/** @brief Initialize the oil quality sensor */
bool oil_quality_sensor_init(const oil_quality_sensor_config_t *config);

/** @brief Deinitialize the oil quality sensor */
bool oil_quality_sensor_deinit(void);

/** @brief Reconfigure the oil quality sensor */
bool oil_quality_sensor_configure(const oil_quality_sensor_config_t *config);

/** @brief Start oil quality measurement */
bool oil_quality_sensor_start_measurement(void);

/** @brief Stop oil quality measurement */
bool oil_quality_sensor_stop_measurement(void);

/** @brief Read oil quality data with analysis and recommendations */
bool oil_quality_sensor_read_data(oil_quality_sensor_data_t *data, uint32_t timeout_ms);

/** @brief Get oil quality sensor status */
bool oil_quality_sensor_get_status(oil_quality_sensor_status_t *status);

/** @brief Run the oil quality sensor self-test */
bool oil_quality_sensor_self_test(void);

/** @brief Calibrate the oil quality sensor */
bool oil_quality_sensor_calibrate(void);

/** @brief Get oil quality sensor capabilities */
const esocore_sensor_capabilities_t *oil_quality_sensor_get_capabilities(void);

/** @brief Get the current oil quality sensor configuration */
bool oil_quality_sensor_get_configuration(oil_quality_sensor_config_t *config);

/** @brief Export oil quality data to a file */
bool oil_quality_export_data(const oil_quality_sensor_data_t *data,
                             const char *filename, uint8_t format);

#endif /* OIL_QUALITY_SENSOR_H */
