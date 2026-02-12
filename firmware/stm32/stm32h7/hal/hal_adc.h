/**
 * @file hal_adc.h
 * @brief Internal ADC HAL for STM32H747 (Power Monitoring)
 *
 * Uses ADC3 to monitor power rail voltages.
 * Note: This is for the MCU's internal ADC, NOT the external ADS1274.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_HAL_ADC_H
#define ESOCORE_HAL_ADC_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ADC3 channel assignments matching pin_mapping_v150.h */
typedef enum {
    ADC_CHANNEL_24V_INPUT   = 10,  /* PC0  ADC3_INP10 */
    ADC_CHANNEL_5V_BUCK     = 12,  /* PC2  ADC3_INP12 */
    ADC_CHANNEL_3V3_DIGITAL = 13,  /* PC3  ADC3_INP13 */
    ADC_CHANNEL_1V2_CORE    = 15,  /* PH4  ADC3_INP15 */
    ADC_CHANNEL_12V_SBUS    = 16,  /* PH5  ADC3_INP16 */
    ADC_CHANNEL_TEMP_SENSOR = 18,  /* Internal temperature sensor */
    ADC_CHANNEL_VREFINT     = 19,  /* Internal voltage reference */
} hal_adc_channel_t;

/**
 * @brief Initialize ADC3 for power monitoring
 */
bool hal_adc_init(void);

/**
 * @brief Deinitialize ADC3
 */
void hal_adc_deinit(void);

/**
 * @brief Read a single ADC channel (blocking, single conversion)
 * @param channel ADC channel to read
 * @return 16-bit ADC raw value (0-65535 for 16-bit mode)
 */
uint16_t hal_adc_read_channel(hal_adc_channel_t channel);

/**
 * @brief Convert ADC raw value to millivolts
 * @param raw_value ADC raw reading
 * @param divider_ratio Voltage divider ratio (e.g., 7.8 for 24V with 6.8k/1k divider)
 * @return Voltage in millivolts
 */
uint32_t hal_adc_raw_to_mv(uint16_t raw_value, float divider_ratio);

/**
 * @brief Read internal temperature sensor
 * @return Temperature in degrees Celsius
 */
int8_t hal_adc_read_temperature(void);

/**
 * @brief Read all power rail voltages
 * @param rail_mv Array of at least 5 elements to receive rail voltages in mV
 *                [0]=24V, [1]=5V, [2]=3.3Vd, [3]=1.2V, [4]=12V
 */
void hal_adc_read_all_rails(uint32_t rail_mv[5]);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_HAL_ADC_H */
