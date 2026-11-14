/**
 * @file proximity_position_sensor.h
 * @brief Proximity Position Sensor Driver
 *
 * This header defines the proximity position sensor driver for inductive,
 * capacitive, quadrature encoder, magnetic (reed switch) and laser distance
 * (VL53L0X) sensors, with accuracy, motion, detection and health analysis.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef PROXIMITY_POSITION_SENSOR_H
#define PROXIMITY_POSITION_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

/* ============================================================================
 * Proximity Position Data Types
 * ============================================================================ */

/**
 * @brief Proximity position sensor configuration
 */
typedef struct {
    esocore_sensor_config_t base_config;    /**< Base sensor configuration */
    uint8_t sensor_type;                    /**< Type of proximity/position sensor */
    float detection_range_mm;               /**< Detection range (mm) */
    uint8_t measurement_mode;               /**< Measurement mode */
    bool enable_multitarget;                /**< Enable multi-target detection */
    uint8_t output_format;                  /**< Output data format */
} proximity_position_sensor_config_t;

/**
 * @brief Inductive proximity sensor readings (two channels)
 */
typedef struct {
    bool sensor_states[2];                  /**< Target detected per channel */
    uint8_t target_count;                   /**< Number of channels detecting a target */
    bool valid;                             /**< Reading is valid */
} proximity_inductive_data_t;

/**
 * @brief Capacitive proximity sensor reading
 */
typedef struct {
    float distance_mm;                      /**< Estimated target distance (mm) */
    float capacitance_pf;                   /**< Measured capacitance (pF) */
    bool target_detected;                   /**< Target within sensing distance */
    bool valid;                             /**< Reading is valid */
} proximity_capacitive_data_t;

/**
 * @brief Quadrature encoder reading
 */
typedef struct {
    int32_t position_counts;                /**< Encoder position (counts) */
    float position_mm;                      /**< Position (mm) */
    float velocity_mm_s;                    /**< Velocity (mm/s) */
    float acceleration_mm_s2;               /**< Acceleration (mm/s^2) */
    int8_t direction;                       /**< Direction of motion (1 or -1) */
    bool valid;                             /**< Reading is valid */
} proximity_encoder_data_t;

/**
 * @brief Magnetic (reed switch) sensor readings (two channels)
 */
typedef struct {
    bool sensor_states[2];                  /**< Switch activated per channel */
    float magnetic_field_mt;                /**< Magnetic field strength (mT) */
    bool valid;                             /**< Reading is valid */
} proximity_magnetic_data_t;

/**
 * @brief Laser distance sensor (VL53L0X) reading
 */
typedef struct {
    float distance_mm;                      /**< Measured distance (mm) */
    uint16_t signal_rate_mcps;              /**< Signal rate (MCPS) */
    uint16_t ambient_rate_mcps;             /**< Ambient rate (MCPS) */
    uint8_t range_status;                   /**< Range status (0 = valid) */
    bool valid;                             /**< Reading is valid */
} proximity_laser_data_t;

/**
 * @brief Position accuracy analysis
 */
typedef struct {
    float encoder_resolution_mm;            /**< Encoder resolution (mm per pulse) */
    float encoder_accuracy_mm;              /**< Encoder accuracy (mm) */
    uint8_t encoder_precision_bits;         /**< Encoder precision (bits) */
    char encoder_accuracy_assessment[48];   /**< Encoder accuracy assessment */
    uint8_t encoder_accuracy_rating;        /**< Encoder accuracy rating (0-100) */
    float laser_accuracy_mm;                /**< Laser accuracy (mm) */
    float laser_precision_mm;               /**< Laser precision (mm) */
    char laser_accuracy_assessment[48];     /**< Laser accuracy assessment */
    uint8_t laser_accuracy_rating;          /**< Laser accuracy rating (0-100) */
    float overall_accuracy_mm;              /**< Overall position accuracy (mm) */
    char redundancy_benefits[64];           /**< Sensor redundancy assessment */
} position_accuracy_analysis_t;

/**
 * @brief Motion pattern analysis
 */
typedef struct {
    char speed_profile[24];                 /**< Speed classification */
    uint8_t motion_stability;               /**< Motion stability (0-100) */
    float acceleration_mm_s2;               /**< Acceleration (mm/s^2) */
    char acceleration_profile[32];          /**< Acceleration classification */
    float vibration_level;                  /**< Vibration/jerk level */
    char direction_consistency[32];         /**< Direction consistency assessment */
    uint8_t direction_stability;            /**< Direction stability (0-100) */
} motion_analysis_t;

/**
 * @brief Proximity detection performance analysis
 */
typedef struct {
    char detection_reliability[64];         /**< Detection reliability assessment */
    float false_positive_rate;              /**< Estimated false positive rate (0-1) */
    uint8_t detection_confidence;           /**< Detection confidence (0-100) */
    float effective_range_mm;               /**< Effective sensing range (mm) */
    float blind_zone_mm;                    /**< Blind zone (mm) */
    char coverage_assessment[48];           /**< Coverage assessment */
} proximity_detection_analysis_t;

/**
 * @brief System health monitoring
 */
typedef struct {
    uint8_t sensor_health_percentage;       /**< Healthy sensors (%) */
    char system_health_status[48];          /**< System health status */
    uint8_t system_reliability_rating;      /**< System reliability rating (0-100) */
    bool calibration_drift_detected;        /**< Calibration drift detected */
    float environmental_interference_level; /**< Environmental interference level */
} system_health_monitoring_t;

/**
 * @brief Complete proximity position sensor data
 */
typedef struct {
    float distance_mm;                      /**< Distance to target (mm) */
    float position_x_mm;                    /**< X position (mm) */
    float position_y_mm;                    /**< Y position (mm) */
    float position_z_mm;                    /**< Z position (mm) */
    uint8_t target_count;                   /**< Number of detected targets */
    proximity_inductive_data_t inductive_data;   /**< Inductive sensor readings */
    proximity_capacitive_data_t capacitive_data; /**< Capacitive sensor reading */
    proximity_encoder_data_t encoder_data;       /**< Quadrature encoder reading */
    proximity_magnetic_data_t magnetic_data;     /**< Magnetic sensor readings */
    proximity_laser_data_t laser_data;           /**< Laser distance reading */
    position_accuracy_analysis_t accuracy_analysis;     /**< Position accuracy analysis */
    motion_analysis_t motion_analysis;                  /**< Motion pattern analysis */
    proximity_detection_analysis_t detection_analysis;  /**< Detection performance analysis */
    system_health_monitoring_t health_monitoring;       /**< System health monitoring */
    uint8_t position_condition_score;       /**< Overall condition score (0-100) */
    char condition_description[64];         /**< Condition description */
    uint32_t timestamp;                     /**< Data timestamp */
} proximity_position_sensor_data_t;

/**
 * @brief Proximity position sensor status
 */
typedef struct {
    esocore_sensor_status_t base_status;    /**< Base sensor status */
    uint32_t measurements_taken;            /**< Number of measurements taken */
    uint32_t last_measurement_time;         /**< Timestamp of the last measurement */
    bool calibration_valid;                 /**< Calibration is valid */
} proximity_position_sensor_status_t;

/* ============================================================================
 * Public API
 * ============================================================================ */

/** @brief Initialize the proximity position sensor */
bool proximity_position_sensor_init(const proximity_position_sensor_config_t *config);

/** @brief Deinitialize the proximity position sensor */
bool proximity_position_sensor_deinit(void);

/** @brief Apply a new configuration */
bool proximity_position_sensor_configure(const proximity_position_sensor_config_t *config);

/** @brief Start measurement */
bool proximity_position_sensor_start_measurement(void);

/** @brief Stop measurement */
bool proximity_position_sensor_stop_measurement(void);

/** @brief Read and analyze one set of sensor data */
bool proximity_position_sensor_read_data(proximity_position_sensor_data_t *data, uint32_t timeout_ms);

/** @brief Get the sensor status */
bool proximity_position_sensor_get_status(proximity_position_sensor_status_t *status);

/** @brief Run the sensor self-test */
bool proximity_position_sensor_self_test(void);

/** @brief Calibrate the sensor */
bool proximity_position_sensor_calibrate(void);

/** @brief Calculate the overall position condition score (0-100) */
uint8_t proximity_position_sensor_calculate_condition(const position_accuracy_analysis_t *accuracy,
                                                    const motion_analysis_t *motion,
                                                    const proximity_detection_analysis_t *detection,
                                                    const system_health_monitoring_t *health);

/** @brief Get the sensor capabilities */
const esocore_sensor_capabilities_t *proximity_position_sensor_get_capabilities(void);

/** @brief Get the current configuration */
bool proximity_position_sensor_get_configuration(proximity_position_sensor_config_t *config);

/** @brief Export sensor data to a file */
bool proximity_position_sensor_export_data(const proximity_position_sensor_data_t *data,
                                         const char *filename, uint8_t format);

#endif /* PROXIMITY_POSITION_SENSOR_H */
