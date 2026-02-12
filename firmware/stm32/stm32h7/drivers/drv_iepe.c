/**
 * @file drv_iepe.c
 * @brief IEPE Current Source Control Driver Implementation
 *
 * GPIO-only driver for controlling the IEPE 4 mA constant current
 * excitation circuit. Uses Si2301CDS P-channel MOSFETs switched by
 * GPIO enable pins to gate the REF200AU current source output.
 *
 * Gate logic (P-channel MOSFET):
 *   - GPIO LOW  -> gate pulled low  -> P-FET ON  -> 4 mA flows to sensor
 *   - GPIO HIGH -> gate at VCC      -> P-FET OFF -> no current to sensor
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

#include "drv_iepe.h"
#include "../hal/hal_gpio.h"

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

bool drv_iepe_init(iepe_handle_t *handle, uint8_t channel, uint16_t en_pin)
{
    if (!handle || channel > 3) {
        return false;
    }

    handle->channel = channel;
    handle->en_pin  = en_pin;

    /* Configure enable pin as push-pull output */
    hal_gpio_init_output(en_pin);

    /* Default: disabled (HIGH = P-FET off, no current) */
    hal_gpio_set(en_pin);
    handle->enabled = false;

    return true;
}

bool drv_iepe_enable(iepe_handle_t *handle)
{
    if (!handle) {
        return false;
    }

    /* Drive LOW to turn on P-channel MOSFET, enabling 4 mA source */
    hal_gpio_reset(handle->en_pin);
    handle->enabled = true;

    return true;
}

bool drv_iepe_disable(iepe_handle_t *handle)
{
    if (!handle) {
        return false;
    }

    /* Drive HIGH to turn off P-channel MOSFET, disabling current */
    hal_gpio_set(handle->en_pin);
    handle->enabled = false;

    return true;
}

bool drv_iepe_is_enabled(const iepe_handle_t *handle)
{
    if (!handle) {
        return false;
    }

    return handle->enabled;
}
