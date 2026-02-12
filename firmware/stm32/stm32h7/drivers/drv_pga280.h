/**
 * @file drv_pga280.h
 * @brief PGA280AIDR Programmable Gain Amplifier Driver for EsoCore Edge
 *
 * Driver for the Texas Instruments PGA280AIDR instrumentation amplifier
 * used in the analog front-end of the EsoCore Edge. Four PGA280 devices
 * (one per measurement channel) are connected to SPI4 with independent
 * chip-select lines. Supports gain settings from 1/8x to 128x.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_DRV_PGA280_H
#define ESOCORE_DRV_PGA280_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * PGA280 Register Addresses
 * ============================================================================ */

#define PGA280_REG_GAIN_MUX     0x00    /* Gain and MUX configuration */
#define PGA280_REG_GPIO_DIR     0x02    /* GPIO direction control */
#define PGA280_REG_GPIO_DAT     0x03    /* GPIO data */
#define PGA280_REG_BUF_TIMEOUT  0x04    /* Buffer timeout configuration */
#define PGA280_REG_CHECKSUM     0x05    /* Checksum register */
#define PGA280_REG_ERROR        0x06    /* Error/status register */

/* ============================================================================
 * PGA280 Gain Code Definitions
 *
 * The GAIN_MUX register bits [3:0] select the binary-weighted gain.
 * Gain = 2^(code-3), giving 1/8x at code 0 through 128x at code 10.
 * ============================================================================ */

typedef enum {
    PGA280_GAIN_1_8   = 0,      /* 0.125x  (2^-3) */
    PGA280_GAIN_1_4   = 1,      /* 0.25x   (2^-2) */
    PGA280_GAIN_1_2   = 2,      /* 0.5x    (2^-1) */
    PGA280_GAIN_1     = 3,      /* 1x      (2^0)  */
    PGA280_GAIN_2     = 4,      /* 2x      (2^1)  */
    PGA280_GAIN_4     = 5,      /* 4x      (2^2)  */
    PGA280_GAIN_8     = 6,      /* 8x      (2^3)  */
    PGA280_GAIN_16    = 7,      /* 16x     (2^4)  */
    PGA280_GAIN_32    = 8,      /* 32x     (2^5)  */
    PGA280_GAIN_64    = 9,      /* 64x     (2^6)  */
    PGA280_GAIN_128   = 10,     /* 128x    (2^7)  */
    PGA280_GAIN_COUNT = 11,
} pga280_gain_t;

/* ============================================================================
 * Application-Level Gain Mapping
 *
 * Maps common application gain settings to PGA280 gain codes.
 * These represent the typical gains used in EsoCore sensor channels.
 * ============================================================================ */

typedef struct {
    const char *label;          /**< Human-readable gain label */
    pga280_gain_t pga_code;     /**< Corresponding PGA280 gain code */
} pga280_app_gain_t;

/** Predefined application gain table (defined in drv_pga280.c) */
extern const pga280_app_gain_t pga280_app_gains[];
extern const uint8_t pga280_app_gain_count;

/* ============================================================================
 * PGA280 Error Register Bit Definitions (0x06)
 * ============================================================================ */

#define PGA280_ERR_CHECKSUM     (1U << 0)   /* SPI checksum error */
#define PGA280_ERR_BUF_OVF      (1U << 1)   /* Input buffer overflow */
#define PGA280_ERR_BUF_TIMEOUT  (1U << 2)   /* Buffer timeout */
#define PGA280_ERR_SPI_FRAME    (1U << 3)   /* SPI framing error */
#define PGA280_ERR_MASK         0x0F        /* All error bits */

/* ============================================================================
 * GAIN_MUX Register Bit Definitions (0x00)
 * ============================================================================ */

#define PGA280_GAIN_MUX_GAIN_MASK   0x0F    /* Bits [3:0]: gain code */
#define PGA280_GAIN_MUX_MUX_MASK    0xF0    /* Bits [7:4]: mux select */

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/** Handle for one PGA280 device instance */
typedef struct {
    uint8_t       channel;        /**< Analog channel index (0-3) */
    uint16_t      cs_pin;         /**< Chip-select GPIO pin (PIN_DEF encoded) */
    pga280_gain_t current_gain;   /**< Currently configured gain code */
    bool          initialized;    /**< true after successful init */
} pga280_handle_t;

/* ============================================================================
 * Public API
 * ============================================================================ */

/**
 * @brief Initialize a PGA280 device handle
 *
 * Sets up the handle state, configures the chip-select GPIO as output,
 * and writes the default gain (1x) to the device.
 *
 * @param handle  Pointer to PGA280 handle to initialize
 * @param channel Analog channel index (0-3)
 * @param cs_pin  Chip-select GPIO pin (PIN_DEF encoded)
 * @return true on success
 */
bool drv_pga280_init(pga280_handle_t *handle, uint8_t channel, uint16_t cs_pin);

/**
 * @brief Set the amplifier gain
 *
 * @param handle    Pointer to initialized PGA280 handle
 * @param gain_code Desired gain setting (PGA280_GAIN_xxx)
 * @return true on success
 */
bool drv_pga280_set_gain(pga280_handle_t *handle, pga280_gain_t gain_code);

/**
 * @brief Get the current gain setting
 *
 * @param handle Pointer to initialized PGA280 handle
 * @return Current gain code
 */
pga280_gain_t drv_pga280_get_gain(const pga280_handle_t *handle);

/**
 * @brief Read a PGA280 register
 *
 * SPI protocol: 16-bit frame, MSB first.
 * Read command: bit 15 = 1, address in bits [14:8], data returned in [7:0].
 *
 * @param handle Pointer to initialized PGA280 handle
 * @param reg    Register address
 * @param value  Pointer to receive register value (8-bit)
 * @return true on success
 */
bool drv_pga280_read_register(pga280_handle_t *handle,
                              uint8_t reg, uint8_t *value);

/**
 * @brief Write a PGA280 register
 *
 * SPI protocol: 16-bit frame, MSB first.
 * Write command: bit 15 = 0, address in bits [14:8], data in [7:0].
 *
 * @param handle Pointer to initialized PGA280 handle
 * @param reg    Register address
 * @param value  Value to write (8-bit)
 * @return true on success
 */
bool drv_pga280_write_register(pga280_handle_t *handle,
                               uint8_t reg, uint8_t value);

/**
 * @brief Check if any error flags are set
 *
 * Reads the ERROR register and returns true if any error bit is active.
 *
 * @param handle Pointer to initialized PGA280 handle
 * @return true if error flags are set, false if no errors (or read failed)
 */
bool drv_pga280_check_error(pga280_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_DRV_PGA280_H */
