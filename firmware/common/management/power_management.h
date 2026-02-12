/**
 * @file power_management.h
 * @brief Power Management Interface for EsoCore Edge v1.5.0.0
 *
 * Manages the 24V DC power input (replacing PoE from v1.4.0), voltage rail
 * monitoring, brownout detection, supercapacitor backup, and power sequencing.
 *
 * Power topology (v1.5.0.0):
 *   24V DC input -> TVS/Varistor/LC surge protection
 *     -> TPS54331 buck  -> 5V rail (3A)
 *       -> TLV1117LV33  -> 3.3V digital rail
 *       -> TPS7A4533     -> 3.3V analog rail (low-noise)
 *       -> TLV70212DBVR  -> 1.2V core rail
 *     -> TPS54331DR buck -> 12V sensor bus rail (2A)
 *   Supercapacitor: SLA3R8L2060813 3.8V 20F (backup for graceful shutdown)
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
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
    ESOCORE_POWER_STATE_NORMAL      = 0,  /* Normal operation, all rails OK */
    ESOCORE_POWER_STATE_LOW_POWER   = 1,  /* Reduced power mode */
    ESOCORE_POWER_STATE_BROWNOUT    = 2,  /* Input voltage dropping, early warning */
    ESOCORE_POWER_STATE_CRITICAL    = 3,  /* Input lost, running on supercap */
    ESOCORE_POWER_STATE_SHUTDOWN    = 4,  /* Graceful shutdown in progress */
} esocore_power_state_t;

/* ============================================================================
 * Power Sources (v1.5.0.0: PoE removed)
 * ============================================================================ */

typedef enum {
    ESOCORE_POWER_SOURCE_DC_INPUT   = 0,  /* 24V DC terminal block input */
    ESOCORE_POWER_SOURCE_SUPERCAP   = 1,  /* Supercapacitor backup (3.8V, 20F) */
} esocore_power_source_t;

/* ============================================================================
 * Power Rails
 * ============================================================================ */

typedef enum {
    ESOCORE_RAIL_24V        = 0,  /* 24V DC input */
    ESOCORE_RAIL_5V         = 1,  /* 5V buck (TPS54331) */
    ESOCORE_RAIL_3V3_DIG    = 2,  /* 3.3V digital (TLV1117LV33) */
    ESOCORE_RAIL_3V3_ANA    = 3,  /* 3.3V analog (TPS7A4533) */
    ESOCORE_RAIL_1V2        = 4,  /* 1.2V core (TLV70212DBVR) */
    ESOCORE_RAIL_12V_SBUS   = 5,  /* 12V sensor bus (TPS54331DR) */
    ESOCORE_RAIL_SUPERCAP   = 6,  /* Supercapacitor voltage */
    ESOCORE_RAIL_COUNT      = 7,
} esocore_power_rail_t;

/* ============================================================================
 * Power Fault Flags (bitmask)
 * ============================================================================ */

#define ESOCORE_POWER_FAULT_NONE            0x0000
#define ESOCORE_POWER_FAULT_OVERVOLTAGE     0x0001
#define ESOCORE_POWER_FAULT_UNDERVOLTAGE    0x0002
#define ESOCORE_POWER_FAULT_OVERCURRENT     0x0004
#define ESOCORE_POWER_FAULT_OVERTEMPERATURE 0x0008
#define ESOCORE_POWER_FAULT_RAIL_5V         0x0010
#define ESOCORE_POWER_FAULT_RAIL_3V3D       0x0020
#define ESOCORE_POWER_FAULT_RAIL_3V3A       0x0040
#define ESOCORE_POWER_FAULT_RAIL_1V2        0x0080
#define ESOCORE_POWER_FAULT_RAIL_12V        0x0100
#define ESOCORE_POWER_FAULT_SUPERCAP_LOW    0x0200
#define ESOCORE_POWER_FAULT_BROWNOUT        0x0400

/* ============================================================================
 * Configuration and Status Structures
 * ============================================================================ */

typedef struct {
    uint16_t brownout_warning_mv;       /* Early warning threshold (default 21000) */
    uint16_t brownout_shutdown_mv;      /* Hard shutdown threshold (default 19000) */
    uint16_t overvoltage_mv;            /* Overvoltage threshold (default 28500) */
    int8_t   temperature_limit_c;       /* Temperature limit (default 70) */
    uint32_t brownout_holdoff_ms;       /* Ignore transients shorter than this */
    bool     enable_supercap_backup;    /* Enable supercap backup on brownout */
    bool     enable_analog_sequencing;  /* Power analog rails before ADC enable */
} esocore_power_config_t;

typedef struct {
    uint16_t rail_voltage_mv[ESOCORE_RAIL_COUNT];  /* Measured rail voltages */
    esocore_power_state_t state;
    esocore_power_source_t active_source;
    uint16_t fault_flags;
    int8_t   temperature_c;
    uint8_t  supercap_charge_percent;
    uint32_t uptime_seconds;
    uint32_t brownout_count;            /* Total brownout events */
} esocore_power_status_t;

/* ============================================================================
 * Power Event Callback
 * ============================================================================ */

typedef enum {
    ESOCORE_POWER_EVENT_BROWNOUT_WARNING = 0,
    ESOCORE_POWER_EVENT_BROWNOUT_SHUTDOWN = 1,
    ESOCORE_POWER_EVENT_RAIL_FAULT = 2,
    ESOCORE_POWER_EVENT_RECOVERED = 3,
    ESOCORE_POWER_EVENT_SUPERCAP_LOW = 4,
    ESOCORE_POWER_EVENT_OVERTEMPERATURE = 5,
} esocore_power_event_t;

typedef void (*esocore_power_callback_t)(esocore_power_event_t event,
                                         uint16_t fault_flags,
                                         void *context);

/* ============================================================================
 * Power Management API
 * ============================================================================ */

/**
 * @brief Initialize power management system
 * @param config Power configuration (NULL for defaults)
 * @return true on success
 */
bool esocore_power_init(const esocore_power_config_t *config);

/**
 * @brief Deinitialize power management
 */
void esocore_power_deinit(void);

/**
 * @brief Poll power system (call periodically from main loop, ~10-100 Hz)
 *
 * Reads all rail voltages, checks thresholds, manages brownout state machine.
 */
void esocore_power_poll(void);

/**
 * @brief Get current power status
 */
bool esocore_power_get_status(esocore_power_status_t *status);

/**
 * @brief Get current power state
 */
esocore_power_state_t esocore_power_get_state(void);

/**
 * @brief Read a specific rail voltage in millivolts
 */
uint16_t esocore_power_read_rail(esocore_power_rail_t rail);

/**
 * @brief Check all power rails against thresholds
 * @return Fault flags bitmask (0 = all OK)
 */
uint16_t esocore_power_check_faults(void);

/**
 * @brief Register power event callback
 */
void esocore_power_register_callback(esocore_power_callback_t callback, void *context);

/**
 * @brief Initiate emergency shutdown
 *
 * Called when input power is lost. Saves state to microSD, de-energizes
 * outputs, and enters lowest power state.
 */
bool esocore_power_emergency_shutdown(void);

/**
 * @brief Execute power-up sequencing
 *
 * Ensures rails come up in correct order:
 *   1. 5V buck stabilized
 *   2. 3.3V digital
 *   3. 1.2V core
 *   4. 3.3V analog (must be stable before ADC/PGA enable)
 *   5. 12V sensor bus (last, on demand)
 *
 * @return true if all rails reach nominal within timeout
 */
bool esocore_power_sequence_up(void);

/**
 * @brief Execute power-down sequencing (reverse order)
 */
void esocore_power_sequence_down(void);

/* ============================================================================
 * Supercapacitor Management
 * ============================================================================ */

/**
 * @brief Get supercapacitor charge level (0-100%)
 */
uint8_t esocore_supercap_get_charge_level(void);

/**
 * @brief Get supercapacitor voltage in millivolts
 */
uint16_t esocore_supercap_get_voltage(void);

/**
 * @brief Estimate remaining backup runtime in seconds
 * @param load_ma Estimated load current in mA
 */
uint32_t esocore_supercap_estimate_runtime(uint16_t load_ma);

/**
 * @brief Check if supercap has enough charge for graceful shutdown
 */
bool esocore_supercap_is_shutdown_safe(void);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

float esocore_voltage_mv_to_v(uint16_t mv);
float esocore_current_ma_to_a(int16_t ma);
float esocore_calculate_power(uint16_t voltage_mv, int16_t current_ma);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_POWER_MANAGEMENT_H */
