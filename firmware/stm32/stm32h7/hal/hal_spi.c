/**
 * @file hal_spi.c
 * @brief SPI HAL Implementation for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "hal_spi.h"
#include "../bsp/system_clock.h"

/* SPI CR1 bits */
#define SPI_CR1_SPE     (1UL << 0)
#define SPI_CR1_MASRX   (1UL << 8)
#define SPI_CR1_CSTART  (1UL << 9)
#define SPI_CR1_SSI     (1UL << 12)

/* SPI CFG1 bits */
#define SPI_CFG1_TXDMAEN (1UL << 15)
#define SPI_CFG1_RXDMAEN (1UL << 14)

/* SPI CFG2 bits */
#define SPI_CFG2_MASTER (1UL << 22)
#define SPI_CFG2_SSM    (1UL << 26)
#define SPI_CFG2_SSOM   (1UL << 30)
#define SPI_CFG2_SSOE   (1UL << 29)
#define SPI_CFG2_CPOL   (1UL << 25)
#define SPI_CFG2_CPHA   (1UL << 24)
#define SPI_CFG2_LSBFRST (1UL << 23)
#define SPI_CFG2_COMM_FULL   (0UL << 17)
#define SPI_CFG2_COMM_TXONLY (1UL << 17)
#define SPI_CFG2_COMM_RXONLY (2UL << 17)

/* SPI SR bits */
#define SPI_SR_TXP      (1UL << 1)
#define SPI_SR_RXP      (1UL << 0)
#define SPI_SR_EOT      (1UL << 3)
#define SPI_SR_TXTF     (1UL << 4)
#define SPI_SR_OVR      (1UL << 6)
#define SPI_SR_MODF     (1UL << 9)

/* Prescaler encoding: value = log2(divider) - 1, range 0-7 for /2 to /256 */
static uint8_t prescaler_to_mbr(uint32_t prescaler)
{
    switch (prescaler) {
        case 2:   return 0;
        case 4:   return 1;
        case 8:   return 2;
        case 16:  return 3;
        case 32:  return 4;
        case 64:  return 5;
        case 128: return 6;
        case 256: return 7;
        default:  return 3; /* Default /16 */
    }
}

bool hal_spi_init(hal_spi_handle_t *handle, const hal_spi_config_t *config)
{
    if (!handle || !config || !config->instance) return false;

    SPI_TypeDef *spi = config->instance;
    handle->instance = spi;

    /* Disable SPI before configuration */
    spi->CR1 &= ~SPI_CR1_SPE;

    /* CFG1: data size, prescaler */
    spi->CFG1 = ((uint32_t)config->data_size << 0) |               /* DSIZE */
                ((uint32_t)prescaler_to_mbr(config->prescaler) << 28);  /* MBR */

    /* CFG2: master mode, software CS, clock polarity/phase */
    uint32_t cfg2 = SPI_CFG2_MASTER | SPI_CFG2_SSM | SPI_CFG2_SSOM;
    if (config->mode & 0x02) cfg2 |= SPI_CFG2_CPOL;
    if (config->mode & 0x01) cfg2 |= SPI_CFG2_CPHA;
    if (!config->msb_first) cfg2 |= SPI_CFG2_LSBFRST;
    spi->CFG2 = cfg2;

    /* CR1: SSI must be set when SSM is used */
    spi->CR1 = SPI_CR1_SSI;

    /* CR2: set TSIZE to 0 for unlimited transfers */
    spi->CR2 = 0;

    /* Clear status flags */
    spi->IFCR = 0xFFFFFFFFUL;

    /* Enable SPI */
    spi->CR1 |= SPI_CR1_SPE;

    handle->initialized = true;
    return true;
}

void hal_spi_deinit(hal_spi_handle_t *handle)
{
    if (!handle || !handle->instance) return;
    handle->instance->CR1 &= ~SPI_CR1_SPE;
    handle->initialized = false;
}

bool hal_spi_transfer(hal_spi_handle_t *handle,
                      const uint8_t *tx_data, uint8_t *rx_data,
                      uint16_t length, uint32_t timeout_ms)
{
    if (!handle || !handle->initialized) return false;

    SPI_TypeDef *spi = handle->instance;
    uint32_t start = system_clock_get_tick();

    /* Start transfer */
    spi->CR1 |= SPI_CR1_CSTART;

    for (uint16_t i = 0; i < length; i++) {
        /* Wait for TX ready */
        while (!(spi->SR & SPI_SR_TXP)) {
            if ((system_clock_get_tick() - start) > timeout_ms) return false;
        }
        *(volatile uint8_t *)&spi->TXDR = tx_data ? tx_data[i] : 0xFF;

        /* Wait for RX ready */
        while (!(spi->SR & SPI_SR_RXP)) {
            if ((system_clock_get_tick() - start) > timeout_ms) return false;
        }
        uint8_t rx = *(volatile uint8_t *)&spi->RXDR;
        if (rx_data) rx_data[i] = rx;
    }

    /* Wait for end of transfer */
    while (!(spi->SR & SPI_SR_EOT)) {
        if ((system_clock_get_tick() - start) > timeout_ms) return false;
    }
    spi->IFCR = SPI_SR_EOT | SPI_SR_TXTF;

    return true;
}

bool hal_spi_transmit(hal_spi_handle_t *handle,
                      const uint8_t *tx_data, uint16_t length,
                      uint32_t timeout_ms)
{
    return hal_spi_transfer(handle, tx_data, NULL, length, timeout_ms);
}

bool hal_spi_receive(hal_spi_handle_t *handle,
                     uint8_t *rx_data, uint16_t length,
                     uint32_t timeout_ms)
{
    return hal_spi_transfer(handle, NULL, rx_data, length, timeout_ms);
}

bool hal_spi_receive_dma(hal_spi_handle_t *handle,
                         uint8_t *buffer0, uint8_t *buffer1,
                         uint16_t length)
{
    if (!handle || !handle->initialized) return false;

    SPI_TypeDef *spi = handle->instance;

    /* Configure DMA for double-buffer mode */
    /* Using DMA1 Stream 0 for SPI3 RX (configured via DMAMUX) */
    DMA_Stream_TypeDef *stream = DMA1_Stream0;

    /* Disable stream first */
    stream->CR &= ~(1UL << 0);
    while (stream->CR & (1UL << 0)) { /* Wait */ }

    /* Clear all interrupt flags */
    DMA1->LIFCR = 0x3FUL << 0;

    /* Configure stream */
    stream->PAR  = (uint32_t)&spi->RXDR;
    stream->M0AR = (uint32_t)buffer0;
    stream->M1AR = (uint32_t)buffer1;
    stream->NDTR = length;

    /* CR: channel, periph-to-mem, circular, double-buffer, byte size, high priority */
    stream->CR = (0UL << 25)  |  /* Channel (set via DMAMUX) */
                 (0UL << 6)   |  /* DIR: peripheral to memory */
                 (1UL << 8)   |  /* CIRC: circular mode */
                 (1UL << 18)  |  /* DBM: double-buffer mode */
                 (0UL << 11)  |  /* PSIZE: byte */
                 (0UL << 13)  |  /* MSIZE: byte */
                 (1UL << 10)  |  /* MINC: memory increment */
                 (2UL << 16)  |  /* PL: high priority */
                 (1UL << 4);     /* TCIE: transfer complete interrupt */

    /* Enable DMA in SPI */
    spi->CFG1 |= SPI_CFG1_RXDMAEN;

    /* Enable DMA stream */
    stream->CR |= (1UL << 0);

    /* Start SPI transfer */
    spi->CR1 |= SPI_CR1_CSTART;

    return true;
}

void hal_spi_stop_dma(hal_spi_handle_t *handle)
{
    if (!handle || !handle->instance) return;

    /* Disable DMA in SPI */
    handle->instance->CFG1 &= ~SPI_CFG1_RXDMAEN;

    /* Disable DMA stream */
    DMA1_Stream0->CR &= ~(1UL << 0);
}
