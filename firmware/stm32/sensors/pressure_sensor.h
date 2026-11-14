/**
 * @file pressure_sensor.h
 * @brief Pressure Sensor Driver
 *
 * This header defines the pressure sensor driver with multi-range sensing,
 * hydraulic and pneumatic system analysis, leak detection and filter monitoring.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

/* ============================================================================
 * Pressure Data Types
 * ============================================================================ */

/**
 * @brief Pressure sensor configuration
 */
typedef struct {
    esocore_sensor_config_t base_config;    /**< Base sensor configuration */
    uint8_t sensor_type;                    /**< Type of pressure sensor */
    float pressure_range_min;               /**< Minimum pressure of the range */
    float pressure_range_max;               /**< Maximum pressure of the range */
    uint8_t measurement_range;              /**< Measurement range (0=1 bar, 1=10 bar, 2=100 bar) */
    uint8_t measurement_unit;               /**< System type (0=hydraulic, 1=pneumatic) */
    bool enable_temperature_comp;           /**< Enable temperature compensation */
    bool enable_burst_mode;                 /**< Enable burst mode */
} pressure_sensor_config_t;

/**
 * @brief Pressure anomaly detection results
 */
typedef struct {
    bool pressure_surge_detected;           /**< Sudden pressure rise detected */
    bool pressure_drop_detected;            /**< Sudden pressure drop detected */
    bool rapid_change_detected;             /**< Pressure rate above limit */
    bool oscillation_detected;              /**< Pressure oscillation detected */
    bool anomaly_detected;                  /**< Any anomaly detected */
    char anomaly_description[96];           /**< Anomaly description */
    uint8_t severity_level;                 /**< Combined severity level (0-8) */
    uint32_t detection_timestamp;           /**< Detection timestamp */
} pressure_anomaly_detection_t;

/**
 * @brief Hydraulic system analysis
 */
typedef struct {
    char pump_performance[48];              /**< Pump performance assessment */
    float pump_efficiency;                  /**< Estimated pump efficiency (0-1) */
    char system_stability[32];              /**< System stability assessment */
    uint8_t stability_rating;               /**< Stability rating (1-5) */
    float fluid_contamination_indicators;   /**< Fluid contamination indicator (0-1) */
    uint32_t analysis_timestamp;            /**< Analysis timestamp */
} pressure_hydraulic_analysis_t;

/**
 * @brief Pneumatic system analysis
 */
typedef struct {
    char compressor_performance[48];        /**< Compressor performance assessment */
    float compressor_efficiency;            /**< Estimated compressor efficiency (0-1) */
    float air_quality_indicators;           /**< Air quality indicator (0-1) */
    char system_responsiveness[48];         /**< System responsiveness assessment */
    uint32_t response_time_ms;              /**< Estimated response time (ms) */
    uint32_t analysis_timestamp;            /**< Analysis timestamp */
} pressure_pneumatic_analysis_t;

/**
 * @brief Leak detection results
 */
typedef struct {
    char leak_status[48];                   /**< Leak status description */
    bool leak_detected;                     /**< Leak detected */
    float leak_rate_bar_min;                /**< Leak rate (bar/min) */
    float leak_severity;                    /**< Leak severity (0-1) */
    uint32_t detection_timestamp;           /**< Detection timestamp */
} pressure_leak_detection_t;

/**
 * @brief Filter monitoring results
 */
typedef struct {
    float differential_pressure_bar;        /**< Differential pressure across the filter (bar) */
    char filter_condition[64];              /**< Filter condition description */
    float filter_efficiency;                /**< Filter efficiency (0-1) */
    float clogging_level;                   /**< Clogging level (0-1) */
    float estimated_life_remaining_percent; /**< Estimated remaining filter life (%) */
    uint32_t monitoring_timestamp;          /**< Monitoring timestamp */
} pressure_filter_monitoring_t;

/**
 * @brief Complete pressure sensor data
 */
typedef struct {
    float pressure_pa;                      /**< Pressure (Pa) */
    float pressure_bar;                     /**< Pressure (bar) */
    float temperature_celsius;              /**< Sensor temperature (°C) */
    uint32_t timestamp;                     /**< Data timestamp */
    uint16_t raw_adc_value;                 /**< Raw ADC value */
    float pressure_rate_bar_s;              /**< Pressure rate of change (bar/s) */
    float pressure_stability;               /**< Pressure stability (0-1) */
    pressure_hydraulic_analysis_t hydraulic_analysis;   /**< Hydraulic system analysis */
    pressure_pneumatic_analysis_t pneumatic_analysis;   /**< Pneumatic system analysis */
    pressure_anomaly_detection_t anomaly_detection;     /**< Anomaly detection */
    pressure_leak_detection_t leak_detection;           /**< Leak detection */
    pressure_filter_monitoring_t filter_monitoring;     /**< Filter monitoring */
    uint8_t pressure_condition_score;       /**< Pressure system condition (0-100) */
    char condition_description[80];         /**< Condition description */
} pressure_sensor_data_t;

/**
 * @brief Pressure sensor status
 */
typedef struct {
    esocore_sensor_status_t base_status;    /**< Base sensor status */
    uint32_t measurements_taken;            /**< Number of measurements taken */
    uint32_t last_measurement_time;         /**< Timestamp of the last measurement */
    bool calibration_valid;                 /**< Calibration is valid */
} pressure_sensor_status_t;

/* ============================================================================
 * Public API
 * ============================================================================ */

/** @brief Initialize the pressure sensor */
bool pressure_sensor_init(const pressure_sensor_config_t *config);

/** @brief Deinitialize the pressure sensor */
bool pressure_sensor_deinit(void);

/** @brief Apply a new configuration */
bool pressure_sensor_configure(const pressure_sensor_config_t *config);

/** @brief Start pressure measurement */
bool pressure_sensor_start_measurement(void);

/** @brief Stop pressure measurement */
bool pressure_sensor_stop_measurement(void);

/** @brief Read and analyze one pressure measurement */
bool pressure_sensor_read_data(pressure_sensor_data_t *data, uint32_t timeout_ms);

/** @brief Get the pressure sensor status */
bool pressure_sensor_get_status(pressure_sensor_status_t *status);

/** @brief Run the pressure sensor self-test */
bool pressure_sensor_self_test(void);

/** @brief Calibrate the pressure sensor */
bool pressure_sensor_calibrate(void);

/** @brief Calculate the pressure system condition score (0-100) */
uint8_t pressure_sensor_calculate_condition(float pressure_bar, float pressure_stability,
                                          const pressure_anomaly_detection_t *anomalies);

/** @brief Get the pressure sensor capabilities */
const esocore_sensor_capabilities_t *pressure_sensor_get_capabilities(void);

/** @brief Get the current pressure sensor configuration */
bool pressure_sensor_get_configuration(pressure_sensor_config_t *config);

/** @brief Export pressure sensor data */
bool pressure_sensor_export_data(const pressure_sensor_data_t *data,
                               const char *filename, uint8_t format);

#endif /* PRESSURE_SENSOR_H */
