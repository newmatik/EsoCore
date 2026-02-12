/**
 * @file hal_spi.h
 * @brief SPI Hardware Abstraction Layer for STM32H747
 *
 * Supports SPI3 (ADS1274 ADC) and SPI4 (PGA280 amplifiers).
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_HAL_SPI_H
#define ESOCORE_HAL_SPI_H

#include <stdint.h>
#include <stdbool.h>
#include "../cmsis/stm32h747xx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SPI_MODE_0 = 0,  /* CPOL=0, CPHA=0 */
    SPI_MODE_1 = 1,  /* CPOL=0, CPHA=1 */
    SPI_MODE_2 = 2,  /* CPOL=1, CPHA=0 */
    SPI_MODE_3 = 3,  /* CPOL=1, CPHA=1 */
} hal_spi_mode_t;

typedef enum {
    SPI_DATA_8BIT   = 7,
    SPI_DATA_16BIT  = 15,
    SPI_DATA_24BIT  = 23,
    SPI_DATA_32BIT  = 31,
} hal_spi_data_size_t;

typedef struct {
    SPI_TypeDef         *instance;
    hal_spi_mode_t      mode;
    hal_spi_data_size_t data_size;
    uint32_t            prescaler;  /* Divider: 2, 4, 8, 16, 32, 64, 128, 256 */
    bool                msb_first;
    bool                software_cs;
} hal_spi_config_t;

typedef struct {
    SPI_TypeDef *instance;
    bool         initialized;
} hal_spi_handle_t;

/**
 * @brief Initialize SPI peripheral
 */
bool hal_spi_init(hal_spi_handle_t *handle, const hal_spi_config_t *config);

/**
 * @brief Deinitialize SPI peripheral
 */
void hal_spi_deinit(hal_spi_handle_t *handle);

/**
 * @brief Transmit and receive data (full duplex)
 */
bool hal_spi_transfer(hal_spi_handle_t *handle,
                      const uint8_t *tx_data, uint8_t *rx_data,
                      uint16_t length, uint32_t timeout_ms);

/**
 * @brief Transmit data only
 */
bool hal_spi_transmit(hal_spi_handle_t *handle,
                      const uint8_t *tx_data, uint16_t length,
                      uint32_t timeout_ms);

/**
 * @brief Receive data only
 */
bool hal_spi_receive(hal_spi_handle_t *handle,
                     uint8_t *rx_data, uint16_t length,
                     uint32_t timeout_ms);

/**
 * @brief Start DMA-based SPI receive (for ADC continuous capture)
 */
bool hal_spi_receive_dma(hal_spi_handle_t *handle,
                         uint8_t *buffer0, uint8_t *buffer1,
                         uint16_t length);

/**
 * @brief Stop DMA-based SPI receive
 */
void hal_spi_stop_dma(hal_spi_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_HAL_SPI_H */
