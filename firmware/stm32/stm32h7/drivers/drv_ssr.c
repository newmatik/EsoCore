/**
 * @file drv_ssr.c
 * @brief Solid-State Relay (SSR) driver implementation for AQY212EHAZ
 *
 * Simple GPIO-based driver. The relay has no feedback pin, so state is
 * tracked in software via the handle's is_on field.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "drv_ssr.h"
#include "../hal/hal_gpio.h"

/* ------------------------------------------------------------------ */
/*  Public API                                                        */
/* ------------------------------------------------------------------ */

void drv_ssr_init(ssr_handle_t *handle, uint16_t ctrl_pin)
{
    handle->ctrl_pin = ctrl_pin;
    handle->is_on    = false;

    /* Configure control pin as push-pull output (medium speed, no pull) */
    hal_gpio_init_output(ctrl_pin);

    /* Ensure relay starts in the OFF state */
    hal_gpio_reset(ctrl_pin);
}

void drv_ssr_on(ssr_handle_t *handle)
{
    hal_gpio_set(handle->ctrl_pin);
    handle->is_on = true;
}

void drv_ssr_off(ssr_handle_t *handle)
{
    hal_gpio_reset(handle->ctrl_pin);
    handle->is_on = false;
}

bool drv_ssr_is_on(const ssr_handle_t *handle)
{
    return handle->is_on;
}

void drv_ssr_toggle(ssr_handle_t *handle)
{
    if (handle->is_on) {
        drv_ssr_off(handle);
    } else {
        drv_ssr_on(handle);
    }
}
