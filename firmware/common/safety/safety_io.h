/**
 * @file safety_io.h
 * @brief Safety I/O System for EsoCore Edge Device
 *
 * This file implements the safety I/O system compliant with EN ISO 13849-1
 * functional safety standard for machinery safety. The system provides
 * 6 dual-channel safety digital inputs and relay outputs for industrial
 * safety applications.
 *
 * Safety Integrity Level (SIL): SIL 2
 * Performance Level (PL): PL d
 * Category: Category 3
 *
 * Each of the 6 digital input channels uses a pair of optocouplers
 * (Channel A + Channel B) monitoring the same 24V signal through independent
 * paths. Firmware cross-monitors both channels: if they disagree, a fault is
 * raised and the system enters safe state.
 *
 * Features:
 * - 6 dual-channel safety digital inputs (12 optocouplers, 2 per channel)
 * - Cross-monitoring with automatic fault detection
 * - Periodic test pulses for stuck-at fault detection
 * - Safety relay output (G7SA-2A2B-DC24) and SSR output (AQY212EHAZ)
 * - Watchdog supervision and safe state enforcement
 * - EN ISO 13849-1 Category 3 compliant safety functions
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_SAFETY_IO_H
#define ESOCORE_SAFETY_IO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Safety I/O Configuration
 * ============================================================================ */

#define SAFETY_INPUT_CHANNELS       6   /* Number of dual-channel safety input channels */
#define SAFETY_OUTPUT_CHANNELS      2   /* Number of relay output channels (safety relay + SSR) */
#define SAFETY_TEST_PULSE_INTERVAL  100 /* Test pulse interval (ms) */
#define SAFETY_WATCHDOG_TIMEOUT     500 /* Watchdog timeout (ms) */
#define SAFETY_FAULT_RESET_TIME     3000 /* Fault reset time (ms) */
#define SAFETY_RESPONSE_TIME_MAX    10  /* Maximum allowed response time (ms) */

/* Safety Input Types */
typedef enum {
    SAFETY_INPUT_EMERGENCY_STOP = 0,    /* Emergency stop button */
    SAFETY_INPUT_SAFETY_EDGE   = 1,    /* Safety edge contact */
    SAFETY_INPUT_LIGHT_CURTAIN = 2,    /* Light curtain interruption */
    SAFETY_INPUT_DOOR_SENSOR   = 3,    /* Door position sensor */
    SAFETY_INPUT_TWO_HAND      = 4,    /* Two-hand control */
    SAFETY_INPUT_GENERAL       = 5,    /* General-purpose digital input */
} safety_input_type_t;

/* Safety Output Types */
typedef enum {
    SAFETY_OUTPUT_RELAY       = 0,    /* Dual-channel safety relay (G7SA-2A2B-DC24) */
    SAFETY_OUTPUT_SSR         = 1,    /* Solid-state relay (AQY212EHAZ) */
} safety_output_type_t;

/* Safety Input Configuration (dual-channel per input) */
typedef struct {
    safety_input_type_t type;             /* Input type */
    bool enabled;                         /* Input enabled */
    bool normally_closed;                 /* NC or NO configuration */
    uint32_t debounce_time_ms;            /* Debounce time */
    uint8_t channel_a_pin;                /* Channel A optocoupler GPIO pin */
    uint8_t channel_b_pin;                /* Channel B optocoupler GPIO pin */
    char description[32];                 /* Input description */
} safety_input_config_t;

/* Safety Output Configuration */
typedef struct {
    safety_output_type_t type;            /* Output type */
    bool enabled;                         /* Output enabled */
    bool fail_safe;                       /* Fail-safe configuration */
    uint8_t relay_pin;                    /* Relay control GPIO pin */
    uint8_t feedback_pin;                 /* Feedback monitoring pin */
    char description[32];                 /* Output description */
} safety_output_config_t;

/* Safety System Configuration */
typedef struct {
    uint32_t watchdog_timeout_ms;         /* Watchdog timeout */
    uint32_t test_pulse_interval_ms;      /* Test pulse interval */
    uint32_t fault_reset_time_ms;         /* Fault reset time */
    bool enable_cross_monitoring;         /* Enable dual-channel cross-monitoring */
    bool enable_safe_state_enforcement;   /* Enable safe state enforcement */
    uint8_t safety_category;              /* Safety category (1-4) */
    char system_description[64];          /* System description */
} safety_system_config_t;

/* ============================================================================
 * Safety States and Events
 * ============================================================================ */

typedef enum {
    SAFETY_STATE_SAFE         = 0,    /* System in safe state */
    SAFETY_STATE_OPERATIONAL  = 1,    /* Normal operation */
    SAFETY_STATE_STOP         = 2,    /* Safety stop initiated */
    SAFETY_STATE_FAULT        = 3,    /* Safety fault detected */
    SAFETY_STATE_RESET        = 4,    /* Reset in progress */
    SAFETY_STATE_EMERGENCY    = 5,    /* Emergency stop */
} safety_state_t;

/* Safety Events */
typedef enum {
    SAFETY_EVENT_NONE         = 0,    /* No event */
    SAFETY_EVENT_EMERGENCY_STOP = 1,  /* Emergency stop activated */
    SAFETY_EVENT_SAFETY_EDGE  = 2,    /* Safety edge triggered */
    SAFETY_EVENT_LIGHT_CURTAIN = 3,   /* Light curtain breached */
    SAFETY_EVENT_DOOR_OPEN    = 4,    /* Door opened during operation */
    SAFETY_EVENT_FAULT_DETECTED = 5,  /* Safety fault detected */
    SAFETY_EVENT_WATCHDOG_TIMEOUT = 6, /* Watchdog timeout */
    SAFETY_EVENT_CROSS_MONITOR_FAIL = 7, /* Cross-monitoring failure */
    SAFETY_EVENT_POWER_LOSS   = 8,    /* Power supply fault */
    SAFETY_EVENT_RESET_REQUEST = 9,   /* Reset requested */
} safety_event_t;

/* ============================================================================
 * Safety Input/Output Data
 * ============================================================================ */

typedef struct {
    bool active;                          /* Input active state (agreed by both channels) */
    bool channel_a_state;                 /* Channel A optocoupler state */
    bool channel_b_state;                 /* Channel B optocoupler state */
    bool fault;                           /* Input fault detected */
    bool stuck_at;                        /* Stuck-at fault */
    bool cross_monitor_fail;              /* Cross-monitoring failure (A != B) */
    uint32_t activation_time;             /* Last activation timestamp */
    uint32_t deactivation_time;           /* Last deactivation timestamp */
    uint32_t activation_count;            /* Total activation count */
    uint8_t diagnostic_code;              /* Diagnostic information */
} safety_input_status_t;

typedef struct {
    bool energized;                       /* Output energized */
    bool fault;                           /* Output fault detected */
    bool feedback_mismatch;               /* Feedback mismatch */
    bool short_circuit;                   /* Short circuit detected */
    uint32_t energize_time;               /* Last energize timestamp */
    uint32_t deenergize_time;             /* Last deenergize timestamp */
    uint32_t cycle_count;                 /* Total cycle count */
    uint8_t diagnostic_code;              /* Diagnostic information */
} safety_output_status_t;

typedef struct {
    safety_state_t current_state;          /* Current safety state */
    safety_event_t last_event;             /* Last safety event */
    uint32_t event_timestamp;              /* Event timestamp */
    uint32_t state_change_time;            /* Last state change time */
    uint8_t fault_code;                    /* Active fault code */
    uint8_t diagnostic_status;             /* Diagnostic status */
    bool watchdog_active;                  /* Watchdog supervision active */
    bool safe_state_enforced;              /* Safe state enforced */
    uint32_t uptime_seconds;               /* System uptime */
    uint32_t fault_count;                  /* Total fault count */
} safety_system_status_t;

/* ============================================================================
 * Fault Codes
 * ============================================================================ */

#define SAFETY_FAULT_NONE              0x00
#define SAFETY_FAULT_INPUT_STUCK       0x01  /* Input stuck at high/low */
#define SAFETY_FAULT_CROSS_MONITOR     0x02  /* Dual-channel cross-monitoring failure */
#define SAFETY_FAULT_WATCHDOG          0x03  /* Watchdog timeout */
#define SAFETY_FAULT_OUTPUT_SHORT      0x04  /* Output short circuit */
#define SAFETY_FAULT_OUTPUT_FEEDBACK   0x05  /* Output feedback mismatch */
#define SAFETY_FAULT_POWER_SUPPLY      0x06  /* Power supply fault */
#define SAFETY_FAULT_CONFIGURATION     0x07  /* Configuration error */
#define SAFETY_FAULT_INTERNAL          0x08  /* Internal system fault */

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize safety I/O system
 *
 * @param config Pointer to safety system configuration
 * @return true if initialization successful, false otherwise
 */
bool safety_io_init(const safety_system_config_t *config);

/**
 * @brief Deinitialize safety I/O system
 *
 * @return true if deinitialization successful, false otherwise
 */
bool safety_io_deinit(void);

/**
 * @brief Configure safety input channel
 *
 * @param channel Input channel number (0-5)
 * @param config Pointer to input configuration (includes both GPIO pins for dual-channel)
 * @return true if configuration successful, false otherwise
 */
bool safety_io_configure_input(uint8_t channel, const safety_input_config_t *config);

/**
 * @brief Configure safety output channel
 *
 * @param channel Output channel number (0-1)
 * @param config Pointer to output configuration
 * @return true if configuration successful, false otherwise
 */
bool safety_io_configure_output(uint8_t channel, const safety_output_config_t *config);

/**
 * @brief Get safety input status
 *
 * @param channel Input channel number (0-5)
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool safety_io_get_input_status(uint8_t channel, safety_input_status_t *status);

/**
 * @brief Get safety output status
 *
 * @param channel Output channel number
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool safety_io_get_output_status(uint8_t channel, safety_output_status_t *status);

/**
 * @brief Get safety system status
 *
 * @param status Pointer to system status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool safety_io_get_system_status(safety_system_status_t *status);

/**
 * @brief Set safety output state
 *
 * @param channel Output channel number
 * @param state Desired output state (true = energized)
 * @return true if state set successfully, false otherwise
 */
bool safety_io_set_output(uint8_t channel, bool state);

/**
 * @brief Request safety system reset
 *
 * @return true if reset request accepted, false otherwise
 */
bool safety_io_request_reset(void);

/**
 * @brief Force safety stop
 *
 * @param reason Stop reason description
 * @return true if stop initiated successfully, false otherwise
 */
bool safety_io_force_stop(const char *reason);

/**
 * @brief Perform safety system diagnostics
 *
 * @return Diagnostic result code (0 = OK, non-zero = fault)
 */
uint16_t safety_io_run_diagnostics(void);

/**
 * @brief Test safety input channel (dual-channel consistency check)
 *
 * @param channel Input channel number (0-5)
 * @return true if test passed, false otherwise
 */
bool safety_io_test_input(uint8_t channel);

/**
 * @brief Test safety output channel
 *
 * @param channel Output channel number
 * @return true if test passed, false otherwise
 */
bool safety_io_test_output(uint8_t channel);

/**
 * @brief Clear safety fault
 *
 * @param fault_code Fault code to clear
 * @return true if fault cleared successfully, false otherwise
 */
bool safety_io_clear_fault(uint8_t fault_code);

/**
 * @brief Get safety fault description
 *
 * @param fault_code Fault code
 * @param buffer Pointer to buffer for description
 * @param buffer_size Size of buffer
 * @return true if description retrieved successfully, false otherwise
 */
bool safety_io_get_fault_description(uint8_t fault_code, char *buffer, uint16_t buffer_size);

/**
 * @brief Enable/disable safety supervision
 *
 * @param enable true to enable, false to disable
 * @return true if operation successful, false otherwise
 */
bool safety_io_enable_supervision(bool enable);

/**
 * @brief Check if system is in safe state
 *
 * @return true if system is safe, false otherwise
 */
bool safety_io_is_safe_state(void);

/**
 * @brief Get safety event log
 *
 * @param buffer Pointer to buffer for event log
 * @param buffer_size Size of buffer
 * @param num_entries Pointer to store number of entries retrieved
 * @return true if log retrieved successfully, false otherwise
 */
bool safety_io_get_event_log(char *buffer, uint16_t buffer_size, uint16_t *num_entries);

/**
 * @brief Set safety category
 *
 * @param category Safety category (1-4)
 * @return true if category set successfully, false otherwise
 */
bool safety_io_set_category(uint8_t category);

/**
 * @brief Perform safety function validation
 *
 * @param function_type Function type to validate
 * @return true if validation passed, false otherwise
 */
bool safety_io_validate_function(const char *function_type);

/**
 * @brief Enter safety mode (de-energize all outputs)
 *
 * @return true if safety mode entered successfully, false otherwise
 */
bool safety_io_enter_safety_mode(void);

/**
 * @brief Exit safety mode (restore normal operation)
 *
 * @return true if safety mode exited successfully, false otherwise
 */
bool safety_io_exit_safety_mode(void);

/**
 * @brief Get safety system statistics
 *
 * @param uptime Pointer to store system uptime (seconds)
 * @param fault_count Pointer to store total fault count
 * @param reset_count Pointer to store reset count
 * @return true if statistics retrieved successfully, false otherwise
 */
bool safety_io_get_statistics(uint32_t *uptime, uint32_t *fault_count, uint32_t *reset_count);

/**
 * @brief Register safety event callback
 *
 * @param callback Function pointer to event callback
 * @return true if callback registered successfully, false otherwise
 */
bool safety_io_register_event_callback(void (*callback)(safety_event_t event, void *context));

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_SAFETY_IO_H */
