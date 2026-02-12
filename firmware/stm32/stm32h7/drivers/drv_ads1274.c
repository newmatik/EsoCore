/**
 * @file drv_ads1274.c
 * @brief ADS1274IPAPR 4-Channel 24-Bit Simultaneous ADC Driver Implementation
 *
 * Uses SPI3 with DMA double-buffering for continuous high-speed capture.
 * Pin assignments are sourced from pin_mapping_v150.h.
 *
 * Hardware connections (EsoCore Edge v1.5.0.0):
 *   - SPI3_SCK  = PB3  (AF6)   -- serial clock
 *   - SPI3_MISO = PB4  (AF6)   -- data out (DOUT)
 *   - SPI3_MOSI = PB5  (AF6)   -- data in (DIN, unused for readout)
 *   - CS        = PD7  (GPIO)  -- chip select (software controlled)
 *   - DRDY      = PD4  (EXTI)  -- data ready interrupt
 *   - SYNC      = PD5  (GPIO)  -- sync pulse
 *   - CLKDIV    = PD6  (GPIO)  -- clock divider select
 *   - FORMAT0   = PD3  (GPIO)  -- data format bit 0
 *   - FORMAT1   = PD0  (GPIO)  -- data format bit 1
 *   - MODE0     = PD1  (GPIO)  -- mode select bit 0
 *   - MODE1     = PE7  (GPIO)  -- mode select bit 1
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "drv_ads1274.h"
#include "../hal/hal_spi.h"
#include "../hal/hal_gpio.h"
#include "../hal/hal_dma.h"
#include "../bsp/pin_mapping_v150.h"

#include <string.h>

/* ============================================================================
 * Private Definitions
 * ============================================================================ */

/** SPI clock prescaler for 16 MHz from 64 MHz APB1 (64 / 4 = 16) */
#define ADC_SPI_PRESCALER       4

/** Timeout for single-shot DRDY wait in milliseconds */
#define DRDY_TIMEOUT_MS         100

/** DMA buffer size in bytes per buffer */
#define DMA_BUF_SIZE            (ADS1274_DMA_BUFFER_SAMPLES * ADS1274_FRAME_SIZE)

/* ============================================================================
 * Private Data
 * ============================================================================ */

/** SPI handle for ADS1274 communication */
static hal_spi_handle_t spi_handle;

/** DMA handle for continuous capture */
static hal_dma_handle_t dma_handle;

/** Current driver configuration */
static ads1274_config_t current_config;

/** DMA double-buffer 0 (placed in D2 SRAM for DMA access) */
static uint8_t __attribute__((aligned(4), section(".dma_buffer")))
    dma_buffer_0[DMA_BUF_SIZE];

/** DMA double-buffer 1 */
static uint8_t __attribute__((aligned(4), section(".dma_buffer")))
    dma_buffer_1[DMA_BUF_SIZE];

/** User data-ready callback */
static ads1274_data_ready_cb_t user_callback;

/** User callback context */
static void *user_callback_ctx;

/** Running sample counter */
static volatile uint32_t sample_count;

/** Driver initialized flag */
static bool initialized;

/** Continuous acquisition active flag */
static bool continuous_active;

/* ============================================================================
 * Private Helper Functions
 * ============================================================================ */

/**
 * @brief Convert 3 raw bytes to a sign-extended 24-bit int32_t
 *
 * The ADS1274 outputs 24-bit two's complement data, MSB first.
 * If bit 23 is set (negative value), the upper byte is filled
 * with 0xFF to sign-extend to 32 bits.
 *
 * @param data  Pointer to 3 bytes (MSB first)
 * @return Sign-extended 24-bit value as int32_t
 */
static int32_t ads1274_parse_24bit(const uint8_t *data)
{
    int32_t value = ((int32_t)data[0] << 16) |
                    ((int32_t)data[1] << 8)  |
                    ((int32_t)data[2]);

    /* Sign-extend: if bit 23 is set, OR with 0xFF000000 */
    if (value & 0x00800000) {
        value |= (int32_t)0xFF000000;
    }

    return value;
}

/**
 * @brief Parse a complete 12-byte frame into a sample structure
 *
 * Frame layout: [CH0_MSB, CH0_MID, CH0_LSB, CH1_MSB, ... CH3_LSB]
 *
 * @param frame   Pointer to 12-byte raw frame data
 * @param sample  Pointer to sample structure to fill
 */
static void ads1274_parse_frame(const uint8_t *frame, ads1274_sample_t *sample)
{
    for (int ch = 0; ch < 4; ch++) {
        sample->channels[ch] = ads1274_parse_24bit(&frame[ch * 3]);
    }
}

/**
 * @brief Configure the MODE0/MODE1 pins for the selected operating mode
 *
 * @param mode  Operating mode (ADS1274_MODE_*)
 */
static void ads1274_set_mode_pins(uint8_t mode)
{
    /* MODE0 = bit 0, MODE1 = bit 1 */
    hal_gpio_write(ADC_MODE0, (mode & 0x01) ? true : false);
    hal_gpio_write(ADC_MODE1, (mode & 0x02) ? true : false);
}

/**
 * @brief Configure the FORMAT0/FORMAT1 pins for the selected data format
 *
 * @param format  Data format (ADS1274_FORMAT_*)
 */
static void ads1274_set_format_pins(uint8_t format)
{
    /* FORMAT0 = bit 0, FORMAT1 = bit 1 */
    hal_gpio_write(ADC_FORMAT0, (format & 0x01) ? true : false);
    hal_gpio_write(ADC_FORMAT1, (format & 0x02) ? true : false);
}

/**
 * @brief DMA transfer complete callback
 *
 * Called from DMA interrupt context when one of the double-buffers
 * is full. Parses all frames in the completed buffer and invokes
 * the user callback for each sample.
 *
 * @param context  Unused (DMA callback context)
 */
static void ads1274_dma_tc_callback(void *context)
{
    (void)context;

    if (!user_callback) {
        return;
    }

    /* Determine which buffer just completed */
    uint8_t current_buf = hal_dma_get_current_buffer(&dma_handle);
    const uint8_t *completed_buffer = (current_buf == 0) ? dma_buffer_1
                                                         : dma_buffer_0;

    /* Parse each frame in the completed buffer */
    ads1274_sample_t sample;
    for (uint32_t i = 0; i < ADS1274_DMA_BUFFER_SAMPLES; i++) {
        ads1274_parse_frame(&completed_buffer[i * ADS1274_FRAME_SIZE],
                            &sample);
        sample.timestamp_us = 0; /* TODO: capture timer value */
        sample_count++;

        user_callback(&sample, user_callback_ctx);
    }
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

bool drv_ads1274_init(const ads1274_config_t *config)
{
    if (!config) {
        return false;
    }

    if (config->mode > ADS1274_MODE_LOW_SPEED) {
        return false;
    }

    /* Store configuration */
    memcpy(&current_config, config, sizeof(ads1274_config_t));

    /* ---- Configure control GPIO pins as outputs ---- */
    hal_gpio_init_output(ADC_SPI_CS);
    hal_gpio_init_output(ADC_SYNC);
    hal_gpio_init_output(ADC_CLKDIV);
    hal_gpio_init_output(ADC_FORMAT0);
    hal_gpio_init_output(ADC_FORMAT1);
    hal_gpio_init_output(ADC_MODE0);
    hal_gpio_init_output(ADC_MODE1);

    /* CS idle high */
    hal_gpio_set(ADC_SPI_CS);

    /* SYNC idle high (active-low pulse triggers conversion) */
    hal_gpio_set(ADC_SYNC);

    /* Configure DRDY as input with pull-up (active-low data ready) */
    hal_gpio_init_input(ADC_DRDY, GPIO_PULL_UP);

    /* Set operating mode via MODE0/MODE1 pins */
    ads1274_set_mode_pins(config->mode);

    /* Set data format via FORMAT0/FORMAT1 pins */
    ads1274_set_format_pins(config->format);

    /* CLKDIV LOW = CLK/1 (full speed), HIGH = CLK/2 */
    hal_gpio_reset(ADC_CLKDIV);

    /* ---- Configure SPI3 in receive-only mode ---- */
    hal_spi_config_t spi_config = {
        .instance   = SPI3,
        .mode       = SPI_MODE_1,  /* CPOL=0, CPHA=1 per ADS1274 datasheet */
        .data_size  = SPI_DATA_8BIT,
        .prescaler  = ADC_SPI_PRESCALER,
        .msb_first  = true,
        .software_cs = true,
    };

    /* Configure SPI pins for alternate function */
    hal_gpio_init_af(ADC_SPI_SCK,  ADC_SPI_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(ADC_SPI_MISO, ADC_SPI_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(ADC_SPI_MOSI, ADC_SPI_AF, GPIO_SPEED_VERY_HIGH);

    if (!hal_spi_init(&spi_handle, &spi_config)) {
        return false;
    }

    /* ---- Initialize DMA for double-buffered receive ---- */
    hal_dma_config_t dma_config = {
        .stream         = DMA1_Stream0,
        .direction      = DMA_DIR_PERIPH_TO_MEM,
        .periph_size    = DMA_DATA_BYTE,
        .mem_size       = DMA_DATA_BYTE,
        .periph_inc     = false,
        .mem_inc        = true,
        .circular       = true,
        .double_buffer  = true,
        .priority       = DMA_PRIORITY_HIGH,
        .dmamux_request = DMA_ADC_SPI_RX,
    };

    if (!hal_dma_init(&dma_handle, &dma_config)) {
        hal_spi_deinit(&spi_handle);
        return false;
    }

    /* Register DMA transfer-complete callback */
    hal_dma_register_callbacks(&dma_handle,
                               ads1274_dma_tc_callback,
                               NULL,  /* no half-transfer callback */
                               NULL);

    /* Reset state */
    user_callback = NULL;
    user_callback_ctx = NULL;
    sample_count = 0;
    continuous_active = false;
    initialized = true;

    return true;
}

void drv_ads1274_deinit(void)
{
    if (!initialized) {
        return;
    }

    /* Stop any active acquisition */
    drv_ads1274_stop();

    /* Release peripherals */
    hal_dma_deinit(&dma_handle);
    hal_spi_deinit(&spi_handle);

    /* Drive CS high to deselect */
    hal_gpio_set(ADC_SPI_CS);

    user_callback = NULL;
    user_callback_ctx = NULL;
    sample_count = 0;
    initialized = false;
}

bool drv_ads1274_start_continuous(void)
{
    if (!initialized || continuous_active) {
        return false;
    }

    /* Clear DMA buffers */
    memset(dma_buffer_0, 0, DMA_BUF_SIZE);
    memset(dma_buffer_1, 0, DMA_BUF_SIZE);

    /* Assert CS low for continuous SPI receive */
    hal_gpio_reset(ADC_SPI_CS);

    /* Start DMA double-buffer receive from SPI3 data register */
    if (!hal_spi_receive_dma(&spi_handle,
                             dma_buffer_0, dma_buffer_1,
                             DMA_BUF_SIZE)) {
        hal_gpio_set(ADC_SPI_CS);
        return false;
    }

    /* Pulse SYNC to start conversions */
    hal_gpio_reset(ADC_SYNC);
    /* Short delay -- inline NOP loop for ~1 us */
    for (volatile int i = 0; i < 64; i++) { /* ~1 us at 64 MHz */ }
    hal_gpio_set(ADC_SYNC);

    sample_count = 0;
    continuous_active = true;

    return true;
}

void drv_ads1274_stop(void)
{
    if (!continuous_active) {
        return;
    }

    /* Stop DMA and SPI receive */
    hal_spi_stop_dma(&spi_handle);

    /* Deassert CS */
    hal_gpio_set(ADC_SPI_CS);

    continuous_active = false;
}

bool drv_ads1274_read_single(ads1274_sample_t *sample)
{
    if (!initialized || !sample) {
        return false;
    }

    if (continuous_active) {
        return false;  /* Cannot do single-shot while continuous is active */
    }

    /* Pulse SYNC low then high to trigger a single conversion */
    hal_gpio_reset(ADC_SYNC);
    for (volatile int i = 0; i < 64; i++) { /* ~1 us delay */ }
    hal_gpio_set(ADC_SYNC);

    /* Wait for DRDY to go low (data ready, active-low) */
    uint32_t timeout = DRDY_TIMEOUT_MS * 1000;  /* rough loop count */
    while (hal_gpio_read(ADC_DRDY)) {
        if (--timeout == 0) {
            return false;
        }
    }

    /* Read 12 bytes via SPI: assert CS, clock out data, deassert CS */
    uint8_t rx_buf[ADS1274_FRAME_SIZE];

    hal_gpio_reset(ADC_SPI_CS);

    bool ok = hal_spi_receive(&spi_handle, rx_buf, ADS1274_FRAME_SIZE, 10);

    hal_gpio_set(ADC_SPI_CS);

    if (!ok) {
        return false;
    }

    /* Parse 24-bit two's complement values from 3 bytes each */
    ads1274_parse_frame(rx_buf, sample);
    sample->timestamp_us = 0; /* TODO: capture timer value */

    sample_count++;

    return true;
}

void drv_ads1274_register_callback(ads1274_data_ready_cb_t cb, void *ctx)
{
    user_callback = cb;
    user_callback_ctx = ctx;
}

bool drv_ads1274_set_mode(uint8_t mode)
{
    if (mode > ADS1274_MODE_LOW_SPEED) {
        return false;
    }

    current_config.mode = mode;
    ads1274_set_mode_pins(mode);

    return true;
}

uint32_t drv_ads1274_get_sample_count(void)
{
    return sample_count;
}
