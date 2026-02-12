/**
 * @file hal_timer.h
 * @brief Timer HAL for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_HAL_TIMER_H
#define ESOCORE_HAL_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include "../cmsis/stm32h747xx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*hal_timer_callback_t)(void *context);

typedef struct {
    TIM_TypeDef *instance;
    uint32_t     period_us;     /* Timer period in microseconds */
    bool         auto_reload;   /* Enable auto-reload */
} hal_timer_config_t;

typedef struct {
    TIM_TypeDef         *instance;
    hal_timer_callback_t callback;
    void                *callback_ctx;
    bool                 initialized;
} hal_timer_handle_t;

bool hal_timer_init(hal_timer_handle_t *handle, const hal_timer_config_t *config);
void hal_timer_deinit(hal_timer_handle_t *handle);
void hal_timer_start(hal_timer_handle_t *handle);
void hal_timer_stop(hal_timer_handle_t *handle);
void hal_timer_set_period(hal_timer_handle_t *handle, uint32_t period_us);
uint32_t hal_timer_get_count(hal_timer_handle_t *handle);
void hal_timer_register_callback(hal_timer_handle_t *handle,
                                 hal_timer_callback_t callback, void *context);

/** Microsecond delay using a hardware timer (TIM2, 32-bit) */
void hal_timer_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_HAL_TIMER_H */
