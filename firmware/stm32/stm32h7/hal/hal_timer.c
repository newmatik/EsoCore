/**
 * @file hal_timer.c
 * @brief Timer HAL Implementation for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "hal_timer.h"
#include "../bsp/system_clock.h"

/* TIM register bits */
#define TIM_CR1_CEN     (1UL << 0)
#define TIM_CR1_ARPE    (1UL << 7)
#define TIM_DIER_UIE    (1UL << 0)
#define TIM_SR_UIF      (1UL << 0)
#define TIM_EGR_UG      (1UL << 0)

/**
 * @brief Get timer clock frequency (APB1 or APB2 timers run at 2x APBx when APB divider > 1)
 */
static uint32_t get_timer_clock(TIM_TypeDef *tim)
{
    /* TIM1, TIM15-17 are on APB2; all others on APB1 */
    /* When APBx prescaler is != 1, timer clock = 2 * APBx clock */
    if (tim == TIM1) {
        return system_clock_get_pclk2() * 2;  /* 240 MHz */
    }
    return system_clock_get_pclk1() * 2;  /* 240 MHz */
}

bool hal_timer_init(hal_timer_handle_t *handle, const hal_timer_config_t *config)
{
    if (!handle || !config || !config->instance) return false;

    TIM_TypeDef *tim = config->instance;
    handle->instance = tim;

    /* Disable timer */
    tim->CR1 &= ~TIM_CR1_CEN;

    uint32_t timer_clk = get_timer_clock(tim);

    /* Calculate prescaler and period for desired microsecond period */
    /* Target: 1 MHz timer clock (1 us resolution) */
    uint32_t prescaler = (timer_clk / 1000000UL) - 1;
    uint32_t period = config->period_us - 1;

    tim->PSC = prescaler;
    tim->ARR = period;

    if (config->auto_reload) {
        tim->CR1 |= TIM_CR1_ARPE;
    }

    /* Generate update event to load prescaler */
    tim->EGR = TIM_EGR_UG;
    tim->SR &= ~TIM_SR_UIF;

    handle->initialized = true;
    return true;
}

void hal_timer_deinit(hal_timer_handle_t *handle)
{
    if (!handle || !handle->instance) return;
    handle->instance->CR1 &= ~TIM_CR1_CEN;
    handle->initialized = false;
}

void hal_timer_start(hal_timer_handle_t *handle)
{
    if (!handle || !handle->instance) return;

    /* Enable update interrupt if callback is registered */
    if (handle->callback) {
        handle->instance->DIER |= TIM_DIER_UIE;
    }

    handle->instance->CR1 |= TIM_CR1_CEN;
}

void hal_timer_stop(hal_timer_handle_t *handle)
{
    if (!handle || !handle->instance) return;
    handle->instance->CR1 &= ~TIM_CR1_CEN;
}

void hal_timer_set_period(hal_timer_handle_t *handle, uint32_t period_us)
{
    if (!handle || !handle->instance) return;
    handle->instance->ARR = period_us - 1;
}

uint32_t hal_timer_get_count(hal_timer_handle_t *handle)
{
    if (!handle || !handle->instance) return 0;
    return handle->instance->CNT;
}

void hal_timer_register_callback(hal_timer_handle_t *handle,
                                 hal_timer_callback_t callback, void *context)
{
    if (!handle) return;
    handle->callback = callback;
    handle->callback_ctx = context;
}

void hal_timer_delay_us(uint32_t us)
{
    /* Use TIM2 (32-bit counter) for microsecond delays */
    TIM_TypeDef *tim = TIM2;

    /* Configure TIM2 for 1 MHz counting if not already */
    uint32_t timer_clk = system_clock_get_pclk1() * 2;
    tim->PSC = (timer_clk / 1000000UL) - 1;
    tim->ARR = 0xFFFFFFFF;
    tim->EGR = TIM_EGR_UG;
    tim->SR &= ~TIM_SR_UIF;
    tim->CR1 |= TIM_CR1_CEN;

    uint32_t start = tim->CNT;
    while ((tim->CNT - start) < us) {
        /* Busy wait */
    }
}
