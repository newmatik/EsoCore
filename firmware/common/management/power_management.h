/**
 * @file power_management.h
 * @brief Power Management Interface for EsoCore Devices
 *
 * This file defines the power management interface for both Edge devices
 * and sensor modules, providing standardized power monitoring and control.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_POWER_MANAGEMENT_H
#define ESOCORE_POWER_MANAGEMENT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Power States
 * ============================================================================ */

typedef enum {
    POWER_STATE_NORMAL        = 0,      /* Normal operation */
    POWER_STATE_LOW_POWER     = 1,      /* Reduced power mode */
    POWER_STATE_SLEEP         = 2,      /* Sleep mode */
    POWER_STATE_DEEP_SLEEP    = 3,      /* Deep sleep mode */
    POWER_STATE_SHUTDOWN      = 4,      /* Shutdown mode */
} esocore_power_state_t;

/* ============================================================================
 * Power Sources
 * ============================================================================ */

typedef enum {
    POWER_SOURCE_DC_INPUT     = 0,      /* 12-24V DC input */
    POWER_SOURCE_POE          = 1,      /* Power over Ethernet */
    POWER_SOURCE_BATTERY      = 2,      /* Battery backup */
    POWER_SOURCE_SUPERCAP     = 3,      /* Supercapacitor backup */
} esocore_power_source_t;

/* ============================================================================
 * Power Thresholds
 * ============================================================================ */

typedef struct {
    uint16_t voltage_high_mv;           /* High voltage threshold (mV) */
    uint16_t voltage_low_mv;            /* Low voltage threshold (mV) */
    uint16_t current_limit_ma;          /* Current limit (mA) */
    uint8_t temperature_high_c;         /* High temperature threshold (°C) */
    uint32_t brownout_timeout_ms;       /* Brownout timeout (ms) */
    uint32_t shutdown_timeout_ms;       /* Shutdown timeout (ms) */
} esocore_power_thresholds_t;

/* ============================================================================
 * Power Monitoring Data
 * ============================================================================ */

typedef struct {
    uint32_t timestamp;                 /* Measurement timestamp */
    uint16_t input_voltage_mv;          /* Input voltage (mV) */
    uint16_t output_voltage_mv;         /* Output voltage (mV) */
    int16_t input_current_ma;           /* Input current (mA) */
    int16_t output_current_ma;          /* Output current (mA) */
    uint16_t bus_voltage_mv;            /* Sensor bus voltage (mV) */
    int16_t bus_current_ma;             /* Sensor bus current (mA) */
    int8_t temperature_c;               /* Device temperature (°C) */
    esocore_power_source_t active_source; /* Currently active power source */
    uint8_t power_quality_score;        /* Power quality score (0-100) */
} esocore_power_monitoring_t;

/* ============================================================================
 * Power Management Interface
 * ============================================================================ */

/**
 * @brief Initialize power management system
 *
 * @param thresholds Pointer to power thresholds configuration
 * @return true if initialization successful, false otherwise
 */
bool esocore_power_init(const esocore_power_thresholds_t *thresholds);

/**
 * @brief Set power state
 *
 * @param state New power state to enter
 * @return true if state change successful, false otherwise
 */
bool esocore_power_set_state(esocore_power_state_t state);

/**
 * @brief Get current power state
 *
 * @return Current power state
 */
esocore_power_state_t esocore_power_get_state(void);

/**
 * @brief Get power monitoring data
 *
 * @param data Pointer to monitoring data structure to fill
 * @return true if data retrieved successfully, false otherwise
 */
bool esocore_power_get_monitoring_data(esocore_power_monitoring_t *data);

/**
 * @brief Check for power-related faults
 *
 * @return Bitmask of active power faults (0 = no faults)
 */
uint16_t esocore_power_check_faults(void);

/**
 * @brief Perform emergency shutdown
 *
 * @return true if shutdown initiated successfully, false otherwise
 */
bool esocore_power_emergency_shutdown(void);

/**
 * @brief Configure power source switching
 *
 * @param primary_source Primary power source
 * @param backup_source Backup power source
 * @param auto_switch Enable automatic source switching
 * @return true if configuration successful, false otherwise
 */
bool esocore_power_configure_sources(esocore_power_source_t primary_source,
                                    esocore_power_source_t backup_source,
                                    bool auto_switch);

/**
 * @brief Set power thresholds
 *
 * @param thresholds Pointer to new threshold values
 * @return true if thresholds set successfully, false otherwise
 */
bool esocore_power_set_thresholds(const esocore_power_thresholds_t *thresholds);

/* ============================================================================
 * Supercapacitor Management (Edge Device)
 * ============================================================================ */

/**
 * @brief Initialize supercapacitor backup system
 *
 * @param capacitance_f Total capacitance in Farads
 * @return true if initialization successful, false otherwise
 */
bool esocore_supercap_init(float capacitance_f);

/**
 * @brief Get supercapacitor charge level
 *
 * @return Charge level as percentage (0-100)
 */
uint8_t esocore_supercap_get_charge_level(void);

/**
 * @brief Get estimated backup runtime
 *
 * @param current_consumption_ma Current consumption in mA
 * @return Estimated runtime in seconds
 */
uint32_t esocore_supercap_get_backup_runtime(uint16_t current_consumption_ma);

/**
 * @brief Start supercapacitor charging
 *
 * @return true if charging started successfully, false otherwise
 */
bool esocore_supercap_start_charging(void);

/**
 * @brief Stop supercapacitor charging
 *
 * @return true if charging stopped successfully, false otherwise
 */
bool esocore_supercap_stop_charging(void);

/* ============================================================================
 * Power over Ethernet (PoE) Management
 * ============================================================================ */

/**
 * @brief Initialize PoE interface
 *
 * @return true if initialization successful, false otherwise
 */
bool esocore_poe_init(void);

/**
 * @brief Get PoE power class
 *
 * @return PoE power class (0-4), 255 if not detected
 */
uint8_t esocore_poe_get_power_class(void);

/**
 * @brief Check if PoE power is available
 *
 * @return true if PoE power is available, false otherwise
 */
bool esocore_poe_is_power_available(void);

/**
 * @brief Get PoE power consumption
 *
 * @return Power consumption in watts
 */
float esocore_poe_get_power_consumption(void);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Convert voltage from millivolts to volts
 *
 * @param mv Voltage in millivolts
 * @return Voltage in volts
 */
float esocore_voltage_mv_to_v(uint16_t mv);

/**
 * @brief Convert current from milliamps to amps
 *
 * @param ma Current in milliamps
 * @return Current in amps
 */
float esocore_current_ma_to_a(int16_t ma);

/**
 * @brief Calculate power consumption
 *
 * @param voltage_mv Voltage in millivolts
 * @param current_ma Current in milliamps
 * @return Power consumption in watts
 */
float esocore_calculate_power(uint16_t voltage_mv, int16_t current_ma);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_POWER_MANAGEMENT_H */
