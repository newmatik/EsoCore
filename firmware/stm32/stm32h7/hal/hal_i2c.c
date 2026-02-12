/**
 * @file hal_i2c.c
 * @brief I2C HAL Implementation for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "hal_i2c.h"
#include "../bsp/system_clock.h"

/* I2C CR1 bits */
#define I2C_CR1_PE      (1UL << 0)
#define I2C_CR1_TXIE    (1UL << 1)
#define I2C_CR1_RXIE    (1UL << 2)
#define I2C_CR1_ANFOFF  (1UL << 12)

/* I2C CR2 bits */
#define I2C_CR2_START   (1UL << 13)
#define I2C_CR2_STOP    (1UL << 14)
#define I2C_CR2_NACK    (1UL << 15)
#define I2C_CR2_RD_WRN  (1UL << 10)
#define I2C_CR2_AUTOEND (1UL << 25)

/* I2C ISR bits */
#define I2C_ISR_TXE     (1UL << 0)
#define I2C_ISR_TXIS    (1UL << 1)
#define I2C_ISR_RXNE    (1UL << 2)
#define I2C_ISR_TC      (1UL << 6)
#define I2C_ISR_TCR     (1UL << 7)
#define I2C_ISR_STOPF   (1UL << 5)
#define I2C_ISR_NACKF   (1UL << 4)
#define I2C_ISR_BUSY    (1UL << 15)

/* I2C ICR bits */
#define I2C_ICR_STOPCF  (1UL << 5)
#define I2C_ICR_NACKCF  (1UL << 4)

bool hal_i2c_init(hal_i2c_handle_t *handle, const hal_i2c_config_t *config)
{
    if (!handle || !config || !config->instance) return false;

    I2C_TypeDef *i2c = config->instance;
    handle->instance = i2c;

    /* Disable I2C */
    i2c->CR1 &= ~I2C_CR1_PE;

    /* Configure timing register for I2C clock from APB1 (120 MHz) */
    /* TIMINGR values for 120 MHz APB1 clock: */
    if (config->speed_hz >= 400000) {
        /* Fast mode 400 kHz: PRESC=5, SCLDEL=3, SDADEL=0, SCLH=3, SCLL=9 */
        i2c->TIMINGR = (5UL << 28) | (3UL << 20) | (0UL << 16) |
                       (3UL << 8) | (9UL << 0);
    } else {
        /* Standard mode 100 kHz: PRESC=11, SCLDEL=4, SDADEL=2, SCLH=0x32, SCLL=0x38 */
        i2c->TIMINGR = (11UL << 28) | (4UL << 20) | (2UL << 16) |
                       (0x32UL << 8) | (0x38UL << 0);
    }

    /* Enable analog filter, disable digital filter */
    i2c->CR1 &= ~I2C_CR1_ANFOFF;

    /* Enable I2C */
    i2c->CR1 |= I2C_CR1_PE;

    handle->initialized = true;
    return true;
}

void hal_i2c_deinit(hal_i2c_handle_t *handle)
{
    if (!handle || !handle->instance) return;
    handle->instance->CR1 &= ~I2C_CR1_PE;
    handle->initialized = false;
}

bool hal_i2c_write(hal_i2c_handle_t *handle, uint8_t addr,
                   const uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
    if (!handle || !handle->initialized || !data) return false;

    I2C_TypeDef *i2c = handle->instance;
    uint32_t start = system_clock_get_tick();

    /* Configure transfer: slave addr, write, nbytes, autoend */
    i2c->CR2 = ((uint32_t)(addr & 0xFE) << 0) |
               ((uint32_t)length << 16) |
               I2C_CR2_AUTOEND;

    /* Generate start */
    i2c->CR2 |= I2C_CR2_START;

    for (uint16_t i = 0; i < length; i++) {
        while (!(i2c->ISR & I2C_ISR_TXIS)) {
            if (i2c->ISR & I2C_ISR_NACKF) {
                i2c->ICR = I2C_ICR_NACKCF;
                return false;
            }
            if ((system_clock_get_tick() - start) > timeout_ms) return false;
        }
        i2c->TXDR = data[i];
    }

    /* Wait for stop condition */
    while (!(i2c->ISR & I2C_ISR_STOPF)) {
        if ((system_clock_get_tick() - start) > timeout_ms) return false;
    }
    i2c->ICR = I2C_ICR_STOPCF;

    return true;
}

bool hal_i2c_read(hal_i2c_handle_t *handle, uint8_t addr,
                  uint8_t *data, uint16_t length, uint32_t timeout_ms)
{
    if (!handle || !handle->initialized || !data) return false;

    I2C_TypeDef *i2c = handle->instance;
    uint32_t start = system_clock_get_tick();

    /* Configure transfer: slave addr, read, nbytes, autoend */
    i2c->CR2 = ((uint32_t)(addr & 0xFE) << 0) |
               ((uint32_t)length << 16) |
               I2C_CR2_RD_WRN |
               I2C_CR2_AUTOEND;

    /* Generate start */
    i2c->CR2 |= I2C_CR2_START;

    for (uint16_t i = 0; i < length; i++) {
        while (!(i2c->ISR & I2C_ISR_RXNE)) {
            if ((system_clock_get_tick() - start) > timeout_ms) return false;
        }
        data[i] = (uint8_t)(i2c->RXDR & 0xFF);
    }

    /* Wait for stop */
    while (!(i2c->ISR & I2C_ISR_STOPF)) {
        if ((system_clock_get_tick() - start) > timeout_ms) return false;
    }
    i2c->ICR = I2C_ICR_STOPCF;

    return true;
}

bool hal_i2c_write_reg(hal_i2c_handle_t *handle, uint8_t addr,
                       uint8_t reg, const uint8_t *data, uint16_t length,
                       uint32_t timeout_ms)
{
    if (!handle || !handle->initialized) return false;

    I2C_TypeDef *i2c = handle->instance;
    uint32_t start = system_clock_get_tick();
    uint16_t total = length + 1;

    /* Write register address + data in one transfer */
    i2c->CR2 = ((uint32_t)(addr & 0xFE) << 0) |
               ((uint32_t)total << 16) |
               I2C_CR2_AUTOEND;
    i2c->CR2 |= I2C_CR2_START;

    /* Send register address */
    while (!(i2c->ISR & I2C_ISR_TXIS)) {
        if (i2c->ISR & I2C_ISR_NACKF) { i2c->ICR = I2C_ICR_NACKCF; return false; }
        if ((system_clock_get_tick() - start) > timeout_ms) return false;
    }
    i2c->TXDR = reg;

    /* Send data */
    for (uint16_t i = 0; i < length; i++) {
        while (!(i2c->ISR & I2C_ISR_TXIS)) {
            if ((system_clock_get_tick() - start) > timeout_ms) return false;
        }
        i2c->TXDR = data[i];
    }

    while (!(i2c->ISR & I2C_ISR_STOPF)) {
        if ((system_clock_get_tick() - start) > timeout_ms) return false;
    }
    i2c->ICR = I2C_ICR_STOPCF;

    return true;
}

bool hal_i2c_read_reg(hal_i2c_handle_t *handle, uint8_t addr,
                      uint8_t reg, uint8_t *data, uint16_t length,
                      uint32_t timeout_ms)
{
    if (!handle || !handle->initialized || !data) return false;

    /* Write register address (no stop) then read */
    I2C_TypeDef *i2c = handle->instance;
    uint32_t start = system_clock_get_tick();

    /* Write register address without autoend */
    i2c->CR2 = ((uint32_t)(addr & 0xFE) << 0) | (1UL << 16);
    i2c->CR2 |= I2C_CR2_START;

    while (!(i2c->ISR & I2C_ISR_TXIS)) {
        if (i2c->ISR & I2C_ISR_NACKF) { i2c->ICR = I2C_ICR_NACKCF; return false; }
        if ((system_clock_get_tick() - start) > timeout_ms) return false;
    }
    i2c->TXDR = reg;

    while (!(i2c->ISR & I2C_ISR_TC)) {
        if ((system_clock_get_tick() - start) > timeout_ms) return false;
    }

    /* Now read data with repeated start */
    return hal_i2c_read(handle, addr, data, length, timeout_ms);
}

bool hal_i2c_is_device_ready(hal_i2c_handle_t *handle, uint8_t addr,
                             uint32_t timeout_ms)
{
    if (!handle || !handle->initialized) return false;

    I2C_TypeDef *i2c = handle->instance;
    uint32_t start = system_clock_get_tick();

    /* Try a zero-length write */
    i2c->CR2 = ((uint32_t)(addr & 0xFE) << 0) | I2C_CR2_AUTOEND;
    i2c->CR2 |= I2C_CR2_START;

    while (!(i2c->ISR & (I2C_ISR_STOPF | I2C_ISR_NACKF))) {
        if ((system_clock_get_tick() - start) > timeout_ms) return false;
    }

    bool ready = !(i2c->ISR & I2C_ISR_NACKF);
    i2c->ICR = I2C_ICR_STOPCF | I2C_ICR_NACKCF;

    return ready;
}
