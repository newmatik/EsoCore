/**
 * @file drv_pga280.c
 * @brief PGA280AIDR Programmable Gain Amplifier Driver Implementation
 *
 * Communicates with the PGA280 via SPI4 using 16-bit frames, MSB first.
 * Each PGA280 has an independent software-controlled chip-select line.
 * The SPI4 handle is initialized during BSP setup and declared as an
 * extern global here.
 *
 * SPI protocol:
 *   - 16-bit frame: [R/W (1)] [ADDR (7)] [DATA (8)]
 *   - R/W = 1 for read, 0 for write
 *   - CS is driven low before the frame and high after
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "drv_pga280.h"
#include "../hal/hal_spi.h"
#include "../hal/hal_gpio.h"

/* ============================================================================
 * Extern SPI Handle
 *
 * The SPI4 peripheral handle is initialized by the BSP layer during
 * system startup (bsp_edge_v150 or equivalent). The driver uses this
 * shared handle for all PGA280 communication.
 * ============================================================================ */

extern hal_spi_handle_t g_spi4_handle;

/* ============================================================================
 * SPI Protocol Constants
 * ============================================================================ */

#define PGA280_SPI_READ_BIT     (1U << 15)  /* Bit 15 = 1 for read command */
#define PGA280_SPI_ADDR_SHIFT   8           /* Address starts at bit 8 */
#define PGA280_SPI_TIMEOUT_MS   10          /* SPI transfer timeout */

/* ============================================================================
 * Application Gain Mapping Table
 * ============================================================================ */

const pga280_app_gain_t pga280_app_gains[] = {
    { "1x",   PGA280_GAIN_1   },
    { "2x",   PGA280_GAIN_2   },
    { "5x",   PGA280_GAIN_4   },  /* Closest binary gain: 4x for 5x range */
    { "10x",  PGA280_GAIN_8   },  /* Closest binary gain: 8x for 10x range */
    { "20x",  PGA280_GAIN_16  },  /* Closest binary gain: 16x for 20x range */
};

const uint8_t pga280_app_gain_count =
    sizeof(pga280_app_gains) / sizeof(pga280_app_gains[0]);

/* ============================================================================
 * Private Helpers
 * ============================================================================ */

/**
 * @brief Perform a 16-bit SPI transfer with software CS control
 *
 * Asserts CS low, transfers the 16-bit command/data frame, and releases CS.
 * The frame is split into two bytes for the HAL SPI transfer function.
 *
 * @param handle  PGA280 handle (for CS pin)
 * @param tx_word 16-bit word to transmit
 * @param rx_word Pointer to receive 16-bit response (may be NULL)
 * @return true on success
 */
static bool pga280_spi_transfer_16(const pga280_handle_t *handle,
                                   uint16_t tx_word, uint16_t *rx_word)
{
    uint8_t tx_buf[2];
    uint8_t rx_buf[2] = {0, 0};

    /* Pack 16-bit word into bytes, MSB first */
    tx_buf[0] = (uint8_t)(tx_word >> 8);
    tx_buf[1] = (uint8_t)(tx_word & 0xFF);

    /* Assert chip select (active low) */
    hal_gpio_reset(handle->cs_pin);

    /* Full-duplex 16-bit transfer */
    bool ok = hal_spi_transfer(&g_spi4_handle,
                               tx_buf, rx_buf, 2,
                               PGA280_SPI_TIMEOUT_MS);

    /* Release chip select */
    hal_gpio_set(handle->cs_pin);

    if (ok && rx_word) {
        *rx_word = ((uint16_t)rx_buf[0] << 8) | rx_buf[1];
    }

    return ok;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

bool drv_pga280_init(pga280_handle_t *handle, uint8_t channel, uint16_t cs_pin)
{
    if (!handle || channel > 3) {
        return false;
    }

    handle->channel      = channel;
    handle->cs_pin       = cs_pin;
    handle->current_gain = PGA280_GAIN_1;
    handle->initialized  = false;

    /* Configure CS pin as output and deassert (high) */
    hal_gpio_init_output(cs_pin);
    hal_gpio_set(cs_pin);

    /* Set default gain to 1x */
    if (!drv_pga280_set_gain(handle, PGA280_GAIN_1)) {
        return false;
    }

    handle->initialized = true;
    return true;
}

bool drv_pga280_set_gain(pga280_handle_t *handle, pga280_gain_t gain_code)
{
    if (!handle || gain_code >= PGA280_GAIN_COUNT) {
        return false;
    }

    /* Read current GAIN_MUX register to preserve MUX bits */
    uint8_t reg_val = 0;
    if (!drv_pga280_read_register(handle, PGA280_REG_GAIN_MUX, &reg_val)) {
        return false;
    }

    /* Clear gain bits [3:0] and set new gain code */
    reg_val = (reg_val & PGA280_GAIN_MUX_MUX_MASK) |
              ((uint8_t)gain_code & PGA280_GAIN_MUX_GAIN_MASK);

    if (!drv_pga280_write_register(handle, PGA280_REG_GAIN_MUX, reg_val)) {
        return false;
    }

    handle->current_gain = gain_code;
    return true;
}

pga280_gain_t drv_pga280_get_gain(const pga280_handle_t *handle)
{
    if (!handle) {
        return PGA280_GAIN_1;  /* Safe default */
    }
    return handle->current_gain;
}

bool drv_pga280_read_register(pga280_handle_t *handle,
                              uint8_t reg, uint8_t *value)
{
    if (!handle || !value) {
        return false;
    }

    /* Build read command: bit 15 = 1, address in bits [14:8], data = 0 */
    uint16_t cmd = PGA280_SPI_READ_BIT |
                   ((uint16_t)(reg & 0x7F) << PGA280_SPI_ADDR_SHIFT);

    uint16_t response = 0;

    if (!pga280_spi_transfer_16(handle, cmd, &response)) {
        return false;
    }

    /* Data is in the lower 8 bits of the response */
    *value = (uint8_t)(response & 0xFF);
    return true;
}

bool drv_pga280_write_register(pga280_handle_t *handle,
                               uint8_t reg, uint8_t value)
{
    if (!handle) {
        return false;
    }

    /* Build write command: bit 15 = 0, address in bits [14:8], data in [7:0] */
    uint16_t cmd = ((uint16_t)(reg & 0x7F) << PGA280_SPI_ADDR_SHIFT) |
                   (uint16_t)value;

    return pga280_spi_transfer_16(handle, cmd, NULL);
}

bool drv_pga280_check_error(pga280_handle_t *handle)
{
    if (!handle) {
        return false;
    }

    uint8_t err_reg = 0;

    if (!drv_pga280_read_register(handle, PGA280_REG_ERROR, &err_reg)) {
        /* Communication failure -- treat as error condition */
        return true;
    }

    return (err_reg & PGA280_ERR_MASK) != 0;
}
