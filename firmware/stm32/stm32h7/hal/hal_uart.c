/**
 * @file hal_uart.c
 * @brief UART HAL Implementation for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "hal_uart.h"
#include "hal_gpio.h"
#include "../bsp/system_clock.h"

/* USART CR1 bits */
#define USART_CR1_UE      (1UL << 0)
#define USART_CR1_RE      (1UL << 2)
#define USART_CR1_TE      (1UL << 3)
#define USART_CR1_RXNEIE  (1UL << 5)
#define USART_CR1_PCE     (1UL << 10)
#define USART_CR1_PS      (1UL << 9)
#define USART_CR1_M0      (1UL << 12)
#define USART_CR1_OVER8   (1UL << 15)
#define USART_CR1_FIFOEN  (1UL << 29)

/* USART CR2 bits */
#define USART_CR2_STOP_1  (0UL << 12)
#define USART_CR2_STOP_2  (2UL << 12)

/* USART CR3 bits */
#define USART_CR3_DEM     (1UL << 14)  /* Driver enable mode */

/* USART ISR bits */
#define USART_ISR_TXE     (1UL << 7)
#define USART_ISR_RXNE    (1UL << 5)
#define USART_ISR_TC      (1UL << 6)
#define USART_ISR_ORE     (1UL << 3)
#define USART_ISR_FE      (1UL << 1)

/* USART ICR bits */
#define USART_ICR_ORECF   (1UL << 3)
#define USART_ICR_FECF    (1UL << 1)
#define USART_ICR_TCCF    (1UL << 6)

/**
 * @brief Get kernel clock for USART (depends on which bus it's on)
 */
static uint32_t get_uart_clock(USART_TypeDef *instance)
{
    if (instance == USART1) {
        return system_clock_get_pclk2();  /* APB2 */
    }
    /* USART2, USART3, UART4, UART5 are on APB1 */
    return system_clock_get_pclk1();
}

bool hal_uart_init(hal_uart_handle_t *handle, const hal_uart_config_t *config)
{
    if (!handle || !config || !config->instance) return false;

    USART_TypeDef *uart = config->instance;
    handle->instance = uart;
    handle->de_re_pin = config->de_re_pin;
    handle->rs485_mode = config->rs485_mode;

    /* Disable UART during configuration */
    uart->CR1 = 0;

    /* Configure baudrate */
    uint32_t clock = get_uart_clock(uart);
    uint32_t usartdiv = (clock + config->baudrate / 2) / config->baudrate;
    uart->BRR = (uint16_t)usartdiv;

    /* Configure CR2: stop bits */
    uint32_t cr2 = 0;
    if (config->stop_bits == UART_STOP_2) {
        cr2 |= USART_CR2_STOP_2;
    }
    uart->CR2 = cr2;

    /* Configure CR3: RS-485 driver enable if applicable */
    uint32_t cr3 = 0;
    if (config->rs485_mode) {
        cr3 |= USART_CR3_DEM;
    }
    uart->CR3 = cr3;

    /* Configure CR1: enable, word length, parity, TX/RX, FIFO */
    uint32_t cr1 = USART_CR1_FIFOEN;
    if (config->tx_enable) cr1 |= USART_CR1_TE;
    if (config->rx_enable) cr1 |= USART_CR1_RE;
    if (config->word_length == UART_WORDLEN_9) cr1 |= USART_CR1_M0;
    if (config->parity != UART_PARITY_NONE) {
        cr1 |= USART_CR1_PCE;
        if (config->parity == UART_PARITY_ODD) {
            cr1 |= USART_CR1_PS;
        }
    }

    /* Enable UART */
    cr1 |= USART_CR1_UE;
    uart->CR1 = cr1;

    /* Initialize RS-485 direction pin if not using hardware DE */
    if (config->de_re_pin != 0 && !config->rs485_mode) {
        hal_gpio_init_output(config->de_re_pin);
        hal_gpio_reset(config->de_re_pin);  /* Default: receive mode */
    }

    handle->initialized = true;
    return true;
}

void hal_uart_deinit(hal_uart_handle_t *handle)
{
    if (!handle || !handle->instance) return;
    handle->instance->CR1 &= ~USART_CR1_UE;
    handle->initialized = false;
}

bool hal_uart_transmit(hal_uart_handle_t *handle,
                       const uint8_t *data, uint16_t length,
                       uint32_t timeout_ms)
{
    if (!handle || !handle->initialized || !data) return false;

    USART_TypeDef *uart = handle->instance;
    uint32_t start = system_clock_get_tick();

    /* Set RS-485 to transmit mode */
    if (handle->de_re_pin != 0 && !handle->rs485_mode) {
        hal_gpio_set(handle->de_re_pin);
    }

    for (uint16_t i = 0; i < length; i++) {
        while (!(uart->ISR & USART_ISR_TXE)) {
            if ((system_clock_get_tick() - start) > timeout_ms) {
                if (handle->de_re_pin != 0) hal_gpio_reset(handle->de_re_pin);
                return false;
            }
        }
        uart->TDR = data[i];
    }

    /* Wait for transmission complete */
    while (!(uart->ISR & USART_ISR_TC)) {
        if ((system_clock_get_tick() - start) > timeout_ms) {
            if (handle->de_re_pin != 0) hal_gpio_reset(handle->de_re_pin);
            return false;
        }
    }
    uart->ICR = USART_ICR_TCCF;

    /* Return RS-485 to receive mode */
    if (handle->de_re_pin != 0 && !handle->rs485_mode) {
        hal_gpio_reset(handle->de_re_pin);
    }

    return true;
}

bool hal_uart_receive(hal_uart_handle_t *handle,
                      uint8_t *data, uint16_t length,
                      uint32_t timeout_ms)
{
    if (!handle || !handle->initialized || !data) return false;

    USART_TypeDef *uart = handle->instance;
    uint32_t start = system_clock_get_tick();

    for (uint16_t i = 0; i < length; i++) {
        while (!(uart->ISR & USART_ISR_RXNE)) {
            if ((system_clock_get_tick() - start) > timeout_ms) return false;
        }
        data[i] = (uint8_t)(uart->RDR & 0xFF);
    }

    /* Clear overrun if set */
    if (uart->ISR & USART_ISR_ORE) {
        uart->ICR = USART_ICR_ORECF;
    }

    return true;
}

bool hal_uart_transmit_byte(hal_uart_handle_t *handle, uint8_t data)
{
    return hal_uart_transmit(handle, &data, 1, 100);
}

bool hal_uart_receive_byte(hal_uart_handle_t *handle, uint8_t *data)
{
    if (!handle || !handle->initialized || !data) return false;
    if (!(handle->instance->ISR & USART_ISR_RXNE)) return false;
    *data = (uint8_t)(handle->instance->RDR & 0xFF);
    return true;
}

void hal_uart_rs485_tx_enable(hal_uart_handle_t *handle)
{
    if (handle && handle->de_re_pin != 0) {
        hal_gpio_set(handle->de_re_pin);
    }
}

void hal_uart_rs485_rx_enable(hal_uart_handle_t *handle)
{
    if (handle && handle->de_re_pin != 0) {
        hal_gpio_reset(handle->de_re_pin);
    }
}

/* Callback storage (per-instance -- simplified for main 5 UARTs) */
static hal_uart_rx_callback_t rx_callbacks[5] = {0};
static void *rx_callback_contexts[5] = {0};

static int get_uart_index(USART_TypeDef *inst)
{
    if (inst == USART1) return 0;
    if (inst == USART2) return 1;
    if (inst == USART3) return 2;
    if (inst == UART4)  return 3;
    if (inst == UART5)  return 4;
    return -1;
}

void hal_uart_register_rx_callback(hal_uart_handle_t *handle,
                                   hal_uart_rx_callback_t callback,
                                   void *context)
{
    if (!handle) return;
    int idx = get_uart_index(handle->instance);
    if (idx >= 0) {
        rx_callbacks[idx] = callback;
        rx_callback_contexts[idx] = context;
    }
}

void hal_uart_enable_rx_interrupt(hal_uart_handle_t *handle)
{
    if (!handle || !handle->instance) return;
    handle->instance->CR1 |= USART_CR1_RXNEIE;
}

bool hal_uart_tx_ready(hal_uart_handle_t *handle)
{
    if (!handle || !handle->instance) return false;
    return (handle->instance->ISR & USART_ISR_TXE) != 0;
}

bool hal_uart_rx_available(hal_uart_handle_t *handle)
{
    if (!handle || !handle->instance) return false;
    return (handle->instance->ISR & USART_ISR_RXNE) != 0;
}
