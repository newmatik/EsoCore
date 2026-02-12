/**
 * @file system_clock.h
 * @brief STM32H747 System Clock Configuration
 *
 * Configures the PLL tree from the 25 MHz HSE active oscillator:
 *   PLL1 -> 480 MHz SYSCLK (M7 core)
 *   PLL2 -> 50 MHz Ethernet RMII reference clock
 *   PLL3 -> Peripheral clocks (SPI, USB, etc.)
 *   LSE  -> 32.768 kHz for RTC
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_SYSTEM_CLOCK_H
#define ESOCORE_SYSTEM_CLOCK_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure the full system clock tree
 *
 * Must be called early in startup before any peripheral initialization.
 * Configures voltage regulator scale, flash latency, and all PLLs.
 *
 * @return true if clock configuration succeeds
 */
bool system_clock_init(void);

/**
 * @brief Get the current SYSCLK frequency in Hz
 */
uint32_t system_clock_get_sysclk(void);

/**
 * @brief Get the current HCLK (AHB bus) frequency in Hz
 */
uint32_t system_clock_get_hclk(void);

/**
 * @brief Get APB1 peripheral clock frequency in Hz
 */
uint32_t system_clock_get_pclk1(void);

/**
 * @brief Get APB2 peripheral clock frequency in Hz
 */
uint32_t system_clock_get_pclk2(void);

/**
 * @brief Initialize SysTick for 1 ms interrupts
 */
void system_clock_init_systick(void);

/**
 * @brief Get SysTick counter value (milliseconds since boot)
 */
uint32_t system_clock_get_tick(void);

/**
 * @brief Increment tick counter (called from SysTick_Handler)
 */
void system_clock_tick_increment(void);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_SYSTEM_CLOCK_H */
