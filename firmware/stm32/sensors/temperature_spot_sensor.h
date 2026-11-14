/**
 * @file temperature_spot_sensor.h
 * @brief Temperature Spot Sensor Driver
 */

#ifndef TEMPERATURE_SPOT_SENSOR_H
#define TEMPERATURE_SPOT_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/sensors/sensor_interface.h"

/**
 * @brief Temperature spot sensor configuration
 */
typedef struct {
    esocore_sensor_config_t base_config;    /**< Base sensor configuration */
    uint8_t sensor_type;                    /**< Type of temperature sensor */
    float temp_range_min;                   /**< Minimum measurement temperature (°C) */
    float temp_range_max;                   /**< Maximum measurement temperature (°C) */
    uint8_t thermocouple_type;              /**< Thermocouple type */
    bool enable_cold_junction_comp;         /**< Enable cold junction compensation */
} temperature_spot_sensor_config_t;

/**
 * @brief Temperature spot sensor status
 */
typedef struct {
    esocore_sensor_status_t base_status;    /**< Base sensor status */
    uint32_t measurements_taken;            /**< Measurements since start of measurement */
    uint32_t last_measurement_time;         /**< Timestamp of the last measurement */
    bool calibration_valid;                 /**< Calibration data is valid */
} temperature_spot_sensor_status_t;

/**
 * @brief Resistive sensor reading (NTC thermistor or RTD)
 */
typedef struct {
    float temperature_celsius;              /**< Temperature (°C) */
    float resistance_ohms;                  /**< Measured resistance (Ω) */
    bool valid;                             /**< Reading is valid */
} temperature_resistive_reading_t;

/**
 * @brief Thermocouple reading
 */
typedef struct {
    float temperature_celsius;              /**< Temperature (°C) */
    float voltage_mv;                       /**< Thermocouple voltage (mV) */
    float cold_junction_temp_c;             /**< Cold junction temperature (°C) */
    bool valid;                             /**< Reading is valid */
} temperature_thermocouple_reading_t;

/**
 * @brief IR temperature reading
 */
typedef struct {
    float object_temperature_celsius;       /**< Object temperature (°C) */
    float ambient_temperature_celsius;      /**< Ambient temperature (°C) */
    bool valid;                             /**< Reading is valid */
} temperature_ir_reading_t;

/**
 * @brief Temperature statistics across all valid sensors
 */
typedef struct {
    float average_temperature_c;            /**< Average temperature (°C) */
    float min_temperature_c;                /**< Minimum temperature (°C) */
    float max_temperature_c;                /**< Maximum temperature (°C) */
    float temperature_range_c;              /**< Temperature range (°C) */
    float temperature_std_dev;              /**< Standard deviation (°C) */
    float thermal_stability;                /**< Thermal stability (0-1) */
    char stability_assessment[32];          /**< Stability assessment */
} temperature_statistics_t;

/**
 * @brief Thermal gradient and hotspot analysis
 */
typedef struct {
    float max_thermal_gradient_c;           /**< Maximum gradient between sensors (°C) */
    bool hotspot_detected;                  /**< Hotspot detected */
    uint8_t hotspot_sensor_index;           /**< Index of the hotspot sensor */
    char thermal_distribution[48];          /**< Thermal distribution description */
} temperature_gradient_analysis_t;

/**
 * @brief Thermal stress and fatigue analysis
 */
typedef struct {
    float thermal_stress_level;             /**< Thermal stress level (0-1) */
    float thermal_fatigue_accumulation;     /**< Accumulated thermal fatigue (0-1) */
    char stress_assessment[64];             /**< Stress assessment */
} temperature_stress_analysis_t;

/**
 * @brief Operating temperature limit monitoring
 */
typedef struct {
    float max_temperature_c;                /**< Maximum measured temperature (°C) */
    float min_temperature_c;                /**< Minimum measured temperature (°C) */
    uint8_t limit_violation_severity;       /**< Severity (0=OK, 1-2=warning, 3=critical) */
    char limit_status[80];                  /**< Limit status description */
} temperature_limit_monitoring_t;

/**
 * @brief Temperature spot sensor data
 */
typedef struct {
    float temperature_celsius;              /**< Temperature (°C) */
    float cold_junction_temp;               /**< Cold junction temperature (°C) */
    temperature_resistive_reading_t ntc_temperatures[4];            /**< NTC thermistor readings */
    temperature_resistive_reading_t rtd_temperatures[2];            /**< RTD readings */
    temperature_thermocouple_reading_t thermocouple_temperatures[2]; /**< Thermocouple readings */
    temperature_ir_reading_t ir_temperature;                        /**< IR temperature reading */
    temperature_statistics_t statistics;    /**< Temperature statistics */
    temperature_gradient_analysis_t gradient_analysis; /**< Gradient analysis */
    temperature_stress_analysis_t stress_analysis;     /**< Stress analysis */
    temperature_limit_monitoring_t limit_monitoring;   /**< Limit monitoring */
    uint8_t temperature_condition_score;    /**< Overall condition score (0-100) */
    char condition_description[64];         /**< Condition description */
    uint32_t timestamp;                     /**< Data timestamp */
} temperature_spot_sensor_data_t;

/** @brief Initialize the temperature spot sensor */
bool temperature_spot_sensor_init(const temperature_spot_sensor_config_t *config);
/** @brief Deinitialize the temperature spot sensor */
bool temperature_spot_sensor_deinit(void);
/** @brief Apply a new configuration */
bool temperature_spot_sensor_configure(const temperature_spot_sensor_config_t *config);
/** @brief Start temperature measurement */
bool temperature_spot_sensor_start_measurement(void);
/** @brief Stop temperature measurement */
bool temperature_spot_sensor_stop_measurement(void);
/** @brief Read and analyze temperature data from all sensors */
bool temperature_spot_sensor_read_data(temperature_spot_sensor_data_t *data, uint32_t timeout_ms);
/** @brief Get the sensor status */
bool temperature_spot_sensor_get_status(temperature_spot_sensor_status_t *status);
/** @brief Perform a self-test */
bool temperature_spot_sensor_self_test(void);
/** @brief Calibrate the sensor */
bool temperature_spot_sensor_calibrate(void);
/** @brief Calculate the overall temperature condition score (0-100) */
uint8_t temperature_spot_sensor_calculate_condition(const temperature_statistics_t *stats,
                                                  const temperature_gradient_analysis_t *gradient,
                                                  const temperature_stress_analysis_t *stress,
                                                  const temperature_limit_monitoring_t *limits);
/** @brief Get the sensor capabilities */
const esocore_sensor_capabilities_t *temperature_spot_sensor_get_capabilities(void);
/** @brief Get the current configuration */
bool temperature_spot_sensor_get_configuration(temperature_spot_sensor_config_t *config);
/** @brief Export temperature data */
bool temperature_spot_sensor_export_data(const temperature_spot_sensor_data_t *data,
                                       const char *filename, uint8_t format);

#endif /* TEMPERATURE_SPOT_SENSOR_H */
