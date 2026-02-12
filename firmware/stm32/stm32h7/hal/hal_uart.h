/**
 * @file hal_uart.h
 * @brief UART/USART Hardware Abstraction Layer for STM32H747
 *
 * Supports USART1 (ESP32), USART2 (sensor bus), USART3 (Modbus),
 * UART4 (PROFIBUS), UART5 (Interbus).
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_HAL_UART_H
#define ESOCORE_HAL_UART_H

#include <stdint.h>
#include <stdbool.h>
#include "../cmsis/stm32h747xx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UART_PARITY_NONE = 0,
    UART_PARITY_EVEN = 1,
    UART_PARITY_ODD  = 2,
} hal_uart_parity_t;

typedef enum {
    UART_STOP_1 = 0,
    UART_STOP_2 = 1,
} hal_uart_stop_bits_t;

typedef enum {
    UART_WORDLEN_8 = 0,
    UART_WORDLEN_9 = 1,
} hal_uart_wordlen_t;

typedef struct {
    USART_TypeDef       *instance;
    uint32_t            baudrate;
    hal_uart_wordlen_t  word_length;
    hal_uart_stop_bits_t stop_bits;
    hal_uart_parity_t   parity;
    bool                rx_enable;
    bool                tx_enable;
    uint16_t            de_re_pin;    /* RS-485 direction pin (0 = none) */
    bool                rs485_mode;   /* Enable hardware RS-485 DE control */
} hal_uart_config_t;

typedef struct {
    USART_TypeDef *instance;
    uint16_t       de_re_pin;
    bool           rs485_mode;
    bool           initialized;
} hal_uart_handle_t;

/* Callback type for UART RX interrupt */
typedef void (*hal_uart_rx_callback_t)(uint8_t data, void *context);

/**
 * @brief Initialize UART peripheral
 */
bool hal_uart_init(hal_uart_handle_t *handle, const hal_uart_config_t *config);

/**
 * @brief Deinitialize UART peripheral
 */
void hal_uart_deinit(hal_uart_handle_t *handle);

/**
 * @brief Transmit a buffer (blocking)
 */
bool hal_uart_transmit(hal_uart_handle_t *handle,
                       const uint8_t *data, uint16_t length,
                       uint32_t timeout_ms);

/**
 * @brief Receive a buffer (blocking)
 */
bool hal_uart_receive(hal_uart_handle_t *handle,
                      uint8_t *data, uint16_t length,
                      uint32_t timeout_ms);

/**
 * @brief Transmit a single byte
 */
bool hal_uart_transmit_byte(hal_uart_handle_t *handle, uint8_t data);

/**
 * @brief Receive a single byte (non-blocking)
 * @return true if byte received, false if nothing available
 */
bool hal_uart_receive_byte(hal_uart_handle_t *handle, uint8_t *data);

/**
 * @brief Set RS-485 transceiver to transmit mode
 */
void hal_uart_rs485_tx_enable(hal_uart_handle_t *handle);

/**
 * @brief Set RS-485 transceiver to receive mode
 */
void hal_uart_rs485_rx_enable(hal_uart_handle_t *handle);

/**
 * @brief Register RX interrupt callback
 */
void hal_uart_register_rx_callback(hal_uart_handle_t *handle,
                                   hal_uart_rx_callback_t callback,
                                   void *context);

/**
 * @brief Enable UART RX interrupt
 */
void hal_uart_enable_rx_interrupt(hal_uart_handle_t *handle);

/**
 * @brief Check if transmit register is empty
 */
bool hal_uart_tx_ready(hal_uart_handle_t *handle);

/**
 * @brief Check if receive data is available
 */
bool hal_uart_rx_available(hal_uart_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_HAL_UART_H */
