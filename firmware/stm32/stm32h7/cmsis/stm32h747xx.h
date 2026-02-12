/**
 * @file stm32h747xx.h
 * @brief CMSIS Cortex-M7 Device Peripheral Access Layer Header (Stub)
 *
 * This is a minimal stub providing base addresses and core definitions
 * for the STM32H747xx. For production, replace with the official CMSIS
 * device header from STMicroelectronics (stm32h747xx.h from STM32CubeH7).
 *
 * @note This file provides only the subset of definitions used by the
 *       EsoCore firmware HAL. The complete header is ~30,000 lines and
 *       should be obtained from:
 *       https://github.com/STMicroelectronics/cmsis_device_h7
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef STM32H747xx_H
#define STM32H747xx_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Interrupt Number Definitions (Cortex-M7 core + STM32H747 peripherals)
 * ============================================================================ */

typedef enum {
    /* Cortex-M7 core exceptions */
    NonMaskableInt_IRQn     = -14,
    HardFault_IRQn          = -13,
    MemoryManagement_IRQn   = -12,
    BusFault_IRQn           = -11,
    UsageFault_IRQn         = -10,
    SVCall_IRQn             = -5,
    DebugMonitor_IRQn       = -4,
    PendSV_IRQn             = -2,
    SysTick_IRQn            = -1,

    /* STM32H747 peripheral interrupts (subset) */
    WWDG_IRQn               = 0,
    PVD_AVD_IRQn            = 1,
    TAMP_STAMP_IRQn         = 2,
    RTC_WKUP_IRQn           = 3,
    FLASH_IRQn              = 4,
    RCC_IRQn                = 5,
    EXTI0_IRQn              = 6,
    EXTI1_IRQn              = 7,
    EXTI2_IRQn              = 8,
    EXTI3_IRQn              = 9,
    EXTI4_IRQn              = 10,
    DMA1_Stream0_IRQn       = 11,
    DMA1_Stream1_IRQn       = 12,
    DMA1_Stream2_IRQn       = 13,
    DMA1_Stream3_IRQn       = 14,
    DMA1_Stream4_IRQn       = 15,
    DMA1_Stream5_IRQn       = 16,
    DMA1_Stream6_IRQn       = 17,
    ADC_IRQn                = 18,
    TIM1_BRK_IRQn           = 24,
    TIM1_UP_IRQn            = 25,
    TIM2_IRQn               = 28,
    TIM3_IRQn               = 29,
    TIM4_IRQn               = 30,
    I2C1_EV_IRQn            = 31,
    I2C1_ER_IRQn            = 32,
    SPI1_IRQn               = 35,
    SPI2_IRQn               = 36,
    USART1_IRQn             = 37,
    USART2_IRQn             = 38,
    USART3_IRQn             = 39,
    EXTI15_10_IRQn          = 40,
    UART4_IRQn              = 52,
    UART5_IRQn              = 53,
    DMA1_Stream7_IRQn       = 47,
    DMA2_Stream0_IRQn       = 56,
    DMA2_Stream1_IRQn       = 57,
    DMA2_Stream2_IRQn       = 58,
    DMA2_Stream3_IRQn       = 59,
    DMA2_Stream4_IRQn       = 60,
    ETH_IRQn                = 61,
    DMA2_Stream5_IRQn       = 68,
    DMA2_Stream6_IRQn       = 69,
    DMA2_Stream7_IRQn       = 70,
    SPI3_IRQn               = 51,
    SPI4_IRQn               = 84,
    TIM6_DAC_IRQn           = 54,
    TIM7_IRQn               = 55,
    SDMMC1_IRQn             = 49,
    IWDG1_IRQn              = 0,   /* shared with WWDG on some configs */
} IRQn_Type;

/* ============================================================================
 * Peripheral Base Addresses
 * ============================================================================ */

/* AHB1 */
#define DMA1_BASE               0x40020000UL
#define DMA2_BASE               0x40020400UL
#define DMAMUX1_BASE            0x40020800UL
#define ETH_BASE                0x40028000UL

/* AHB2 */
#define DCMI_BASE               0x48020000UL
#define RNG_BASE                0x48021800UL

/* AHB3 */
#define SDMMC1_BASE             0x52007000UL

/* AHB4 */
#define GPIOA_BASE              0x58020000UL
#define GPIOB_BASE              0x58020400UL
#define GPIOC_BASE              0x58020800UL
#define GPIOD_BASE              0x58020C00UL
#define GPIOE_BASE              0x58021000UL
#define GPIOF_BASE              0x58021400UL
#define GPIOG_BASE              0x58021800UL
#define GPIOH_BASE              0x58021C00UL
#define GPIOI_BASE              0x58022000UL
#define GPIOJ_BASE              0x58022400UL
#define GPIOK_BASE              0x58022800UL
#define RCC_BASE_ADDR           0x58024400UL
#define PWR_BASE_ADDR           0x58024800UL
#define ADC3_BASE               0x58026000UL

/* APB1 */
#define TIM2_BASE               0x40000000UL
#define TIM3_BASE               0x40000400UL
#define TIM4_BASE               0x40000800UL
#define TIM5_BASE               0x40000C00UL
#define TIM6_BASE               0x40001000UL
#define TIM7_BASE               0x40001400UL
#define SPI2_BASE               0x40003800UL
#define SPI3_BASE               0x40003C00UL
#define USART2_BASE             0x40004400UL
#define USART3_BASE             0x40004800UL
#define UART4_BASE              0x40004C00UL
#define UART5_BASE              0x40005000UL
#define I2C1_BASE               0x40005400UL
#define I2C2_BASE               0x40005800UL
#define I2C3_BASE               0x40005C00UL

/* APB2 */
#define TIM1_BASE               0x40010000UL
#define USART1_BASE             0x40011000UL
#define SPI1_BASE               0x40013000UL
#define SPI4_BASE               0x40013400UL
#define TIM15_BASE              0x40014000UL
#define TIM16_BASE              0x40014400UL
#define TIM17_BASE              0x40014800UL

/* ============================================================================
 * GPIO Register Structure
 * ============================================================================ */

typedef struct {
    volatile uint32_t MODER;        /* Mode register */
    volatile uint32_t OTYPER;       /* Output type register */
    volatile uint32_t OSPEEDR;      /* Output speed register */
    volatile uint32_t PUPDR;        /* Pull-up/pull-down register */
    volatile uint32_t IDR;          /* Input data register */
    volatile uint32_t ODR;          /* Output data register */
    volatile uint32_t BSRR;        /* Bit set/reset register */
    volatile uint32_t LCKR;        /* Configuration lock register */
    volatile uint32_t AFR[2];      /* Alternate function registers [L, H] */
} GPIO_TypeDef;

#define GPIOA   ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB   ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD   ((GPIO_TypeDef *)GPIOD_BASE)
#define GPIOE   ((GPIO_TypeDef *)GPIOE_BASE)
#define GPIOF   ((GPIO_TypeDef *)GPIOF_BASE)
#define GPIOG   ((GPIO_TypeDef *)GPIOG_BASE)
#define GPIOH   ((GPIO_TypeDef *)GPIOH_BASE)
#define GPIOI   ((GPIO_TypeDef *)GPIOI_BASE)

/* Array of GPIO base pointers indexed by port number (GPIO_PORT_A..GPIO_PORT_I) */
static GPIO_TypeDef * const GPIO_PORTS[] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI
};

/* ============================================================================
 * SPI Register Structure
 * ============================================================================ */

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CFG1;
    volatile uint32_t CFG2;
    volatile uint32_t IER;
    volatile uint32_t SR;
    volatile uint32_t IFCR;
    uint32_t RESERVED0;
    volatile uint32_t TXDR;
    uint32_t RESERVED1[3];
    volatile uint32_t RXDR;
    uint32_t RESERVED2[3];
    volatile uint32_t CRCPOLY;
    volatile uint32_t TXCRC;
    volatile uint32_t RXCRC;
    volatile uint32_t UDRDR;
    volatile uint32_t I2SCFGR;
} SPI_TypeDef;

#define SPI1    ((SPI_TypeDef *)SPI1_BASE)
#define SPI2    ((SPI_TypeDef *)SPI2_BASE)
#define SPI3    ((SPI_TypeDef *)SPI3_BASE)
#define SPI4    ((SPI_TypeDef *)SPI4_BASE)

/* ============================================================================
 * USART Register Structure
 * ============================================================================ */

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
    volatile uint32_t PRESC;
} USART_TypeDef;

#define USART1  ((USART_TypeDef *)USART1_BASE)
#define USART2  ((USART_TypeDef *)USART2_BASE)
#define USART3  ((USART_TypeDef *)USART3_BASE)
#define UART4   ((USART_TypeDef *)UART4_BASE)
#define UART5   ((USART_TypeDef *)UART5_BASE)

/* ============================================================================
 * I2C Register Structure
 * ============================================================================ */

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t TIMINGR;
    volatile uint32_t TIMEOUTR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t PECR;
    volatile uint32_t RXDR;
    volatile uint32_t TXDR;
} I2C_TypeDef;

#define I2C1    ((I2C_TypeDef *)I2C1_BASE)
#define I2C2    ((I2C_TypeDef *)I2C2_BASE)
#define I2C3    ((I2C_TypeDef *)I2C3_BASE)

/* ============================================================================
 * DMA Register Structure (Subset)
 * ============================================================================ */

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t NDTR;
    volatile uint32_t PAR;
    volatile uint32_t M0AR;
    volatile uint32_t M1AR;
    volatile uint32_t FCR;
} DMA_Stream_TypeDef;

typedef struct {
    volatile uint32_t LISR;
    volatile uint32_t HISR;
    volatile uint32_t LIFCR;
    volatile uint32_t HIFCR;
} DMA_TypeDef;

#define DMA1            ((DMA_TypeDef *)DMA1_BASE)
#define DMA2            ((DMA_TypeDef *)DMA2_BASE)
#define DMA1_Stream0    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x010UL))
#define DMA1_Stream1    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x028UL))
#define DMA1_Stream2    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x040UL))
#define DMA1_Stream3    ((DMA_Stream_TypeDef *)(DMA1_BASE + 0x058UL))
#define DMA2_Stream0    ((DMA_Stream_TypeDef *)(DMA2_BASE + 0x010UL))
#define DMA2_Stream1    ((DMA_Stream_TypeDef *)(DMA2_BASE + 0x028UL))

/* ============================================================================
 * Timer Register Structure (Subset)
 * ============================================================================ */

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RCR;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t BDTR;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_TypeDef;

#define TIM1    ((TIM_TypeDef *)TIM1_BASE)
#define TIM2    ((TIM_TypeDef *)TIM2_BASE)
#define TIM3    ((TIM_TypeDef *)TIM3_BASE)
#define TIM4    ((TIM_TypeDef *)TIM4_BASE)

/* ============================================================================
 * Ethernet MAC Register Structure (Subset)
 * ============================================================================ */

typedef struct {
    volatile uint32_t MACCR;
    volatile uint32_t MACECR;
    volatile uint32_t MACPFR;
    volatile uint32_t MACWTR;
    volatile uint32_t MACHT0R;
    volatile uint32_t MACHT1R;
    uint32_t RESERVED0[14];
    volatile uint32_t MACVTR;
    uint32_t RESERVED1[1];
    volatile uint32_t MACVHTR;
    uint32_t RESERVED2[1];
    volatile uint32_t MACVIR;
    volatile uint32_t MACIVIR;
    uint32_t RESERVED3[2];
    volatile uint32_t MACTFCR;
    uint32_t RESERVED4[7];
    volatile uint32_t MACRFCR;
    uint32_t RESERVED5[7];
    volatile uint32_t MACISR;
    volatile uint32_t MACIER;
    volatile uint32_t MACRXTXSR;
    uint32_t RESERVED6[1];
    volatile uint32_t MACPCSR;
    volatile uint32_t MACRWKPFR;
    uint32_t RESERVED7[2];
    volatile uint32_t MACLCSR;
    volatile uint32_t MACLTCR;
    volatile uint32_t MACLETR;
    volatile uint32_t MAC1USTCR;
    uint32_t RESERVED8[12];
    volatile uint32_t MACVR;
    volatile uint32_t MACDR;
    uint32_t RESERVED9[2];
    volatile uint32_t MACHWF0R;
    volatile uint32_t MACHWF1R;
    volatile uint32_t MACHWF2R;
    uint32_t RESERVED10[54];
    volatile uint32_t MACMDIOAR;
    volatile uint32_t MACMDIODR;
    uint32_t RESERVED11[2];
    volatile uint32_t MACARPAR;
    uint32_t RESERVED12[59];
    volatile uint32_t MACA0HR;
    volatile uint32_t MACA0LR;
    volatile uint32_t MACA1HR;
    volatile uint32_t MACA1LR;
    volatile uint32_t MACA2HR;
    volatile uint32_t MACA2LR;
    volatile uint32_t MACA3HR;
    volatile uint32_t MACA3LR;
} ETH_TypeDef;

#define ETH     ((ETH_TypeDef *)ETH_BASE)

/* ============================================================================
 * NVIC Functions (inline, CMSIS-compatible)
 * ============================================================================ */

#define NVIC_ISER_BASE  0xE000E100UL
#define NVIC_ICER_BASE  0xE000E180UL
#define NVIC_ISPR_BASE  0xE000E200UL
#define NVIC_IPR_BASE   0xE000E400UL

static inline void NVIC_EnableIRQ(IRQn_Type irqn)
{
    if ((int32_t)irqn >= 0) {
        ((volatile uint32_t *)NVIC_ISER_BASE)[(uint32_t)irqn >> 5] =
            (1UL << ((uint32_t)irqn & 0x1FUL));
    }
}

static inline void NVIC_DisableIRQ(IRQn_Type irqn)
{
    if ((int32_t)irqn >= 0) {
        ((volatile uint32_t *)NVIC_ICER_BASE)[(uint32_t)irqn >> 5] =
            (1UL << ((uint32_t)irqn & 0x1FUL));
    }
}

static inline void NVIC_SetPriority(IRQn_Type irqn, uint32_t priority)
{
    if ((int32_t)irqn >= 0) {
        ((volatile uint8_t *)NVIC_IPR_BASE)[(uint32_t)irqn] =
            (uint8_t)((priority << 4) & 0xFFUL);
    }
}

static inline void NVIC_SystemReset(void)
{
    volatile uint32_t *AIRCR = (volatile uint32_t *)0xE000ED0CUL;
    *AIRCR = (0x5FAUL << 16) | (1UL << 2);
    while (1) { /* Wait for reset */ }
}

static inline void __disable_irq(void)
{
    __asm volatile ("cpsid i" ::: "memory");
}

static inline void __enable_irq(void)
{
    __asm volatile ("cpsie i" ::: "memory");
}

static inline void __DSB(void)
{
    __asm volatile ("dsb 0xF" ::: "memory");
}

static inline void __ISB(void)
{
    __asm volatile ("isb 0xF" ::: "memory");
}

/* ============================================================================
 * Unique Device ID (96-bit, read-only)
 * ============================================================================ */

#define UID_BASE    0x1FF1E800UL

#ifdef __cplusplus
}
#endif

#endif /* STM32H747xx_H */
