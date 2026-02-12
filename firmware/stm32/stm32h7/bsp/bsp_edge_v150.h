/**
 * @file bsp_edge_v150.h
 * @brief EsoCore Edge v1.5.0.0 Board Support Package
 *
 * Top-level board configuration: version info, feature flags, power rail
 * definitions, and BSP initialization/shutdown API.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_BSP_EDGE_V150_H
#define ESOCORE_BSP_EDGE_V150_H

#include <stdint.h>
#include <stdbool.h>
#include "pin_mapping_v150.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Firmware Version
 * ============================================================================ */

#define ESOCORE_FW_VERSION_MAJOR    1
#define ESOCORE_FW_VERSION_MINOR    5
#define ESOCORE_FW_VERSION_PATCH    0
#define ESOCORE_FW_VERSION_BUILD    0
#define ESOCORE_FW_VERSION_STRING   "1.5.0.0"
#define ESOCORE_FW_BUILD_DATE       __DATE__
#define ESOCORE_FW_BUILD_TIME       __TIME__

/* Hardware revision this firmware targets */
#define ESOCORE_HW_VERSION_STRING   "1.5.0.0"

/* ============================================================================
 * Feature Flags -- compile-time enable/disable of subsystems
 * ============================================================================ */

#define ESOCORE_FEATURE_DUAL_ETHERNET       1
#define ESOCORE_FEATURE_ANALOG_INPUTS       1
#define ESOCORE_FEATURE_SAFETY_IO           1
#define ESOCORE_FEATURE_SENSOR_BUS          1
#define ESOCORE_FEATURE_MODBUS_RTU          1
#define ESOCORE_FEATURE_PROFIBUS            1
#define ESOCORE_FEATURE_INTERBUS            1
#define ESOCORE_FEATURE_WIFI_ESP32          1
#define ESOCORE_FEATURE_OLED_DISPLAY        1
#define ESOCORE_FEATURE_TINYML              1
#define ESOCORE_FEATURE_MICROSD             1
#define ESOCORE_FEATURE_USB                 1
#define ESOCORE_FEATURE_SUPERCAP_BACKUP     1
#define ESOCORE_FEATURE_SSR_OUTPUT          1

/* v1.5.0.0: PoE is removed */
#define ESOCORE_FEATURE_POE                 0

/* ============================================================================
 * MCU Configuration
 * ============================================================================ */

#define ESOCORE_MCU_STM32H747       1
#define ESOCORE_MCU_CORE            "Cortex-M7"
#define ESOCORE_MCU_PACKAGE         "LQFP176"
#define ESOCORE_MCU_FLASH_SIZE      (2 * 1024 * 1024)   /* 2 MB */
#define ESOCORE_MCU_SRAM_SIZE       (1 * 1024 * 1024)   /* 1 MB total */

/* Clock speeds (configured by system_clock.c) */
#define ESOCORE_HSE_FREQ_HZ        25000000U    /* 25 MHz active oscillator */
#define ESOCORE_LSE_FREQ_HZ        32768U       /* 32.768 kHz RTC crystal */
#define ESOCORE_SYSCLK_FREQ_HZ     480000000U   /* 480 MHz (PLL1) */
#define ESOCORE_HCLK_FREQ_HZ       240000000U   /* 240 MHz (AHB) */
#define ESOCORE_APB1_FREQ_HZ       120000000U   /* 120 MHz (APB1) */
#define ESOCORE_APB2_FREQ_HZ       120000000U   /* 120 MHz (APB2) */
#define ESOCORE_APB3_FREQ_HZ       120000000U   /* 120 MHz (APB3) */
#define ESOCORE_APB4_FREQ_HZ       120000000U   /* 120 MHz (APB4) */

/* ============================================================================
 * Power Rail Definitions
 * ============================================================================ */

/** 24V DC input from terminal block */
#define ESOCORE_RAIL_24V_NOMINAL_MV     24000
#define ESOCORE_RAIL_24V_MIN_MV         20000
#define ESOCORE_RAIL_24V_MAX_MV         28000
#define ESOCORE_RAIL_24V_BROWNOUT_MV    21000   /* Early warning threshold */
#define ESOCORE_RAIL_24V_SHUTDOWN_MV    19000   /* Hard shutdown threshold */

/** 5V buck converter (TPS54331, 3A) */
#define ESOCORE_RAIL_5V_NOMINAL_MV      5000
#define ESOCORE_RAIL_5V_MIN_MV          4750
#define ESOCORE_RAIL_5V_MAX_MV          5250

/** 3.3V digital LDO (TLV1117LV33) */
#define ESOCORE_RAIL_3V3D_NOMINAL_MV    3300
#define ESOCORE_RAIL_3V3D_MIN_MV        3135
#define ESOCORE_RAIL_3V3D_MAX_MV        3465

/** 3.3V analog LDO (TPS7A4533) -- low-noise, dedicated analog rail */
#define ESOCORE_RAIL_3V3A_NOMINAL_MV    3300
#define ESOCORE_RAIL_3V3A_MIN_MV        3135
#define ESOCORE_RAIL_3V3A_MAX_MV        3465

/** 1.2V LDO (TLV70212DBVR) -- STM32H747 internal regulator bypass */
#define ESOCORE_RAIL_1V2_NOMINAL_MV     1200
#define ESOCORE_RAIL_1V2_MIN_MV         1140
#define ESOCORE_RAIL_1V2_MAX_MV         1260

/** 12V sensor bus power (TPS54331DR, 2A) */
#define ESOCORE_RAIL_12V_NOMINAL_MV     12000
#define ESOCORE_RAIL_12V_MIN_MV         11400
#define ESOCORE_RAIL_12V_MAX_MV         12600

/** Supercapacitor (SLA3R8L2060813, 3.8V rated, 20F) */
#define ESOCORE_SUPERCAP_VOLTAGE_MAX_MV     3800
#define ESOCORE_SUPERCAP_VOLTAGE_MIN_MV     2000    /* Unusable below this */
#define ESOCORE_SUPERCAP_CAPACITANCE_F      20
#define ESOCORE_SUPERCAP_INRUSH_OHMS        680     /* Charge current limiting */
#define ESOCORE_SUPERCAP_TARGET_RUNTIME_S   30      /* Minimum backup runtime */

/* ============================================================================
 * Peripheral Baudrates and Timing
 * ============================================================================ */

#define ESOCORE_ESP32_BAUDRATE          115200
#define ESOCORE_SBUS_BAUDRATE           115200
#define ESOCORE_MODBUS_BAUDRATE         19200
#define ESOCORE_PROFIBUS_BAUDRATE       1500000     /* Up to 12 Mbps */
#define ESOCORE_INTERBUS_BAUDRATE       500000
#define ESOCORE_I2C_SPEED_HZ           400000      /* 400 kHz Fast Mode */
#define ESOCORE_ADC_SPI_SPEED_HZ       16000000    /* 16 MHz SPI for ADS1274 */
#define ESOCORE_PGA_SPI_SPEED_HZ       5000000     /* 5 MHz SPI for PGA280 */

/* ============================================================================
 * Analog Input Configuration
 * ============================================================================ */

#define ESOCORE_ANALOG_CHANNELS         4
#define ESOCORE_ADC_BITS                24
#define ESOCORE_ADC_SAMPLE_RATE_HZ      50000       /* 50 kSPS per channel */
#define ESOCORE_ADC_FRAME_SIZE_BYTES    12          /* 3 bytes x 4 channels */
#define ESOCORE_IEPE_CURRENT_MA         4.0f        /* REF200AU scaled to 4 mA */
#define ESOCORE_IEPE_SETTLING_MS        50          /* Settling after mode switch */
#define ESOCORE_AAF_CUTOFF_HZ           15000       /* Anti-aliasing filter */

/* PGA280 gain settings (subset used for this application) */
#define ESOCORE_PGA_GAIN_1X             0
#define ESOCORE_PGA_GAIN_2X             1
#define ESOCORE_PGA_GAIN_5X             2
#define ESOCORE_PGA_GAIN_10X            3
#define ESOCORE_PGA_GAIN_20X            4
#define ESOCORE_PGA_GAIN_COUNT          5

/* ============================================================================
 * Network Configuration Defaults
 * ============================================================================ */

/** Port A (Machine/OT) -- static IP default */
#define ESOCORE_ETH_A_DEFAULT_IP        "192.168.10.100"
#define ESOCORE_ETH_A_DEFAULT_MASK      "255.255.255.0"
#define ESOCORE_ETH_A_DEFAULT_GW        "192.168.10.1"

/** Port B (IT) -- DHCP by default */
#define ESOCORE_ETH_B_USE_DHCP          1

/* ============================================================================
 * BSP API
 * ============================================================================ */

/**
 * @brief Initialize board support package
 *
 * Performs the full board initialization sequence:
 *   1. Enable FPU and caches (I-Cache, D-Cache)
 *   2. Configure system clocks (HSE -> PLL1/2/3)
 *   3. Configure power rails and verify voltages
 *   4. Initialize all GPIO pins per pin_mapping_v150.h
 *   5. Initialize HAL peripherals (SPI, UART, I2C, Ethernet, DMA, Timers)
 *
 * @return true if all initialization steps succeed
 */
bool bsp_init(void);

/**
 * @brief Get system tick count in milliseconds
 * @return Milliseconds since boot
 */
uint32_t bsp_get_tick_ms(void);

/**
 * @brief Blocking delay in milliseconds
 * @param ms Delay duration
 */
void bsp_delay_ms(uint32_t ms);

/**
 * @brief Blocking delay in microseconds
 * @param us Delay duration
 */
void bsp_delay_us(uint32_t us);

/**
 * @brief Perform board-level shutdown sequence
 *
 * Saves critical state to microSD, de-energizes safety outputs,
 * disables non-essential peripherals, and enters lowest power state.
 */
void bsp_shutdown(void);

/**
 * @brief Trigger a system reset via NVIC
 */
void bsp_system_reset(void);

/**
 * @brief Check if power rail voltages are within acceptable range
 * @return Bitmask of failed rails (0 = all OK)
 */
uint8_t bsp_check_power_rails(void);

/**
 * @brief Get the unique 96-bit device ID from STM32 OTP
 * @param id Buffer to receive 12 bytes of device ID
 */
void bsp_get_device_id(uint8_t id[12]);

/**
 * @brief Get board temperature from internal sensor
 * @return Temperature in degrees Celsius
 */
int8_t bsp_get_temperature(void);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_BSP_EDGE_V150_H */
