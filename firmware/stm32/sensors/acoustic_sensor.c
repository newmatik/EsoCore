/**
 * @file acoustic_sensor.c
 * @brief Acoustic Sensor Driver Implementation
 *
 * This file contains the implementation of the acoustic sensor driver with
 * dual-frequency monitoring (audible + ultrasonic) using ICS-43434 microphone
 * and MA40S4R ultrasound transducer.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "acoustic_sensor.h"
#include "../../common/sensors/sensor_interface.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================================
 * Private Data Structures and Constants
 * ============================================================================ */

static acoustic_sensor_config_t sensor_config;
static acoustic_sensor_status_t sensor_status;
static bool sensor_initialized = false;

/* Audio buffers */
static int16_t audio_buffer[ACOUSTIC_SENSOR_MAX_SAMPLES];
static uint32_t buffer_index = 0;
static bool buffer_full = false;

/* FFT working buffers */
static float fft_input_buffer[ACOUSTIC_SENSOR_FFT_SIZE];
static float fft_output_buffer[ACOUSTIC_SENSOR_FFT_SIZE];

/* Window functions */
static float hanning_window[ACOUSTIC_SENSOR_FFT_SIZE];
static float hamming_window[ACOUSTIC_SENSOR_FFT_SIZE];
static float blackman_window[ACOUSTIC_SENSOR_FFT_SIZE];

/* A-weighting filter coefficients */
static const float a_weighting_coeffs[5] = {
    1.0f,    /* DC gain */
    -4.019f, /* Pole 1 */
    6.189f,  /* Pole 2 */
    -3.789f, /* Pole 3 */
    0.618f   /* Pole 4 */
};

/* Ultrasound detection */
static const uint32_t ULTRASOUND_LOW_FREQ = 20000;  /* 20kHz */
static const uint32_t ULTRASOUND_HIGH_FREQ = 80000; /* 80kHz */
static const float ULTRASOUND_THRESHOLD_DB = 40.0f;

/* Calibration data */
static struct {
    float microphone_sensitivity_mv_pa;
    float ultrasound_sensitivity_mv_pa;
    int16_t microphone_offset;
    int16_t ultrasound_offset;
    bool valid;
} calibration_data;

/* ============================================================================
 * Hardware Abstraction Layer - ICS-43434 Digital MEMS Microphone
 * ============================================================================ */

/**
 * @brief Initialize ICS-43434 digital microphone
 */
static bool ics43434_init(void) {
    /* TODO: Implement ICS-43434 initialization */
    /* This would typically involve:
     * - Configuring I2S/PDM interface
     * - Setting sample rate (48kHz for wide bandwidth)
     * - Configuring clock source
     * - Setting gain levels
     * - Enabling microphone
     */

    /* Placeholder for hardware initialization */
    printf("Initializing ICS-43434 digital microphone...\n");
    return true;
}

/**
 * @brief Read audio samples from ICS-43434
 */
static bool ics43434_read_samples(int16_t *buffer, uint32_t num_samples) {
    /* TODO: Implement PDM/I2S data reading */
    /* This would typically involve:
     * - Reading from PDM interface
     * - Converting PDM to PCM
     * - Applying DC blocking filter
     * - Outputting 16-bit signed samples
     */

    /* Placeholder: Generate test data */
    for (uint32_t i = 0; i < num_samples; i++) {
        /* Simulate microphone signal with some noise and signals */
        float time = (float)(buffer_index + i) / 48000.0f;
        float signal = 1000.0f * sinf(2.0f * M_PI * 1000.0f * time); /* 1kHz tone */
        signal += 500.0f * sinf(2.0f * M_PI * 100.0f * time);        /* 100Hz tone */
        signal += (float)(rand() % 100) - 50.0f;                    /* Noise */
        buffer[i] = (int16_t)signal;
    }

    return true;
}

/**
 * @brief Configure ICS-43434 microphone parameters
 */
static bool ics43434_configure(float sensitivity_mv_pa, uint32_t sample_rate_hz) {
    /* TODO: Implement microphone configuration */
    /* This would typically involve:
     * - Setting sensitivity calibration
     * - Configuring sample rate
     * - Setting filter parameters
     * - Adjusting gain
     */

    calibration_data.microphone_sensitivity_mv_pa = sensitivity_mv_pa;
    return true;
}

/* ============================================================================
 * Hardware Abstraction Layer - MA40S4R Ultrasound Transducer
 * ============================================================================ */

/**
 * @brief Initialize MA40S4R ultrasound transducer
 */
static bool ma40s4r_init(void) {
    /* TODO: Implement MA40S4R initialization */
    /* This would typically involve:
     * - Configuring ADC for ultrasound signal
     * - Setting up amplifier gain
     * - Configuring bandpass filter (30-50kHz)
     * - Enabling transducer power
     */

    /* Placeholder for hardware initialization */
    printf("Initializing MA40S4R ultrasound transducer...\n");
    return true;
}

/**
 * @brief Read ultrasound samples from MA40S4R
 */
static bool ma40s4r_read_samples(int16_t *buffer, uint32_t num_samples) {
    /* TODO: Implement ADC reading for ultrasound */
    /* This would typically involve:
     * - Reading from ADC channel
     * - Applying bandpass filtering
     * - Converting to digital samples
     * - Outputting 16-bit signed samples
     */

    /* Placeholder: Generate test ultrasound data */
    for (uint32_t i = 0; i < num_samples; i++) {
        /* Simulate ultrasound signal at 40kHz */
        float time = (float)(buffer_index + i) / 48000.0f;
        float signal = 800.0f * sinf(2.0f * M_PI * 40000.0f * time); /* 40kHz tone */
        signal += (float)(rand() % 50) - 25.0f;                     /* Low noise */
        buffer[i] = (int16_t)signal;
    }

    return true;
}

/**
 * @brief Configure MA40S4R ultrasound parameters
 */
static bool ma40s4r_configure(float sensitivity_mv_pa) {
    /* TODO: Implement ultrasound configuration */
    /* This would typically involve:
     * - Setting sensitivity calibration
     * - Configuring amplifier gain
     * - Setting filter bandwidth
     * - Adjusting threshold levels
     */

    calibration_data.ultrasound_sensitivity_mv_pa = sensitivity_mv_pa;
    return true;
}

/* ============================================================================
 * Digital Signal Processing Functions
 * ============================================================================ */

/**
 * @brief Initialize FFT window functions
 */
static void initialize_fft_windows(void) {
    for (uint16_t i = 0; i < ACOUSTIC_SENSOR_FFT_SIZE; i++) {
        float ratio = (float)i / (float)(ACOUSTIC_SENSOR_FFT_SIZE - 1);

        /* Hanning window */
        hanning_window[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * ratio));

        /* Hamming window */
        hamming_window[i] = 0.54f - 0.46f * cosf(2.0f * M_PI * ratio);

        /* Blackman window */
        blackman_window[i] = 0.42f - 0.5f * cosf(2.0f * M_PI * ratio) +
                           0.08f * cosf(4.0f * M_PI * ratio);
    }
}

/**
 * @brief Apply window function to audio data
 */
static void apply_window(float *data, uint16_t size, uint8_t window_type) {
    float *window = NULL;

    switch (window_type) {
        case 0: window = hanning_window; break;
        case 1: window = hamming_window; break;
        case 2: window = blackman_window; break;
        default: return;
    }

    for (uint16_t i = 0; i < size; i++) {
        data[i] *= window[i];
    }
}

/**
 * @brief Simplified FFT implementation (Radix-2 Cooley-Tukey)
 */
static bool perform_fft(float *input, float *output, uint16_t size) {
    if (size != ACOUSTIC_SENSOR_FFT_SIZE) {
        return false;
    }

    /* TODO: Implement complete FFT algorithm */
    /* This is a simplified placeholder */

    /* Copy input to output with some frequency domain processing */
    for (uint16_t i = 0; i < size / 2; i++) {
        output[i] = fabsf(input[i * 2]) + fabsf(input[i * 2 + 1]);
    }

    return true;
}

/**
 * @brief Calculate frequency domain parameters
 */
static void calculate_fft_parameters(const float *fft_data, acoustic_fft_data_t *result,
                                   uint32_t sampling_rate) {
    float max_amplitude = 0.0f;
    uint16_t max_index = 0;
    float total_power = 0.0f;
    float spectral_centroid = 0.0f;
    float weighted_sum = 0.0f;

    /* Find dominant frequency */
    for (uint16_t i = 0; i < ACOUSTIC_SENSOR_FFT_SIZE / 2; i++) {
        result->frequency_bins[i] = fft_data[i];

        if (fft_data[i] > max_amplitude) {
            max_amplitude = fft_data[i];
            max_index = i;
        }

        total_power += fft_data[i] * fft_data[i];
    }

    result->dominant_amplitude = max_amplitude;
    result->dominant_frequency_hz = (uint16_t)((float)max_index * (float)sampling_rate /
                                              (float)ACOUSTIC_SENSOR_FFT_SIZE);
    result->total_power = total_power;

    /* Calculate spectral centroid */
    for (uint16_t i = 1; i < ACOUSTIC_SENSOR_FFT_SIZE / 2; i++) {
        float frequency = (float)i * (float)sampling_rate / (float)ACOUSTIC_SENSOR_FFT_SIZE;
        weighted_sum += frequency * fft_data[i];
    }

    if (total_power > 0) {
        result->spectral_centroid = weighted_sum / total_power;
    } else {
        result->spectral_centroid = 0.0f;
    }

    /* Calculate additional spectral parameters */
    result->spectral_rolloff = 0.0f; /* TODO: Implement */
    result->tonality_index = 0.0f;   /* TODO: Implement */
}

/* ============================================================================
 * A-Weighting Filter Implementation
 * ============================================================================ */

/**
 * @brief Apply A-weighting filter to audio data
 */
static bool apply_a_weighting(const float *input, float *output, uint32_t length,
                             uint32_t sampling_rate) {
    /* TODO: Implement complete A-weighting filter */
    /* This is a simplified placeholder that approximates A-weighting */

    /* A-weighting frequency response approximation */
    for (uint32_t i = 0; i < length; i++) {
        /* Simplified A-weighting: boost low frequencies, cut high frequencies */
        float freq_response = 1.0f;

        /* Approximate A-weighting curve */
        if (i < length / 2) {
            float freq = (float)i * (float)sampling_rate / (float)length;
            if (freq > 20.0f && freq < 20000.0f) {
                /* Simplified A-weighting approximation */
                if (freq < 1000.0f) {
                    freq_response = 1.0f + (1000.0f - freq) / 5000.0f;
                } else {
                    freq_response = 1.0f - (freq - 1000.0f) / 19000.0f;
                    freq_response = fmaxf(freq_response, 0.1f);
                }
            }
        }

        output[i] = input[i] * freq_response;
    }

    return true;
}

/* ============================================================================
 * Sound Pressure Level Calculations
 * ============================================================================ */

/**
 * @brief Calculate RMS sound pressure level
 */
static float calculate_rms_spl(const int16_t *audio_data, uint32_t length,
                              float sensitivity_mv_pa, uint32_t sampling_rate) {
    /* Calculate RMS of the audio signal */
    float sum_squares = 0.0f;

    for (uint32_t i = 0; i < length; i++) {
        float voltage_mv = (float)audio_data[i] * 3.3f / 32768.0f; /* ADC to voltage */
        float pressure_pa = voltage_mv / sensitivity_mv_pa * 1000.0f; /* Voltage to pressure */
        sum_squares += pressure_pa * pressure_pa;
    }

    float rms_pressure = sqrtf(sum_squares / (float)length);

    /* Convert to dB SPL (20µPa reference) */
    if (rms_pressure > 0) {
        return 20.0f * log10f(rms_pressure / 20e-6f);
    } else {
        return 0.0f;
    }
}

/**
 * @brief Calculate peak sound pressure level
 */
static float calculate_peak_spl(const int16_t *audio_data, uint32_t length,
                               float sensitivity_mv_pa) {
    float max_pressure = 0.0f;

    for (uint32_t i = 0; i < length; i++) {
        float voltage_mv = fabsf((float)audio_data[i]) * 3.3f / 32768.0f;
        float pressure_pa = voltage_mv / sensitivity_mv_pa * 1000.0f;
        if (pressure_pa > max_pressure) {
            max_pressure = pressure_pa;
        }
    }

    /* Convert to dB SPL */
    if (max_pressure > 0) {
        return 20.0f * log10f(max_pressure / 20e-6f);
    } else {
        return 0.0f;
    }
}

/**
 * @brief Calculate crest factor
 */
static float calculate_crest_factor(float rms_level, float peak_level) {
    if (rms_level > 0) {
        return peak_level / rms_level;
    } else {
        return 0.0f;
    }
}

/* ============================================================================
 * Ultrasound Detection
 * ============================================================================ */

/**
 * @brief Detect ultrasound presence in frequency spectrum
 */
static void detect_ultrasound(const acoustic_fft_data_t *fft_data,
                             acoustic_ultrasound_data_t *ultrasound_data,
                             uint32_t sampling_rate) {
    /* Find frequency bins corresponding to ultrasound range */
    uint16_t low_bin = (uint16_t)((float)ULTRASOUND_LOW_FREQ * ACOUSTIC_SENSOR_FFT_SIZE / sampling_rate);
    uint16_t high_bin = (uint16_t)((float)ULTRASOUND_HIGH_FREQ * ACOUSTIC_SENSOR_FFT_SIZE / sampling_rate);

    if (high_bin >= ACOUSTIC_SENSOR_FFT_SIZE / 2) {
        high_bin = ACOUSTIC_SENSOR_FFT_SIZE / 2 - 1;
    }

    float ultrasound_power = 0.0f;
    float max_ultrasound_amplitude = 0.0f;
    uint16_t max_ultrasound_index = 0;

    /* Sum power in ultrasound frequency range */
    for (uint16_t i = low_bin; i <= high_bin && i < ACOUSTIC_SENSOR_FFT_SIZE / 2; i++) {
        ultrasound_power += fft_data->frequency_bins[i] * fft_data->frequency_bins[i];

        if (fft_data->frequency_bins[i] > max_ultrasound_amplitude) {
            max_ultrasound_amplitude = fft_data->frequency_bins[i];
            max_ultrasound_index = i;
        }
    }

    /* Calculate ultrasound level in dB */
    if (ultrasound_power > 0) {
        ultrasound_data->ultrasound_level_db = 10.0f * log10f(ultrasound_power);
    } else {
        ultrasound_data->ultrasound_level_db = 0.0f;
    }

    /* Determine if ultrasound is detected */
    ultrasound_data->ultrasound_detected = (ultrasound_data->ultrasound_level_db > ULTRASOUND_THRESHOLD_DB);

    if (ultrasound_data->ultrasound_detected) {
        ultrasound_data->ultrasound_frequency_hz = (uint16_t)((float)max_ultrasound_index *
                                                             (float)sampling_rate / ACOUSTIC_SENSOR_FFT_SIZE);
        ultrasound_data->ultrasound_bandwidth_hz = ULTRASOUND_HIGH_FREQ - ULTRASOUND_LOW_FREQ;

        /* Determine ultrasound type based on frequency */
        if (ultrasound_data->ultrasound_frequency_hz < 25000) {
            ultrasound_data->ultrasound_type = 0; /* Leak detection */
        } else if (ultrasound_data->ultrasound_frequency_hz < 50000) {
            ultrasound_data->ultrasound_type = 1; /* Bearing monitoring */
        } else {
            ultrasound_data->ultrasound_type = 2; /* PD detection */
        }
    } else {
        ultrasound_data->ultrasound_frequency_hz = 0;
        ultrasound_data->ultrasound_bandwidth_hz = 0;
        ultrasound_data->ultrasound_type = 0;
    }
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

/**
 * @brief Initialize acoustic sensor
 */
bool acoustic_sensor_init(const acoustic_sensor_config_t *config) {
    if (!config || sensor_initialized) {
        return false;
    }

    /* Copy configuration */
    sensor_config = *config;

    /* Initialize hardware */
    if (!ics43434_init() || !ma40s4r_init()) {
        return false;
    }

    /* Configure sensors */
    if (!ics43434_configure(sensor_config.sensitivity_mv_pa, sensor_config.base_config.sampling_rate_hz) ||
        !ma40s4r_configure(sensor_config.sensitivity_mv_pa)) {
        return false;
    }

    /* Initialize FFT windows */
    initialize_fft_windows();

    /* Initialize status */
    memset(&sensor_status, 0, sizeof(acoustic_sensor_status_t));
    sensor_status.base_status.sensor_type = ESOCORE_SENSOR_ACOUSTIC;
    sensor_status.base_status.sensor_id = sensor_config.base_config.sensor_id;
    sensor_status.base_status.current_mode = ESOCORE_MODE_STANDBY;
    sensor_status.microphone_ready = true;
    sensor_status.adc_ready = true;

    /* Initialize calibration */
    memset(&calibration_data, 0, sizeof(calibration_data));
    calibration_data.valid = false;

    sensor_initialized = true;
    return true;
}

/**
 * @brief Deinitialize acoustic sensor
 */
bool acoustic_sensor_deinit(void) {
    if (!sensor_initialized) {
        return false;
    }

    /* Stop acquisition if active */
    if (sensor_status.base_status.current_mode == ESOCORE_MODE_ACTIVE) {
        acoustic_sensor_stop_measurement();
    }

    /* TODO: Deinitialize hardware */

    sensor_initialized = false;
    return true;
}

/**
 * @brief Configure acoustic sensor
 */
bool acoustic_sensor_configure(const acoustic_sensor_config_t *config) {
    if (!sensor_initialized || !config) {
        return false;
    }

    sensor_config = *config;
    return ics43434_configure(sensor_config.sensitivity_mv_pa, sensor_config.base_config.sampling_rate_hz) &&
           ma40s4r_configure(sensor_config.sensitivity_mv_pa);
}

/**
 * @brief Start acoustic measurement
 */
bool acoustic_sensor_start_measurement(void) {
    if (!sensor_initialized) {
        return false;
    }

    /* Reset buffer */
    buffer_index = 0;
    buffer_full = false;

    /* Update status */
    sensor_status.base_status.current_mode = ESOCORE_MODE_ACTIVE;
    sensor_status.base_status.is_initialized = true;
    sensor_status.samples_acquired = 0;

    return true;
}

/**
 * @brief Stop acoustic measurement
 */
bool acoustic_sensor_stop_measurement(void) {
    if (!sensor_initialized) {
        return false;
    }

    /* Update status */
    sensor_status.base_status.current_mode = ESOCORE_MODE_STANDBY;

    return true;
}

/**
 * @brief Read acoustic sensor data
 */
bool acoustic_sensor_read_data(acoustic_sensor_data_t *data, uint32_t timeout_ms) {
    if (!sensor_initialized || !data) {
        return false;
    }

    /* Read audio samples from both microphone and ultrasound sensor */
    if (!ics43434_read_samples(audio_buffer, ACOUSTIC_SENSOR_MAX_SAMPLES)) {
        return false;
    }

    /* Copy raw data */
    memcpy(data->raw_data.audio_samples, audio_buffer, sizeof(audio_buffer));
    data->raw_data.sample_count = ACOUSTIC_SENSOR_MAX_SAMPLES;
    data->raw_data.timestamp = 0; /* TODO: Get actual timestamp */
    data->raw_data.temperature_celsius = 25; /* TODO: Read from sensor */

    /* Calculate SPL parameters */
    acoustic_processed_data_t processed;
    if (!acoustic_sensor_calculate_spl(audio_buffer, ACOUSTIC_SENSOR_MAX_SAMPLES,
                                      sensor_config.sensitivity_mv_pa,
                                      sensor_config.base_config.sampling_rate_hz, &processed)) {
        return false;
    }
    data->processed_data = processed;

    /* Perform FFT analysis */
    acoustic_fft_data_t fft_data;
    if (!acoustic_sensor_perform_fft(audio_buffer, ACOUSTIC_SENSOR_MAX_SAMPLES, &fft_data)) {
        return false;
    }
    data->fft_data = fft_data;

    /* Analyze frequency bands */
    acoustic_frequency_bands_t bands;
    if (!acoustic_sensor_analyze_frequency_bands(&fft_data, &bands)) {
        return false;
    }
    data->frequency_bands = bands;

    /* Detect ultrasound */
    acoustic_ultrasound_data_t ultrasound;
    acoustic_sensor_detect_ultrasound(&fft_data, &ultrasound);
    data->ultrasound_data = ultrasound;

    /* Calculate overall condition score */
    data->air_quality_score = acoustic_sensor_calculate_condition(&processed, &bands);

    /* Generate condition description */
    if (data->air_quality_score >= 90) {
        strcpy(data->condition_description, "Excellent acoustic environment");
    } else if (data->air_quality_score >= 70) {
        strcpy(data->condition_description, "Good acoustic conditions");
    } else if (data->air_quality_score >= 50) {
        strcpy(data->condition_description, "Moderate acoustic environment");
    } else if (data->air_quality_score >= 30) {
        strcpy(data->condition_description, "Poor acoustic conditions - monitor closely");
    } else {
        strcpy(data->condition_description, "Critical acoustic environment - immediate attention required");
    }

    /* Update status */
    sensor_status.samples_acquired += ACOUSTIC_SENSOR_MAX_SAMPLES;
    sensor_status.current_rms_level_db = processed.rms_level_db;
    sensor_status.current_temperature = data->raw_data.temperature_celsius;

    return true;
}

/**
 * @brief Get acoustic sensor status
 */
bool acoustic_sensor_get_status(acoustic_sensor_status_t *status) {
    if (!sensor_initialized || !status) {
        return false;
    }

    *status = sensor_status;
    return true;
}

/**
 * @brief Perform acoustic sensor self-test
 */
bool acoustic_sensor_self_test(void) {
    if (!sensor_initialized) {
        return false;
    }

    /* Test microphone */
    bool mic_test = ics43434_init();
    /* Test ultrasound sensor */
    bool ultrasound_test = ma40s4r_init();

    /* Update status */
    sensor_status.base_status.has_error = !(mic_test && ultrasound_test);
    sensor_status.base_status.error_code = (mic_test && ultrasound_test) ? 0 : 1;

    return mic_test && ultrasound_test;
}

/**
 * @brief Calibrate acoustic sensor
 */
bool acoustic_sensor_calibrate(void) {
    if (!sensor_initialized) {
        return false;
    }

    /* TODO: Implement calibration procedure */
    /* This would involve:
     * - Playing known reference signals
     * - Measuring response
     * - Calculating calibration factors
     */

    calibration_data.microphone_sensitivity_mv_pa = sensor_config.sensitivity_mv_pa;
    calibration_data.ultrasound_sensitivity_mv_pa = sensor_config.sensitivity_mv_pa;
    calibration_data.valid = true;

    /* Update status */
    sensor_status.calibration_valid = true;
    sensor_status.base_status.is_calibrated = true;

    return true;
}

/**
 * @brief Perform FFT analysis on acoustic data
 */
bool acoustic_sensor_perform_fft(const int16_t *audio_data, uint32_t data_length,
                                acoustic_fft_data_t *fft_result) {
    if (!audio_data || !fft_result || data_length == 0) {
        return false;
    }

    /* Prepare input buffer */
    memset(fft_input_buffer, 0, sizeof(fft_input_buffer));

    /* Convert to float and copy input data */
    uint32_t copy_length = (data_length < ACOUSTIC_SENSOR_FFT_SIZE) ?
                          data_length : ACOUSTIC_SENSOR_FFT_SIZE;

    for (uint32_t i = 0; i < copy_length; i++) {
        fft_input_buffer[i] = (float)audio_data[i];
    }

    /* Apply window function */
    apply_window(fft_input_buffer, ACOUSTIC_SENSOR_FFT_SIZE, sensor_config.fft_window_type);

    /* Perform FFT */
    if (!perform_fft(fft_input_buffer, fft_output_buffer, ACOUSTIC_SENSOR_FFT_SIZE)) {
        return false;
    }

    /* Calculate frequency domain parameters */
    calculate_fft_parameters(fft_output_buffer, fft_result, sensor_config.base_config.sampling_rate_hz);

    return true;
}

/**
 * @brief Calculate sound pressure level
 */
bool acoustic_sensor_calculate_spl(const int16_t *audio_data, uint32_t data_length,
                                  float sensitivity_mv_pa, uint32_t sampling_rate,
                                  acoustic_processed_data_t *processed_data) {
    if (!audio_data || !processed_data) {
        return false;
    }

    /* Calculate RMS SPL */
    processed_data->rms_level_db = calculate_rms_spl(audio_data, data_length,
                                                   sensitivity_mv_pa, sampling_rate);

    /* Calculate peak SPL */
    processed_data->peak_level_db = calculate_peak_spl(audio_data, data_length, sensitivity_mv_pa);

    /* Calculate crest factor */
    processed_data->crest_factor = calculate_crest_factor(processed_data->rms_level_db,
                                                        processed_data->peak_level_db);

    /* Apply A-weighting if enabled */
    if (sensor_config.enable_a_weighting) {
        /* Convert to float for filtering */
        float temp_buffer[ACOUSTIC_SENSOR_MAX_SAMPLES];
        for (uint32_t i = 0; i < data_length && i < ACOUSTIC_SENSOR_MAX_SAMPLES; i++) {
            temp_buffer[i] = (float)audio_data[i];
        }

        float a_weighted_buffer[ACOUSTIC_SENSOR_MAX_SAMPLES];
        if (apply_a_weighting(temp_buffer, a_weighted_buffer, data_length, sampling_rate)) {
            processed_data->a_weighted_level_db = calculate_rms_spl(
                (int16_t *)a_weighted_buffer, data_length, sensitivity_mv_pa, sampling_rate);
        } else {
            processed_data->a_weighted_level_db = processed_data->rms_level_db;
        }
    } else {
        processed_data->a_weighted_level_db = processed_data->rms_level_db;
    }

    /* Calculate equivalent continuous level */
    processed_data->equivalent_level_db = processed_data->rms_level_db;

    /* Calculate sound exposure level */
    processed_data->sound_exposure_db = processed_data->rms_level_db + 10.0f * log10f(
        (float)processed_data->measurement_duration_ms / 1000.0f);

    processed_data->measurement_duration_ms = (uint32_t)((float)data_length * 1000.0f / (float)sampling_rate);
    processed_data->timestamp = 0; /* TODO: Get actual timestamp */

    return true;
}

/**
 * @brief Apply A-weighting filter
 */
bool acoustic_sensor_apply_a_weighting(const float *frequency_data, float *a_weighted_data,
                                     uint32_t data_length, uint32_t sampling_rate) {
    return apply_a_weighting(frequency_data, a_weighted_data, data_length, sampling_rate);
}

/**
 * @brief Analyze frequency bands
 */
bool acoustic_sensor_analyze_frequency_bands(const acoustic_fft_data_t *fft_data,
                                           acoustic_frequency_bands_t *bands) {
    if (!fft_data || !bands) {
        return false;
    }

    uint32_t sampling_rate = sensor_config.base_config.sampling_rate_hz;
    float band_power[9] = {0};

    /* Define frequency band edges (IEC 61672-1) */
    const float band_edges[10] = {31.5f, 63.0f, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f};

    /* Sum power in each frequency band */
    for (uint16_t i = 1; i < ACOUSTIC_SENSOR_FFT_SIZE / 2; i++) {
        float frequency = (float)i * (float)sampling_rate / (float)ACOUSTIC_SENSOR_FFT_SIZE;
        float power = fft_data->frequency_bins[i] * fft_data->frequency_bins[i];

        /* Find which band this frequency belongs to */
        for (int band = 0; band < 9; band++) {
            if (frequency >= band_edges[band] && frequency < band_edges[band + 1]) {
                band_power[band] += power;
                break;
            }
        }
    }

    /* Convert to dB levels */
    for (int band = 0; band < 9; band++) {
        if (band_power[band] > 0) {
            float db_level = 10.0f * log10f(band_power[band]);
            switch (band) {
                case 0: bands->band_31_5_hz = db_level; break;
                case 1: bands->band_63_hz = db_level; break;
                case 2: bands->band_125_hz = db_level; break;
                case 3: bands->band_250_hz = db_level; break;
                case 4: bands->band_500_hz = db_level; break;
                case 5: bands->band_1000_hz = db_level; break;
                case 6: bands->band_2000_hz = db_level; break;
                case 7: bands->band_4000_hz = db_level; break;
                case 8: bands->band_8000_hz = db_level; break;
            }
        } else {
            switch (band) {
                case 0: bands->band_31_5_hz = 0.0f; break;
                case 1: bands->band_63_hz = 0.0f; break;
                case 2: bands->band_125_hz = 0.0f; break;
                case 3: bands->band_250_hz = 0.0f; break;
                case 4: bands->band_500_hz = 0.0f; break;
                case 5: bands->band_1000_hz = 0.0f; break;
                case 6: bands->band_2000_hz = 0.0f; break;
                case 7: bands->band_4000_hz = 0.0f; break;
                case 8: bands->band_8000_hz = 0.0f; break;
            }
        }
    }

    return true;
}

/**
 * @brief Detect ultrasound
 */
bool acoustic_sensor_detect_ultrasound(const acoustic_fft_data_t *fft_data,
                                     acoustic_ultrasound_data_t *ultrasound_data) {
    if (!fft_data || !ultrasound_data) {
        return false;
    }

    detect_ultrasound(fft_data, ultrasound_data, sensor_config.base_config.sampling_rate_hz);
    return true;
}

/**
 * @brief Calculate acoustic condition score
 */
uint8_t acoustic_sensor_calculate_condition(const acoustic_processed_data_t *processed_data,
                                          const acoustic_frequency_bands_t *bands) {
    uint8_t score = 100;

    /* Penalize based on RMS level */
    if (processed_data->rms_level_db > ACOUSTIC_THRESHOLD_DANGEROUS) {
        score -= 40;
    } else if (processed_data->rms_level_db > ACOUSTIC_THRESHOLD_LOUD) {
        score -= 20;
    } else if (processed_data->rms_level_db > ACOUSTIC_THRESHOLD_NORMAL) {
        score -= 10;
    }

    /* Penalize for high crest factor (impulsive noise) */
    if (processed_data->crest_factor > 10.0f) {
        score -= 15;
    } else if (processed_data->crest_factor > 5.0f) {
        score -= 10;
    }

    /* Check for ultrasound presence (could indicate issues) */
    /* Note: Ultrasound detection would be checked separately */

    /* Penalize for uneven frequency distribution */
    float avg_level = (bands->band_31_5_hz + bands->band_63_hz + bands->band_125_hz +
                      bands->band_250_hz + bands->band_500_hz + bands->band_1000_hz +
                      bands->band_2000_hz + bands->band_4000_hz + bands->band_8000_hz) / 9.0f;

    /* Check for tonal components (potential issues) */
    float max_deviation = 0.0f;
    float band_levels[9] = {bands->band_31_5_hz, bands->band_63_hz, bands->band_125_hz,
                           bands->band_250_hz, bands->band_500_hz, bands->band_1000_hz,
                           bands->band_2000_hz, bands->band_4000_hz, bands->band_8000_hz};

    for (int i = 0; i < 9; i++) {
        float deviation = fabsf(band_levels[i] - avg_level);
        if (deviation > max_deviation) {
            max_deviation = deviation;
        }
    }

    if (max_deviation > 15.0f) {
        score -= 10; /* Uneven spectrum */
    }

    return score;
}

/**
 * @brief Convert raw samples to pressure
 */
bool acoustic_sensor_convert_to_pressure(const int16_t *raw_samples, float *pressure_pa,
                                       uint32_t sample_count, float sensitivity_mv_pa) {
    if (!raw_samples || !pressure_pa) {
        return false;
    }

    for (uint32_t i = 0; i < sample_count; i++) {
        float voltage_mv = (float)raw_samples[i] * 3.3f / 32768.0f;
        pressure_pa[i] = voltage_mv / sensitivity_mv_pa * 1000.0f;
    }

    return true;
}

/**
 * @brief Calculate equivalent continuous sound level
 */
bool acoustic_sensor_calculate_lequ(const float *pressure_pa, uint32_t sample_count,
                                   uint32_t sampling_rate, float *lequ_db) {
    if (!pressure_pa || !lequ_db) {
        return false;
    }

    float sum_squares = 0.0f;
    for (uint32_t i = 0; i < sample_count; i++) {
        sum_squares += pressure_pa[i] * pressure_pa[i];
    }

    float rms_pressure = sqrtf(sum_squares / (float)sample_count);

    if (rms_pressure > 0) {
        *lequ_db = 20.0f * log10f(rms_pressure / 20e-6f);
    } else {
        *lequ_db = 0.0f;
    }

    return true;
}

/**
 * @brief Get acoustic sensor capabilities
 */
const esocore_sensor_capabilities_t *acoustic_sensor_get_capabilities(void) {
    static esocore_sensor_capabilities_t capabilities = {
        .sensor_type = ESOCORE_SENSOR_ACOUSTIC,
        .max_sampling_rate_hz = ACOUSTIC_SENSOR_SAMPLE_RATE_MAX,
        .min_sampling_rate_hz = ACOUSTIC_SENSOR_SAMPLE_RATE_MIN,
        .max_gain_level = 16,
        .max_data_points = ACOUSTIC_SENSOR_MAX_SAMPLES,
        .supports_calibration = true,
        .supports_temperature_compensation = true,
        .supports_self_test = true,
        .supports_sleep_mode = true,
        .supports_burst_mode = false,
        .power_consumption_ua = 2000
    };

    return &capabilities;
}

/**
 * @brief Get acoustic sensor configuration
 */
bool acoustic_sensor_get_configuration(acoustic_sensor_config_t *config) {
    if (!sensor_initialized || !config) {
        return false;
    }

    *config = sensor_config;
    return true;
}

/**
 * @brief Enable/disable A-weighting
 */
bool acoustic_sensor_enable_a_weighting(bool enable) {
    if (!sensor_initialized) {
        return false;
    }

    sensor_config.enable_a_weighting = enable;
    return true;
}

/**
 * @brief Set microphone sensitivity
 */
bool acoustic_sensor_set_sensitivity(float sensitivity_mv_pa) {
    if (!sensor_initialized) {
        return false;
    }

    sensor_config.sensitivity_mv_pa = sensitivity_mv_pa;
    return ics43434_configure(sensor_config.sensitivity_mv_pa, sensor_config.base_config.sampling_rate_hz);
}

/**
 * @brief Get current RMS level
 */
bool acoustic_sensor_get_rms_level(float *rms_level_db) {
    if (!sensor_initialized || !rms_level_db) {
        return false;
    }

    *rms_level_db = sensor_status.current_rms_level_db;
    return true;
}

/**
 * @brief Clear audio buffer
 */
bool acoustic_sensor_clear_buffer(void) {
    if (!sensor_initialized) {
        return false;
    }

    buffer_index = 0;
    buffer_full = false;
    sensor_status.samples_acquired = 0;

    return true;
}

/**
 * @brief Get acoustic diagnostics
 */
bool acoustic_sensor_get_diagnostics(uint8_t *diagnostic_data, uint16_t buffer_size) {
    if (!sensor_initialized || !diagnostic_data) {
        return false;
    }

    /* TODO: Collect diagnostic information */
    memset(diagnostic_data, 0, buffer_size);
    return true;
}

/**
 * @brief Export acoustic data
 */
bool acoustic_sensor_export_data(const acoustic_sensor_data_t *data,
                               const char *filename, uint8_t format) {
    /* TODO: Implement data export functionality */
    return false;
}
