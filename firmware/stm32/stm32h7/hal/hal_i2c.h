/**
 * @file hal_i2c.h
 * @brief I2C Hardware Abstraction Layer for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_HAL_I2C_H
#define ESOCORE_HAL_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "../cmsis/stm32h747xx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    I2C_TypeDef *instance;
    uint32_t     speed_hz;      /* 100000 for standard, 400000 for fast */
} hal_i2c_config_t;

typedef struct {
    I2C_TypeDef *instance;
    bool         initialized;
} hal_i2c_handle_t;

bool hal_i2c_init(hal_i2c_handle_t *handle, const hal_i2c_config_t *config);
void hal_i2c_deinit(hal_i2c_handle_t *handle);

bool hal_i2c_write(hal_i2c_handle_t *handle, uint8_t addr,
                   const uint8_t *data, uint16_t length, uint32_t timeout_ms);

bool hal_i2c_read(hal_i2c_handle_t *handle, uint8_t addr,
                  uint8_t *data, uint16_t length, uint32_t timeout_ms);

bool hal_i2c_write_reg(hal_i2c_handle_t *handle, uint8_t addr,
                       uint8_t reg, const uint8_t *data, uint16_t length,
                       uint32_t timeout_ms);

bool hal_i2c_read_reg(hal_i2c_handle_t *handle, uint8_t addr,
                      uint8_t reg, uint8_t *data, uint16_t length,
                      uint32_t timeout_ms);

bool hal_i2c_is_device_ready(hal_i2c_handle_t *handle, uint8_t addr,
                             uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_HAL_I2C_H */
