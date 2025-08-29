/**
 * @file event_system.h
 * @brief Event Logging System for EsoCore Edge Device
 *
 * This file implements a comprehensive event logging system for the EsoCore Edge device,
 * providing structured event recording, audit trails, and compliance reporting.
 * Events are stored in JSON format with timestamps and detailed metadata.
 *
 * Features:
 * - Structured JSON event format
 * - Multiple event categories (connectivity, power, safety, system, security)
 * - Event filtering and searching
 * - Audit trails for compliance
 * - Event correlation and pattern detection
 * - Configurable event retention
 * - Real-time event streaming
 * - Event prioritization and queuing
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_EVENT_SYSTEM_H
#define ESOCORE_EVENT_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Event System Configuration
 * ============================================================================ */

#define EVENT_MAX_MESSAGE_LENGTH    256   /* Maximum event message length */
#define EVENT_MAX_METADATA_SIZE     1024  /* Maximum metadata size */
#define EVENT_QUEUE_SIZE            100   /* Event queue size */
#define EVENT_RETENTION_DAYS        30    /* Default event retention (days) */

typedef enum {
    EVENT_SEVERITY_INFO       = 0,    /* Informational event */
    EVENT_SEVERITY_WARNING    = 1,    /* Warning event */
    EVENT_SEVERITY_ERROR      = 2,    /* Error event */
    EVENT_SEVERITY_CRITICAL   = 3,    /* Critical event */
    EVENT_SEVERITY_EMERGENCY  = 4,    /* Emergency event */
} event_severity_t;

typedef enum {
    EVENT_CATEGORY_CONNECTIVITY = 0,   /* Network connectivity events */
    EVENT_CATEGORY_POWER       = 1,    /* Power system events */
    EVENT_CATEGORY_SAFETY      = 2,    /* Safety system events */
    EVENT_CATEGORY_SYSTEM      = 3,    /* System events */
    EVENT_CATEGORY_SECURITY    = 4,    /* Security events */
    EVENT_CATEGORY_SENSOR      = 5,    /* Sensor events */
    EVENT_CATEGORY_USER        = 6,    /* User action events */
    EVENT_CATEGORY_MAINTENANCE = 7,    /* Maintenance events */
    EVENT_CATEGORY_DIAGNOSTIC  = 8,    /* Diagnostic events */
} event_category_t;

typedef enum {
    EVENT_TYPE_GENERIC         = 0,    /* Generic event */
    EVENT_TYPE_STARTUP         = 1,    /* System startup */
    EVENT_TYPE_SHUTDOWN        = 2,    /* System shutdown */
    EVENT_TYPE_CONFIG_CHANGE   = 3,    /* Configuration change */
    EVENT_TYPE_THRESHOLD_EXCEEDED = 4, /* Threshold exceeded */
    EVENT_TYPE_FAULT_DETECTED  = 5,    /* Fault detected */
    EVENT_TYPE_RECOVERY        = 6,    /* Recovery event */
    EVENT_TYPE_MAINTENANCE     = 7,    /* Maintenance event */
    EVENT_TYPE_SECURITY        = 8,    /* Security event */
} event_type_t;

/* Event System Configuration */
typedef struct {
    uint32_t max_events_per_day;       /* Maximum events per day */
    uint32_t retention_days;           /* Event retention period (days) */
    uint32_t max_event_size_bytes;     /* Maximum event size */
    bool enable_compression;           /* Enable event compression */
    bool enable_encryption;            /* Enable event encryption */
    bool enable_remote_logging;        /* Enable remote logging */
    uint32_t queue_size;               /* Event queue size */
    uint32_t batch_size;               /* Batch size for uploads */
    char remote_endpoint[256];         /* Remote logging endpoint */
} event_system_config_t;

/* ============================================================================
 * Event Data Structures
 * ============================================================================ */

typedef struct {
    uint32_t timestamp;                /* Event timestamp (Unix epoch) */
    event_category_t category;         /* Event category */
    event_type_t type;                 /* Event type */
    event_severity_t severity;         /* Event severity */
    char source[64];                  /* Event source (device/sensor name) */
    char message[EVENT_MAX_MESSAGE_LENGTH]; /* Event message */
    char metadata[EVENT_MAX_METADATA_SIZE]; /* JSON metadata */
    uint32_t event_id;                /* Unique event ID */
    char correlation_id[32];          /* Correlation ID for related events */
    uint32_t sequence_number;         /* Sequence number */
    bool requires_acknowledgment;     /* Requires acknowledgment */
    bool acknowledged;                /* Acknowledgment status */
    uint32_t acknowledgment_time;     /* Acknowledgment timestamp */
} event_data_t;

/* ============================================================================
 * Event Filter and Search
 * ============================================================================ */

typedef struct {
    event_category_t category;         /* Category filter (use -1 for all) */
    event_type_t type;                 /* Type filter (use -1 for all) */
    event_severity_t min_severity;     /* Minimum severity level */
    event_severity_t max_severity;     /* Maximum severity level */
    uint32_t start_time;               /* Start time filter */
    uint32_t end_time;                 /* End time filter */
    char source_filter[64];            /* Source filter (empty for all) */
    char message_filter[128];          /* Message filter (empty for all) */
    bool acknowledged_only;            /* Show only acknowledged events */
    bool unacknowledged_only;          /* Show only unacknowledged events */
} event_filter_t;

/* ============================================================================
 * Event Statistics and Monitoring
 * ============================================================================ */

typedef struct {
    uint32_t total_events;             /* Total events logged */
    uint32_t events_today;             /* Events logged today */
    uint32_t events_this_hour;         /* Events logged this hour */
    uint32_t critical_events;          /* Critical events count */
    uint32_t error_events;             /* Error events count */
    uint32_t warning_events;           /* Warning events count */
    uint32_t info_events;              /* Info events count */
    uint32_t events_by_category[9];    /* Events by category */
    uint32_t oldest_event_timestamp;   /* Oldest event timestamp */
    uint32_t newest_event_timestamp;   /* Newest event timestamp */
    uint32_t storage_used_bytes;       /* Storage used by events */
    uint32_t storage_available_bytes;  /* Storage available */
    float compression_ratio;           /* Average compression ratio */
} event_statistics_t;

/* ============================================================================
 * Event Correlation and Patterns
 * ============================================================================ */

typedef struct {
    uint32_t pattern_id;               /* Pattern identifier */
    char pattern_name[64];             /* Pattern name */
    event_category_t primary_category; /* Primary event category */
    uint32_t event_count;              /* Number of events in pattern */
    uint32_t first_occurrence;         /* First occurrence timestamp */
    uint32_t last_occurrence;          /* Last occurrence timestamp */
    uint32_t frequency_per_hour;       /* Frequency per hour */
    event_severity_t severity;         /* Pattern severity */
    char description[256];             /* Pattern description */
    bool requires_attention;           /* Requires attention flag */
} event_pattern_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize event system
 *
 * @param config Pointer to event system configuration
 * @return true if initialization successful, false otherwise
 */
bool event_system_init(const event_system_config_t *config);

/**
 * @brief Deinitialize event system
 *
 * @return true if deinitialization successful, false otherwise
 */
bool event_system_deinit(void);

/**
 * @brief Log an event
 *
 * @param event Pointer to event data structure
 * @return true if event logged successfully, false otherwise
 */
bool event_log(const event_data_t *event);

/**
 * @brief Log an event with parameters
 *
 * @param category Event category
 * @param type Event type
 * @param severity Event severity
 * @param source Event source
 * @param message Event message
 * @param metadata JSON metadata (can be NULL)
 * @return true if event logged successfully, false otherwise
 */
bool event_log_simple(event_category_t category, event_type_t type,
                     event_severity_t severity, const char *source,
                     const char *message, const char *metadata);

/**
 * @brief Log a connectivity event
 *
 * @param event_type Specific connectivity event type
 * @param details Connection details
 * @param error_code Error code (0 for success)
 * @return true if event logged successfully, false otherwise
 */
bool event_log_connectivity(const char *event_type, const char *details, uint32_t error_code);

/**
 * @brief Log a power event
 *
 * @param event_type Specific power event type
 * @param voltage_mv Current voltage in mV
 * @param current_ma Current in mA
 * @param details Additional details
 * @return true if event logged successfully, false otherwise
 */
bool event_log_power(const char *event_type, uint32_t voltage_mv, uint32_t current_ma,
                    const char *details);

/**
 * @brief Log a safety event
 *
 * @param event_type Specific safety event type
 * @param input_channel Safety input channel
 * @param response_time_ms Response time in milliseconds
 * @param details Additional details
 * @return true if event logged successfully, false otherwise
 */
bool event_log_safety(const char *event_type, uint8_t input_channel,
                     uint32_t response_time_ms, const char *details);

/**
 * @brief Log a sensor event
 *
 * @param sensor_name Sensor name
 * @param sensor_value Sensor value
 * @param threshold_value Threshold value
 * @param event_type Specific sensor event type
 * @return true if event logged successfully, false otherwise
 */
bool event_log_sensor(const char *sensor_name, float sensor_value, float threshold_value,
                     const char *event_type);

/**
 * @brief Log a system event
 *
 * @param event_type Specific system event type
 * @param component Component name
 * @param details Additional details
 * @param error_code Error code (0 for success)
 * @return true if event logged successfully, false otherwise
 */
bool event_log_system(const char *event_type, const char *component,
                     const char *details, uint32_t error_code);

/**
 * @brief Log a security event
 *
 * @param event_type Specific security event type
 * @param source_ip Source IP address (if applicable)
 * @param details Security event details
 * @return true if event logged successfully, false otherwise
 */
bool event_log_security(const char *event_type, const char *source_ip, const char *details);

/**
 * @brief Search events with filter
 *
 * @param filter Pointer to event filter
 * @param events Array to store matching events
 * @param max_events Maximum number of events to return
 * @param num_found Pointer to store number of events found
 * @return true if search successful, false otherwise
 */
bool event_search(const event_filter_t *filter, event_data_t *events,
                 uint32_t max_events, uint32_t *num_found);

/**
 * @brief Get recent events
 *
 * @param count Number of recent events to retrieve
 * @param events Array to store events
 * @param num_retrieved Pointer to store number of events retrieved
 * @return true if retrieval successful, false otherwise
 */
bool event_get_recent(uint32_t count, event_data_t *events, uint32_t *num_retrieved);

/**
 * @brief Get event by ID
 *
 * @param event_id Event ID to retrieve
 * @param event Pointer to event structure to fill
 * @return true if event found and retrieved, false otherwise
 */
bool event_get_by_id(uint32_t event_id, event_data_t *event);

/**
 * @brief Acknowledge event
 *
 * @param event_id Event ID to acknowledge
 * @param acknowledgment_message Acknowledgment message
 * @return true if acknowledgment successful, false otherwise
 */
bool event_acknowledge(uint32_t event_id, const char *acknowledgment_message);

/**
 * @brief Delete event
 *
 * @param event_id Event ID to delete
 * @return true if deletion successful, false otherwise
 */
bool event_delete(uint32_t event_id);

/**
 * @brief Get event statistics
 *
 * @param statistics Pointer to statistics structure to fill
 * @return true if statistics retrieved successfully, false otherwise
 */
bool event_get_statistics(event_statistics_t *statistics);

/**
 * @brief Clear event statistics
 *
 * @return true if statistics cleared successfully, false otherwise
 */
bool event_clear_statistics(void);

/**
 * @brief Export events to JSON format
 *
 * @param filter Pointer to event filter
 * @param json_buffer Buffer to store JSON data
 * @param buffer_size Size of buffer
 * @param exported_count Pointer to store number of events exported
 * @return true if export successful, false otherwise
 */
bool event_export_json(const event_filter_t *filter, char *json_buffer,
                      uint32_t buffer_size, uint32_t *exported_count);

/**
 * @brief Analyze event patterns
 *
 * @param time_window_hours Analysis time window in hours
 * @param patterns Array to store detected patterns
 * @param max_patterns Maximum number of patterns to detect
 * @param num_patterns Pointer to store number of patterns found
 * @return true if pattern analysis successful, false otherwise
 */
bool event_analyze_patterns(uint32_t time_window_hours, event_pattern_t *patterns,
                           uint32_t max_patterns, uint32_t *num_patterns);

/**
 * @brief Get event correlation information
 *
 * @param event_id Primary event ID
 * @param correlated_events Array to store correlated events
 * @param max_events Maximum number of correlated events
 * @param num_found Pointer to store number of correlated events
 * @return true if correlation analysis successful, false otherwise
 */
bool event_get_correlations(uint32_t event_id, event_data_t *correlated_events,
                           uint32_t max_events, uint32_t *num_found);

/**
 * @brief Set event retention policy
 *
 * @param retention_days Number of days to retain events
 * @return true if retention policy set successfully, false otherwise
 */
bool event_set_retention_policy(uint32_t retention_days);

/**
 * @brief Clean up old events based on retention policy
 *
 * @param deleted_count Pointer to store number of events deleted
 * @return true if cleanup successful, false otherwise
 */
bool event_cleanup_old_events(uint32_t *deleted_count);

/**
 * @brief Enable/disable remote event logging
 *
 * @param enable true to enable remote logging, false to disable
 * @param endpoint Remote logging endpoint URL
 * @return true if configuration successful, false otherwise
 */
bool event_enable_remote_logging(bool enable, const char *endpoint);

/**
 * @brief Flush event queue to storage
 *
 * @return true if flush successful, false otherwise
 */
bool event_flush_queue(void);

/**
 * @brief Get event queue status
 *
 * @param queued_events Pointer to store number of queued events
 * @param queue_capacity Pointer to store queue capacity
 * @param oldest_event_age Pointer to store age of oldest queued event (seconds)
 * @return true if status retrieved successfully, false otherwise
 */
bool event_get_queue_status(uint32_t *queued_events, uint32_t *queue_capacity,
                           uint32_t *oldest_event_age);

/**
 * @brief Create event correlation ID
 *
 * @param correlation_id Buffer to store correlation ID
 * @param buffer_size Size of buffer
 * @return true if correlation ID created successfully, false otherwise
 */
bool event_create_correlation_id(char *correlation_id, uint16_t buffer_size);

/**
 * @brief Format event as JSON string
 *
 * @param event Pointer to event data
 * @param json_buffer Buffer to store JSON string
 * @param buffer_size Size of buffer
 * @return true if formatting successful, false otherwise
 */
bool event_format_json(const event_data_t *event, char *json_buffer, uint32_t buffer_size);

/**
 * @brief Parse JSON string into event structure
 *
 * @param json_string JSON string to parse
 * @param event Pointer to event structure to fill
 * @return true if parsing successful, false otherwise
 */
bool event_parse_json(const char *json_string, event_data_t *event);

/**
 * @brief Get event severity description
 *
 * @param severity Event severity
 * @return Severity description string
 */
const char *event_get_severity_description(event_severity_t severity);

/**
 * @brief Get event category description
 *
 * @param category Event category
 * @return Category description string
 */
const char *event_get_category_description(event_category_t category);

/**
 * @brief Get event type description
 *
 * @param type Event type
 * @return Type description string
 */
const char *event_get_type_description(event_type_t type);

/**
 * @brief Validate event data
 *
 * @param event Pointer to event data to validate
 * @return true if event data is valid, false otherwise
 */
bool event_validate(const event_data_t *event);

/**
 * @brief Get event system status
 *
 * @param initialized Pointer to store initialization status
 * @param storage_available Pointer to store available storage (bytes)
 * @param queue_full Pointer to store queue full status
 * @return true if status retrieved successfully, false otherwise
 */
bool event_get_system_status(bool *initialized, uint32_t *storage_available, bool *queue_full);

/**
 * @brief Enable/disable event compression
 *
 * @param enable true to enable compression, false to disable
 * @return true if configuration successful, false otherwise
 */
bool event_enable_compression(bool enable);

/**
 * @brief Set event batch size for uploads
 *
 * @param batch_size Number of events per batch
 * @return true if batch size set successfully, false otherwise
 */
bool event_set_batch_size(uint32_t batch_size);

/**
 * @brief Register event callback function
 *
 * @param callback Function pointer to event callback
 * @return true if callback registered successfully, false otherwise
 */
bool event_register_callback(void (*callback)(const event_data_t *event));

/**
 * @brief Unregister event callback function
 *
 * @return true if callback unregistered successfully, false otherwise
 */
bool event_unregister_callback(void);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_EVENT_SYSTEM_H */
