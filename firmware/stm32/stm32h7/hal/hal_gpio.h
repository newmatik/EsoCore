/**
 * @file hal_gpio.h
 * @brief GPIO Hardware Abstraction Layer for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_HAL_GPIO_H
#define ESOCORE_HAL_GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "../cmsis/stm32h747xx.h"
#include "../bsp/pin_mapping_v150.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO mode definitions */
typedef enum {
    GPIO_MODE_INPUT     = 0x00,
    GPIO_MODE_OUTPUT    = 0x01,
    GPIO_MODE_AF        = 0x02,
    GPIO_MODE_ANALOG    = 0x03,
} hal_gpio_mode_t;

/* GPIO output type */
typedef enum {
    GPIO_OTYPE_PUSH_PULL    = 0,
    GPIO_OTYPE_OPEN_DRAIN   = 1,
} hal_gpio_otype_t;

/* GPIO speed */
typedef enum {
    GPIO_SPEED_LOW      = 0x00,
    GPIO_SPEED_MEDIUM   = 0x01,
    GPIO_SPEED_HIGH     = 0x02,
    GPIO_SPEED_VERY_HIGH = 0x03,
} hal_gpio_speed_t;

/* GPIO pull-up/pull-down */
typedef enum {
    GPIO_PULL_NONE  = 0x00,
    GPIO_PULL_UP    = 0x01,
    GPIO_PULL_DOWN  = 0x02,
} hal_gpio_pull_t;

/* GPIO configuration structure */
typedef struct {
    uint16_t            pin;        /* PIN_DEF(port, pin) */
    hal_gpio_mode_t     mode;
    hal_gpio_otype_t    otype;
    hal_gpio_speed_t    speed;
    hal_gpio_pull_t     pull;
    uint8_t             af;         /* Alternate function number (0-15) */
} hal_gpio_config_t;

/**
 * @brief Configure a GPIO pin
 * @param config Pin configuration
 */
void hal_gpio_init(const hal_gpio_config_t *config);

/**
 * @brief Configure a pin as output (push-pull, no pull, medium speed)
 * @param pin PIN_DEF encoded pin
 */
void hal_gpio_init_output(uint16_t pin);

/**
 * @brief Configure a pin as input with pull-up
 * @param pin PIN_DEF encoded pin
 */
void hal_gpio_init_input(uint16_t pin, hal_gpio_pull_t pull);

/**
 * @brief Configure a pin for alternate function
 * @param pin PIN_DEF encoded pin
 * @param af Alternate function number (0-15)
 * @param speed GPIO speed
 */
void hal_gpio_init_af(uint16_t pin, uint8_t af, hal_gpio_speed_t speed);

/**
 * @brief Write a GPIO output pin
 * @param pin PIN_DEF encoded pin
 * @param state true = high, false = low
 */
void hal_gpio_write(uint16_t pin, bool state);

/**
 * @brief Read a GPIO input pin
 * @param pin PIN_DEF encoded pin
 * @return true if pin is high, false if low
 */
bool hal_gpio_read(uint16_t pin);

/**
 * @brief Toggle a GPIO output pin
 * @param pin PIN_DEF encoded pin
 */
void hal_gpio_toggle(uint16_t pin);

/**
 * @brief Set pin high using atomic BSRR register
 * @param pin PIN_DEF encoded pin
 */
void hal_gpio_set(uint16_t pin);

/**
 * @brief Reset pin low using atomic BSRR register
 * @param pin PIN_DEF encoded pin
 */
void hal_gpio_reset(uint16_t pin);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_HAL_GPIO_H */
