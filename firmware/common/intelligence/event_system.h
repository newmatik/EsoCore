/**
 * @file event_system.h
 * @brief Event Logging and Audit System
 *
 * This header defines the comprehensive event logging and audit system for EsoCore devices,
 * providing structured event recording, audit trails, and event-driven processing.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Event System Configuration
 * ============================================================================ */

#define ESOCORE_MAX_EVENT_SUBSCRIBERS    8
#define ESOCORE_MAX_EVENT_QUEUE_SIZE     64
#define ESOCORE_MAX_EVENT_DATA_SIZE      128
#define ESOCORE_MAX_EVENT_MESSAGE_SIZE   256
#define ESOCORE_EVENT_LOG_BUFFER_SIZE    2048

/* ============================================================================
 * Event Types and Categories
 * ============================================================================ */

/**
 * @brief Event categories
 */
typedef enum {
    ESOCORE_EVENT_CATEGORY_SYSTEM      = 0x01,  /**< System events */
    ESOCORE_EVENT_CATEGORY_SENSOR      = 0x02,  /**< Sensor events */
    ESOCORE_EVENT_CATEGORY_NETWORK     = 0x03,  /**< Network events */
    ESOCORE_EVENT_CATEGORY_POWER       = 0x04,  /**< Power management events */
    ESOCORE_EVENT_CATEGORY_STORAGE     = 0x05,  /**< Storage events */
    ESOCORE_EVENT_CATEGORY_SAFETY      = 0x06,  /**< Safety system events */
    ESOCORE_EVENT_CATEGORY_USER        = 0x07,  /**< User interface events */
    ESOCORE_EVENT_CATEGORY_AUDIT       = 0x08,  /**< Audit trail events */
    ESOCORE_EVENT_CATEGORY_DIAGNOSTIC  = 0x09,  /**< Diagnostic events */
    ESOCORE_EVENT_CATEGORY_MAX
} esocore_event_category_t;

/**
 * @brief Event severity levels
 */
typedef enum {
    ESOCORE_EVENT_SEVERITY_DEBUG       = 0x01,  /**< Debug information */
    ESOCORE_EVENT_SEVERITY_INFO        = 0x02,  /**< Informational message */
    ESOCORE_EVENT_SEVERITY_WARNING     = 0x03,  /**< Warning condition */
    ESOCORE_EVENT_SEVERITY_ERROR       = 0x04,  /**< Error condition */
    ESOCORE_EVENT_SEVERITY_CRITICAL    = 0x05,  /**< Critical error */
    ESOCORE_EVENT_SEVERITY_EMERGENCY   = 0x06,  /**< Emergency condition */
    ESOCORE_EVENT_SEVERITY_MAX
} esocore_event_severity_t;

/**
 * @brief Specific event types
 */
typedef enum {
    /* System Events */
    ESOCORE_EVENT_SYSTEM_STARTUP       = 0x0101, /**< System startup */
    ESOCORE_EVENT_SYSTEM_SHUTDOWN      = 0x0102, /**< System shutdown */
    ESOCORE_EVENT_SYSTEM_RESET         = 0x0103, /**< System reset */
    ESOCORE_EVENT_SYSTEM_WATCHDOG      = 0x0104, /**< Watchdog reset */
    ESOCORE_EVENT_SYSTEM_LOW_MEMORY    = 0x0105, /**< Low memory condition */
    ESOCORE_EVENT_SYSTEM_CONFIG_CHANGE = 0x0106, /**< Configuration changed */

    /* Sensor Events */
    ESOCORE_EVENT_SENSOR_STARTUP       = 0x0201, /**< Sensor startup */
    ESOCORE_EVENT_SENSOR_CALIBRATION   = 0x0202, /**< Sensor calibration */
    ESOCORE_EVENT_SENSOR_ERROR         = 0x0203, /**< Sensor error */
    ESOCORE_EVENT_SENSOR_OVER_RANGE    = 0x0204, /**< Sensor over range */
    ESOCORE_EVENT_SENSOR_DATA_READY    = 0x0205, /**< Sensor data ready */
    ESOCORE_EVENT_SENSOR_MAINTENANCE   = 0x0206, /**< Sensor maintenance needed */

    /* Network Events */
    ESOCORE_EVENT_NETWORK_CONNECTED    = 0x0301, /**< Network connected */
    ESOCORE_EVENT_NETWORK_DISCONNECTED = 0x0302, /**< Network disconnected */
    ESOCORE_EVENT_NETWORK_ERROR        = 0x0303, /**< Network error */
    ESOCORE_EVENT_NETWORK_TIMEOUT      = 0x0304, /**< Network timeout */
    ESOCORE_EVENT_NETWORK_CONFIG       = 0x0305, /**< Network configuration */
    ESOCORE_EVENT_NETWORK_OTA          = 0x0306, /**< OTA update event */

    /* Power Events */
    ESOCORE_EVENT_POWER_SOURCE_CHANGE  = 0x0401, /**< Power source changed */
    ESOCORE_EVENT_POWER_LOW_VOLTAGE    = 0x0402, /**< Low voltage warning */
    ESOCORE_EVENT_POWER_CRITICAL       = 0x0403, /**< Critical power condition */
    ESOCORE_EVENT_POWER_SUPERCAP_LOW   = 0x0404, /**< Supercapacitor low */
    ESOCORE_EVENT_POWER_POE_DETECTED   = 0x0405, /**< PoE power detected */
    ESOCORE_EVENT_POWER_FAULT          = 0x0406, /**< Power fault */

    /* Storage Events */
    ESOCORE_EVENT_STORAGE_FULL         = 0x0501, /**< Storage full */
    ESOCORE_EVENT_STORAGE_ERROR        = 0x0502, /**< Storage error */
    ESOCORE_EVENT_STORAGE_MAINTENANCE  = 0x0503, /**< Storage maintenance */
    ESOCORE_EVENT_STORAGE_BACKUP       = 0x0504, /**< Storage backup */

    /* Safety Events */
    ESOCORE_EVENT_SAFETY_EMERGENCY     = 0x0601, /**< Safety emergency stop */
    ESOCORE_EVENT_SAFETY_FAULT         = 0x0602, /**< Safety fault */
    ESOCORE_EVENT_SAFETY_RESET         = 0x0603, /**< Safety system reset */
    ESOCORE_EVENT_SAFETY_TEST          = 0x0604, /**< Safety system test */

    /* User Interface Events */
    ESOCORE_EVENT_UI_BUTTON_PRESS      = 0x0701, /**< Button press */
    ESOCORE_EVENT_UI_MENU_NAVIGATION   = 0x0702, /**< Menu navigation */
    ESOCORE_EVENT_UI_CONFIGURATION     = 0x0703, /**< Configuration change */
    ESOCORE_EVENT_UI_DISPLAY_ERROR     = 0x0704, /**< Display error */

    /* Audit Events */
    ESOCORE_EVENT_AUDIT_LOGIN          = 0x0801, /**< User login */
    ESOCORE_EVENT_AUDIT_LOGOUT         = 0x0802, /**< User logout */
    ESOCORE_EVENT_AUDIT_ACCESS         = 0x0803, /**< Access attempt */
    ESOCORE_EVENT_AUDIT_CONFIG_CHANGE  = 0x0804, /**< Configuration change */
    ESOCORE_EVENT_AUDIT_FIRMWARE_UPDATE= 0x0805, /**< Firmware update */

    /* Diagnostic Events */
    ESOCORE_EVENT_DIAG_SELF_TEST       = 0x0901, /**< Self-test completed */
    ESOCORE_EVENT_DIAG_HEALTH_CHECK    = 0x0902, /**< Health check result */
    ESOCORE_EVENT_DIAG_CALIBRATION     = 0x0903, /**< Calibration completed */
    ESOCORE_EVENT_DIAG_MAINTENANCE     = 0x0904  /**< Maintenance required */
} esocore_event_type_t;

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief Event record structure
 */
typedef struct {
    uint32_t timestamp;                    /**< Event timestamp (Unix time) */
    uint16_t event_id;                     /**< Unique event identifier */
    esocore_event_type_t event_type;       /**< Type of event */
    esocore_event_category_t category;     /**< Event category */
    esocore_event_severity_t severity;     /**< Event severity level */
    uint8_t source_module;                 /**< Source module identifier */
    uint8_t source_instance;               /**< Source instance identifier */
    uint16_t data_length;                  /**< Length of event data */
    uint8_t data[ESOCORE_MAX_EVENT_DATA_SIZE]; /**< Event-specific data */
    uint32_t sequence_number;              /**< Event sequence number */
} esocore_event_record_t;

/**
 * @brief Event subscription configuration
 */
typedef struct {
    esocore_event_category_t category_filter; /**< Category filter (0xFF for all) */
    esocore_event_severity_t min_severity;  /**< Minimum severity level */
    uint16_t event_type_filter;            /**< Event type filter (0xFFFF for all) */
    bool enabled;                          /**< Subscription is enabled */
    void *context;                         /**< User context pointer */
} esocore_event_subscription_t;

/**
 * @brief Event system configuration
 */
typedef struct {
    bool enable_buffering;                 /**< Enable event buffering */
    bool enable_compression;               /**< Enable event data compression */
    bool enable_encryption;                /**< Enable event data encryption */
    uint32_t max_buffer_size;              /**< Maximum buffer size */
    uint32_t flush_interval_ms;            /**< Buffer flush interval */
    uint8_t max_subscribers;               /**< Maximum number of subscribers */
    uint32_t retention_days;               /**< Event retention period */
    bool enable_remote_logging;            /**< Enable remote logging */
} esocore_event_config_t;

/**
 * @brief Event system status
 */
typedef struct {
    uint32_t total_events_logged;          /**< Total events logged */
    uint32_t events_in_buffer;             /**< Events currently in buffer */
    uint32_t buffer_overflows;             /**< Buffer overflow count */
    uint32_t subscribers_active;           /**< Number of active subscribers */
    uint32_t events_filtered;              /**< Events filtered out */
    uint32_t compression_ratio;            /**< Data compression ratio */
    uint32_t storage_used_kb;              /**< Storage used by events (KB) */
    bool buffer_full;                      /**< Event buffer is full */
    bool remote_logging_active;            /**< Remote logging is active */
    uint32_t last_flush_time;              /**< Timestamp of last buffer flush */
} esocore_event_status_t;

/**
 * @brief Event log entry for persistent storage
 */
typedef struct {
    uint32_t timestamp;                    /**< Event timestamp */
    uint16_t event_id;                     /**< Event identifier */
    uint8_t severity;                      /**< Event severity */
    uint8_t category;                      /**< Event category */
    char message[ESOCORE_MAX_EVENT_MESSAGE_SIZE]; /**< Event message */
    char source[32];                       /**< Event source */
} esocore_event_log_entry_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize event system
 *
 * @param config Pointer to event system configuration
 * @return true if initialization successful, false otherwise
 */
bool esocore_event_init(const esocore_event_config_t *config);

/**
 * @brief Deinitialize event system
 *
 * @return true if deinitialization successful, false otherwise
 */
bool esocore_event_deinit(void);

/**
 * @brief Log an event
 *
 * @param event_type Type of event to log
 * @param severity Event severity level
 * @param data Pointer to event-specific data
 * @param data_length Length of event data
 * @return true if event logged successfully, false otherwise
 */
bool esocore_event_log(esocore_event_type_t event_type, esocore_event_severity_t severity,
                      const uint8_t *data, uint16_t data_length);

/**
 * @brief Log an event with message
 *
 * @param event_type Type of event to log
 * @param severity Event severity level
 * @param message Human-readable event message
 * @param data Pointer to event-specific data
 * @param data_length Length of event data
 * @return true if event logged successfully, false otherwise
 */
bool esocore_event_log_message(esocore_event_type_t event_type, esocore_event_severity_t severity,
                              const char *message, const uint8_t *data, uint16_t data_length);

/**
 * @brief Subscribe to events
 *
 * @param subscription Pointer to subscription configuration
 * @param callback Callback function for event notifications
 * @param subscriber_id Pointer to store subscriber identifier
 * @return true if subscription successful, false otherwise
 */
bool esocore_event_subscribe(const esocore_event_subscription_t *subscription,
                           void (*callback)(const esocore_event_record_t *event, void *context),
                           uint8_t *subscriber_id);

/**
 * @brief Unsubscribe from events
 *
 * @param subscriber_id Subscriber identifier to remove
 * @return true if unsubscription successful, false otherwise
 */
bool esocore_event_unsubscribe(uint8_t subscriber_id);

/**
 * @brief Get event system status
 *
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool esocore_event_get_status(esocore_event_status_t *status);

/**
 * @brief Get event system configuration
 *
 * @param config Pointer to configuration structure to fill
 * @return true if configuration retrieved successfully, false otherwise
 */
bool esocore_event_get_config(esocore_event_config_t *config);

/**
 * @brief Set event system configuration
 *
 * @param config Pointer to new configuration
 * @return true if configuration set successfully, false otherwise
 */
bool esocore_event_set_config(const esocore_event_config_t *config);

/**
 * @brief Flush event buffer to storage
 *
 * @return true if buffer flushed successfully, false otherwise
 */
bool esocore_event_flush_buffer(void);

/**
 * @brief Clear event buffer
 *
 * @return true if buffer cleared successfully, false otherwise
 */
bool esocore_event_clear_buffer(void);

/**
 * @brief Retrieve events from buffer
 *
 * @param events Array to store retrieved events
 * @param max_events Maximum number of events to retrieve
 * @param num_events Pointer to store number of events retrieved
 * @return true if events retrieved successfully, false otherwise
 */
bool esocore_event_retrieve(esocore_event_record_t *events, uint32_t max_events,
                           uint32_t *num_events);

/**
 * @brief Search events by criteria
 *
 * @param category Event category filter (0xFF for all)
 * @param severity Minimum severity level
 * @param start_time Start time for search
 * @param end_time End time for search
 * @param events Array to store found events
 * @param max_events Maximum number of events to find
 * @param num_events Pointer to store number of events found
 * @return true if search completed successfully, false otherwise
 */
bool esocore_event_search(esocore_event_category_t category, esocore_event_severity_t severity,
                         uint32_t start_time, uint32_t end_time,
                         esocore_event_record_t *events, uint32_t max_events,
                         uint32_t *num_events);

/**
 * @brief Get event statistics
 *
 * @param category Event category
 * @param events_count Pointer to store event count
 * @param last_event_time Pointer to store last event timestamp
 * @return true if statistics retrieved successfully, false otherwise
 */
bool esocore_event_get_statistics(esocore_event_category_t category,
                                 uint32_t *events_count, uint32_t *last_event_time);

/**
 * @brief Export events to external storage
 *
 * @param filename Export filename
 * @param format Export format (0=JSON, 1=CSV, 2=Binary)
 * @param start_time Start time for export
 * @param end_time End time for export
 * @return true if export completed successfully, false otherwise
 */
bool esocore_event_export(const char *filename, uint8_t format,
                         uint32_t start_time, uint32_t end_time);

/**
 * @brief Import events from external storage
 *
 * @param filename Import filename
 * @param format Import format
 * @return true if import completed successfully, false otherwise
 */
bool esocore_event_import(const char *filename, uint8_t format);

/**
 * @brief Enable/disable event compression
 *
 * @param enable true to enable compression, false to disable
 * @return true if compression setting changed successfully, false otherwise
 */
bool esocore_event_enable_compression(bool enable);

/**
 * @brief Enable/disable remote event logging
 *
 * @param enable true to enable remote logging, false to disable
 * @param server_url Remote server URL
 * @param api_key API key for remote server
 * @return true if remote logging configured successfully, false otherwise
 */
bool esocore_event_enable_remote_logging(bool enable, const char *server_url,
                                        const char *api_key);

/**
 * @brief Get event compression ratio
 *
 * @param ratio Pointer to store compression ratio (percentage)
 * @return true if ratio retrieved successfully, false otherwise
 */
bool esocore_event_get_compression_ratio(uint8_t *ratio);

/**
 * @brief Validate event data integrity
 *
 * @param event Pointer to event to validate
 * @return true if event data is valid, false otherwise
 */
bool esocore_event_validate(const esocore_event_record_t *event);

/**
 * @brief Create audit trail event
 *
 * @param action Audit action performed
 * @param user User identifier
 * @param resource Resource affected
 * @param success true if action was successful, false otherwise
 * @param details Additional details about the action
 * @return true if audit event created successfully, false otherwise
 */
bool esocore_event_audit_log(const char *action, const char *user, const char *resource,
                            bool success, const char *details);

/**
 * @brief Generate event report
 *
 * @param report_type Type of report to generate
 * @param start_time Start time for report
 * @param end_time End time for report
 * @param report_buffer Buffer to store report
 * @param buffer_size Size of report buffer
 * @return true if report generated successfully, false otherwise
 */
bool esocore_event_generate_report(uint8_t report_type, uint32_t start_time, uint32_t end_time,
                                  char *report_buffer, uint32_t buffer_size);

/**
 * @brief Clean up old events based on retention policy
 *
 * @return true if cleanup completed successfully, false otherwise
 */
bool esocore_event_cleanup_old_events(void);

/**
 * @brief Set event log level
 *
 * @param level Minimum severity level to log
 * @return true if log level set successfully, false otherwise
 */
bool esocore_event_set_log_level(esocore_event_severity_t level);

/**
 * @brief Get event log level
 *
 * @param level Pointer to store current log level
 * @return true if log level retrieved successfully, false otherwise
 */
bool esocore_event_get_log_level(esocore_event_severity_t *level);

/**
 * @brief Enable/disable event buffering
 *
 * @param enable true to enable buffering, false to disable
 * @return true if buffering setting changed successfully, false otherwise
 */
bool esocore_event_enable_buffering(bool enable);

/**
 * @brief Get event queue status
 *
 * @param queued_events Pointer to store number of queued events
 * @param processed_events Pointer to store number of processed events
 * @param dropped_events Pointer to store number of dropped events
 * @return true if queue status retrieved successfully, false otherwise
 */
bool esocore_event_get_queue_status(uint32_t *queued_events, uint32_t *processed_events,
                                   uint32_t *dropped_events);

/**
 * @brief Register event filter
 *
 * @param filter_function Filter function pointer
 * @param context User context pointer
 * @return true if filter registered successfully, false otherwise
 */
bool esocore_event_register_filter(bool (*filter_function)(const esocore_event_record_t *event, void *context),
                                  void *context);

/**
 * @brief Unregister event filter
 *
 * @return true if filter unregistered successfully, false otherwise
 */
bool esocore_event_unregister_filter(void);

#endif /* EVENT_SYSTEM_H */
