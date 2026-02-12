/**
 * @file analog_input.c
 * @brief 4-Channel Dual-Mode Analog Input Manager Implementation
 *
 * Coordinates TMUX1101, IEPE current source, PGA280, and ADS1274 drivers
 * to provide unified 4-channel analog acquisition with automatic signal
 * processing (RMS, peak, crest factor).
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "analog_input.h"

#include <string.h>
#include <math.h>

/* Driver headers */
#include "../../stm32/stm32h7/drivers/drv_ads1274.h"
#include "../../stm32/stm32h7/drivers/drv_pga280.h"
#include "../../stm32/stm32h7/drivers/drv_tmux1101.h"
#include "../../stm32/stm32h7/drivers/drv_iepe.h"

/* BSP headers */
#include "../../stm32/stm32h7/bsp/bsp_edge_v150.h"
#include "../../stm32/stm32h7/bsp/pin_mapping_v150.h"

/* ============================================================================
 * Private Constants
 * ============================================================================ */

/** ADC full-scale value for 24-bit two's complement */
#define ADC_FULL_SCALE          ((int32_t)0x7FFFFF)

/** Target level for auto-calibration (70% of full scale) */
#define AUTO_CAL_TARGET_RATIO   0.70f

/** IEPE DC bias voltage range for sensor detection (volts) */
#define IEPE_BIAS_MIN_V         8.0f
#define IEPE_BIAS_MAX_V         12.0f

/** Number of samples to read during auto-calibration */
#define AUTO_CAL_SAMPLES        64

/* ============================================================================
 * Application Gain to PGA280 Gain Code Mapping
 * ============================================================================ */

static const pga280_gain_t gain_map[ANALOG_GAIN_COUNT] = {
    [ANALOG_GAIN_1X]  = PGA280_GAIN_1,      /* 1x   */
    [ANALOG_GAIN_2X]  = PGA280_GAIN_2,       /* 2x   */
    [ANALOG_GAIN_5X]  = PGA280_GAIN_4,       /* 4x   (nearest binary step to 5x) */
    [ANALOG_GAIN_10X] = PGA280_GAIN_8,       /* 8x   (nearest binary step to 10x) */
    [ANALOG_GAIN_20X] = PGA280_GAIN_16,      /* 16x  (nearest binary step to 20x) */
};

/** Nominal gain multiplier for each application gain setting */
static const float gain_multiplier[ANALOG_GAIN_COUNT] = {
    [ANALOG_GAIN_1X]  = 1.0f,
    [ANALOG_GAIN_2X]  = 2.0f,
    [ANALOG_GAIN_5X]  = 4.0f,
    [ANALOG_GAIN_10X] = 8.0f,
    [ANALOG_GAIN_20X] = 16.0f,
};

/* ============================================================================
 * Private State
 * ============================================================================ */

static bool subsystem_initialized = false;
static bool acquisition_active = false;

/* Per-channel driver handles */
static tmux1101_handle_t    tmux_handles[ANALOG_NUM_CHANNELS];
static iepe_handle_t        iepe_handles[ANALOG_NUM_CHANNELS];
static pga280_handle_t      pga_handles[ANALOG_NUM_CHANNELS];

/* Configuration and status */
static analog_config_t      current_config;
static analog_status_t      current_status;

/* Processing state */
static int32_t              sample_buffer[ANALOG_NUM_CHANNELS][ADS1274_DMA_BUFFER_SAMPLES];
static uint32_t             sample_index[ANALOG_NUM_CHANNELS];

/* User callback */
static analog_data_callback_t  user_callback = NULL;
static void                    *user_callback_ctx = NULL;

/* ============================================================================
 * Pin Mapping Tables
 * ============================================================================ */

static const uint16_t tmux_sel_pins[ANALOG_NUM_CHANNELS] = {
    TMUX_SEL_CH0, TMUX_SEL_CH1, TMUX_SEL_CH2, TMUX_SEL_CH3
};

static const uint16_t iepe_en_pins[ANALOG_NUM_CHANNELS] = {
    IEPE_EN_CH0, IEPE_EN_CH1, IEPE_EN_CH2, IEPE_EN_CH3
};

static const uint16_t pga_cs_pins[ANALOG_NUM_CHANNELS] = {
    PGA_CS_CH0, PGA_CS_CH1, PGA_CS_CH2, PGA_CS_CH3
};

/* ============================================================================
 * Private Functions -- Signal Processing
 * ============================================================================ */

/**
 * @brief Compute signal statistics from a block of raw ADC samples
 */
static void compute_statistics(const int32_t *samples, uint32_t count,
                               analog_processed_data_t *out) {
    if (count == 0) {
        return;
    }

    float sum = 0.0f;
    float sum_sq = 0.0f;
    int32_t min_val = samples[0];
    int32_t max_val = samples[0];

    for (uint32_t i = 0; i < count; i++) {
        float s = (float)samples[i];
        sum += s;
        sum_sq += s * s;

        if (samples[i] < min_val) {
            min_val = samples[i];
        }
        if (samples[i] > max_val) {
            max_val = samples[i];
        }
    }

    float mean = sum / (float)count;
    float mean_sq = sum_sq / (float)count;
    float rms = sqrtf(mean_sq);

    float abs_peak_min = fabsf((float)min_val);
    float abs_peak_max = fabsf((float)max_val);
    float peak = (abs_peak_max > abs_peak_min) ? abs_peak_max : abs_peak_min;

    out->dc_offset = mean;
    out->rms = rms;
    out->peak = peak;
    out->peak_to_peak = (float)(max_val - min_val);
    out->crest_factor = (rms > 0.0f) ? (peak / rms) : 0.0f;
}

/* ============================================================================
 * Private Functions -- ADC Callback
 * ============================================================================ */

/**
 * @brief ADS1274 data-ready callback (invoked from DMA/interrupt context)
 *
 * Receives one 4-channel sample, distributes to per-channel buffers,
 * and when a full block is accumulated, computes statistics and invokes
 * the user callback.
 */
static void adc_data_callback(ads1274_sample_t *sample, void *ctx) {
    (void)ctx;

    if (!acquisition_active) {
        return;
    }

    for (uint8_t ch = 0; ch < ANALOG_NUM_CHANNELS; ch++) {
        if (current_status.channel_state[ch] != ANALOG_CH_ACQUIRING) {
            continue;
        }

        uint32_t idx = sample_index[ch];
        sample_buffer[ch][idx] = sample->channels[ch];
        sample_index[ch] = idx + 1;

        /* Check for overrange */
        if (sample->channels[ch] >= ADC_FULL_SCALE ||
            sample->channels[ch] <= -ADC_FULL_SCALE) {
            current_status.overrange_count[ch]++;
        }

        /* Block complete -- process and deliver */
        if (sample_index[ch] >= current_config.samples_per_block) {
            current_status.total_samples += sample_index[ch];

            if (user_callback) {
                analog_processed_data_t data;
                data.raw_samples = sample_buffer[ch];
                data.sample_count = sample_index[ch];
                data.timestamp_us = sample->timestamp_us;

                compute_statistics(sample_buffer[ch], sample_index[ch], &data);
                user_callback(ch, &data, user_callback_ctx);
            }

            sample_index[ch] = 0;
        }
    }
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

bool analog_input_init(const analog_config_t *config) {
    if (!config || subsystem_initialized) {
        return false;
    }

    /* Store configuration */
    memcpy(&current_config, config, sizeof(analog_config_t));

    /* Cap samples_per_block to DMA buffer size */
    if (current_config.samples_per_block > ADS1274_DMA_BUFFER_SAMPLES) {
        current_config.samples_per_block = ADS1274_DMA_BUFFER_SAMPLES;
    }

    /* Initialize per-channel driver handles */
    for (uint8_t ch = 0; ch < ANALOG_NUM_CHANNELS; ch++) {
        /* TMUX1101 analog switch */
        if (!drv_tmux1101_init(&tmux_handles[ch], ch, tmux_sel_pins[ch])) {
            return false;
        }

        /* IEPE current source */
        if (!drv_iepe_init(&iepe_handles[ch], ch, iepe_en_pins[ch])) {
            return false;
        }

        /* PGA280 programmable gain amplifier */
        if (!drv_pga280_init(&pga_handles[ch], ch, pga_cs_pins[ch])) {
            return false;
        }

        /* Apply initial channel configuration */
        analog_gain_t gain = config->channel_config[ch].gain;
        if (gain < ANALOG_GAIN_COUNT) {
            drv_pga280_set_gain(&pga_handles[ch], gain_map[gain]);
        }

        /* Initialize processing state */
        sample_index[ch] = 0;
        memset(sample_buffer[ch], 0, sizeof(sample_buffer[ch]));

        /* Set initial status */
        current_status.channel_state[ch] = ANALOG_CH_IDLE;
        current_status.channel_mode[ch] = config->channel_config[ch].mode;
        current_status.channel_gain[ch] = config->channel_config[ch].gain;
        current_status.overrange_count[ch] = 0;
    }

    current_status.total_samples = 0;

    /* Initialize ADS1274 ADC */
    ads1274_config_t adc_config;
    adc_config.mode = ADS1274_MODE_HIGH_RES;
    adc_config.format = ADS1274_FORMAT_FIXED;
    adc_config.sample_rate_hz = config->sample_rate_hz;
    for (uint8_t ch = 0; ch < ANALOG_NUM_CHANNELS; ch++) {
        adc_config.enable_channels[ch] = true;
    }

    if (!drv_ads1274_init(&adc_config)) {
        return false;
    }

    /* Register our internal ADC callback */
    drv_ads1274_register_callback(adc_data_callback, NULL);

    /* Apply initial channel modes (IEPE/DC) */
    for (uint8_t ch = 0; ch < ANALOG_NUM_CHANNELS; ch++) {
        analog_mode_t mode = config->channel_config[ch].mode;
        if (mode == ANALOG_MODE_IEPE) {
            drv_tmux1101_set_mode(&tmux_handles[ch], TMUX1101_MODE_IEPE);
            drv_iepe_enable(&iepe_handles[ch]);
        } else {
            drv_iepe_disable(&iepe_handles[ch]);
            drv_tmux1101_set_mode(&tmux_handles[ch], TMUX1101_MODE_DC);
        }
    }

    subsystem_initialized = true;
    return true;
}

void analog_input_deinit(void) {
    if (!subsystem_initialized) {
        return;
    }

    /* Stop acquisition if running */
    if (acquisition_active) {
        analog_input_stop_acquisition();
    }

    /* Disable all IEPE current sources */
    for (uint8_t ch = 0; ch < ANALOG_NUM_CHANNELS; ch++) {
        drv_iepe_disable(&iepe_handles[ch]);
        current_status.channel_state[ch] = ANALOG_CH_DISABLED;
    }

    /* Shutdown ADC */
    drv_ads1274_deinit();

    user_callback = NULL;
    user_callback_ctx = NULL;
    acquisition_active = false;
    subsystem_initialized = false;
}

bool analog_input_set_channel_mode(uint8_t channel, analog_mode_t mode) {
    if (!subsystem_initialized || channel >= ANALOG_NUM_CHANNELS) {
        return false;
    }

    /* Skip if already in the requested mode */
    if (current_status.channel_mode[channel] == mode) {
        return true;
    }

    /* Stop acquisition temporarily for safe switching */
    bool was_acquiring = acquisition_active;
    if (was_acquiring) {
        drv_ads1274_stop();
        acquisition_active = false;
    }

    bool ok = true;

    if (mode == ANALOG_MODE_IEPE) {
        /* Switching to IEPE: set TMUX first, then enable current source */
        ok = drv_tmux1101_set_mode(&tmux_handles[channel], TMUX1101_MODE_IEPE);
        if (ok) {
            ok = drv_iepe_enable(&iepe_handles[channel]);
        }
    } else {
        /* Switching to DC: disable current source first, then set TMUX */
        ok = drv_iepe_disable(&iepe_handles[channel]);
        if (ok) {
            ok = drv_tmux1101_set_mode(&tmux_handles[channel], TMUX1101_MODE_DC);
        }
    }

    /* Wait for analog settling */
    if (ok) {
        bsp_delay_ms(ESOCORE_IEPE_SETTLING_MS);
        current_status.channel_mode[channel] = mode;
        current_config.channel_config[channel].mode = mode;
    } else {
        current_status.channel_state[channel] = ANALOG_CH_ERROR;
    }

    /* Resume acquisition if it was running */
    if (was_acquiring) {
        drv_ads1274_start_continuous();
        acquisition_active = true;
    }

    return ok;
}

bool analog_input_set_gain(uint8_t channel, analog_gain_t gain) {
    if (!subsystem_initialized || channel >= ANALOG_NUM_CHANNELS) {
        return false;
    }

    if (gain >= ANALOG_GAIN_COUNT) {
        return false;
    }

    if (!drv_pga280_set_gain(&pga_handles[channel], gain_map[gain])) {
        return false;
    }

    current_status.channel_gain[channel] = gain;
    current_config.channel_config[channel].gain = gain;
    return true;
}

bool analog_input_start_acquisition(void) {
    if (!subsystem_initialized || acquisition_active) {
        return false;
    }

    /* Reset processing state */
    for (uint8_t ch = 0; ch < ANALOG_NUM_CHANNELS; ch++) {
        sample_index[ch] = 0;
        if (current_status.channel_state[ch] == ANALOG_CH_IDLE ||
            current_status.channel_state[ch] == ANALOG_CH_ERROR) {
            current_status.channel_state[ch] = ANALOG_CH_ACQUIRING;
        }
    }

    if (!drv_ads1274_start_continuous()) {
        return false;
    }

    acquisition_active = true;
    return true;
}

bool analog_input_stop_acquisition(void) {
    if (!subsystem_initialized || !acquisition_active) {
        return false;
    }

    drv_ads1274_stop();
    acquisition_active = false;

    /* Transition active channels back to idle */
    for (uint8_t ch = 0; ch < ANALOG_NUM_CHANNELS; ch++) {
        if (current_status.channel_state[ch] == ANALOG_CH_ACQUIRING) {
            current_status.channel_state[ch] = ANALOG_CH_IDLE;
        }
    }

    return true;
}

bool analog_input_get_status(analog_status_t *status) {
    if (!subsystem_initialized || !status) {
        return false;
    }

    memcpy(status, &current_status, sizeof(analog_status_t));
    return true;
}

bool analog_input_register_callback(analog_data_callback_t callback, void *ctx) {
    if (!subsystem_initialized) {
        return false;
    }

    user_callback = callback;
    user_callback_ctx = ctx;
    return true;
}

bool analog_input_auto_calibrate(uint8_t channel) {
    if (!subsystem_initialized || channel >= ANALOG_NUM_CHANNELS) {
        return false;
    }

    /* Stop acquisition during calibration */
    bool was_acquiring = acquisition_active;
    if (was_acquiring) {
        drv_ads1274_stop();
        acquisition_active = false;
    }

    analog_gain_t best_gain = ANALOG_GAIN_1X;
    float best_ratio = 0.0f;

    /* Test each gain setting from lowest to highest */
    for (uint8_t g = 0; g < ANALOG_GAIN_COUNT; g++) {
        drv_pga280_set_gain(&pga_handles[channel], gain_map[g]);
        bsp_delay_ms(ESOCORE_IEPE_SETTLING_MS);

        /* Take a short burst of samples */
        int32_t peak_val = 0;
        for (uint32_t i = 0; i < AUTO_CAL_SAMPLES; i++) {
            ads1274_sample_t sample;
            if (drv_ads1274_read_single(&sample)) {
                int32_t abs_val = (sample.channels[channel] < 0)
                                  ? -sample.channels[channel]
                                  : sample.channels[channel];
                if (abs_val > peak_val) {
                    peak_val = abs_val;
                }
            }
        }

        float ratio = (float)peak_val / (float)ADC_FULL_SCALE;

        /* Select the gain that brings the signal closest to 70% */
        if (ratio <= AUTO_CAL_TARGET_RATIO) {
            if (ratio > best_ratio) {
                best_ratio = ratio;
                best_gain = (analog_gain_t)g;
            }
        } else {
            /* Signal would clip at this gain -- stop escalating */
            break;
        }
    }

    /* Apply the best gain */
    drv_pga280_set_gain(&pga_handles[channel], gain_map[best_gain]);
    current_status.channel_gain[channel] = best_gain;
    current_config.channel_config[channel].gain = best_gain;

    /* Resume acquisition if it was running */
    if (was_acquiring) {
        drv_ads1274_start_continuous();
        acquisition_active = true;
    }

    return true;
}

bool analog_input_check_sensor(uint8_t channel) {
    if (!subsystem_initialized || channel >= ANALOG_NUM_CHANNELS) {
        return false;
    }

    /* Take a single-shot reading */
    ads1274_sample_t sample;
    if (!drv_ads1274_read_single(&sample)) {
        return false;
    }

    int32_t raw = sample.channels[channel];
    float voltage = (float)raw / (float)ADC_FULL_SCALE
                    * (float)ESOCORE_RAIL_3V3A_NOMINAL_MV / 1000.0f;

    if (current_status.channel_mode[channel] == ANALOG_MODE_IEPE) {
        /*
         * IEPE sensor detection: the 4 mA excitation creates a DC bias
         * across the sensor. A connected sensor should show 8-12 V bias.
         * Scale the measured voltage by the gain to get the actual bias.
         */
        float gain = gain_multiplier[current_status.channel_gain[channel]];
        float bias_v = voltage / gain;

        /* Account for the external signal conditioning ratio */
        if (bias_v < IEPE_BIAS_MIN_V || bias_v > IEPE_BIAS_MAX_V) {
            return false;   /* Open or shorted sensor */
        }
    } else {
        /* DC mode: check for stuck-at-zero or overrange */
        int32_t abs_raw = (raw < 0) ? -raw : raw;

        if (abs_raw == 0) {
            return false;   /* Input stuck at zero -- likely disconnected */
        }
        if (abs_raw >= ADC_FULL_SCALE) {
            return false;   /* Overrange -- possible short or excessive signal */
        }
    }

    return true;
}
