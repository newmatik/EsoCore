/**
 * @file system_stm32h7xx.h
 * @brief CMSIS System Header for STM32H7xx
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef SYSTEM_STM32H7XX_H
#define SYSTEM_STM32H7XX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t SystemCoreClock;

/**
 * @brief Initialize the system (called before main by startup code)
 */
void SystemInit(void);

/**
 * @brief Update SystemCoreClock variable after clock configuration change
 */
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_STM32H7XX_H */
