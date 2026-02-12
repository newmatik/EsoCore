/**
 * @file drv_tmux1101.h
 * @brief TMUX1101DCR SPDT Analog Switch Driver
 *
 * Driver for the Texas Instruments TMUX1101DCR single-pole double-throw
 * (SPDT) analog switch used to select between IEPE (AC-coupled) and DC
 * (direct) signal paths for each analog input channel.
 *
 * Signal routing:
 *   - IEPE mode (SEL LOW):  Signal routed through AC coupling capacitor,
 *                            removing DC bias from IEPE accelerometers
 *   - DC mode   (SEL HIGH): Signal routed directly, bypassing AC coupling
 *                            for DC-coupled voltage/current measurements
 *
 * Each channel has an independent TMUX1101 controlled by a dedicated
 * GPIO select pin defined in pin_mapping_v150.h.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_DRV_TMUX1101_H
#define ESOCORE_DRV_TMUX1101_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Mode Definitions
 * ============================================================================ */

/** IEPE mode: SEL pin LOW, routes signal through AC coupling capacitor */
#define TMUX1101_MODE_IEPE      0

/** DC mode: SEL pin HIGH, direct signal path (bypass AC coupling) */
#define TMUX1101_MODE_DC        1

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/**
 * @brief TMUX1101 switch handle
 *
 * One handle per analog channel. Tracks the GPIO select pin
 * and the current signal routing mode.
 */
typedef struct {
    uint8_t     channel;        /**< Channel index (0-3) */
    uint16_t    sel_pin;        /**< GPIO select pin (PIN_DEF encoded) */
    uint8_t     current_mode;   /**< Current mode (TMUX1101_MODE_*) */
} tmux1101_handle_t;

/* ============================================================================
 * Public API
 * ============================================================================ */

/**
 * @brief Initialize a TMUX1101 analog switch
 *
 * Configures the select GPIO pin as an output and sets the default
 * mode to DC (SEL HIGH, direct path).
 *
 * @param handle   Pointer to switch handle (caller-allocated)
 * @param channel  Channel index (0-3)
 * @param sel_pin  GPIO select pin (PIN_DEF encoded)
 * @return true on success, false on invalid parameters
 */
bool drv_tmux1101_init(tmux1101_handle_t *handle,
                       uint8_t channel, uint16_t sel_pin);

/**
 * @brief Set the signal routing mode
 *
 * @param handle  Pointer to initialized switch handle
 * @param mode    TMUX1101_MODE_IEPE or TMUX1101_MODE_DC
 * @return true on success, false on invalid parameters
 */
bool drv_tmux1101_set_mode(tmux1101_handle_t *handle, uint8_t mode);

/**
 * @brief Get the current signal routing mode
 *
 * @param handle  Pointer to initialized switch handle
 * @return Current mode (TMUX1101_MODE_IEPE or TMUX1101_MODE_DC)
 */
uint8_t drv_tmux1101_get_mode(const tmux1101_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_DRV_TMUX1101_H */
