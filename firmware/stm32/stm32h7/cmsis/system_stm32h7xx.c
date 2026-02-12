/**
 * @file system_stm32h7xx.c
 * @brief CMSIS System Initialization for STM32H747 (CM7 core)
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "system_stm32h7xx.h"

/* Default to HSI (64 MHz) until PLL is configured */
uint32_t SystemCoreClock = 64000000UL;

/**
 * @brief System initialization called from Reset_Handler before main()
 *
 * Performs minimal setup:
 *   - Enable FPU (Cortex-M7 FPv5)
 *   - Enable I-Cache and D-Cache
 *   - Set vector table offset
 */
void SystemInit(void)
{
    /* FPU enable: set CP10 and CP11 full access */
    volatile uint32_t *CPACR = (volatile uint32_t *)0xE000ED88UL;
    *CPACR |= (0xFUL << 20);

    /* Data Synchronization Barrier and Instruction Synchronization Barrier */
    __asm volatile ("dsb 0xF" ::: "memory");
    __asm volatile ("isb 0xF" ::: "memory");

    /* Enable I-Cache */
    volatile uint32_t *SCB_CCR = (volatile uint32_t *)0xE000ED14UL;
    volatile uint32_t *ICIALLU = (volatile uint32_t *)0xE000EF50UL;
    *ICIALLU = 0;  /* Invalidate I-Cache */
    __asm volatile ("dsb 0xF" ::: "memory");
    __asm volatile ("isb 0xF" ::: "memory");
    *SCB_CCR |= (1UL << 17);  /* IC bit */

    /* Enable D-Cache */
    volatile uint32_t *DCISW = (volatile uint32_t *)0xE000EF60UL;
    volatile uint32_t *SCB_CCSIDR = (volatile uint32_t *)0xE000ED80UL;
    volatile uint32_t *SCB_CSSELR = (volatile uint32_t *)0xE000ED84UL;

    *SCB_CSSELR = 0;  /* Select D-Cache */
    __asm volatile ("dsb 0xF" ::: "memory");
    uint32_t ccsidr = *SCB_CCSIDR;
    uint32_t sets = (ccsidr >> 13) & 0x7FFFUL;
    uint32_t ways = (ccsidr >> 3) & 0x3FFUL;

    /* Invalidate entire D-Cache */
    for (uint32_t s = 0; s <= sets; s++) {
        for (uint32_t w = 0; w <= ways; w++) {
            *DCISW = (w << 30) | (s << 5);
        }
    }
    __asm volatile ("dsb 0xF" ::: "memory");
    *SCB_CCR |= (1UL << 16);  /* DC bit */
    __asm volatile ("dsb 0xF" ::: "memory");
    __asm volatile ("isb 0xF" ::: "memory");

    /* Set vector table base to beginning of flash */
    volatile uint32_t *SCB_VTOR = (volatile uint32_t *)0xE000ED08UL;
    *SCB_VTOR = 0x08000000UL;
}

void SystemCoreClockUpdate(void)
{
    /* After PLL configuration, update to actual frequency */
    SystemCoreClock = 480000000UL;
}
