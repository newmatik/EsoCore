/**
 * @file hal_init.c
 * @brief HAL Initialization for EsoCore Edge v1.5.0.0
 *
 * Initializes all GPIO pins per pin_mapping_v150.h and configures
 * peripheral alternate functions. Called by bsp_init().
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "hal_gpio.h"
#include "hal_spi.h"
#include "hal_uart.h"
#include "hal_i2c.h"
#include "hal_timer.h"
#include "hal_adc.h"
#include "../bsp/bsp_edge_v150.h"
#include "../bsp/pin_mapping_v150.h"
#include "../bsp/system_clock.h"

/**
 * @brief Initialize all GPIO pins for v1.5.0.0 board
 */
void hal_gpio_init_all(void)
{
    /* ---- Analog input control ---- */
    /* TMUX1101 mode select pins (default: DC mode = HIGH) */
    hal_gpio_init_output(TMUX_SEL_CH0);
    hal_gpio_init_output(TMUX_SEL_CH1);
    hal_gpio_init_output(TMUX_SEL_CH2);
    hal_gpio_init_output(TMUX_SEL_CH3);
    hal_gpio_set(TMUX_SEL_CH0);  /* Default: DC mode */
    hal_gpio_set(TMUX_SEL_CH1);
    hal_gpio_set(TMUX_SEL_CH2);
    hal_gpio_set(TMUX_SEL_CH3);

    /* IEPE current source enable (default: disabled = HIGH for P-MOSFET) */
    hal_gpio_init_output(IEPE_EN_CH0);
    hal_gpio_init_output(IEPE_EN_CH1);
    hal_gpio_init_output(IEPE_EN_CH2);
    hal_gpio_init_output(IEPE_EN_CH3);
    hal_gpio_set(IEPE_EN_CH0);  /* Disabled */
    hal_gpio_set(IEPE_EN_CH1);
    hal_gpio_set(IEPE_EN_CH2);
    hal_gpio_set(IEPE_EN_CH3);

    /* ADC control pins */
    hal_gpio_init_output(ADC_SPI_CS);
    hal_gpio_set(ADC_SPI_CS);  /* CS inactive (high) */
    hal_gpio_init_output(ADC_SYNC);
    hal_gpio_init_output(ADC_CLKDIV);
    hal_gpio_init_output(ADC_FORMAT0);
    hal_gpio_init_output(ADC_FORMAT1);
    hal_gpio_init_output(ADC_MODE0);
    hal_gpio_init_output(ADC_MODE1);
    hal_gpio_init_input(ADC_DRDY, GPIO_PULL_UP);

    /* PGA chip selects (default: inactive high) */
    hal_gpio_init_output(PGA_CS_CH0);
    hal_gpio_init_output(PGA_CS_CH1);
    hal_gpio_init_output(PGA_CS_CH2);
    hal_gpio_init_output(PGA_CS_CH3);
    hal_gpio_set(PGA_CS_CH0);
    hal_gpio_set(PGA_CS_CH1);
    hal_gpio_set(PGA_CS_CH2);
    hal_gpio_set(PGA_CS_CH3);

    /* ---- Safety I/O ---- */
    /* Safety digital inputs (all 12 optocoupler outputs, pull-up) */
    hal_gpio_init_input(SAFETY_IN_CH0_A, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH0_B, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH1_A, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH1_B, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH2_A, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH2_B, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH3_A, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH3_B, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH4_A, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH4_B, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH5_A, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_IN_CH5_B, GPIO_PULL_UP);

    /* Safety relay and SSR outputs (default: off) */
    hal_gpio_init_output(SAFETY_RELAY_CTRL);
    hal_gpio_reset(SAFETY_RELAY_CTRL);
    hal_gpio_init_input(SAFETY_RELAY_FB_NO1, GPIO_PULL_DOWN);
    hal_gpio_init_input(SAFETY_RELAY_FB_NO2, GPIO_PULL_DOWN);
    hal_gpio_init_input(SAFETY_RELAY_FB_NC1, GPIO_PULL_UP);
    hal_gpio_init_input(SAFETY_RELAY_FB_NC2, GPIO_PULL_UP);
    hal_gpio_init_output(SSR_CTRL);
    hal_gpio_reset(SSR_CTRL);

    /* ---- RS-485 bus direction and termination pins ---- */
    hal_gpio_init_output(SBUS_DE_RE);
    hal_gpio_reset(SBUS_DE_RE);
    hal_gpio_init_output(SBUS_TERM_EN);
    hal_gpio_init_output(SBUS_12V_EN);
    hal_gpio_reset(SBUS_12V_EN);

    hal_gpio_init_output(MODBUS_DE_RE);
    hal_gpio_reset(MODBUS_DE_RE);
    hal_gpio_init_output(MODBUS_TERM_EN);

    hal_gpio_init_output(PROFIBUS_DE_RE);
    hal_gpio_reset(PROFIBUS_DE_RE);
    hal_gpio_init_output(PROFIBUS_TERM_EN);

    hal_gpio_init_output(INTERBUS_DE_RE);
    hal_gpio_reset(INTERBUS_DE_RE);
    hal_gpio_init_output(INTERBUS_TERM_EN);

    /* ---- ESP32 control ---- */
    hal_gpio_init_output(ESP32_EN);
    hal_gpio_set(ESP32_EN);  /* Enable ESP32 */
    hal_gpio_init_output(ESP32_BOOT);
    hal_gpio_set(ESP32_BOOT);  /* Normal boot mode */

    /* ---- User interface ---- */
    hal_gpio_init_input(BTN_UP,    GPIO_PULL_UP);
    hal_gpio_init_input(BTN_DOWN,  GPIO_PULL_UP);
    hal_gpio_init_input(BTN_ENTER, GPIO_PULL_UP);
    hal_gpio_init_input(BTN_BACK,  GPIO_PULL_UP);
    hal_gpio_init_input(BTN_RESET, GPIO_PULL_UP);
    hal_gpio_init_input(BTN_BOOT,  GPIO_PULL_UP);

    hal_gpio_init_output(LED_STATUS_GREEN);
    hal_gpio_init_output(LED_STATUS_RED);
    hal_gpio_init_output(LED_NETWORK_A);
    hal_gpio_init_output(LED_NETWORK_B);
    hal_gpio_init_output(LED_SENSOR_BUS);
    hal_gpio_init_output(LED_SAFETY);

    /* ---- OLED display reset ---- */
    hal_gpio_init_output(OLED_RESET);
    hal_gpio_set(OLED_RESET);

    /* ---- SDMMC card detect ---- */
    hal_gpio_init_input(SDMMC_CD, GPIO_PULL_UP);

    /* ---- SPI alternate function pins ---- */
    /* SPI3 for ADS1274 ADC */
    hal_gpio_init_af(ADC_SPI_SCK,  ADC_SPI_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(ADC_SPI_MISO, ADC_SPI_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(ADC_SPI_MOSI, ADC_SPI_AF, GPIO_SPEED_VERY_HIGH);

    /* SPI4 for PGA280 */
    hal_gpio_init_af(PGA_SPI_SCK,  PGA_SPI_AF, GPIO_SPEED_HIGH);
    hal_gpio_init_af(PGA_SPI_MISO, PGA_SPI_AF, GPIO_SPEED_HIGH);
    hal_gpio_init_af(PGA_SPI_MOSI, PGA_SPI_AF, GPIO_SPEED_HIGH);

    /* ---- UART alternate function pins ---- */
    /* USART1: ESP32 */
    hal_gpio_init_af(ESP32_UART_TX, ESP32_UART_AF, GPIO_SPEED_MEDIUM);
    hal_gpio_init_af(ESP32_UART_RX, ESP32_UART_AF, GPIO_SPEED_MEDIUM);

    /* USART2: Sensor bus */
    hal_gpio_init_af(SBUS_UART_TX, SBUS_UART_AF, GPIO_SPEED_MEDIUM);
    hal_gpio_init_af(SBUS_UART_RX, SBUS_UART_AF, GPIO_SPEED_MEDIUM);

    /* USART3: Modbus RTU */
    hal_gpio_init_af(MODBUS_UART_TX, MODBUS_UART_AF, GPIO_SPEED_MEDIUM);
    hal_gpio_init_af(MODBUS_UART_RX, MODBUS_UART_AF, GPIO_SPEED_MEDIUM);

    /* UART4: PROFIBUS */
    hal_gpio_init_af(PROFIBUS_UART_TX, PROFIBUS_UART_AF, GPIO_SPEED_HIGH);
    hal_gpio_init_af(PROFIBUS_UART_RX, PROFIBUS_UART_AF, GPIO_SPEED_HIGH);

    /* UART5: Interbus */
    hal_gpio_init_af(INTERBUS_UART_TX, INTERBUS_UART_AF, GPIO_SPEED_MEDIUM);
    hal_gpio_init_af(INTERBUS_UART_RX, INTERBUS_UART_AF, GPIO_SPEED_MEDIUM);

    /* ---- I2C alternate function pins ---- */
    /* I2C1: OLED display (open-drain for I2C) */
    hal_gpio_config_t i2c_cfg = {
        .mode  = GPIO_MODE_AF,
        .otype = GPIO_OTYPE_OPEN_DRAIN,
        .speed = GPIO_SPEED_MEDIUM,
        .pull  = GPIO_PULL_UP,
        .af    = OLED_I2C_AF,
    };
    i2c_cfg.pin = OLED_I2C_SCL;
    hal_gpio_init(&i2c_cfg);
    i2c_cfg.pin = OLED_I2C_SDA;
    hal_gpio_init(&i2c_cfg);

    /* ---- USB alternate function pins ---- */
    hal_gpio_init_af(USB_DM, USB_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(USB_DP, USB_AF, GPIO_SPEED_VERY_HIGH);

    /* ---- SDMMC alternate function pins ---- */
    hal_gpio_init_af(SDMMC_CLK, SDMMC_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(SDMMC_CMD, SDMMC_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(SDMMC_D0,  SDMMC_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(SDMMC_D1,  SDMMC_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(SDMMC_D2,  SDMMC_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(SDMMC_D3,  SDMMC_AF, GPIO_SPEED_VERY_HIGH);

    /* ---- Power monitoring ADC pins (analog mode) ---- */
    hal_gpio_config_t adc_pin_cfg = {
        .mode  = GPIO_MODE_ANALOG,
        .otype = GPIO_OTYPE_PUSH_PULL,
        .speed = GPIO_SPEED_LOW,
        .pull  = GPIO_PULL_NONE,
        .af    = 0,
    };
    adc_pin_cfg.pin = PWR_MON_24V_INPUT; hal_gpio_init(&adc_pin_cfg);
    adc_pin_cfg.pin = PWR_MON_5V_BUCK;   hal_gpio_init(&adc_pin_cfg);
    adc_pin_cfg.pin = PWR_MON_3V3_DIGITAL; hal_gpio_init(&adc_pin_cfg);
    adc_pin_cfg.pin = PWR_MON_1V2_CORE;  hal_gpio_init(&adc_pin_cfg);
    adc_pin_cfg.pin = PWR_MON_12V_SBUS;  hal_gpio_init(&adc_pin_cfg);
    adc_pin_cfg.pin = PWR_MON_SUPERCAP;  hal_gpio_init(&adc_pin_cfg);
}
