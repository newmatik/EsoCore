/**
 * @file analog_input.h
 * @brief 4-Channel Dual-Mode Analog Input Manager
 *
 * Coordinates the TMUX1101 analog switch, IEPE current source, PGA280
 * programmable gain amplifier, and ADS1274 24-bit ADC to provide a
 * unified 4-channel analog acquisition interface. Each channel can
 * operate independently in IEPE (AC-coupled) or DC (direct) mode.
 *
 * Signal chain per channel:
 *   Sensor -> TMUX1101 (AC/DC select) -> PGA280 (gain) -> ADS1274 (ADC)
 *                        |
 *              IEPE 4 mA source (optional)
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_ANALOG_INPUT_H
#define ESOCORE_ANALOG_INPUT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Constants
 * ============================================================================ */

#define ANALOG_NUM_CHANNELS     4   /**< Number of analog input channels */

/* ============================================================================
 * Channel Mode
 * ============================================================================ */

/**
 * @brief Analog input channel mode
 *
 * Determines the signal routing path through the TMUX1101 analog switch
 * and whether the IEPE constant current source is active.
 */
typedef enum {
    ANALOG_MODE_IEPE = 0,   /**< AC-coupled IEPE mode (4 mA excitation enabled) */
    ANALOG_MODE_DC   = 1,   /**< DC-coupled direct voltage mode */
} analog_mode_t;

/* ============================================================================
 * Channel State
 * ============================================================================ */

/**
 * @brief Analog channel operating state
 */
typedef enum {
    ANALOG_CH_DISABLED  = 0,    /**< Channel is disabled */
    ANALOG_CH_IDLE      = 1,    /**< Channel is configured but not acquiring */
    ANALOG_CH_ACQUIRING = 2,    /**< Channel is actively acquiring data */
    ANALOG_CH_ERROR     = 3,    /**< Channel has encountered an error */
} analog_channel_state_t;

/* ============================================================================
 * Gain Settings
 * ============================================================================ */

/**
 * @brief Application-level gain settings
 *
 * These are mapped internally to the appropriate PGA280 gain codes.
 * Values correspond to BSP ESOCORE_PGA_GAIN_* definitions.
 */
typedef enum {
    ANALOG_GAIN_1X  = 0,    /**< Unity gain */
    ANALOG_GAIN_2X  = 1,    /**< 2x gain */
    ANALOG_GAIN_5X  = 2,    /**< 5x gain (nearest PGA280 binary step) */
    ANALOG_GAIN_10X = 3,    /**< 10x gain (nearest PGA280 binary step) */
    ANALOG_GAIN_20X = 4,    /**< 20x gain (nearest PGA280 binary step) */
    ANALOG_GAIN_COUNT = 5,
} analog_gain_t;

/* ============================================================================
 * Trigger Mode
 * ============================================================================ */

/**
 * @brief Acquisition trigger mode
 */
typedef enum {
    ANALOG_TRIGGER_CONTINUOUS = 0,  /**< Free-running continuous acquisition */
    ANALOG_TRIGGER_SINGLE     = 1,  /**< Single-shot block acquisition */
    ANALOG_TRIGGER_THRESHOLD  = 2,  /**< Trigger when signal exceeds threshold */
} analog_trigger_mode_t;

/* ============================================================================
 * Configuration Structures
 * ============================================================================ */

/**
 * @brief Per-channel configuration
 */
typedef struct {
    analog_mode_t           mode;           /**< IEPE or DC mode */
    analog_gain_t           gain;           /**< PGA gain setting */
    analog_trigger_mode_t   trigger_mode;   /**< Acquisition trigger mode */
    float                   threshold;      /**< Trigger threshold (engineering units) */
} analog_channel_config_t;

/**
 * @brief Processed data block from a single channel
 *
 * Delivered to the user callback after the ADC data has been captured
 * and basic signal statistics have been computed.
 */
typedef struct {
    int32_t    *raw_samples;    /**< Pointer to raw 24-bit ADC samples (sign-extended) */
    uint32_t    sample_count;   /**< Number of samples in this block */
    float       rms;            /**< RMS value of the block */
    float       peak;           /**< Peak absolute value */
    float       peak_to_peak;   /**< Peak-to-peak amplitude */
    float       crest_factor;   /**< Peak / RMS ratio */
    float       dc_offset;      /**< Mean (DC offset) of the block */
    uint32_t    timestamp_us;   /**< Timestamp of first sample (microseconds) */
} analog_processed_data_t;

/**
 * @brief System-wide analog input configuration
 */
typedef struct {
    analog_channel_config_t channel_config[ANALOG_NUM_CHANNELS];
    uint32_t                sample_rate_hz;     /**< ADC sample rate in Hz */
    uint32_t                samples_per_block;  /**< Samples per processing block */
} analog_config_t;

/**
 * @brief System status snapshot
 */
typedef struct {
    analog_channel_state_t  channel_state[ANALOG_NUM_CHANNELS];
    analog_mode_t           channel_mode[ANALOG_NUM_CHANNELS];
    analog_gain_t           channel_gain[ANALOG_NUM_CHANNELS];
    uint32_t                total_samples;  /**< Total samples acquired since start */
    uint32_t                overrange_count[ANALOG_NUM_CHANNELS];
} analog_status_t;

/* ============================================================================
 * Callback Type
 * ============================================================================ */

/**
 * @brief Data-ready callback
 *
 * Invoked when a complete block of processed data is available for
 * a channel. Called from ADC interrupt/DMA context -- keep processing
 * brief or defer to a task.
 *
 * @param channel  Channel index (0-3)
 * @param data     Pointer to processed data block
 * @param ctx      User-provided context pointer
 */
typedef void (*analog_data_callback_t)(uint8_t channel,
                                       analog_processed_data_t *data,
                                       void *ctx);

/* ============================================================================
 * Public API
 * ============================================================================ */

/**
 * @brief Initialize the analog input subsystem
 *
 * Initializes all 4 TMUX1101 switches, IEPE current sources, PGA280
 * amplifiers, and the ADS1274 ADC using pin definitions from
 * pin_mapping_v150.h. Channels are left in the idle state.
 *
 * @param config  Pointer to system configuration
 * @return true on success, false on initialization failure
 */
bool analog_input_init(const analog_config_t *config);

/**
 * @brief Shutdown the analog input subsystem
 *
 * Stops acquisition, disables all IEPE current sources, and releases
 * driver resources.
 */
void analog_input_deinit(void);

/**
 * @brief Set the operating mode for a channel
 *
 * Handles the proper TMUX1101 and IEPE sequencing:
 *   - IEPE: set TMUX to IEPE first, then enable IEPE current source
 *   - DC:   disable IEPE current source first, then set TMUX to DC
 * Waits for the BSP-defined settling time after switching.
 *
 * @param channel  Channel index (0-3)
 * @param mode     Desired mode (ANALOG_MODE_IEPE or ANALOG_MODE_DC)
 * @return true on success
 */
bool analog_input_set_channel_mode(uint8_t channel, analog_mode_t mode);

/**
 * @brief Set the PGA gain for a channel
 *
 * Delegates to the appropriate PGA280 driver instance.
 *
 * @param channel  Channel index (0-3)
 * @param gain     Desired gain setting
 * @return true on success
 */
bool analog_input_set_gain(uint8_t channel, analog_gain_t gain);

/**
 * @brief Start acquisition on all enabled channels
 *
 * Initiates continuous DMA-based ADC sampling via the ADS1274.
 *
 * @return true on success
 */
bool analog_input_start_acquisition(void);

/**
 * @brief Stop acquisition on all channels
 *
 * @return true on success
 */
bool analog_input_stop_acquisition(void);

/**
 * @brief Get a snapshot of the subsystem status
 *
 * @param status  Pointer to status structure to fill
 * @return true on success
 */
bool analog_input_get_status(analog_status_t *status);

/**
 * @brief Register the data-ready callback
 *
 * @param callback  Function to call when a data block is ready
 * @param ctx       User context pointer passed to the callback
 * @return true on success
 */
bool analog_input_register_callback(analog_data_callback_t callback, void *ctx);

/**
 * @brief Auto-calibrate gain for a channel
 *
 * Tests each gain setting and selects the one that brings the signal
 * level to approximately 70% of full scale for optimal dynamic range.
 *
 * @param channel  Channel index (0-3)
 * @return true on success, false if calibration failed
 */
bool analog_input_auto_calibrate(uint8_t channel);

/**
 * @brief Check sensor connection status
 *
 * In IEPE mode: verifies the DC bias voltage is within the expected
 * 8-12 V range (sensor connected and functional).
 * In DC mode: checks for stuck-at-zero or overrange conditions.
 *
 * @param channel  Channel index (0-3)
 * @return true if sensor is connected and healthy
 */
bool analog_input_check_sensor(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_ANALOG_INPUT_H */
