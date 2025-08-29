/**
 * @file tinyml_engine.h
 * @brief TinyML Engine for EsoCore Edge Device
 *
 * This file implements the TinyML engine for the EsoCore Edge device, providing
 * on-device machine learning capabilities using TensorFlow Lite Micro.
 * The engine supports real-time anomaly detection, pattern recognition,
 * and predictive maintenance for industrial applications.
 *
 * Features:
 * - TensorFlow Lite Micro integration
 * - Real-time inference with <100ms latency
 * - Multiple model support (vibration, acoustic, current analysis)
 * - Model management and updates (OTA model deployment)
 * - Anomaly detection algorithms
 * - Pattern recognition and classification
 * - Predictive maintenance algorithms
 * - Model performance monitoring
 * - Memory-efficient inference (<32KB RAM usage)
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_TINYML_ENGINE_H
#define ESOCORE_TINYML_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TinyML Engine Configuration
 * ============================================================================ */

#define TINYML_MAX_MODELS             8     /* Maximum number of models */
#define TINYML_MAX_MODEL_SIZE         (64 * 1024) /* Maximum model size (64KB) */
#define TINYML_TENSOR_ARENA_SIZE      (32 * 1024) /* Tensor arena size (32KB) */
#define TINYML_MAX_INPUT_SIZE         1024  /* Maximum input data size */
#define TINYML_MAX_OUTPUT_SIZE        256   /* Maximum output data size */
#define TINYML_INFERENCE_TIMEOUT_MS   100   /* Inference timeout (100ms) */

typedef enum {
    TINYML_MODEL_VIBRATION_ANOMALY  = 0,    /* Vibration anomaly detection */
    TINYML_MODEL_ACOUSTIC_CLASSIFIER = 1,   /* Acoustic pattern classification */
    TINYML_MODEL_CURRENT_ANALYSIS   = 2,    /* Current waveform analysis */
    TINYML_MODEL_MOTOR_HEALTH       = 3,    /* Motor health assessment */
    TINYML_MODEL_BEARING_FAULT      = 4,    /* Bearing fault detection */
    TINYML_MODEL_GEAR_ANALYSIS      = 5,    /* Gear mesh analysis */
    TINYML_MODEL_PUMP_CAVITATION    = 6,    /* Pump cavitation detection */
    TINYML_MODEL_CUSTOM             = 7,    /* Custom user model */
} tinyml_model_type_t;

typedef enum {
    TINYML_STATUS_UNINITIALIZED     = 0,    /* Engine not initialized */
    TINYML_STATUS_READY             = 1,    /* Engine ready for inference */
    TINYML_STATUS_BUSY              = 2,    /* Engine performing inference */
    TINYML_STATUS_ERROR             = 3,    /* Engine error state */
    TINYML_STATUS_UPDATING          = 4,    /* Model update in progress */
} tinyml_engine_status_t;

/* TinyML Engine Configuration */
typedef struct {
    uint32_t tensor_arena_size;             /* Tensor arena size in bytes */
    uint32_t max_models;                    /* Maximum number of models */
    uint32_t inference_timeout_ms;          /* Inference timeout */
    bool enable_model_caching;              /* Enable model caching */
    bool enable_performance_monitoring;     /* Enable performance monitoring */
    bool enable_auto_calibration;           /* Enable automatic model calibration */
    uint32_t max_inference_frequency_hz;    /* Maximum inference frequency */
    uint32_t memory_limit_kb;               /* Memory usage limit (KB) */
} tinyml_engine_config_t;

/* ============================================================================
 * Model Management Structures
 * ============================================================================ */

typedef struct {
    tinyml_model_type_t model_type;         /* Model type */
    char model_name[32];                   /* Model name */
    char model_version[16];                /* Model version string */
    uint32_t model_size_bytes;             /* Model size in bytes */
    uint32_t input_tensor_size;            /* Input tensor size */
    uint32_t output_tensor_size;           /* Output tensor size */
    uint8_t input_tensor_type;             /* Input tensor data type */
    uint8_t output_tensor_type;            /* Output tensor data type */
    uint32_t input_shape[4];               /* Input tensor shape */
    uint32_t output_shape[4];              /* Output tensor shape */
    float quantization_scale;              /* Quantization scale factor */
    int32_t quantization_zero_point;       /* Quantization zero point */
    uint32_t creation_timestamp;           /* Model creation timestamp */
    uint32_t deployment_timestamp;         /* Model deployment timestamp */
    bool is_active;                        /* Model is currently active */
    uint32_t usage_count;                  /* Number of inferences performed */
    float average_inference_time_ms;       /* Average inference time */
    float model_accuracy;                  /* Model accuracy (0-100) */
} tinyml_model_info_t;

/* ============================================================================
 * Inference Data Structures
 * ============================================================================ */

typedef struct {
    tinyml_model_type_t model_type;         /* Model to use for inference */
    void *input_data;                      /* Input data buffer */
    uint32_t input_size;                   /* Input data size */
    uint32_t input_data_type;              /* Input data type */
    uint32_t timestamp;                    /* Inference timestamp */
    char context[64];                      /* Inference context information */
} tinyml_inference_request_t;

typedef struct {
    tinyml_model_type_t model_type;         /* Model used for inference */
    void *output_data;                     /* Output data buffer */
    uint32_t output_size;                  /* Output data size */
    uint32_t output_data_type;             /* Output data type */
    float confidence_score;                /* Confidence score (0-100) */
    uint32_t inference_time_ms;            /* Inference execution time */
    uint32_t timestamp;                    /* Inference completion timestamp */
    bool success;                          /* Inference success flag */
    char error_message[64];                /* Error message if failed */
} tinyml_inference_result_t;

/* ============================================================================
 * Anomaly Detection Structures
 * ============================================================================ */

typedef enum {
    ANOMALY_TYPE_POINT      = 0,    /* Point anomaly */
    ANOMALY_TYPE_CONTEXTUAL = 1,    /* Contextual anomaly */
    ANOMALY_TYPE_COLLECTIVE = 2,    /* Collective anomaly */
} anomaly_type_t;

typedef struct {
    anomaly_type_t anomaly_type;           /* Type of anomaly detected */
    float anomaly_score;                   /* Anomaly score (0-100) */
    float confidence_level;                /* Detection confidence (0-100) */
    uint32_t detection_timestamp;          /* Detection timestamp */
    char anomaly_description[128];         /* Anomaly description */
    char recommended_action[128];          /* Recommended action */
    uint32_t severity_level;               /* Severity level (1-5) */
    char affected_component[32];           /* Affected component */
} anomaly_detection_result_t;

/* ============================================================================
 * Performance Monitoring
 * ============================================================================ */

typedef struct {
    uint32_t total_inferences;             /* Total inferences performed */
    uint32_t successful_inferences;        /* Successful inferences */
    uint32_t failed_inferences;            /* Failed inferences */
    float average_inference_time_ms;       /* Average inference time */
    float min_inference_time_ms;           /* Minimum inference time */
    float max_inference_time_ms;           /* Maximum inference time */
    uint32_t memory_usage_kb;              /* Current memory usage */
    uint32_t peak_memory_usage_kb;         /* Peak memory usage */
    float model_accuracy;                  /* Model accuracy */
    uint32_t false_positives;              /* False positive detections */
    uint32_t false_negatives;              /* False negative detections */
    float cpu_utilization_percent;         /* CPU utilization during inference */
} tinyml_performance_stats_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize TinyML engine
 *
 * @param config Pointer to TinyML engine configuration
 * @return true if initialization successful, false otherwise
 */
bool tinyml_engine_init(const tinyml_engine_config_t *config);

/**
 * @brief Deinitialize TinyML engine
 *
 * @return true if deinitialization successful, false otherwise
 */
bool tinyml_engine_deinit(void);

/**
 * @brief Load model into TinyML engine
 *
 * @param model_type Model type to load
 * @param model_data Pointer to model data
 * @param model_size Model data size
 * @param model_info Pointer to model information structure
 * @return true if model loaded successfully, false otherwise
 */
bool tinyml_load_model(tinyml_model_type_t model_type, const uint8_t *model_data,
                      uint32_t model_size, tinyml_model_info_t *model_info);

/**
 * @brief Unload model from TinyML engine
 *
 * @param model_type Model type to unload
 * @return true if model unloaded successfully, false otherwise
 */
bool tinyml_unload_model(tinyml_model_type_t model_type);

/**
 * @brief Get model information
 *
 * @param model_type Model type
 * @param model_info Pointer to model information structure to fill
 * @return true if model information retrieved successfully, false otherwise
 */
bool tinyml_get_model_info(tinyml_model_type_t model_type, tinyml_model_info_t *model_info);

/**
 * @brief Perform inference with specified model
 *
 * @param request Pointer to inference request
 * @param result Pointer to inference result structure to fill
 * @return true if inference successful, false otherwise
 */
bool tinyml_perform_inference(const tinyml_inference_request_t *request,
                             tinyml_inference_result_t *result);

/**
 * @brief Perform vibration anomaly detection
 *
 * @param vibration_data Pointer to vibration data (3-axis accelerometer)
 * @param data_length Number of data points
 * @param sampling_rate_hz Sampling rate in Hz
 * @param result Pointer to anomaly detection result
 * @return true if detection successful, false otherwise
 */
bool tinyml_detect_vibration_anomaly(const float *vibration_data, uint32_t data_length,
                                    uint32_t sampling_rate_hz, anomaly_detection_result_t *result);

/**
 * @brief Perform acoustic pattern classification
 *
 * @param audio_data Pointer to audio data
 * @param data_length Number of audio samples
 * @param sampling_rate_hz Audio sampling rate
 * @param result Pointer to classification result
 * @return true if classification successful, false otherwise
 */
bool tinyml_classify_acoustic_pattern(const int16_t *audio_data, uint32_t data_length,
                                     uint32_t sampling_rate_hz, tinyml_inference_result_t *result);

/**
 * @brief Perform motor current analysis
 *
 * @param current_data Pointer to current waveform data
 * @param data_length Number of current samples
 * @param sampling_rate_hz Current sampling rate
 * @param result Pointer to analysis result
 * @return true if analysis successful, false otherwise
 */
bool tinyml_analyze_motor_current(const float *current_data, uint32_t data_length,
                                 uint32_t sampling_rate_hz, tinyml_inference_result_t *result);

/**
 * @brief Detect bearing faults using vibration data
 *
 * @param vibration_data Pointer to vibration data
 * @param data_length Number of data points
 * @param bearing_type Bearing type identifier
 * @param result Pointer to fault detection result
 * @return true if detection successful, false otherwise
 */
bool tinyml_detect_bearing_fault(const float *vibration_data, uint32_t data_length,
                                const char *bearing_type, anomaly_detection_result_t *result);

/**
 * @brief Analyze gear mesh patterns
 *
 * @param vibration_data Pointer to vibration data
 * @param data_length Number of data points
 * @param gear_ratio Gear ratio
 * @param result Pointer to analysis result
 * @return true if analysis successful, false otherwise
 */
bool tinyml_analyze_gear_mesh(const float *vibration_data, uint32_t data_length,
                             float gear_ratio, tinyml_inference_result_t *result);

/**
 * @brief Detect pump cavitation
 *
 * @param vibration_data Pointer to vibration data
 * @param pressure_data Pointer to pressure data
 * @param data_length Number of data points
 * @param result Pointer to detection result
 * @return true if detection successful, false otherwise
 */
bool tinyml_detect_pump_cavitation(const float *vibration_data, const float *pressure_data,
                                  uint32_t data_length, anomaly_detection_result_t *result);

/**
 * @brief Get TinyML engine status
 *
 * @param status Pointer to store engine status
 * @return true if status retrieved successfully, false otherwise
 */
bool tinyml_get_engine_status(tinyml_engine_status_t *status);

/**
 * @brief Get performance statistics
 *
 * @param stats Pointer to performance statistics structure to fill
 * @return true if statistics retrieved successfully, false otherwise
 */
bool tinyml_get_performance_stats(tinyml_performance_stats_t *stats);

/**
 * @brief Clear performance statistics
 *
 * @return true if statistics cleared successfully, false otherwise
 */
bool tinyml_clear_performance_stats(void);

/**
 * @brief Update model parameters
 *
 * @param model_type Model type to update
 * @param parameters Pointer to parameter data
 * @param param_size Parameter data size
 * @return true if parameters updated successfully, false otherwise
 */
bool tinyml_update_model_parameters(tinyml_model_type_t model_type,
                                   const uint8_t *parameters, uint32_t param_size);

/**
 * @brief Calibrate model with reference data
 *
 * @param model_type Model type to calibrate
 * @param reference_data Pointer to reference data
 * @param data_length Reference data length
 * @return true if calibration successful, false otherwise
 */
bool tinyml_calibrate_model(tinyml_model_type_t model_type,
                           const float *reference_data, uint32_t data_length);

/**
 * @brief Get model health status
 *
 * @param model_type Model type to check
 * @param health_score Pointer to store health score (0-100)
 * @param issues Pointer to store number of detected issues
 * @return true if health status retrieved successfully, false otherwise
 */
bool tinyml_get_model_health(tinyml_model_type_t model_type, float *health_score, uint32_t *issues);

/**
 * @brief Enable/disable model
 *
 * @param model_type Model type
 * @param enable true to enable, false to disable
 * @return true if operation successful, false otherwise
 */
bool tinyml_enable_model(tinyml_model_type_t model_type, bool enable);

/**
 * @brief Set inference priority for model
 *
 * @param model_type Model type
 * @param priority Priority level (0-255, higher = more priority)
 * @return true if priority set successfully, false otherwise
 */
bool tinyml_set_model_priority(tinyml_model_type_t model_type, uint8_t priority);

/**
 * @brief Get supported model types
 *
 * @param supported_models Array to store supported model types
 * @param max_models Maximum number of models to list
 * @param num_models Pointer to store number of supported models
 * @return true if list retrieved successfully, false otherwise
 */
bool tinyml_get_supported_models(tinyml_model_type_t *supported_models,
                                uint32_t max_models, uint32_t *num_models);

/**
 * @brief Perform batch inference on multiple data samples
 *
 * @param requests Array of inference requests
 * @param num_requests Number of requests
 * @param results Array to store inference results
 * @return true if batch inference successful, false otherwise
 */
bool tinyml_batch_inference(const tinyml_inference_request_t *requests, uint32_t num_requests,
                           tinyml_inference_result_t *results);

/**
 * @brief Get inference queue status
 *
 * @param queued_requests Pointer to store number of queued requests
 * @param processing_requests Pointer to store number of processing requests
 * @param completed_requests Pointer to store number of completed requests
 * @return true if status retrieved successfully, false otherwise
 */
bool tinyml_get_queue_status(uint32_t *queued_requests, uint32_t *processing_requests,
                            uint32_t *completed_requests);

/**
 * @brief Set anomaly detection threshold
 *
 * @param model_type Model type
 * @param threshold Anomaly detection threshold (0-100)
 * @return true if threshold set successfully, false otherwise
 */
bool tinyml_set_anomaly_threshold(tinyml_model_type_t model_type, float threshold);

/**
 * @brief Get anomaly detection threshold
 *
 * @param model_type Model type
 * @param threshold Pointer to store current threshold
 * @return true if threshold retrieved successfully, false otherwise
 */
bool tinyml_get_anomaly_threshold(tinyml_model_type_t model_type, float *threshold);

/**
 * @brief Enable/disable continuous monitoring mode
 *
 * @param model_type Model type
 * @param enable true to enable continuous monitoring, false to disable
 * @param interval_ms Monitoring interval in milliseconds
 * @return true if configuration successful, false otherwise
 */
bool tinyml_enable_continuous_monitoring(tinyml_model_type_t model_type, bool enable,
                                        uint32_t interval_ms);

/**
 * @brief Register anomaly detection callback
 *
 * @param callback Function pointer to anomaly detection callback
 * @return true if callback registered successfully, false otherwise
 */
bool tinyml_register_anomaly_callback(void (*callback)(const anomaly_detection_result_t *result));

/**
 * @brief Unregister anomaly detection callback
 *
 * @return true if callback unregistered successfully, false otherwise
 */
bool tinyml_unregister_anomaly_callback(void);

/**
 * @brief Get TinyML engine memory usage
 *
 * @param used_memory_kb Pointer to store used memory in KB
 * @param available_memory_kb Pointer to store available memory in KB
 * @return true if memory information retrieved successfully, false otherwise
 */
bool tinyml_get_memory_usage(uint32_t *used_memory_kb, uint32_t *available_memory_kb);

/**
 * @brief Perform engine self-test
 *
 * @return true if self-test passed, false otherwise
 */
bool tinyml_self_test(void);

/**
 * @brief Reset TinyML engine
 *
 * @return true if reset successful, false otherwise
 */
bool tinyml_reset_engine(void);

/**
 * @brief Get engine version information
 *
 * @param version_string Buffer to store version string
 * @param buffer_size Size of buffer
 * @return true if version information retrieved successfully, false otherwise
 */
bool tinyml_get_version(char *version_string, uint16_t buffer_size);

/**
 * @brief Set debug logging level
 *
 * @param level Debug level (0 = none, 1 = basic, 2 = detailed, 3 = verbose)
 * @return true if debug level set successfully, false otherwise
 */
bool tinyml_set_debug_level(uint8_t level);

/**
 * @brief Export model performance metrics
 *
 * @param model_type Model type
 * @param metrics_buffer Buffer to store metrics in JSON format
 * @param buffer_size Size of buffer
 * @return true if metrics exported successfully, false otherwise
 */
bool tinyml_export_metrics(tinyml_model_type_t model_type, char *metrics_buffer,
                          uint16_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_TINYML_ENGINE_H */
