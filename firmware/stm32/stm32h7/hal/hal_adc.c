/**
 * @file hal_adc.c
 * @brief Internal ADC HAL Implementation for STM32H747
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "hal_adc.h"
#include "../bsp/bsp_edge_v150.h"
#include "../cmsis/stm32h747xx.h"
#include "../bsp/system_clock.h"

/* ADC3 register base */
#define ADC3_ISR    (*(volatile uint32_t *)(ADC3_BASE + 0x00))
#define ADC3_IER    (*(volatile uint32_t *)(ADC3_BASE + 0x04))
#define ADC3_CR     (*(volatile uint32_t *)(ADC3_BASE + 0x08))
#define ADC3_CFGR   (*(volatile uint32_t *)(ADC3_BASE + 0x0C))
#define ADC3_CFGR2  (*(volatile uint32_t *)(ADC3_BASE + 0x10))
#define ADC3_SMPR1  (*(volatile uint32_t *)(ADC3_BASE + 0x14))
#define ADC3_SMPR2  (*(volatile uint32_t *)(ADC3_BASE + 0x18))
#define ADC3_SQR1   (*(volatile uint32_t *)(ADC3_BASE + 0x30))
#define ADC3_DR     (*(volatile uint32_t *)(ADC3_BASE + 0x40))
#define ADC3_CCR    (*(volatile uint32_t *)(ADC3_BASE + 0x308))

/* ADC calibration voltage (VREFINT typical = 1.21V, ADC at 3.3V VDDA) */
#define ADC_VREF_MV     3300
#define ADC_MAX_VALUE   65535  /* 16-bit resolution */

/* Voltage divider ratios for power rail monitoring */
#define DIVIDER_24V     7.8f    /* 6.8k / 1k divider */
#define DIVIDER_5V      1.52f   /* 10k / 18.2k divider */
#define DIVIDER_12V     4.7f    /* 3.3k / 1k divider */
#define DIVIDER_3V3     1.0f    /* Direct connection */
#define DIVIDER_1V2     1.0f    /* Direct connection */

static bool adc_initialized = false;

bool hal_adc_init(void)
{
    if (adc_initialized) return true;

    /* ADC3 clock is already enabled by system_clock_init (AHB4ENR) */

    /* Exit deep power-down mode */
    ADC3_CR &= ~(1UL << 29);  /* DEEPPWD = 0 */

    /* Enable ADC voltage regulator */
    ADC3_CR |= (1UL << 28);  /* ADVREGEN = 1 */

    /* Wait for regulator startup (~10 us) */
    volatile uint32_t delay = system_clock_get_hclk() / 100000;
    while (delay--) { }

    /* Calibration (single-ended) */
    ADC3_CR &= ~(1UL << 30);  /* ADCALDIF = 0 (single-ended) */
    ADC3_CR |= (1UL << 31);   /* ADCAL = 1 (start calibration) */
    while (ADC3_CR & (1UL << 31)) { /* Wait for calibration complete */ }

    /* Configure resolution: 16-bit */
    ADC3_CFGR = (0UL << 2);  /* RES = 00 (16-bit) */

    /* Configure sampling time for all used channels: 810.5 cycles (slow for accuracy) */
    ADC3_SMPR1 = 0x3FFFFFFFUL;  /* All channels 0-9 = 810.5 cycles */
    ADC3_SMPR2 = 0x3FFFFFFFUL;  /* All channels 10-19 = 810.5 cycles */

    /* Common configuration: internal paths for temp sensor and VREFINT */
    ADC3_CCR = (1UL << 23) |  /* VREFEN */
               (1UL << 22);   /* TSEN (temperature sensor) */

    /* Enable ADC */
    ADC3_ISR |= (1UL << 0);   /* Clear ADRDY flag */
    ADC3_CR |= (1UL << 0);    /* ADEN = 1 */
    while (!(ADC3_ISR & (1UL << 0))) { /* Wait for ADRDY */ }

    adc_initialized = true;
    return true;
}

void hal_adc_deinit(void)
{
    if (!adc_initialized) return;
    ADC3_CR |= (1UL << 1);  /* ADDIS */
    while (ADC3_CR & (1UL << 0)) { }
    ADC3_CR |= (1UL << 29);  /* DEEPPWD */
    adc_initialized = false;
}

uint16_t hal_adc_read_channel(hal_adc_channel_t channel)
{
    if (!adc_initialized) return 0;

    /* Configure sequence: single channel, one conversion */
    ADC3_SQR1 = ((uint32_t)channel << 6) | (0UL << 0);  /* L=0 (1 conversion), SQ1=channel */

    /* Start conversion */
    ADC3_CR |= (1UL << 2);  /* ADSTART */

    /* Wait for end of conversion */
    while (!(ADC3_ISR & (1UL << 2))) { /* EOC flag */ }

    /* Read data */
    uint16_t result = (uint16_t)(ADC3_DR & 0xFFFF);

    /* Clear EOC flag */
    ADC3_ISR |= (1UL << 2);

    return result;
}

uint32_t hal_adc_raw_to_mv(uint16_t raw_value, float divider_ratio)
{
    /* Convert raw ADC value to voltage at pin, then scale by divider */
    uint32_t pin_mv = ((uint32_t)raw_value * ADC_VREF_MV) / ADC_MAX_VALUE;
    return (uint32_t)(pin_mv * divider_ratio);
}

int8_t hal_adc_read_temperature(void)
{
    uint16_t raw = hal_adc_read_channel(ADC_CHANNEL_TEMP_SENSOR);

    /* STM32H7 temperature sensor calibration values (from factory OTP) */
    /* TS_CAL1 at 30C, TS_CAL2 at 110C (stored in system memory) */
    uint16_t ts_cal1 = *(volatile uint16_t *)0x1FF1E820UL;  /* 30C calibration */
    uint16_t ts_cal2 = *(volatile uint16_t *)0x1FF1E840UL;  /* 110C calibration */

    if (ts_cal2 == ts_cal1) return 25;  /* Avoid divide by zero */

    int32_t temp = 30 + ((int32_t)(raw - ts_cal1) * 80) / (int32_t)(ts_cal2 - ts_cal1);
    return (int8_t)temp;
}

void hal_adc_read_all_rails(uint32_t rail_mv[5])
{
    if (!rail_mv) return;

    rail_mv[0] = hal_adc_raw_to_mv(hal_adc_read_channel(ADC_CHANNEL_24V_INPUT), DIVIDER_24V);
    rail_mv[1] = hal_adc_raw_to_mv(hal_adc_read_channel(ADC_CHANNEL_5V_BUCK), DIVIDER_5V);
    rail_mv[2] = hal_adc_raw_to_mv(hal_adc_read_channel(ADC_CHANNEL_3V3_DIGITAL), DIVIDER_3V3);
    rail_mv[3] = hal_adc_raw_to_mv(hal_adc_read_channel(ADC_CHANNEL_1V2_CORE), DIVIDER_1V2);
    rail_mv[4] = hal_adc_raw_to_mv(hal_adc_read_channel(ADC_CHANNEL_12V_SBUS), DIVIDER_12V);
}
