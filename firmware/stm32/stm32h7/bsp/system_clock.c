/**
 * @file system_clock.c
 * @brief STM32H747 System Clock Configuration Implementation
 *
 * Register-level clock tree setup for the EsoCore Edge v1.5.0.0.
 *
 * Clock tree:
 *   HSE 25 MHz (active oscillator OT3EL89CJI, shared with Ethernet PHYs)
 *     -> PLL1: VCO = 25/5 * 192 = 960 MHz
 *              PLL1P = 960/2 = 480 MHz -> SYSCLK
 *              PLL1Q = 960/4 = 240 MHz -> available for peripherals
 *     -> PLL2: VCO = 25/5 * 100 = 500 MHz
 *              PLL2P = 500/10 = 50 MHz  -> Ethernet RMII reference
 *     -> PLL3: VCO = 25/5 * 96 = 480 MHz
 *              PLL3Q = 480/10 = 48 MHz  -> USB OTG FS
 *              PLL3R = 480/4 = 120 MHz  -> SPI clocks
 *   LSE 32.768 kHz -> RTC
 *
 * SYSCLK = 480 MHz, HCLK = 240 MHz (D1CPRE /1, HPRE /2)
 * APB1/APB2/APB3 = 120 MHz (D2PPRE1 /2, D2PPRE2 /2)
 * APB4 = 120 MHz (D3PPRE /2)
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "system_clock.h"
#include "bsp_edge_v150.h"

/* STM32H747 register base addresses */
#define RCC_BASE            0x58024400UL
#define FLASH_BASE_ADDR     0x52002000UL
#define PWR_BASE            0x58024800UL
#define SCB_BASE            0xE000ED00UL

/* RCC register offsets */
#define RCC_CR              (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_HSICFGR         (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_CRRCR           (*(volatile uint32_t *)(RCC_BASE + 0x08))
#define RCC_CFGR            (*(volatile uint32_t *)(RCC_BASE + 0x10))
#define RCC_D1CFGR          (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_D2CFGR          (*(volatile uint32_t *)(RCC_BASE + 0x1C))
#define RCC_D3CFGR          (*(volatile uint32_t *)(RCC_BASE + 0x20))
#define RCC_PLLCKSELR       (*(volatile uint32_t *)(RCC_BASE + 0x28))
#define RCC_PLLCFGR         (*(volatile uint32_t *)(RCC_BASE + 0x2C))
#define RCC_PLL1DIVR        (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_PLL1FRACR       (*(volatile uint32_t *)(RCC_BASE + 0x34))
#define RCC_PLL2DIVR        (*(volatile uint32_t *)(RCC_BASE + 0x38))
#define RCC_PLL2FRACR       (*(volatile uint32_t *)(RCC_BASE + 0x3C))
#define RCC_PLL3DIVR        (*(volatile uint32_t *)(RCC_BASE + 0x40))
#define RCC_PLL3FRACR       (*(volatile uint32_t *)(RCC_BASE + 0x44))
#define RCC_D1CCIPR         (*(volatile uint32_t *)(RCC_BASE + 0x4C))
#define RCC_D2CCIP1R        (*(volatile uint32_t *)(RCC_BASE + 0x50))
#define RCC_D2CCIP2R        (*(volatile uint32_t *)(RCC_BASE + 0x54))
#define RCC_D3CCIPR         (*(volatile uint32_t *)(RCC_BASE + 0x58))
#define RCC_AHB1ENR         (*(volatile uint32_t *)(RCC_BASE + 0xD8))
#define RCC_AHB2ENR         (*(volatile uint32_t *)(RCC_BASE + 0xDC))
#define RCC_AHB3ENR         (*(volatile uint32_t *)(RCC_BASE + 0xD4))
#define RCC_AHB4ENR         (*(volatile uint32_t *)(RCC_BASE + 0xE0))
#define RCC_APB1LENR        (*(volatile uint32_t *)(RCC_BASE + 0xE8))
#define RCC_APB1HENR        (*(volatile uint32_t *)(RCC_BASE + 0xEC))
#define RCC_APB2ENR         (*(volatile uint32_t *)(RCC_BASE + 0xF0))
#define RCC_APB3ENR         (*(volatile uint32_t *)(RCC_BASE + 0xF4))
#define RCC_APB4ENR         (*(volatile uint32_t *)(RCC_BASE + 0xF8))
#define RCC_BDCR            (*(volatile uint32_t *)(RCC_BASE + 0x70))

/* FLASH register offsets */
#define FLASH_ACR           (*(volatile uint32_t *)(FLASH_BASE_ADDR + 0x00))

/* PWR register offsets */
#define PWR_CR3             (*(volatile uint32_t *)(PWR_BASE + 0x0C))
#define PWR_D3CR            (*(volatile uint32_t *)(PWR_BASE + 0x18))

/* SCB registers */
#define SCB_VTOR            (*(volatile uint32_t *)(SCB_BASE + 0x08))

/* SysTick registers */
#define SYSTICK_CTRL        (*(volatile uint32_t *)0xE000E010UL)
#define SYSTICK_LOAD        (*(volatile uint32_t *)0xE000E014UL)
#define SYSTICK_VAL         (*(volatile uint32_t *)0xE000E018UL)

/* RCC_CR bit definitions */
#define RCC_CR_HSEON        (1UL << 16)
#define RCC_CR_HSERDY       (1UL << 17)
#define RCC_CR_HSEBYP       (1UL << 18)
#define RCC_CR_PLL1ON       (1UL << 24)
#define RCC_CR_PLL1RDY      (1UL << 25)
#define RCC_CR_PLL2ON       (1UL << 26)
#define RCC_CR_PLL2RDY      (1UL << 27)
#define RCC_CR_PLL3ON       (1UL << 28)
#define RCC_CR_PLL3RDY      (1UL << 29)

/* Timeout for oscillator startup */
#define HSE_STARTUP_TIMEOUT 5000U
#define PLL_STARTUP_TIMEOUT 5000U

/* Tick counter */
static volatile uint32_t systick_counter = 0;

/**
 * @brief Configure the STM32H747 system clock tree
 */
bool system_clock_init(void)
{
    uint32_t timeout;

    /* ---- Step 1: Configure power supply and voltage scaling ---- */

    /* Enable SYSCFG clock for voltage scaling */
    RCC_APB4ENR |= (1UL << 1);  /* SYSCFGEN */

    /* Set VOS to Scale 1 (highest performance, required for 480 MHz) */
    PWR_D3CR = (PWR_D3CR & ~(3UL << 14)) | (3UL << 14);  /* VOS = 11 (Scale 1) */

    /* Wait for voltage scaling to stabilize */
    timeout = HSE_STARTUP_TIMEOUT;
    while (!(PWR_D3CR & (1UL << 13)) && timeout--) {
        /* Wait for VOSRDY */
    }
    if (timeout == 0) {
        return false;
    }

    /* ---- Step 2: Configure Flash latency for 480 MHz ---- */

    /* 4 wait states for 480 MHz at VOS1 */
    FLASH_ACR = (FLASH_ACR & ~0x0FUL) | 4UL;
    /* Programming delay = 2 (for VOS1) */
    FLASH_ACR = (FLASH_ACR & ~(0x0FUL << 4)) | (2UL << 4);

    /* ---- Step 3: Enable HSE (25 MHz active oscillator -- bypass mode) ---- */

    /* HSE bypass for active oscillator (clock signal, not crystal) */
    RCC_CR |= RCC_CR_HSEBYP;
    RCC_CR |= RCC_CR_HSEON;

    timeout = HSE_STARTUP_TIMEOUT;
    while (!(RCC_CR & RCC_CR_HSERDY) && timeout--) {
        /* Wait for HSE ready */
    }
    if (timeout == 0) {
        return false;
    }

    /* ---- Step 4: Configure PLL source and prescalers ---- */

    /* PLL source = HSE, PLL1M = 5, PLL2M = 5, PLL3M = 5 */
    /* PLLCKSELR: PLLSRC = HSE (10), DIVM1 = 5, DIVM2 = 5, DIVM3 = 5 */
    RCC_PLLCKSELR = (5UL << 20) |   /* DIVM3 = 5 */
                    (5UL << 12) |   /* DIVM2 = 5 */
                    (5UL << 4)  |   /* DIVM1 = 5 */
                    (2UL << 0);     /* PLLSRC = HSE */

    /* ---- Step 5: Configure PLL1 (SYSCLK = 480 MHz) ---- */

    /* PLL1: VCO = 25/5 * 192 = 960 MHz (wide range) */
    /* PLLCFGR: PLL1 VCO range 2-16 MHz input (after /M), wide VCO range */
    RCC_PLLCFGR = (RCC_PLLCFGR & ~(0x3FUL)) |
                  (1UL << 0) |      /* PLL1FRACEN = 0 (integer mode) */
                  (3UL << 2) |      /* PLL1RGE = 11 (8-16 MHz input: 25/5 = 5 MHz... */
                                    /* actually 4-8 MHz range: use 10 for 4-8 MHz) */
                  (1UL << 1);       /* PLL1VCOSEL = 0 (wide VCO 192-960 MHz) */

    /* Correct PLL1 input range: 25 MHz / 5 = 5 MHz -> PLL1RGE = 10 (4-8 MHz) */
    RCC_PLLCFGR = (RCC_PLLCFGR & ~(3UL << 2)) | (2UL << 2);
    /* Wide VCO range */
    RCC_PLLCFGR &= ~(1UL << 1);

    /* Enable PLL1 P and Q outputs */
    RCC_PLLCFGR |= (1UL << 16) | (1UL << 17);  /* DIVP1EN, DIVQ1EN */

    /* PLL1DIVR: DIVN1 = 191 (N=192), DIVP1 = 1 (/2), DIVQ1 = 3 (/4) */
    RCC_PLL1DIVR = (3UL << 16) |    /* DIVQ1 = 3 -> /4 = 240 MHz */
                   (1UL << 9)  |    /* DIVP1 = 1 -> /2 = 480 MHz */
                   (191UL << 0);    /* DIVN1 = 191 -> N = 192 */

    /* Enable PLL1 */
    RCC_CR |= RCC_CR_PLL1ON;

    timeout = PLL_STARTUP_TIMEOUT;
    while (!(RCC_CR & RCC_CR_PLL1RDY) && timeout--) {
        /* Wait for PLL1 lock */
    }
    if (timeout == 0) {
        return false;
    }

    /* ---- Step 6: Configure PLL2 (Ethernet 50 MHz) ---- */

    /* PLL2: VCO = 25/5 * 100 = 500 MHz, P = /10 = 50 MHz */
    RCC_PLLCFGR = (RCC_PLLCFGR & ~(3UL << 6)) | (2UL << 6);  /* PLL2RGE 4-8 MHz */
    RCC_PLLCFGR &= ~(1UL << 5);  /* Wide VCO */
    RCC_PLLCFGR |= (1UL << 19);  /* DIVP2EN */

    RCC_PLL2DIVR = (9UL << 9) |     /* DIVP2 = 9 -> /10 = 50 MHz */
                   (99UL << 0);     /* DIVN2 = 99 -> N = 100 */

    RCC_CR |= RCC_CR_PLL2ON;

    timeout = PLL_STARTUP_TIMEOUT;
    while (!(RCC_CR & RCC_CR_PLL2RDY) && timeout--) {
        /* Wait for PLL2 lock */
    }
    if (timeout == 0) {
        return false;
    }

    /* ---- Step 7: Configure PLL3 (USB 48 MHz, SPI clocks) ---- */

    /* PLL3: VCO = 25/5 * 96 = 480 MHz, Q = /10 = 48 MHz, R = /4 = 120 MHz */
    RCC_PLLCFGR = (RCC_PLLCFGR & ~(3UL << 10)) | (2UL << 10);  /* PLL3RGE 4-8 MHz */
    RCC_PLLCFGR &= ~(1UL << 9);   /* Wide VCO */
    RCC_PLLCFGR |= (1UL << 22) | (1UL << 23);  /* DIVQ3EN, DIVR3EN */

    RCC_PLL3DIVR = (3UL << 24)  |   /* DIVR3 = 3 -> /4 = 120 MHz */
                   (9UL << 16)  |   /* DIVQ3 = 9 -> /10 = 48 MHz */
                   (95UL << 0);     /* DIVN3 = 95 -> N = 96 */

    RCC_CR |= RCC_CR_PLL3ON;

    timeout = PLL_STARTUP_TIMEOUT;
    while (!(RCC_CR & RCC_CR_PLL3RDY) && timeout--) {
        /* Wait for PLL3 lock */
    }
    if (timeout == 0) {
        return false;
    }

    /* ---- Step 8: Configure bus prescalers ---- */

    /* D1CFGR: D1CPRE = /1, HPRE = /2 (HCLK = 240 MHz), D1PPRE = /2 */
    RCC_D1CFGR = (4UL << 4) |     /* HPRE = 1000 -> /2 */
                 (4UL << 8) |     /* D1PPRE = 100 -> /2 (APB3 = 120 MHz) */
                 (0UL << 0);      /* D1CPRE = /1 (SYSCLK = 480 MHz) */

    /* D2CFGR: D2PPRE1 = /2, D2PPRE2 = /2 */
    RCC_D2CFGR = (4UL << 4) |     /* D2PPRE1 = 100 -> /2 (APB1 = 120 MHz) */
                 (4UL << 8);      /* D2PPRE2 = 100 -> /2 (APB2 = 120 MHz) */

    /* D3CFGR: D3PPRE = /2 */
    RCC_D3CFGR = (4UL << 4);      /* D3PPRE = 100 -> /2 (APB4 = 120 MHz) */

    /* ---- Step 9: Switch SYSCLK to PLL1 ---- */

    /* CFGR: SW = PLL1 (011) */
    RCC_CFGR = (RCC_CFGR & ~(7UL << 0)) | (3UL << 0);

    timeout = PLL_STARTUP_TIMEOUT;
    while (((RCC_CFGR >> 3) & 7UL) != 3UL && timeout--) {
        /* Wait for SWS = PLL1 */
    }
    if (timeout == 0) {
        return false;
    }

    /* ---- Step 10: Enable LSE for RTC ---- */

    /* Enable backup domain write access */
    PWR_CR3 |= (1UL << 2);  /* Enable backup domain regulator */

    RCC_BDCR |= (1UL << 0);  /* LSEON */
    timeout = HSE_STARTUP_TIMEOUT;
    while (!(RCC_BDCR & (1UL << 1)) && timeout--) {
        /* Wait for LSERDY */
    }
    /* LSE failure is non-fatal -- RTC will not work but system continues */

    /* Select LSE as RTC clock source */
    RCC_BDCR = (RCC_BDCR & ~(3UL << 8)) | (1UL << 8);
    RCC_BDCR |= (1UL << 15);  /* RTCEN */

    /* ---- Step 11: Enable GPIO clocks (all ports used) ---- */

    RCC_AHB4ENR |= (1UL << 0)  |  /* GPIOAEN */
                   (1UL << 1)  |  /* GPIOBEN */
                   (1UL << 2)  |  /* GPIOCEN */
                   (1UL << 3)  |  /* GPIODEN */
                   (1UL << 4)  |  /* GPIOEEN */
                   (1UL << 5)  |  /* GPIOFEN */
                   (1UL << 6)  |  /* GPIOGEN */
                   (1UL << 7)  |  /* GPIOHEN */
                   (1UL << 8);    /* GPIOIEN */

    /* ---- Step 12: Enable peripheral clocks ---- */

    /* AHB1: DMA1, DMA2, ETH1MAC */
    RCC_AHB1ENR |= (1UL << 0)  |  /* DMA1EN */
                   (1UL << 1)  |  /* DMA2EN */
                   (1UL << 15) |  /* ETH1MACEN */
                   (1UL << 16) |  /* ETH1TXEN */
                   (1UL << 17);   /* ETH1RXEN */

    /* AHB2: SDMMC2, DCMI (if needed) */

    /* AHB3: SDMMC1 */
    RCC_AHB3ENR |= (1UL << 16);  /* SDMMC1EN */

    /* APB1L: USART2, USART3, UART4, UART5, I2C1, TIM2-7 */
    RCC_APB1LENR |= (1UL << 17) |  /* USART2EN */
                    (1UL << 18) |  /* USART3EN */
                    (1UL << 19) |  /* UART4EN */
                    (1UL << 20) |  /* UART5EN */
                    (1UL << 21) |  /* I2C1EN */
                    (1UL << 0)  |  /* TIM2EN */
                    (1UL << 1)  |  /* TIM3EN */
                    (1UL << 2);    /* TIM4EN */

    /* APB2: USART1, SPI4, TIM1 */
    RCC_APB2ENR |= (1UL << 4)  |  /* USART1EN */
                   (1UL << 13) |  /* SPI4EN */
                   (1UL << 0);    /* TIM1EN */

    /* APB1H: SPI3 (for ADS1274 ADC -- on APB1) */
    /* Note: SPI3 is on APB1, but clock enable is in APB1LENR */
    RCC_APB1LENR |= (1UL << 15);  /* SPI3EN */

    /* ADC3 clock (for power monitoring) */
    RCC_AHB4ENR |= (1UL << 24);   /* ADC3EN */

    return true;
}

/**
 * @brief Initialize SysTick for 1 ms interrupt period
 */
void system_clock_init_systick(void)
{
    /* SysTick reload = HCLK / 1000 - 1 for 1 ms tick */
    uint32_t reload = (ESOCORE_HCLK_FREQ_HZ / 1000U) - 1U;

    SYSTICK_LOAD = reload;
    SYSTICK_VAL = 0;
    /* Enable SysTick with processor clock and interrupt */
    SYSTICK_CTRL = (1UL << 2) |  /* CLKSOURCE = processor clock */
                   (1UL << 1) |  /* TICKINT = enable interrupt */
                   (1UL << 0);   /* ENABLE */
}

/**
 * @brief Increment the tick counter (called from SysTick_Handler)
 */
void system_clock_tick_increment(void)
{
    systick_counter++;
}

/**
 * @brief Get current tick count in milliseconds
 */
uint32_t system_clock_get_tick(void)
{
    return systick_counter;
}

uint32_t system_clock_get_sysclk(void)
{
    return ESOCORE_SYSCLK_FREQ_HZ;
}

uint32_t system_clock_get_hclk(void)
{
    return ESOCORE_HCLK_FREQ_HZ;
}

uint32_t system_clock_get_pclk1(void)
{
    return ESOCORE_APB1_FREQ_HZ;
}

uint32_t system_clock_get_pclk2(void)
{
    return ESOCORE_APB2_FREQ_HZ;
}
