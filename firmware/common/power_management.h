/**
 * @file power_management.h
 * @brief Power Management System
 *
 * This header defines the power management system for EsoCore devices,
 * providing support for PoE (Power over Ethernet), supercapacitor backup,
 * and intelligent power distribution.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Power Management Configuration
 * ============================================================================ */

#define ESOCORE_MAX_POWER_SOURCES        4
#define ESOCORE_MAX_POWER_CONSUMERS     8
#define ESOCORE_SUPERCAP_VOLTAGE_MAX    5500   /**< Maximum supercap voltage (mV) */
#define ESOCORE_SUPERCAP_VOLTAGE_MIN    2000   /**< Minimum supercap voltage (mV) */
#define ESOCORE_POE_VOLTAGE_NOMINAL     48000  /**< Nominal PoE voltage (mV) */

/* ============================================================================
 * Power Source Types
 * ============================================================================ */

/**
 * @brief Power source types
 */
typedef enum {
    ESOCORE_POWER_SOURCE_POE       = 0x01,  /**< Power over Ethernet */
    ESOCORE_POWER_SOURCE_DC_INPUT  = 0x02,  /**< DC power input */
    ESOCORE_POWER_SOURCE_BATTERY   = 0x03,  /**< Battery power */
    ESOCORE_POWER_SOURCE_SUPERCAP  = 0x04,  /**< Supercapacitor */
    ESOCORE_POWER_SOURCE_SOLAR     = 0x05,  /**< Solar panel */
    ESOCORE_POWER_SOURCE_USB       = 0x06,  /**< USB power */
    ESOCORE_POWER_SOURCE_MAX
} esocore_power_source_t;

/**
 * @brief Power consumer types
 */
typedef enum {
    ESOCORE_POWER_CONSUMER_MAIN_MCU    = 0x01,  /**< Main microcontroller */
    ESOCORE_POWER_CONSUMER_SENSOR_MCU  = 0x02,  /**< Sensor microcontroller */
    ESOCORE_POWER_CONSUMER_WIFI_MODULE = 0x03,  /**< WiFi communication module */
    ESOCORE_POWER_CONSUMER_SENSOR_ARRAY= 0x04,  /**< Sensor array */
    ESOCORE_POWER_CONSUMER_DISPLAY     = 0x05,  /**< OLED display */
    ESOCORE_POWER_CONSUMER_STORAGE     = 0x06,  /**< Storage system */
    ESOCORE_POWER_CONSUMER_SAFETY_IO   = 0x07,  /**< Safety I/O system */
    ESOCORE_POWER_CONSUMER_MAX
} esocore_power_consumer_t;

/**
 * @brief Power management states
 */
typedef enum {
    ESOCORE_POWER_STATE_NORMAL     = 0x00,  /**< Normal operation */
    ESOCORE_POWER_STATE_LOW_POWER  = 0x01,  /**< Low-power mode */
    ESOCORE_POWER_STATE_CRITICAL   = 0x02,  /**< Critical power mode */
    ESOCORE_POWER_STATE_BACKUP     = 0x03,  /**< Backup power mode */
    ESOCORE_POWER_STATE_SHUTDOWN   = 0x04,  /**< Shutdown mode */
    ESOCORE_POWER_STATE_FAULT      = 0x05   /**< Fault condition */
} esocore_power_state_t;

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief Power threshold configuration
 */
typedef struct {
    uint16_t voltage_high_mv;              /**< High voltage threshold (mV) */
    uint16_t voltage_low_mv;               /**< Low voltage threshold (mV) */
    uint16_t current_high_ma;              /**< High current threshold (mA) */
    uint16_t current_low_ma;               /**< Low current threshold (mA) */
    uint8_t temperature_high_celsius;      /**< High temperature threshold (°C) */
    uint8_t temperature_low_celsius;       /**< Low temperature threshold (°C) */
    uint32_t power_budget_mw;              /**< Power budget in milliwatts */
} esocore_power_threshold_t;

/**
 * @brief Power monitoring data
 */
typedef struct {
    uint16_t voltage_mv;                   /**< Current voltage (mV) */
    uint16_t current_ma;                   /**< Current consumption (mA) */
    uint32_t power_mw;                     /**< Power consumption (mW) */
    int8_t temperature_celsius;            /**< Temperature (°C) */
    uint16_t efficiency_percent;           /**< Power efficiency percentage */
    uint32_t uptime_seconds;               /**< Power source uptime */
    uint8_t fault_flags;                   /**< Fault condition flags */
    uint32_t energy_consumed_wh;           /**< Total energy consumed (Wh) */
} esocore_power_monitoring_t;

/**
 * @brief Power source configuration
 */
typedef struct {
    esocore_power_source_t source_type;    /**< Type of power source */
    bool enabled;                          /**< Power source is enabled */
    uint16_t nominal_voltage_mv;           /**< Nominal voltage (mV) */
    uint16_t max_current_ma;               /**< Maximum current (mA) */
    uint8_t priority;                      /**< Power source priority (0=highest) */
    esocore_power_threshold_t thresholds;  /**< Threshold configuration */
    uint32_t switchover_delay_ms;          /**< Delay before switching sources */
} esocore_power_source_config_t;

/**
 * @brief Power consumer configuration
 */
typedef struct {
    esocore_power_consumer_t consumer_type;/**< Type of power consumer */
    bool enabled;                          /**< Power consumer is enabled */
    uint16_t nominal_voltage_mv;           /**< Required voltage (mV) */
    uint16_t max_current_ma;               /**< Maximum current draw (mA) */
    uint8_t priority;                      /**< Consumer priority (0=highest) */
    bool can_sleep;                        /**< Consumer can enter sleep mode */
    uint32_t sleep_current_ua;             /**< Current draw in sleep mode (µA) */
} esocore_power_consumer_config_t;

/**
 * @brief Power management status
 */
typedef struct {
    esocore_power_state_t current_state;   /**< Current power management state */
    esocore_power_source_t active_source;  /**< Currently active power source */
    esocore_power_source_t backup_source;  /**< Backup power source */
    uint16_t system_voltage_mv;            /**< System bus voltage (mV) */
    uint16_t total_current_ma;             /**< Total system current (mA) */
    uint32_t available_power_mw;           /**< Available power budget (mW) */
    uint32_t consumed_power_mw;            /**< Consumed power (mW) */
    uint8_t active_consumers;              /**< Number of active consumers */
    uint8_t fault_count;                   /**< Number of power faults */
    uint32_t last_fault_time;              /**< Timestamp of last fault */
    bool supercap_charging;                /**< Supercapacitor is charging */
    bool poe_detected;                     /**< PoE power is detected */
    bool low_power_warning;                /**< Low power warning active */
} esocore_power_status_t;

/* ============================================================================
 * Fault Flags
 * ============================================================================ */

#define ESOCORE_POWER_FAULT_OVERVOLTAGE      0x01   /**< Overvoltage condition */
#define ESOCORE_POWER_FAULT_UNDERVOLTAGE     0x02   /**< Undervoltage condition */
#define ESOCORE_POWER_FAULT_OVERCURRENT      0x04   /**< Overcurrent condition */
#define ESOCORE_POWER_FAULT_OVERTEMPERATURE  0x08   /**< Overtemperature condition */
#define ESOCORE_POWER_FAULT_SOURCE_FAILURE   0x10   /**< Power source failure */
#define ESOCORE_POWER_FAULT_CONSUMER_FAULT   0x20   /**< Power consumer fault */
#define ESOCORE_POWER_FAULT_SUPERCAP_LOW     0x40   /**< Supercapacitor low voltage */
#define ESOCORE_POWER_FAULT_POE_NEGOTIATION  0x80   /**< PoE negotiation failure */

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize power management system
 *
 * @return true if initialization successful, false otherwise
 */
bool esocore_power_init(void);

/**
 * @brief Configure power source
 *
 * @param source_type Type of power source to configure
 * @param config Pointer to source configuration
 * @return true if source configured successfully, false otherwise
 */
bool esocore_power_configure_source(esocore_power_source_t source_type,
                                   const esocore_power_source_config_t *config);

/**
 * @brief Configure power consumer
 *
 * @param consumer_type Type of power consumer to configure
 * @param config Pointer to consumer configuration
 * @return true if consumer configured successfully, false otherwise
 */
bool esocore_power_configure_consumer(esocore_power_consumer_t consumer_type,
                                     const esocore_power_consumer_config_t *config);

/**
 * @brief Get power management status
 *
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool esocore_power_get_status(esocore_power_status_t *status);

/**
 * @brief Get power source monitoring data
 *
 * @param source_type Type of power source
 * @param monitoring Pointer to monitoring structure to fill
 * @return true if monitoring data retrieved successfully, false otherwise
 */
bool esocore_power_get_source_monitoring(esocore_power_source_t source_type,
                                        esocore_power_monitoring_t *monitoring);

/**
 * @brief Get power consumer monitoring data
 *
 * @param consumer_type Type of power consumer
 * @param monitoring Pointer to monitoring structure to fill
 * @return true if monitoring data retrieved successfully, false otherwise
 */
bool esocore_power_get_consumer_monitoring(esocore_power_consumer_t consumer_type,
                                          esocore_power_monitoring_t *monitoring);

/**
 * @brief Enable power source
 *
 * @param source_type Type of power source to enable
 * @return true if source enabled successfully, false otherwise
 */
bool esocore_power_enable_source(esocore_power_source_t source_type);

/**
 * @brief Disable power source
 *
 * @param source_type Type of power source to disable
 * @return true if source disabled successfully, false otherwise
 */
bool esocore_power_disable_source(esocore_power_source_t source_type);

/**
 * @brief Enable power consumer
 *
 * @param consumer_type Type of power consumer to enable
 * @return true if consumer enabled successfully, false otherwise
 */
bool esocore_power_enable_consumer(esocore_power_consumer_t consumer_type);

/**
 * @brief Disable power consumer
 *
 * @param consumer_type Type of power consumer to disable
 * @return true if consumer disabled successfully, false otherwise
 */
bool esocore_power_disable_consumer(esocore_power_consumer_t consumer_type);

/**
 * @brief Switch to backup power source
 *
 * @return true if switch successful, false otherwise
 */
bool esocore_power_switch_to_backup(void);

/**
 * @brief Enter low-power mode
 *
 * @return true if low-power mode entered successfully, false otherwise
 */
bool esocore_power_enter_low_power(void);

/**
 * @brief Exit low-power mode
 *
 * @return true if low-power mode exited successfully, false otherwise
 */
bool esocore_power_exit_low_power(void);

/**
 * @brief Perform emergency shutdown
 *
 * @return true if shutdown initiated successfully, false otherwise
 */
bool esocore_power_emergency_shutdown(void);

/**
 * @brief Get supercap charge level
 *
 * @param voltage_mv Pointer to store supercap voltage
 * @param charge_percent Pointer to store charge percentage
 * @return true if supercap data retrieved successfully, false otherwise
 */
bool esocore_power_get_supercap_status(uint16_t *voltage_mv, uint8_t *charge_percent);

/**
 * @brief Start supercap charging
 *
 * @return true if charging started successfully, false otherwise
 */
bool esocore_power_start_supercap_charging(void);

/**
 * @brief Stop supercap charging
 *
 * @return true if charging stopped successfully, false otherwise
 */
bool esocore_power_stop_supercap_charging(void);

/**
 * @brief Check PoE power availability
 *
 * @param voltage_mv Pointer to store PoE voltage
 * @param power_class Pointer to store PoE power class
 * @return true if PoE data retrieved successfully, false otherwise
 */
bool esocore_power_check_poe(uint16_t *voltage_mv, uint8_t *power_class);

/**
 * @brief Negotiate PoE power class
 *
 * @param requested_class Requested power class (0-4)
 * @return true if negotiation successful, false otherwise
 */
bool esocore_power_negotiate_poe_class(uint8_t requested_class);

/**
 * @brief Get power consumption statistics
 *
 * @param total_energy_wh Pointer to store total energy consumed
 * @param average_power_mw Pointer to store average power consumption
 * @param peak_power_mw Pointer to store peak power consumption
 * @return true if statistics retrieved successfully, false otherwise
 */
bool esocore_power_get_statistics(uint32_t *total_energy_wh, uint32_t *average_power_mw,
                                 uint32_t *peak_power_mw);

/**
 * @brief Reset power consumption statistics
 *
 * @return true if statistics reset successfully, false otherwise
 */
bool esocore_power_reset_statistics(void);

/**
 * @brief Set power management thresholds
 *
 * @param thresholds Pointer to threshold configuration
 * @return true if thresholds set successfully, false otherwise
 */
bool esocore_power_set_thresholds(const esocore_power_threshold_t *thresholds);

/**
 * @brief Get current power thresholds
 *
 * @param thresholds Pointer to threshold structure to fill
 * @return true if thresholds retrieved successfully, false otherwise
 */
bool esocore_power_get_thresholds(esocore_power_threshold_t *thresholds);

/**
 * @brief Check for power faults
 *
 * @param fault_flags Pointer to store fault flags
 * @return true if fault check completed successfully, false otherwise
 */
bool esocore_power_check_faults(uint8_t *fault_flags);

/**
 * @brief Clear power fault flags
 *
 * @param fault_flags Fault flags to clear
 * @return true if faults cleared successfully, false otherwise
 */
bool esocore_power_clear_faults(uint8_t fault_flags);

/**
 * @brief Get power fault description
 *
 * @param fault_flag Fault flag to describe
 * @param description Pointer to description buffer
 * @param buffer_size Size of description buffer
 * @return true if description retrieved successfully, false otherwise
 */
bool esocore_power_get_fault_description(uint8_t fault_flag, char *description,
                                        uint16_t buffer_size);

/**
 * @brief Perform power system diagnostics
 *
 * @param diagnostic_result Pointer to store diagnostic result
 * @return true if diagnostics completed successfully, false otherwise
 */
bool esocore_power_run_diagnostics(uint16_t *diagnostic_result);

/**
 * @brief Enable power monitoring
 *
 * @param enable true to enable monitoring, false to disable
 * @return true if monitoring setting changed successfully, false otherwise
 */
bool esocore_power_enable_monitoring(bool enable);

/**
 * @brief Set power management state
 *
 * @param state Power management state to set
 * @return true if state set successfully, false otherwise
 */
bool esocore_power_set_state(esocore_power_state_t state);

/**
 * @brief Get power management state
 *
 * @param state Pointer to store current state
 * @return true if state retrieved successfully, false otherwise
 */
bool esocore_power_get_state(esocore_power_state_t *state);

/**
 * @brief Register power event callback
 *
 * @param callback Callback function for power events
 * @param context User context pointer
 * @return true if callback registered successfully, false otherwise
 */
bool esocore_power_register_callback(void (*callback)(uint8_t event, void *context),
                                    void *context);

/**
 * @brief Unregister power event callback
 *
 * @return true if callback unregistered successfully, false otherwise
 */
bool esocore_power_unregister_callback(void);

/**
 * @brief Get power efficiency metrics
 *
 * @param source_efficiency Pointer to store source efficiency
 * @param system_efficiency Pointer to store system efficiency
 * @return true if efficiency metrics retrieved successfully, false otherwise
 */
bool esocore_power_get_efficiency(uint8_t *source_efficiency, uint8_t *system_efficiency);

/**
 * @brief Optimize power distribution
 *
 * @return true if optimization completed successfully, false otherwise
 */
bool esocore_power_optimize_distribution(void);

/**
 * @brief Get power budget information
 *
 * @param available_power Pointer to store available power
 * @param allocated_power Pointer to store allocated power
 * @param reserved_power Pointer to store reserved power
 * @return true if budget information retrieved successfully, false otherwise
 */
bool esocore_power_get_budget(uint32_t *available_power, uint32_t *allocated_power,
                             uint32_t *reserved_power);

/**
 * @brief Reserve power for critical operations
 *
 * @param power_mw Power to reserve in milliwatts
 * @return true if power reserved successfully, false otherwise
 */
bool esocore_power_reserve_power(uint32_t power_mw);

/**
 * @brief Release reserved power
 *
 * @param power_mw Power to release in milliwatts
 * @return true if power released successfully, false otherwise
 */
bool esocore_power_release_reserved_power(uint32_t power_mw);

#endif /* POWER_MANAGEMENT_H */
