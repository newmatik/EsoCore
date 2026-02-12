/**
 * @file drv_tmux1101.c
 * @brief TMUX1101DCR SPDT Analog Switch Driver Implementation
 *
 * GPIO-only driver for the TMUX1101 analog switch. Each switch has a
 * single select pin that determines the signal routing path:
 *   - SEL LOW  = IEPE path (signal routed through AC coupling capacitor)
 *   - SEL HIGH = DC path   (direct signal bypass)
 *
 * Pin assignments from pin_mapping_v150.h:
 *   - TMUX_SEL_CH0 = PF0
 *   - TMUX_SEL_CH1 = PF1
 *   - TMUX_SEL_CH2 = PF2
 *   - TMUX_SEL_CH3 = PF3
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "drv_tmux1101.h"
#include "../hal/hal_gpio.h"

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

bool drv_tmux1101_init(tmux1101_handle_t *handle,
                       uint8_t channel, uint16_t sel_pin)
{
    if (!handle || channel > 3) {
        return false;
    }

    handle->channel = channel;
    handle->sel_pin = sel_pin;

    /* Configure select pin as push-pull output */
    hal_gpio_init_output(sel_pin);

    /* Default to DC mode (SEL HIGH = direct path, bypass AC coupling) */
    hal_gpio_set(sel_pin);
    handle->current_mode = TMUX1101_MODE_DC;

    return true;
}

bool drv_tmux1101_set_mode(tmux1101_handle_t *handle, uint8_t mode)
{
    if (!handle) {
        return false;
    }

    if (mode != TMUX1101_MODE_IEPE && mode != TMUX1101_MODE_DC) {
        return false;
    }

    if (mode == TMUX1101_MODE_IEPE) {
        /* IEPE mode: SEL LOW routes through AC coupling capacitor */
        hal_gpio_reset(handle->sel_pin);
    } else {
        /* DC mode: SEL HIGH for direct signal path */
        hal_gpio_set(handle->sel_pin);
    }

    handle->current_mode = mode;

    return true;
}

uint8_t drv_tmux1101_get_mode(const tmux1101_handle_t *handle)
{
    if (!handle) {
        return TMUX1101_MODE_DC;  /* Safe default */
    }

    return handle->current_mode;
}
