/**
 * @file drv_ssr.h
 * @brief Solid-State Relay (SSR) driver for AQY212EHAZ PhotoMOS relay
 *
 * The AQY212EHAZ is a Panasonic PhotoMOS relay. Internally, a small LED
 * optically drives a photovoltaic cell which gates an output MOSFET pair.
 * Driving the control pin HIGH turns on the LED, which closes the output
 * MOSFET switch. Driving it LOW opens the switch.
 *
 * Electrical characteristics:
 *   - Max load voltage:  60 V (AC/DC)
 *   - Max load current:  500 mA
 *   - On-resistance:     ~1.5 Ohm typical
 *   - Turn-on time:      ~0.5 ms typical
 *   - Turn-off time:     ~0.1 ms typical
 *
 * Hardware notes:
 *   - The current-limiting resistor for the input LED is placed on the
 *     PCB; no software current control is needed.
 *   - There is no feedback or fault-detection pin on this part. The
 *     driver tracks state in software only (is_on flag).
 *   - The control pin is active-high: GPIO HIGH = relay ON.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_DRV_SSR_H
#define ESOCORE_DRV_SSR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SSR driver handle
 *
 * Holds the GPIO control pin definition and the current software-tracked
 * relay state. Since the AQY212EHAZ has no readback pin, the is_on field
 * is maintained by the driver and assumed to match hardware state.
 */
typedef struct {
    uint16_t ctrl_pin;  /**< PIN_DEF encoded GPIO pin driving the SSR input LED */
    bool     is_on;     /**< Software-tracked relay state (true = closed / conducting) */
} ssr_handle_t;

/**
 * @brief Initialize the SSR driver
 *
 * Configures the control pin as a push-pull output and sets the relay
 * to the OFF (open) state.
 *
 * @param handle  Pointer to an uninitialized SSR handle
 * @param ctrl_pin PIN_DEF encoded GPIO pin connected to the SSR input
 */
void drv_ssr_init(ssr_handle_t *handle, uint16_t ctrl_pin);

/**
 * @brief Turn the relay ON (close the output switch)
 *
 * Drives the control pin HIGH, turning on the internal LED and closing
 * the output MOSFET pair.
 *
 * @param handle Pointer to an initialized SSR handle
 */
void drv_ssr_on(ssr_handle_t *handle);

/**
 * @brief Turn the relay OFF (open the output switch)
 *
 * Drives the control pin LOW, turning off the internal LED and opening
 * the output MOSFET pair.
 *
 * @param handle Pointer to an initialized SSR handle
 */
void drv_ssr_off(ssr_handle_t *handle);

/**
 * @brief Check whether the relay is currently ON
 *
 * Returns the software-tracked state. No hardware readback is available
 * on the AQY212EHAZ.
 *
 * @param handle Pointer to an initialized SSR handle
 * @return true if relay is ON (conducting), false if OFF (open)
 */
bool drv_ssr_is_on(const ssr_handle_t *handle);

/**
 * @brief Toggle the relay state
 *
 * If the relay is ON it will be turned OFF, and vice versa.
 *
 * @param handle Pointer to an initialized SSR handle
 */
void drv_ssr_toggle(ssr_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_DRV_SSR_H */
