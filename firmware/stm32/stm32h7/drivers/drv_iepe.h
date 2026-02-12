/**
 * @file drv_iepe.h
 * @brief IEPE Current Source Control Driver
 *
 * Driver for the IEPE (Integrated Electronics Piezo-Electric) constant
 * current excitation circuit. Each channel uses a Si2301CDS P-channel
 * MOSFET to switch a 4 mA constant current source (REF200AU) on or off.
 *
 * P-channel MOSFET gate logic:
 *   - Enable pin LOW  = gate pulled low, P-FET turns ON,
 *                        4 mA current source active
 *   - Enable pin HIGH = gate at VCC, P-FET turns OFF,
 *                        current source disabled
 *
 * The 4 mA excitation current is sourced by the REF200AU precision
 * current reference. This constant current biases IEPE/ICP
 * accelerometers and provides the supply for the sensor's internal
 * charge amplifier.
 *
 * Pin assignments from pin_mapping_v150.h:
 *   - IEPE_EN_CH0 = PF4
 *   - IEPE_EN_CH1 = PF5
 *   - IEPE_EN_CH2 = PF6
 *   - IEPE_EN_CH3 = PF7
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_DRV_IEPE_H
#define ESOCORE_DRV_IEPE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/**
 * @brief IEPE current source handle
 *
 * One handle per channel. Tracks the enable GPIO pin and the
 * current on/off state of the 4 mA excitation source.
 */
typedef struct {
    uint8_t     channel;    /**< Channel index (0-3) */
    uint16_t    en_pin;     /**< GPIO enable pin (PIN_DEF encoded) */
    bool        enabled;    /**< true if 4 mA current source is active */
} iepe_handle_t;

/* ============================================================================
 * Public API
 * ============================================================================ */

/**
 * @brief Initialize an IEPE current source channel
 *
 * Configures the enable GPIO pin as an output and disables the
 * current source (pin HIGH = P-FET off).
 *
 * @param handle   Pointer to IEPE handle (caller-allocated)
 * @param channel  Channel index (0-3)
 * @param en_pin   GPIO enable pin (PIN_DEF encoded)
 * @return true on success, false on invalid parameters
 */
bool drv_iepe_init(iepe_handle_t *handle, uint8_t channel, uint16_t en_pin);

/**
 * @brief Enable the 4 mA IEPE current source
 *
 * Drives the enable pin LOW to turn on the P-channel MOSFET,
 * allowing the REF200AU to supply 4 mA to the sensor.
 *
 * @param handle  Pointer to initialized IEPE handle
 * @return true on success, false on invalid handle
 */
bool drv_iepe_enable(iepe_handle_t *handle);

/**
 * @brief Disable the IEPE current source
 *
 * Drives the enable pin HIGH to turn off the P-channel MOSFET,
 * cutting the 4 mA excitation current.
 *
 * @param handle  Pointer to initialized IEPE handle
 * @return true on success, false on invalid handle
 */
bool drv_iepe_disable(iepe_handle_t *handle);

/**
 * @brief Check if the IEPE current source is enabled
 *
 * @param handle  Pointer to initialized IEPE handle
 * @return true if 4 mA source is active, false if disabled or invalid handle
 */
bool drv_iepe_is_enabled(const iepe_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_DRV_IEPE_H */
