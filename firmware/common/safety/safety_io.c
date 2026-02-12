/**
 * @file safety_io.c
 * @brief Safety I/O System Implementation
 *
 * This file contains the implementation of the safety I/O system compliant with
 * EN ISO 13849-1 functional safety standard for machinery safety.
 *
 * The system manages 6 dual-channel safety digital inputs (12 optocouplers)
 * with cross-monitoring, plus safety relay and SSR outputs.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "safety_io.h"
#include <string.h>
#include <stdio.h>

/* v1.5.0.0: Include HAL and driver headers for actual hardware access */
#include "../../stm32/stm32h7/hal/hal_gpio.h"
#include "../../stm32/stm32h7/bsp/pin_mapping_v150.h"
#include "../../stm32/stm32h7/bsp/system_clock.h"
#include "../../stm32/stm32h7/drivers/drv_ssr.h"

/* v1.5.0.0 pin mapping tables for dual-channel safety inputs */
static const uint16_t safety_input_pins_a[SAFETY_INPUT_CHANNELS] = {
    SAFETY_IN_CH0_A, SAFETY_IN_CH1_A, SAFETY_IN_CH2_A,
    SAFETY_IN_CH3_A, SAFETY_IN_CH4_A, SAFETY_IN_CH5_A,
};
static const uint16_t safety_input_pins_b[SAFETY_INPUT_CHANNELS] = {
    SAFETY_IN_CH0_B, SAFETY_IN_CH1_B, SAFETY_IN_CH2_B,
    SAFETY_IN_CH3_B, SAFETY_IN_CH4_B, SAFETY_IN_CH5_B,
};

/* SSR handle for the AQY212EHAZ solid-state relay output */
static ssr_handle_t ssr_output;

/* ============================================================================
 * Private Data Structures
 * ============================================================================ */

static safety_system_config_t safety_config;
static safety_system_status_t safety_status;
static safety_input_status_t safety_inputs[SAFETY_INPUT_CHANNELS];
static safety_output_status_t safety_outputs[SAFETY_OUTPUT_CHANNELS];
static bool safety_initialized = false;

/* Safety event callback */
static void (*safety_event_callback)(safety_event_t event, void *context) = NULL;

/* Watchdog variables */
static uint32_t watchdog_counter = 0;
static uint32_t last_watchdog_feed = 0;

/* Test pulse variables */
static uint32_t last_test_pulse = 0;

/* ============================================================================
 * Hardware Abstraction Layer
 * ============================================================================ */

/**
 * @brief Initialize safety I/O hardware
 *
 * @return true if hardware initialization successful, false otherwise
 */
static bool safety_hw_init(void) {
    /* v1.5.0.0: Initialize all 12 safety input GPIOs (already configured in hal_init.c) */
    /* The GPIO pins are configured as inputs with pull-ups during BSP init.
     * Optocouplers pull the GPIO LOW when the field signal (24V) is present. */

    /* Initialize safety relay output (G7SA-2A2B-DC24 via DRV110APWR) */
    /* GPIO already configured in hal_init.c, just ensure it's OFF */
    hal_gpio_reset(SAFETY_RELAY_CTRL);

    /* Initialize SSR output (AQY212EHAZ) */
    drv_ssr_init(&ssr_output, SSR_CTRL);

    return true;
}

/**
 * @brief Read safety input channel (single optocoupler)
 *
 * @param channel Input channel number (0-5)
 * @param read_channel_b true to read Channel B, false for Channel A
 * @return true if input is active, false otherwise
 */
static bool safety_hw_read_input(uint8_t channel, bool read_channel_b) {
    if (channel >= SAFETY_INPUT_CHANNELS) return false;

    /* Read the appropriate optocoupler GPIO for this dual-channel input.
     * Optocoupler output is ACTIVE LOW: GPIO reads LOW when 24V field signal
     * is present (optocoupler is conducting). */
    uint16_t pin = read_channel_b ? safety_input_pins_b[channel]
                                  : safety_input_pins_a[channel];
    bool gpio_state = hal_gpio_read(pin);

    /* Invert: LOW = active (signal present), HIGH = inactive */
    return !gpio_state;
}

/**
 * @brief Write safety output channel
 *
 * @param channel Output channel number
 * @param state Output state (true = energized)
 * @return true if write successful, false otherwise
 */
static bool safety_hw_write_output(uint8_t channel, bool state) {
    if (channel == SAFETY_OUTPUT_RELAY) {
        /* Safety relay (G7SA-2A2B-DC24) via DRV110APWR driver */
        hal_gpio_write(SAFETY_RELAY_CTRL, state);
        return true;
    } else if (channel == SAFETY_OUTPUT_SSR) {
        /* Solid-state relay (AQY212EHAZ) */
        if (state) {
            drv_ssr_on(&ssr_output);
        } else {
            drv_ssr_off(&ssr_output);
        }
        return true;
    }
    return false;
}

/**
 * @brief Read safety output feedback
 *
 * @param channel Output channel number
 * @return true if feedback indicates correct state, false otherwise
 */
static bool safety_hw_read_output_feedback(uint8_t channel) {
    if (channel == SAFETY_OUTPUT_RELAY) {
        /* Read relay feedback contacts:
         * NO contacts should be closed (HIGH) when relay is energized
         * NC contacts should be open (LOW) when relay is energized */
        bool no1 = hal_gpio_read(SAFETY_RELAY_FB_NO1);
        bool no2 = hal_gpio_read(SAFETY_RELAY_FB_NO2);
        bool nc1 = hal_gpio_read(SAFETY_RELAY_FB_NC1);
        bool nc2 = hal_gpio_read(SAFETY_RELAY_FB_NC2);

        bool relay_on = safety_outputs[channel].energized;
        if (relay_on) {
            /* Expect NO=HIGH, NC=LOW */
            return (no1 && no2 && !nc1 && !nc2);
        } else {
            /* Expect NO=LOW, NC=HIGH */
            return (!no1 && !no2 && nc1 && nc2);
        }
    } else if (channel == SAFETY_OUTPUT_SSR) {
        /* AQY212EHAZ has no feedback pin; assume correct */
        return true;
    }
    return false;
}

/**
 * @brief Feed safety watchdog
 *
 * @return true if watchdog fed successfully, false otherwise
 */
static bool safety_hw_feed_watchdog(void) {
    /* TODO: Implement watchdog feeding */
    return true;
}

/**
 * @brief Force emergency stop
 *
 * @return true if emergency stop initiated, false otherwise
 */
static bool safety_hw_emergency_stop(void) {
    /* De-energize safety relay */
    hal_gpio_reset(SAFETY_RELAY_CTRL);
    /* De-energize SSR */
    drv_ssr_off(&ssr_output);
    return true;
}

/* ============================================================================
 * Safety Monitoring Functions
 * ============================================================================ */

/**
 * @brief Update safety input status with dual-channel cross-monitoring
 *
 * Reads both optocouplers (Channel A and Channel B) for the given input
 * channel and performs cross-monitoring. If the two channels disagree,
 * a cross-monitor fault is raised.
 *
 * @param channel Input channel number (0-5)
 */
static void safety_update_input_status(uint8_t channel) {
    if (channel >= SAFETY_INPUT_CHANNELS) {
        return;
    }

    safety_input_status_t *input = &safety_inputs[channel];

    /* Read both channels of the dual-channel pair */
    bool channel_a = safety_hw_read_input(channel, false);
    bool channel_b = safety_hw_read_input(channel, true);

    input->channel_a_state = channel_a;
    input->channel_b_state = channel_b;

    /* Cross-monitoring: both channels must agree */
    if (safety_config.enable_cross_monitoring) {
        if (channel_a != channel_b) {
            input->cross_monitor_fail = true;
            safety_status.fault_code = SAFETY_FAULT_CROSS_MONITOR;
        } else {
            input->cross_monitor_fail = false;
        }
    }

    /* Determine agreed input state (only valid if channels agree) */
    bool current_state = channel_a && channel_b;

    /* Update timing */
    uint32_t current_time = system_clock_get_tick();

    if (current_state != input->active) {
        if (current_state) {
            input->activation_time = current_time;
            input->activation_count++;
        } else {
            input->deactivation_time = current_time;
        }
        input->active = current_state;
    }

    /* Update fault status */
    input->fault = input->cross_monitor_fail || input->stuck_at;
}

/**
 * @brief Update safety output status
 *
 * @param channel Output channel number
 */
static void safety_update_output_status(uint8_t channel) {
    if (channel >= SAFETY_OUTPUT_CHANNELS) {
        return;
    }

    safety_output_status_t *output = &safety_outputs[channel];
    bool feedback_state = safety_hw_read_output_feedback(channel);

    /* Check feedback mismatch */
    output->feedback_mismatch = (output->energized != feedback_state);

    /* Update fault status */
    output->fault = output->feedback_mismatch || output->short_circuit;

    /* Update timing */
    uint32_t current_time = system_clock_get_tick();

    if (output->energized) {
        output->energize_time = current_time;
    } else {
        output->deenergize_time = current_time;
    }

    /* Count cycles */
    if (output->energized && !output->short_circuit) {
        output->cycle_count++;
    }
}

/**
 * @brief Check safety system faults
 */
static void safety_check_system_faults(void) {
    /* Reset fault code */
    safety_status.fault_code = SAFETY_FAULT_NONE;

    /* Check input faults (dual-channel cross-monitoring) */
    for (uint8_t i = 0; i < SAFETY_INPUT_CHANNELS; i++) {
        if (safety_inputs[i].fault) {
            if (safety_inputs[i].cross_monitor_fail) {
                safety_status.fault_code = SAFETY_FAULT_CROSS_MONITOR;
            } else {
                safety_status.fault_code = SAFETY_FAULT_INPUT_STUCK;
            }
            break;
        }
    }

    /* Check output faults */
    for (uint8_t i = 0; i < SAFETY_OUTPUT_CHANNELS; i++) {
        if (safety_outputs[i].fault) {
            if (safety_outputs[i].short_circuit) {
                safety_status.fault_code = SAFETY_FAULT_OUTPUT_SHORT;
            } else if (safety_outputs[i].feedback_mismatch) {
                safety_status.fault_code = SAFETY_FAULT_OUTPUT_FEEDBACK;
            }
            break;
        }
    }

    /* Check watchdog */
    uint32_t current_time = system_clock_get_tick();
    if (current_time - last_watchdog_feed > safety_config.watchdog_timeout_ms) {
        safety_status.fault_code = SAFETY_FAULT_WATCHDOG;
    }
}

/**
 * @brief Perform safety test pulse
 *
 * Periodically tests all safety inputs (dual-channel consistency) and
 * safety outputs (feedback verification).
 */
static void safety_perform_test_pulse(void) {
    uint32_t current_time = system_clock_get_tick();

    if (current_time - last_test_pulse >= SAFETY_TEST_PULSE_INTERVAL) {
        /* Test safety inputs (dual-channel cross-monitoring) */
        for (uint8_t i = 0; i < SAFETY_INPUT_CHANNELS; i++) {
            safety_update_input_status(i);
        }

        /* Test safety outputs */
        for (uint8_t i = 0; i < SAFETY_OUTPUT_CHANNELS; i++) {
            safety_update_output_status(i);
        }

        /* Feed watchdog */
        safety_hw_feed_watchdog();
        last_watchdog_feed = current_time;

        last_test_pulse = current_time;
    }
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

/**
 * @brief Initialize safety I/O system
 */
bool safety_io_init(const safety_system_config_t *config) {
    if (!config || safety_initialized) {
        return false;
    }

    memcpy(&safety_config, config, sizeof(safety_system_config_t));

    /* Initialize hardware */
    if (!safety_hw_init()) {
        return false;
    }

    /* Initialize safety status */
    memset(&safety_status, 0, sizeof(safety_system_status_t));
    safety_status.current_state = SAFETY_STATE_SAFE;
    safety_status.safe_state_enforced = true;

    /* Initialize input/output arrays */
    memset(safety_inputs, 0, sizeof(safety_inputs));
    memset(safety_outputs, 0, sizeof(safety_outputs));

    /* Initialize timing */
    last_watchdog_feed = system_clock_get_tick();
    last_test_pulse = system_clock_get_tick();

    safety_initialized = true;
    return true;
}

/**
 * @brief Deinitialize safety I/O system
 */
bool safety_io_deinit(void) {
    if (!safety_initialized) {
        return false;
    }

    /* Force all outputs to safe state */
    for (uint8_t i = 0; i < SAFETY_OUTPUT_CHANNELS; i++) {
        safety_hw_write_output(i, false);
    }

    safety_initialized = false;
    return true;
}

/**
 * @brief Configure safety input channel
 */
bool safety_io_configure_input(uint8_t channel, const safety_input_config_t *config) {
    if (!safety_initialized || channel >= SAFETY_INPUT_CHANNELS || !config) {
        return false;
    }

    /* Store configuration (would be used by hardware layer) */
    /* TODO: Apply dual-channel GPIO configuration to hardware */

    return true;
}

/**
 * @brief Configure safety output channel
 */
bool safety_io_configure_output(uint8_t channel, const safety_output_config_t *config) {
    if (!safety_initialized || channel >= SAFETY_OUTPUT_CHANNELS || !config) {
        return false;
    }

    /* Store configuration (would be used by hardware layer) */
    /* TODO: Apply configuration to hardware */

    return true;
}

/**
 * @brief Get safety input status
 */
bool safety_io_get_input_status(uint8_t channel, safety_input_status_t *status) {
    if (!safety_initialized || channel >= SAFETY_INPUT_CHANNELS || !status) {
        return false;
    }

    memcpy(status, &safety_inputs[channel], sizeof(safety_input_status_t));
    return true;
}

/**
 * @brief Get safety output status
 */
bool safety_io_get_output_status(uint8_t channel, safety_output_status_t *status) {
    if (!safety_initialized || channel >= SAFETY_OUTPUT_CHANNELS || !status) {
        return false;
    }

    memcpy(status, &safety_outputs[channel], sizeof(safety_output_status_t));
    return true;
}

/**
 * @brief Get safety system status
 */
bool safety_io_get_system_status(safety_system_status_t *status) {
    if (!safety_initialized || !status) {
        return false;
    }

    /* Update system status */
    safety_check_system_faults();

    memcpy(status, &safety_status, sizeof(safety_system_status_t));
    return true;
}

/**
 * @brief Set safety output state
 */
bool safety_io_set_output(uint8_t channel, bool state) {
    if (!safety_initialized || channel >= SAFETY_OUTPUT_CHANNELS) {
        return false;
    }

    /* Check if system is in safe state */
    if (!safety_status.safe_state_enforced) {
        return false;
    }

    /* Set output state */
    if (!safety_hw_write_output(channel, state)) {
        return false;
    }

    /* Update output status */
    safety_outputs[channel].energized = state;
    safety_update_output_status(channel);

    return true;
}

/**
 * @brief Request safety system reset
 */
bool safety_io_request_reset(void) {
    if (!safety_initialized) {
        return false;
    }

    /* Check if reset is allowed */
    if (safety_status.fault_code != SAFETY_FAULT_NONE) {
        return false;
    }

    /* Reset system state */
    safety_status.current_state = SAFETY_STATE_RESET;
    safety_status.safe_state_enforced = true;

    /* Reset fault code */
    safety_status.fault_code = SAFETY_FAULT_NONE;

    return true;
}

/**
 * @brief Force safety stop
 */
bool safety_io_force_stop(const char *reason) {
    if (!safety_initialized) {
        return false;
    }

    /* Force emergency stop */
    if (!safety_hw_emergency_stop()) {
        return false;
    }

    /* Update system state */
    safety_status.current_state = SAFETY_STATE_EMERGENCY;
    safety_status.safe_state_enforced = false;
    safety_status.last_event = SAFETY_EVENT_EMERGENCY_STOP;

    /* De-energize all outputs */
    for (uint8_t i = 0; i < SAFETY_OUTPUT_CHANNELS; i++) {
        safety_hw_write_output(i, false);
        safety_outputs[i].energized = false;
    }

    /* Log event if callback is registered */
    if (safety_event_callback) {
        safety_event_callback(SAFETY_EVENT_EMERGENCY_STOP, (void *)reason);
    }

    return true;
}

/**
 * @brief Perform safety system diagnostics
 */
uint16_t safety_io_run_diagnostics(void) {
    uint16_t diagnostic_result = 0;

    if (!safety_initialized) {
        diagnostic_result |= (1 << 0); /* Not initialized */
        return diagnostic_result;
    }

    /* Test safety inputs (dual-channel cross-monitoring) */
    for (uint8_t i = 0; i < SAFETY_INPUT_CHANNELS; i++) {
        safety_update_input_status(i);
        if (safety_inputs[i].fault) {
            diagnostic_result |= (1 << (i + 1));
        }
    }

    /* Test safety outputs */
    for (uint8_t i = 0; i < SAFETY_OUTPUT_CHANNELS; i++) {
        safety_update_output_status(i);
        if (safety_outputs[i].fault) {
            diagnostic_result |= (1 << (i + 1 + SAFETY_INPUT_CHANNELS));
        }
    }

    /* Check system faults */
    safety_check_system_faults();
    if (safety_status.fault_code != SAFETY_FAULT_NONE) {
        diagnostic_result |= (1 << 15); /* System fault */
    }

    return diagnostic_result;
}

/**
 * @brief Test safety input channel (dual-channel consistency)
 */
bool safety_io_test_input(uint8_t channel) {
    if (!safety_initialized || channel >= SAFETY_INPUT_CHANNELS) {
        return false;
    }

    safety_update_input_status(channel);
    return !safety_inputs[channel].fault;
}

/**
 * @brief Test safety output channel
 */
bool safety_io_test_output(uint8_t channel) {
    if (!safety_initialized || channel >= SAFETY_OUTPUT_CHANNELS) {
        return false;
    }

    safety_update_output_status(channel);
    return !safety_outputs[channel].fault;
}

/**
 * @brief Clear safety fault
 */
bool safety_io_clear_fault(uint8_t fault_code) {
    if (!safety_initialized) {
        return false;
    }

    /* Only allow clearing certain faults */
    if (fault_code == SAFETY_FAULT_NONE ||
        fault_code == SAFETY_FAULT_INPUT_STUCK ||
        fault_code == SAFETY_FAULT_OUTPUT_FEEDBACK) {
        safety_status.fault_code = SAFETY_FAULT_NONE;
        return true;
    }

    return false;
}

/**
 * @brief Get safety fault description
 */
bool safety_io_get_fault_description(uint8_t fault_code, char *buffer, uint16_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return false;
    }

    const char *description;

    switch (fault_code) {
        case SAFETY_FAULT_NONE:
            description = "No fault";
            break;
        case SAFETY_FAULT_INPUT_STUCK:
            description = "Safety input stuck at high/low";
            break;
        case SAFETY_FAULT_CROSS_MONITOR:
            description = "Dual-channel cross-monitoring failure";
            break;
        case SAFETY_FAULT_WATCHDOG:
            description = "Watchdog timeout";
            break;
        case SAFETY_FAULT_OUTPUT_SHORT:
            description = "Safety output short circuit";
            break;
        case SAFETY_FAULT_OUTPUT_FEEDBACK:
            description = "Safety output feedback mismatch";
            break;
        case SAFETY_FAULT_POWER_SUPPLY:
            description = "Power supply fault";
            break;
        case SAFETY_FAULT_CONFIGURATION:
            description = "Configuration error";
            break;
        case SAFETY_FAULT_INTERNAL:
            description = "Internal system fault";
            break;
        default:
            description = "Unknown fault";
            break;
    }

    uint16_t len = strlen(description);
    if (len >= buffer_size) {
        return false;
    }

    strcpy(buffer, description);
    return true;
}

/**
 * @brief Enable/disable safety supervision
 */
bool safety_io_enable_supervision(bool enable) {
    if (!safety_initialized) {
        return false;
    }

    /* TODO: Enable/disable safety supervision */

    return true;
}

/**
 * @brief Check if system is in safe state
 */
bool safety_io_is_safe_state(void) {
    if (!safety_initialized) {
        return false;
    }

    return safety_status.safe_state_enforced &&
           safety_status.fault_code == SAFETY_FAULT_NONE;
}

/**
 * @brief Get safety event log
 */
bool safety_io_get_event_log(char *buffer, uint16_t buffer_size, uint16_t *num_entries) {
    /* TODO: Implement event log retrieval */
    return false;
}

/**
 * @brief Set safety category
 */
bool safety_io_set_category(uint8_t category) {
    if (!safety_initialized || category < 1 || category > 4) {
        return false;
    }

    safety_config.safety_category = category;
    return true;
}

/**
 * @brief Perform safety function validation
 */
bool safety_io_validate_function(const char *function_type) {
    /* TODO: Implement function validation */
    /* This would typically involve:
     * - Testing specific safety functions
     * - Verifying SIL/PL compliance
     * - Checking response times (<10ms requirement)
     * - Verifying dual-channel consistency
     */
    return true;
}

/**
 * @brief Enter safety mode (de-energize all outputs)
 */
bool safety_io_enter_safety_mode(void) {
    if (!safety_initialized) {
        return false;
    }

    /* De-energize all outputs */
    for (uint8_t i = 0; i < SAFETY_OUTPUT_CHANNELS; i++) {
        safety_hw_write_output(i, false);
        safety_outputs[i].energized = false;
    }

    /* Update system state */
    safety_status.current_state = SAFETY_STATE_SAFE;
    safety_status.safe_state_enforced = true;

    return true;
}

/**
 * @brief Exit safety mode (restore normal operation)
 */
bool safety_io_exit_safety_mode(void) {
    if (!safety_initialized) {
        return false;
    }

    /* Check if exit is allowed */
    if (safety_status.fault_code != SAFETY_FAULT_NONE) {
        return false;
    }

    /* Update system state */
    safety_status.current_state = SAFETY_STATE_OPERATIONAL;
    safety_status.safe_state_enforced = true;

    return true;
}

/**
 * @brief Get safety system statistics
 */
bool safety_io_get_statistics(uint32_t *uptime, uint32_t *fault_count, uint32_t *reset_count) {
    if (!safety_initialized) {
        return false;
    }

    if (uptime) {
        *uptime = safety_status.uptime_seconds;
    }
    if (fault_count) {
        *fault_count = safety_status.fault_count;
    }
    if (reset_count) {
        *reset_count = 0; /* TODO: Track reset count */
    }

    return true;
}

/**
 * @brief Register safety event callback
 */
bool safety_io_register_event_callback(void (*callback)(safety_event_t event, void *context)) {
    if (!safety_initialized) {
        return false;
    }

    safety_event_callback = callback;
    return true;
}
