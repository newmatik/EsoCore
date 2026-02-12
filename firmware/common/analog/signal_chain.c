/**
 * @file signal_chain.c
 * @brief Signal Chain Configuration and Conversion Utilities Implementation
 *
 * Implements raw/voltage conversions and theoretical noise floor
 * calculation for the EsoCore analog signal chain.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "signal_chain.h"
#include "analog_input.h"

#include <math.h>

/* BSP configuration for default parameters */
#include "../../stm32/stm32h7/bsp/bsp_edge_v150.h"

/* ============================================================================
 * Private Constants
 * ============================================================================ */

/** Nominal gain multipliers matching analog_gain_t enum order */
static const float gain_values[ANALOG_GAIN_COUNT] = {
    1.0f,   /* ANALOG_GAIN_1X  */
    2.0f,   /* ANALOG_GAIN_2X  */
    4.0f,   /* ANALOG_GAIN_5X  (nearest PGA280 binary step) */
    8.0f,   /* ANALOG_GAIN_10X (nearest PGA280 binary step) */
    16.0f,  /* ANALOG_GAIN_20X (nearest PGA280 binary step) */
};

/** Full-scale voltage of the analog input (3.3 V analog rail) */
#define DEFAULT_FULL_SCALE_V    ((float)ESOCORE_RAIL_3V3A_NOMINAL_MV / 1000.0f)

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

bool signal_chain_get_params(uint8_t channel, signal_chain_params_t *params) {
    if (channel >= ANALOG_NUM_CHANNELS || !params) {
        return false;
    }

    /* Query current subsystem status to get the active gain */
    analog_status_t status;
    if (!analog_input_get_status(&status)) {
        return false;
    }

    analog_gain_t gain_setting = status.channel_gain[channel];
    if (gain_setting >= ANALOG_GAIN_COUNT) {
        gain_setting = ANALOG_GAIN_1X;
    }

    params->gain               = gain_values[gain_setting];
    params->aaf_cutoff_hz      = ESOCORE_AAF_CUTOFF_HZ;
    params->adc_sample_rate_hz = ESOCORE_ADC_SAMPLE_RATE_HZ;
    params->adc_bits           = ESOCORE_ADC_BITS;
    params->full_scale_v       = DEFAULT_FULL_SCALE_V;

    return true;
}

int32_t signal_chain_voltage_to_raw(float voltage,
                                    const signal_chain_params_t *params) {
    if (!params || params->full_scale_v == 0.0f || params->adc_bits == 0) {
        return 0;
    }

    /* Maximum positive code for N-bit two's complement: 2^(N-1) - 1 */
    float max_code = (float)((1L << (params->adc_bits - 1)) - 1);

    float raw = voltage * params->gain / params->full_scale_v * max_code;

    /* Clamp to ADC range */
    if (raw > max_code) {
        raw = max_code;
    }
    if (raw < -max_code) {
        raw = -max_code;
    }

    return (int32_t)raw;
}

float signal_chain_raw_to_voltage(int32_t raw_value,
                                  const signal_chain_params_t *params) {
    if (!params || params->gain == 0.0f || params->adc_bits == 0) {
        return 0.0f;
    }

    /* Maximum positive code for N-bit two's complement: 2^(N-1) - 1 */
    float max_code = (float)((1L << (params->adc_bits - 1)) - 1);

    return (float)raw_value / max_code * params->full_scale_v / params->gain;
}

float signal_chain_calculate_noise_floor(const signal_chain_params_t *params) {
    if (!params || params->gain == 0.0f || params->adc_bits == 0) {
        return 0.0f;
    }

    /*
     * Quantization noise of an ideal N-bit ADC:
     *   LSB = full_scale / (2^(N-1) - 1)
     *   RMS noise = LSB / sqrt(12)
     *
     * Referred to the input (before gain):
     *   input_noise = RMS noise / gain
     *
     * Convert to microvolts:
     *   noise_uv = input_noise * 1e6
     */
    float max_code = (float)((1L << (params->adc_bits - 1)) - 1);
    float lsb_v = params->full_scale_v / max_code;
    float rms_noise_v = lsb_v / sqrtf(12.0f);
    float input_noise_v = rms_noise_v / params->gain;

    return input_noise_v * 1.0e6f;
}
