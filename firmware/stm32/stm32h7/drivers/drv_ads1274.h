/**
 * @file drv_ads1274.h
 * @brief ADS1274IPAPR 4-Channel 24-Bit Simultaneous ADC Driver
 *
 * Driver for the Texas Instruments ADS1274IPAPR high-performance 4-channel
 * 24-bit delta-sigma ADC. The ADS1274 supports simultaneous sampling on all
 * four channels with configurable operating modes and data formats.
 *
 * Hardware interface:
 *   - SPI3 for data readout (receive-only, DMA-capable)
 *   - GPIO control pins for MODE, FORMAT, SYNC, CLKDIV
 *   - DRDY interrupt for data-ready notification
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_DRV_ADS1274_H
#define ESOCORE_DRV_ADS1274_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Operating Mode Definitions
 *
 * The ADS1274 operating mode is selected via MODE0 and MODE1 pins:
 *   MODE1:MODE0 = 00 -> High Resolution (default)
 *   MODE1:MODE0 = 01 -> High Speed
 *   MODE1:MODE0 = 10 -> Low Power
 *   MODE1:MODE0 = 11 -> Low Speed
 * ============================================================================ */

#define ADS1274_MODE_HIGH_RES       0  /**< High Resolution mode */
#define ADS1274_MODE_HIGH_SPEED     1  /**< High Speed mode */
#define ADS1274_MODE_LOW_POWER      2  /**< Low Power mode */
#define ADS1274_MODE_LOW_SPEED      3  /**< Low Speed mode */

/* ============================================================================
 * Data Format Definitions
 *
 * The output data format is selected via FORMAT0 and FORMAT1 pins:
 *   FORMAT1:FORMAT0 = 00 -> Fixed position (24-bit conversion data)
 *   FORMAT1:FORMAT0 = 01 -> Modulator output
 * ============================================================================ */

#define ADS1274_FORMAT_FIXED        0  /**< Fixed-position 24-bit data */
#define ADS1274_FORMAT_MODULATOR    1  /**< Modulator output data */

/* ============================================================================
 * DMA Buffer Configuration
 * ============================================================================ */

/** Frame size in bytes: 3 bytes per channel x 4 channels */
#define ADS1274_FRAME_SIZE          12

/** Number of samples per DMA buffer (double-buffered) */
#define ADS1274_DMA_BUFFER_SAMPLES  256

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/**
 * @brief ADS1274 configuration structure
 */
typedef struct {
    uint8_t     mode;               /**< Operating mode (ADS1274_MODE_*) */
    uint8_t     format;             /**< Data format (ADS1274_FORMAT_*) */
    uint32_t    sample_rate_hz;     /**< Desired sample rate in Hz */
    bool        enable_channels[4]; /**< Per-channel enable flags */
} ads1274_config_t;

/**
 * @brief ADS1274 sample data structure
 *
 * Contains one sample from all four ADC channels. Each channel value
 * is a 24-bit two's complement value sign-extended to int32_t.
 */
typedef struct {
    int32_t     channels[4];        /**< 24-bit sign-extended channel data */
    uint32_t    timestamp_us;       /**< Capture timestamp in microseconds */
} ads1274_sample_t;

/**
 * @brief Data-ready callback type
 *
 * Called when a new sample (or batch of samples) is available.
 * Invoked from DMA interrupt context -- keep processing minimal.
 *
 * @param sample  Pointer to the sample data
 * @param ctx     User-provided context pointer
 */
typedef void (*ads1274_data_ready_cb_t)(ads1274_sample_t *sample, void *ctx);

/* ============================================================================
 * Public API
 * ============================================================================ */

/**
 * @brief Initialize the ADS1274 driver
 *
 * Configures MODE0/MODE1, FORMAT0/FORMAT1, CLKDIV, and SYNC pins.
 * Sets up SPI3 in receive-only mode with DMA double-buffering.
 *
 * @param config  Pointer to configuration structure
 * @return true on success, false on failure
 */
bool drv_ads1274_init(const ads1274_config_t *config);

/**
 * @brief Deinitialize the ADS1274 driver
 *
 * Stops acquisition, releases SPI and DMA resources.
 */
void drv_ads1274_deinit(void);

/**
 * @brief Start continuous DMA-based acquisition
 *
 * Enables DMA SPI receive with two alternating 256-sample buffers.
 * The DRDY interrupt triggers data reading. When a buffer is full,
 * the registered callback is invoked with parsed sample data.
 *
 * @return true on success, false on failure
 */
bool drv_ads1274_start_continuous(void);

/**
 * @brief Stop continuous acquisition
 *
 * Disables DMA and SPI receive. The driver remains initialized
 * and can be restarted with drv_ads1274_start_continuous().
 */
void drv_ads1274_stop(void);

/**
 * @brief Perform a single-shot ADC reading
 *
 * Toggles SYNC low then high to trigger a conversion, waits for
 * DRDY, and reads 12 bytes via SPI. Parses 24-bit two's complement
 * values with sign extension.
 *
 * @param sample  Pointer to sample structure to fill
 * @return true on success, false on timeout or error
 */
bool drv_ads1274_read_single(ads1274_sample_t *sample);

/**
 * @brief Register a data-ready callback
 *
 * The callback is invoked from DMA/interrupt context when a complete
 * buffer of samples has been captured during continuous acquisition.
 *
 * @param cb   Callback function pointer (NULL to unregister)
 * @param ctx  User context passed to callback
 */
void drv_ads1274_register_callback(ads1274_data_ready_cb_t cb, void *ctx);

/**
 * @brief Change the ADC operating mode at runtime
 *
 * Updates the MODE0/MODE1 GPIO pins. The ADC should be stopped
 * before changing modes for clean reconfiguration.
 *
 * @param mode  New operating mode (ADS1274_MODE_*)
 * @return true on success, false if mode is invalid
 */
bool drv_ads1274_set_mode(uint8_t mode);

/**
 * @brief Get the total number of samples acquired
 *
 * Returns a running counter of samples captured since the last
 * call to drv_ads1274_init() or drv_ads1274_start_continuous().
 *
 * @return Number of samples acquired
 */
uint32_t drv_ads1274_get_sample_count(void);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_DRV_ADS1274_H */
