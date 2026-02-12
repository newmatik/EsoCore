/**
 * @file signal_chain.h
 * @brief Signal Chain Configuration and Conversion Utilities
 *
 * Provides helpers for characterizing the analog signal chain and
 * performing conversions between raw ADC codes and engineering units.
 * Accounts for PGA gain, anti-alias filter cutoff, ADC resolution,
 * and full-scale voltage range.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_SIGNAL_CHAIN_H
#define ESOCORE_SIGNAL_CHAIN_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Signal Chain Parameters
 * ============================================================================ */

/**
 * @brief Complete description of one channel's signal chain
 *
 * Captures the gain, filter, ADC, and voltage parameters needed to
 * convert between raw ADC codes and input voltage.
 */
typedef struct {
    float       gain;               /**< Total signal chain gain (dimensionless) */
    uint32_t    aaf_cutoff_hz;      /**< Anti-aliasing filter -3 dB frequency */
    uint32_t    adc_sample_rate_hz; /**< ADC sample rate in Hz */
    uint8_t     adc_bits;           /**< ADC resolution in bits */
    float       full_scale_v;       /**< ADC full-scale input voltage (volts) */
} signal_chain_params_t;

/* ============================================================================
 * Public API
 * ============================================================================ */

/**
 * @brief Populate signal chain parameters for a channel
 *
 * Reads the current gain, BSP constants, and ADC configuration to
 * fill a signal_chain_params_t structure for the specified channel.
 *
 * @param channel  Channel index (0-3)
 * @param params   Pointer to parameters structure to fill
 * @return true on success, false if channel is invalid
 */
bool signal_chain_get_params(uint8_t channel, signal_chain_params_t *params);

/**
 * @brief Convert an input voltage to a raw ADC code
 *
 * Applies the signal chain gain and scales to the ADC full-scale
 * range:  raw = voltage * gain / full_scale_v * (2^(bits-1) - 1)
 *
 * @param voltage  Input voltage in volts
 * @param params   Pointer to signal chain parameters
 * @return Corresponding signed raw ADC code
 */
int32_t signal_chain_voltage_to_raw(float voltage,
                                    const signal_chain_params_t *params);

/**
 * @brief Convert a raw ADC code to input voltage
 *
 * Reverses the signal chain scaling:
 *   voltage = raw / (2^(bits-1) - 1) * full_scale_v / gain
 *
 * @param raw_value  Signed raw ADC code
 * @param params     Pointer to signal chain parameters
 * @return Corresponding input voltage in volts
 */
float signal_chain_raw_to_voltage(int32_t raw_value,
                                  const signal_chain_params_t *params);

/**
 * @brief Calculate the theoretical noise floor
 *
 * Computes the quantization-noise-limited noise floor in microvolts
 * RMS based on ADC resolution and signal chain gain:
 *   noise_uv = (full_scale_v / (2^(bits-1) - 1) / gain) / sqrt(12) * 1e6
 *
 * @param params  Pointer to signal chain parameters
 * @return Theoretical noise floor in microvolts RMS
 */
float signal_chain_calculate_noise_floor(const signal_chain_params_t *params);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_SIGNAL_CHAIN_H */
