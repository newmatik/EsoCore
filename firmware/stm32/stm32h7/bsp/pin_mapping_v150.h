/**
 * @file pin_mapping_v150.h
 * @brief EsoCore Edge v1.5.0.0 GPIO Pin Assignments
 *
 * Complete pin mapping for the STM32H747IIT6 (LQFP176) on the EsoCore Edge
 * PCB revision v1.5.0.0. All GPIO assignments are defined here as the single
 * source of truth. Drivers and BSP code include this file instead of
 * hard-coding pin numbers.
 *
 * Pin assignment strategy:
 *   - Ethernet 1 (Port A) uses the default ETH RMII alternate-function pins
 *   - Ethernet 2 (Port B) uses remapped ETH2 pins on Port I/H
 *   - SPI3 for ADS1274 ADC (high-speed, DMA-capable)
 *   - SPI4 for PGA280 gain amplifiers
 *   - USART1 for ESP32-S3 communication
 *   - USART2 for RS-485 sensor bus (SN65HVD72D)
 *   - USART3 for Modbus RTU (ADM2582EBRWZ)
 *   - UART4 for PROFIBUS (SN65HVD1176DR)
 *   - UART5 for Interbus (ADM2582EBRWZ)
 *   - I2C1 for OLED display
 *   - SDMMC1 for microSD (4-bit)
 *   - USB OTG FS for USB-C
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_PIN_MAPPING_V150_H
#define ESOCORE_PIN_MAPPING_V150_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * GPIO Port and Pin Helper Macros
 * ============================================================================ */

/** Encode port letter (A=0 .. K=10) and pin number (0..15) into a uint16_t */
#define PIN_DEF(port, pin)  (((uint16_t)(port) << 8) | (uint16_t)(pin))
#define PIN_PORT(def)       ((uint8_t)((def) >> 8))
#define PIN_NUM(def)        ((uint8_t)((def) & 0xFF))

#define GPIO_PORT_A   0
#define GPIO_PORT_B   1
#define GPIO_PORT_C   2
#define GPIO_PORT_D   3
#define GPIO_PORT_E   4
#define GPIO_PORT_F   5
#define GPIO_PORT_G   6
#define GPIO_PORT_H   7
#define GPIO_PORT_I   8
#define GPIO_PORT_J   9
#define GPIO_PORT_K   10

/* ============================================================================
 * Ethernet Port A (Machine / OT Network) -- KSZ8081RNACA PHY #1
 * RMII interface on default ETH1 pins, MDIO address 0
 * ============================================================================ */

#define ETH1_RMII_REF_CLK       PIN_DEF(GPIO_PORT_A, 1)   /* PA1  AF11 */
#define ETH1_RMII_MDIO          PIN_DEF(GPIO_PORT_A, 2)   /* PA2  AF11 */
#define ETH1_RMII_CRS_DV        PIN_DEF(GPIO_PORT_A, 7)   /* PA7  AF11 */
#define ETH1_RMII_MDC           PIN_DEF(GPIO_PORT_C, 1)   /* PC1  AF11 */
#define ETH1_RMII_RXD0          PIN_DEF(GPIO_PORT_C, 4)   /* PC4  AF11 */
#define ETH1_RMII_RXD1          PIN_DEF(GPIO_PORT_C, 5)   /* PC5  AF11 */
#define ETH1_RMII_TX_EN         PIN_DEF(GPIO_PORT_B, 11)  /* PB11 AF11 */
#define ETH1_RMII_TXD0          PIN_DEF(GPIO_PORT_B, 12)  /* PB12 AF11 */
#define ETH1_RMII_TXD1          PIN_DEF(GPIO_PORT_B, 13)  /* PB13 AF11 */
#define ETH1_PHY_ADDR           0  /* MDIO address for PHY #1 */
#define ETH1_RMII_AF            11 /* Alternate function number */

/* PHY reset and interrupt (directly wired to MCU GPIOs) */
#define ETH1_PHY_RESET          PIN_DEF(GPIO_PORT_G, 0)   /* PG0 -- active low */
#define ETH1_PHY_INT            PIN_DEF(GPIO_PORT_G, 1)   /* PG1 -- active low, EXTI */

/* ============================================================================
 * Ethernet Port B (IT Network) -- KSZ8081RNACA PHY #2
 * RMII interface on remapped pins (PI/PH range), MDIO address 1
 * Note: STM32H747 supports a second Ethernet MAC via alternate pin mapping.
 * MDIO bus is shared with ETH1 (PA2/PC1).
 * ============================================================================ */

#define ETH2_RMII_REF_CLK       PIN_DEF(GPIO_PORT_I, 0)   /* PI0  AF11 */
#define ETH2_RMII_CRS_DV        PIN_DEF(GPIO_PORT_I, 1)   /* PI1  AF11 */
#define ETH2_RMII_RXD0          PIN_DEF(GPIO_PORT_I, 2)   /* PI2  AF11 */
#define ETH2_RMII_RXD1          PIN_DEF(GPIO_PORT_I, 3)   /* PI3  AF11 */
#define ETH2_RMII_TX_EN         PIN_DEF(GPIO_PORT_H, 2)   /* PH2  AF11 */
#define ETH2_RMII_TXD0          PIN_DEF(GPIO_PORT_H, 3)   /* PH3  AF11 */
#define ETH2_RMII_TXD1          PIN_DEF(GPIO_PORT_H, 6)   /* PH6  AF11 */
/* MDIO shared with ETH1: PA2 (MDIO), PC1 (MDC) */
#define ETH2_PHY_ADDR           1  /* MDIO address for PHY #2 */
#define ETH2_RMII_AF            11

/* PHY reset and interrupt */
#define ETH2_PHY_RESET          PIN_DEF(GPIO_PORT_G, 2)   /* PG2 -- active low */
#define ETH2_PHY_INT            PIN_DEF(GPIO_PORT_G, 3)   /* PG3 -- active low, EXTI */

/* ============================================================================
 * ADS1274 24-bit ADC -- SPI3
 * 4-channel simultaneous sampling, SPI + control pins
 * ============================================================================ */

#define ADC_SPI_INSTANCE        3  /* SPI3 */
#define ADC_SPI_SCK             PIN_DEF(GPIO_PORT_B, 3)    /* PB3  AF6 SPI3_SCK */
#define ADC_SPI_MISO            PIN_DEF(GPIO_PORT_B, 4)    /* PB4  AF6 SPI3_MISO (DOUT) */
#define ADC_SPI_MOSI            PIN_DEF(GPIO_PORT_B, 5)    /* PB5  AF6 SPI3_MOSI (DIN) */
#define ADC_SPI_AF              6
#define ADC_SPI_CS              PIN_DEF(GPIO_PORT_D, 7)    /* PD7  GPIO -- software CS */
#define ADC_DRDY                PIN_DEF(GPIO_PORT_D, 4)    /* PD4  EXTI -- data ready IRQ */
#define ADC_SYNC                PIN_DEF(GPIO_PORT_D, 5)    /* PD5  GPIO -- sync pulse */
#define ADC_CLKDIV              PIN_DEF(GPIO_PORT_D, 6)    /* PD6  GPIO -- clock divider */
#define ADC_FORMAT0             PIN_DEF(GPIO_PORT_D, 3)    /* PD3  GPIO -- data format bit 0 */
#define ADC_FORMAT1             PIN_DEF(GPIO_PORT_D, 0)    /* PD0  GPIO -- data format bit 1 */
#define ADC_MODE0               PIN_DEF(GPIO_PORT_D, 1)    /* PD1  GPIO -- mode select bit 0 */
#define ADC_MODE1               PIN_DEF(GPIO_PORT_E, 7)    /* PE7  GPIO -- mode select bit 1 */

/* ============================================================================
 * PGA280 Programmable Gain Amplifiers (4x) -- SPI4
 * Independent chip selects per channel
 * ============================================================================ */

#define PGA_SPI_INSTANCE        4  /* SPI4 */
#define PGA_SPI_SCK             PIN_DEF(GPIO_PORT_E, 2)    /* PE2  AF5 SPI4_SCK */
#define PGA_SPI_MISO            PIN_DEF(GPIO_PORT_E, 5)    /* PE5  AF5 SPI4_MISO */
#define PGA_SPI_MOSI            PIN_DEF(GPIO_PORT_E, 6)    /* PE6  AF5 SPI4_MOSI */
#define PGA_SPI_AF              5
#define PGA_CS_CH0              PIN_DEF(GPIO_PORT_E, 3)    /* PE3  GPIO */
#define PGA_CS_CH1              PIN_DEF(GPIO_PORT_E, 4)    /* PE4  GPIO */
#define PGA_CS_CH2              PIN_DEF(GPIO_PORT_E, 8)    /* PE8  GPIO */
#define PGA_CS_CH3              PIN_DEF(GPIO_PORT_E, 9)    /* PE9  GPIO */

/* ============================================================================
 * TMUX1101 Analog Mode Switches (4x) -- GPIO
 * LOW = IEPE path (AC-coupled), HIGH = DC path (direct)
 * ============================================================================ */

#define TMUX_SEL_CH0            PIN_DEF(GPIO_PORT_F, 0)    /* PF0  GPIO */
#define TMUX_SEL_CH1            PIN_DEF(GPIO_PORT_F, 1)    /* PF1  GPIO */
#define TMUX_SEL_CH2            PIN_DEF(GPIO_PORT_F, 2)    /* PF2  GPIO */
#define TMUX_SEL_CH3            PIN_DEF(GPIO_PORT_F, 3)    /* PF3  GPIO */

/* ============================================================================
 * IEPE Current Source Enable (4x Si2301CDS P-MOSFET) -- GPIO
 * LOW = enable 4 mA source, HIGH = disable (P-channel gate logic)
 * ============================================================================ */

#define IEPE_EN_CH0             PIN_DEF(GPIO_PORT_F, 4)    /* PF4  GPIO */
#define IEPE_EN_CH1             PIN_DEF(GPIO_PORT_F, 5)    /* PF5  GPIO */
#define IEPE_EN_CH2             PIN_DEF(GPIO_PORT_F, 6)    /* PF6  GPIO */
#define IEPE_EN_CH3             PIN_DEF(GPIO_PORT_F, 7)    /* PF7  GPIO */

/* ============================================================================
 * Safety Digital Inputs -- 6 Dual-Channel (12 optocouplers PC817X2NIP1B)
 * Each channel has Channel A and Channel B for cross-monitoring
 * Active LOW (optocoupler pulls GPIO low when field signal is present)
 * 14-pin connector: 2EDGR-5.0-14P
 * ============================================================================ */

#define SAFETY_IN_CH0_A         PIN_DEF(GPIO_PORT_F, 8)    /* PF8  GPIO input */
#define SAFETY_IN_CH0_B         PIN_DEF(GPIO_PORT_F, 9)    /* PF9  GPIO input */
#define SAFETY_IN_CH1_A         PIN_DEF(GPIO_PORT_F, 10)   /* PF10 GPIO input */
#define SAFETY_IN_CH1_B         PIN_DEF(GPIO_PORT_F, 11)   /* PF11 GPIO input */
#define SAFETY_IN_CH2_A         PIN_DEF(GPIO_PORT_F, 12)   /* PF12 GPIO input */
#define SAFETY_IN_CH2_B         PIN_DEF(GPIO_PORT_F, 13)   /* PF13 GPIO input */
#define SAFETY_IN_CH3_A         PIN_DEF(GPIO_PORT_F, 14)   /* PF14 GPIO input */
#define SAFETY_IN_CH3_B         PIN_DEF(GPIO_PORT_F, 15)   /* PF15 GPIO input */
#define SAFETY_IN_CH4_A         PIN_DEF(GPIO_PORT_G, 4)    /* PG4  GPIO input */
#define SAFETY_IN_CH4_B         PIN_DEF(GPIO_PORT_G, 5)    /* PG5  GPIO input */
#define SAFETY_IN_CH5_A         PIN_DEF(GPIO_PORT_G, 6)    /* PG6  GPIO input */
#define SAFETY_IN_CH5_B         PIN_DEF(GPIO_PORT_G, 7)    /* PG7  GPIO input */

#define SAFETY_INPUT_COUNT      6
#define SAFETY_OPTOCOUPLERS     12

/* ============================================================================
 * Safety Relay Output -- G7SA-2A2B-DC24 driven by DRV110APWR
 * ============================================================================ */

#define SAFETY_RELAY_CTRL       PIN_DEF(GPIO_PORT_G, 8)    /* PG8  GPIO output */
#define SAFETY_RELAY_FB_NO1     PIN_DEF(GPIO_PORT_G, 9)    /* PG9  GPIO input (NO contact 1) */
#define SAFETY_RELAY_FB_NO2     PIN_DEF(GPIO_PORT_G, 10)   /* PG10 GPIO input (NO contact 2) */
#define SAFETY_RELAY_FB_NC1     PIN_DEF(GPIO_PORT_G, 11)   /* PG11 GPIO input (NC contact 1) */
#define SAFETY_RELAY_FB_NC2     PIN_DEF(GPIO_PORT_G, 12)   /* PG12 GPIO input (NC contact 2) */

/* ============================================================================
 * Solid-State Relay Output -- AQY212EHAZ
 * Simple GPIO on/off; replaces mechanical relay for signaling/interlock
 * ============================================================================ */

#define SSR_CTRL                PIN_DEF(GPIO_PORT_G, 13)   /* PG13 GPIO output */

/* ============================================================================
 * RS-485 Sensor Bus -- SN65HVD72D, USART2
 * Half-duplex transceiver with direction control
 * ============================================================================ */

#define SBUS_UART_INSTANCE      2  /* USART2 */
#define SBUS_UART_TX            PIN_DEF(GPIO_PORT_A, 3)    /* PA3  AF7 USART2_TX */
#define SBUS_UART_RX            PIN_DEF(GPIO_PORT_A, 0)    /* PA0  AF7 USART2_RX */
#define SBUS_UART_AF            7
#define SBUS_DE_RE              PIN_DEF(GPIO_PORT_A, 4)    /* PA4  GPIO -- DE/RE direction */
#define SBUS_TERM_EN            PIN_DEF(GPIO_PORT_A, 5)    /* PA5  GPIO -- 120 ohm termination */
#define SBUS_12V_EN             PIN_DEF(GPIO_PORT_A, 6)    /* PA6  GPIO -- 12V sensor bus PSU */

/* ============================================================================
 * Modbus RTU -- ADM2582EBRWZ (isolated), USART3
 * ============================================================================ */

#define MODBUS_UART_INSTANCE    3  /* USART3 */
#define MODBUS_UART_TX          PIN_DEF(GPIO_PORT_B, 10)   /* PB10 AF7 USART3_TX */
#define MODBUS_UART_RX          PIN_DEF(GPIO_PORT_D, 9)    /* PD9  AF7 USART3_RX */
#define MODBUS_UART_AF          7
#define MODBUS_DE_RE            PIN_DEF(GPIO_PORT_D, 10)   /* PD10 GPIO -- DE/RE */
#define MODBUS_TERM_EN          PIN_DEF(GPIO_PORT_D, 11)   /* PD11 GPIO -- termination */

/* ============================================================================
 * PROFIBUS DP -- SN65HVD1176DR, UART4 + DB-9 connector
 * ============================================================================ */

#define PROFIBUS_UART_INSTANCE  4  /* UART4 */
#define PROFIBUS_UART_TX        PIN_DEF(GPIO_PORT_B, 9)    /* PB9  AF8 UART4_TX */
#define PROFIBUS_UART_RX        PIN_DEF(GPIO_PORT_B, 8)    /* PB8  AF8 UART4_RX */
#define PROFIBUS_UART_AF        8
#define PROFIBUS_DE_RE          PIN_DEF(GPIO_PORT_D, 12)   /* PD12 GPIO -- DE/RE */
#define PROFIBUS_TERM_EN        PIN_DEF(GPIO_PORT_D, 13)   /* PD13 GPIO -- termination */

/* ============================================================================
 * Interbus -- ADM2582EBRWZ (isolated), UART5
 * Dedicated RS-485 for legacy Bosch Rexroth controller machines
 * ============================================================================ */

#define INTERBUS_UART_INSTANCE  5  /* UART5 */
#define INTERBUS_UART_TX        PIN_DEF(GPIO_PORT_C, 12)   /* PC12 AF8 UART5_TX */
#define INTERBUS_UART_RX        PIN_DEF(GPIO_PORT_D, 2)    /* PD2  AF8 UART5_RX */
#define INTERBUS_UART_AF        8
#define INTERBUS_DE_RE          PIN_DEF(GPIO_PORT_D, 14)   /* PD14 GPIO -- DE/RE */
#define INTERBUS_TERM_EN        PIN_DEF(GPIO_PORT_D, 15)   /* PD15 GPIO -- 120 ohm term */

/* Note: PC12 and PD2 conflict with SDMMC1. SDMMC1 uses alternate pin set
 * (see below). If pin conflict cannot be resolved, Interbus uses UART7
 * on PE8/PE7 instead. For v1.5.0.0 initial build, SDMMC1 is on alternate
 * pins and UART5 is free. */

/* ============================================================================
 * ESP32-S3-WROOM-1 WiFi/BT Module -- USART1
 * AT command interface over UART + auto-programming (UMH3N)
 * ============================================================================ */

#define ESP32_UART_INSTANCE     1  /* USART1 */
#define ESP32_UART_TX           PIN_DEF(GPIO_PORT_A, 9)    /* PA9  AF7 USART1_TX */
#define ESP32_UART_RX           PIN_DEF(GPIO_PORT_A, 10)   /* PA10 AF7 USART1_RX */
#define ESP32_UART_AF           7
#define ESP32_EN                PIN_DEF(GPIO_PORT_A, 8)    /* PA8  GPIO -- module enable */
#define ESP32_BOOT              PIN_DEF(GPIO_PORT_C, 9)    /* PC9  GPIO -- boot mode */
#define ESP32_DTR               PIN_DEF(GPIO_PORT_H, 7)    /* PH7  GPIO -- auto-prog DTR */
#define ESP32_RTS               PIN_DEF(GPIO_PORT_H, 8)    /* PH8  GPIO -- auto-prog RTS */

/* ============================================================================
 * SDMMC1 -- microSD (4-bit mode) with EMIF06-MSD02N16 EMI filter
 * Using alternate pin mapping to avoid conflict with UART5
 * ============================================================================ */

#define SDMMC_CLK               PIN_DEF(GPIO_PORT_C, 12)   /* PC12 AF12 SDMMC1_CK */
#define SDMMC_CMD               PIN_DEF(GPIO_PORT_D, 2)    /* PD2  AF12 SDMMC1_CMD */
#define SDMMC_D0                PIN_DEF(GPIO_PORT_C, 8)    /* PC8  AF12 SDMMC1_D0 */
#define SDMMC_D1                PIN_DEF(GPIO_PORT_C, 9)    /* PC9  AF12 SDMMC1_D1 */
#define SDMMC_D2                PIN_DEF(GPIO_PORT_C, 10)   /* PC10 AF12 SDMMC1_D2 */
#define SDMMC_D3                PIN_DEF(GPIO_PORT_C, 11)   /* PC11 AF12 SDMMC1_D3 */
#define SDMMC_AF                12
#define SDMMC_CD                PIN_DEF(GPIO_PORT_C, 13)   /* PC13 GPIO -- card detect */

/* Note: PC12/PD2 are shared with Interbus UART5. Since SDMMC and Interbus
 * cannot be used simultaneously on these pins, the final PCB routing will
 * resolve this. For firmware, we use runtime mux or move Interbus to UART7.
 * See INTERBUS section note above. */

/* ============================================================================
 * I2C1 -- OLED Display (128x64, address 0x3C) + BSS138LT1G level shifter
 * ============================================================================ */

#define OLED_I2C_INSTANCE       1  /* I2C1 */
#define OLED_I2C_SCL            PIN_DEF(GPIO_PORT_B, 6)    /* PB6  AF4 I2C1_SCL */
#define OLED_I2C_SDA            PIN_DEF(GPIO_PORT_B, 7)    /* PB7  AF4 I2C1_SDA */
#define OLED_I2C_AF             4
#define OLED_RESET              PIN_DEF(GPIO_PORT_B, 14)   /* PB14 GPIO -- display reset */

/* ============================================================================
 * USB OTG FS -- USB-C device/service port
 * Common-mode choke DLW21SN900SQ2L + ESD USBULC6-2M6
 * ============================================================================ */

#define USB_DM                  PIN_DEF(GPIO_PORT_A, 11)   /* PA11 AF10 OTG_FS_DM */
#define USB_DP                  PIN_DEF(GPIO_PORT_A, 12)   /* PA12 AF10 OTG_FS_DP */
#define USB_AF                  10
#define USB_VBUS_SENSE          PIN_DEF(GPIO_PORT_A, 9)    /* Shared -- or use dedicated */
#define USB_ID                  PIN_DEF(GPIO_PORT_A, 10)   /* Shared -- device mode only */

/* ============================================================================
 * User Interface -- Buttons and LEDs
 * 4x menu buttons (Up/Down/Enter/Back) + Reset + Boot
 * ============================================================================ */

#define BTN_UP                  PIN_DEF(GPIO_PORT_H, 9)    /* PH9  GPIO input, pull-up */
#define BTN_DOWN                PIN_DEF(GPIO_PORT_H, 10)   /* PH10 GPIO input, pull-up */
#define BTN_ENTER               PIN_DEF(GPIO_PORT_H, 11)   /* PH11 GPIO input, pull-up */
#define BTN_BACK                PIN_DEF(GPIO_PORT_H, 12)   /* PH12 GPIO input, pull-up */
#define BTN_RESET               PIN_DEF(GPIO_PORT_H, 13)   /* PH13 GPIO input (active low) */
#define BTN_BOOT                PIN_DEF(GPIO_PORT_H, 14)   /* PH14 GPIO input */

#define LED_STATUS_GREEN        PIN_DEF(GPIO_PORT_I, 4)    /* PI4  GPIO output */
#define LED_STATUS_RED          PIN_DEF(GPIO_PORT_I, 5)    /* PI5  GPIO output */
#define LED_NETWORK_A           PIN_DEF(GPIO_PORT_I, 6)    /* PI6  GPIO output */
#define LED_NETWORK_B           PIN_DEF(GPIO_PORT_I, 7)    /* PI7  GPIO output */
#define LED_SENSOR_BUS          PIN_DEF(GPIO_PORT_I, 8)    /* PI8  GPIO output */
#define LED_SAFETY              PIN_DEF(GPIO_PORT_I, 9)    /* PI9  GPIO output */

/* ============================================================================
 * Tag-Connect Debug Ports
 * ============================================================================ */

/* STM32 SWD via TC2050-IDC-NL (directly to MCU) */
#define SWD_SWDIO               PIN_DEF(GPIO_PORT_A, 13)   /* PA13 -- reserved */
#define SWD_SWCLK               PIN_DEF(GPIO_PORT_A, 14)   /* PA14 -- reserved */
#define SWD_SWO                 PIN_DEF(GPIO_PORT_B, 3)    /* PB3  -- shared with SPI3 SCK */

/* ESP32 UART via TC2030-IDC-NL (connected to ESP32 UART0 via UMH3N) */
/* Uses ESP32_UART_TX/RX + DTR/RTS defined above */

/* ============================================================================
 * Power Monitoring (Internal ADC)
 * ADC3 channels for voltage monitoring of each power rail
 * ============================================================================ */

#define PWR_MON_24V_INPUT       PIN_DEF(GPIO_PORT_C, 0)    /* PC0  ADC3_INP10 (via divider) */
#define PWR_MON_5V_BUCK         PIN_DEF(GPIO_PORT_C, 2)    /* PC2  ADC3_INP12 (via divider) */
#define PWR_MON_3V3_DIGITAL     PIN_DEF(GPIO_PORT_C, 3)    /* PC3  ADC3_INP13 */
#define PWR_MON_3V3_ANALOG      PIN_DEF(GPIO_PORT_F, 3)    /* PF3 -- conflict check needed */
#define PWR_MON_1V2_CORE        PIN_DEF(GPIO_PORT_H, 4)    /* PH4  ADC3_INP15 */
#define PWR_MON_12V_SBUS        PIN_DEF(GPIO_PORT_H, 5)    /* PH5  ADC3_INP16 (via divider) */
#define PWR_MON_SUPERCAP        PIN_DEF(GPIO_PORT_C, 6)    /* PC6  ADC3 (via divider) */

/* ============================================================================
 * Clock Inputs
 * ============================================================================ */

#define HSE_IN                  PIN_DEF(GPIO_PORT_H, 0)    /* PH0  OSC_IN  (25 MHz active osc) */
#define HSE_OUT                 PIN_DEF(GPIO_PORT_H, 1)    /* PH1  OSC_OUT (not connected for active osc) */
#define LSE_IN                  PIN_DEF(GPIO_PORT_C, 14)   /* PC14 OSC32_IN  (32.768 kHz) */
#define LSE_OUT                 PIN_DEF(GPIO_PORT_C, 15)   /* PC15 OSC32_OUT */

/* ============================================================================
 * Peripheral DMA Channel Assignments (STM32H7 DMAMUX)
 * STM32H7 uses DMAMUX so channels are flexible -- these are default assignments
 * ============================================================================ */

#define DMA_ADC_SPI_RX          0  /* DMA1 Stream 0 for SPI3 RX (ADS1274) */
#define DMA_ADC_SPI_TX          1  /* DMA1 Stream 1 for SPI3 TX */
#define DMA_SBUS_UART_RX        2  /* DMA1 Stream 2 for USART2 RX */
#define DMA_SBUS_UART_TX        3  /* DMA1 Stream 3 for USART2 TX */
#define DMA_MODBUS_UART_RX      4  /* DMA1 Stream 4 for USART3 RX */
#define DMA_MODBUS_UART_TX      5  /* DMA1 Stream 5 for USART3 TX */
#define DMA_ETH1_TX             6  /* DMA2 Stream 0 for Ethernet 1 */
#define DMA_ETH1_RX             7  /* DMA2 Stream 1 for Ethernet 1 */

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_PIN_MAPPING_V150_H */
