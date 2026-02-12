/**
 * @file hal_gpio.c
 * @brief GPIO HAL Implementation for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "hal_gpio.h"

/**
 * @brief Get GPIO port base pointer from port index
 */
static GPIO_TypeDef *get_port(uint8_t port_idx)
{
    if (port_idx > GPIO_PORT_I) {
        return GPIOA; /* Fallback */
    }
    return GPIO_PORTS[port_idx];
}

void hal_gpio_init(const hal_gpio_config_t *config)
{
    if (!config) return;

    uint8_t port = PIN_PORT(config->pin);
    uint8_t pin  = PIN_NUM(config->pin);
    GPIO_TypeDef *gpio = get_port(port);

    /* Set mode (2 bits per pin) */
    gpio->MODER = (gpio->MODER & ~(3UL << (pin * 2))) |
                  ((uint32_t)config->mode << (pin * 2));

    /* Set output type (1 bit per pin) */
    gpio->OTYPER = (gpio->OTYPER & ~(1UL << pin)) |
                   ((uint32_t)config->otype << pin);

    /* Set speed (2 bits per pin) */
    gpio->OSPEEDR = (gpio->OSPEEDR & ~(3UL << (pin * 2))) |
                    ((uint32_t)config->speed << (pin * 2));

    /* Set pull-up/pull-down (2 bits per pin) */
    gpio->PUPDR = (gpio->PUPDR & ~(3UL << (pin * 2))) |
                  ((uint32_t)config->pull << (pin * 2));

    /* Set alternate function (4 bits per pin, split across AFR[0] and AFR[1]) */
    if (config->mode == GPIO_MODE_AF) {
        uint8_t af_reg = pin >> 3;        /* 0 for pins 0-7, 1 for pins 8-15 */
        uint8_t af_pos = (pin & 7) * 4;   /* Bit position within AFR register */
        gpio->AFR[af_reg] = (gpio->AFR[af_reg] & ~(0xFUL << af_pos)) |
                            ((uint32_t)(config->af & 0xF) << af_pos);
    }
}

void hal_gpio_init_output(uint16_t pin)
{
    hal_gpio_config_t cfg = {
        .pin   = pin,
        .mode  = GPIO_MODE_OUTPUT,
        .otype = GPIO_OTYPE_PUSH_PULL,
        .speed = GPIO_SPEED_MEDIUM,
        .pull  = GPIO_PULL_NONE,
        .af    = 0,
    };
    hal_gpio_init(&cfg);
}

void hal_gpio_init_input(uint16_t pin, hal_gpio_pull_t pull)
{
    hal_gpio_config_t cfg = {
        .pin   = pin,
        .mode  = GPIO_MODE_INPUT,
        .otype = GPIO_OTYPE_PUSH_PULL,
        .speed = GPIO_SPEED_LOW,
        .pull  = pull,
        .af    = 0,
    };
    hal_gpio_init(&cfg);
}

void hal_gpio_init_af(uint16_t pin, uint8_t af, hal_gpio_speed_t speed)
{
    hal_gpio_config_t cfg = {
        .pin   = pin,
        .mode  = GPIO_MODE_AF,
        .otype = GPIO_OTYPE_PUSH_PULL,
        .speed = speed,
        .pull  = GPIO_PULL_NONE,
        .af    = af,
    };
    hal_gpio_init(&cfg);
}

void hal_gpio_write(uint16_t pin, bool state)
{
    GPIO_TypeDef *gpio = get_port(PIN_PORT(pin));
    uint8_t bit = PIN_NUM(pin);
    if (state) {
        gpio->BSRR = (1UL << bit);           /* Set */
    } else {
        gpio->BSRR = (1UL << (bit + 16));    /* Reset */
    }
}

bool hal_gpio_read(uint16_t pin)
{
    GPIO_TypeDef *gpio = get_port(PIN_PORT(pin));
    return (gpio->IDR & (1UL << PIN_NUM(pin))) != 0;
}

void hal_gpio_toggle(uint16_t pin)
{
    GPIO_TypeDef *gpio = get_port(PIN_PORT(pin));
    uint8_t bit = PIN_NUM(pin);
    gpio->ODR ^= (1UL << bit);
}

void hal_gpio_set(uint16_t pin)
{
    GPIO_TypeDef *gpio = get_port(PIN_PORT(pin));
    gpio->BSRR = (1UL << PIN_NUM(pin));
}

void hal_gpio_reset(uint16_t pin)
{
    GPIO_TypeDef *gpio = get_port(PIN_PORT(pin));
    gpio->BSRR = (1UL << (PIN_NUM(pin) + 16));
}
