/**
 * @file config_manager.h
 * @brief Configuration Management System
 *
 * This header defines the advanced configuration management system for EsoCore devices,
 * providing remote parameter updates, configuration profiles, and parameter validation.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Configuration System Constants
 * ============================================================================ */

#define ESOCORE_MAX_CONFIG_PARAMETERS     128
#define ESOCORE_MAX_CONFIG_PROFILES       8
#define ESOCORE_MAX_PARAMETER_NAME_LENGTH 32
#define ESOCORE_MAX_PARAMETER_VALUE_SIZE  64
#define ESOCORE_MAX_CONFIG_VALIDATORS     16
#define ESOCORE_CONFIG_BACKUP_SLOTS       4

/* ============================================================================
 * Configuration Parameter Types
 * ============================================================================ */

/**
 * @brief Configuration parameter data types
 */
typedef enum {
    ESOCORE_CONFIG_TYPE_BOOL        = 0x01,  /**< Boolean value */
    ESOCORE_CONFIG_TYPE_INT8        = 0x02,  /**< 8-bit signed integer */
    ESOCORE_CONFIG_TYPE_INT16       = 0x03,  /**< 16-bit signed integer */
    ESOCORE_CONFIG_TYPE_INT32       = 0x04,  /**< 32-bit signed integer */
    ESOCORE_CONFIG_TYPE_UINT8       = 0x05,  /**< 8-bit unsigned integer */
    ESOCORE_CONFIG_TYPE_UINT16      = 0x06,  /**< 16-bit unsigned integer */
    ESOCORE_CONFIG_TYPE_UINT32      = 0x07,  /**< 32-bit unsigned integer */
    ESOCORE_CONFIG_TYPE_FLOAT       = 0x08,  /**< 32-bit floating point */
    ESOCORE_CONFIG_TYPE_DOUBLE      = 0x09,  /**< 64-bit floating point */
    ESOCORE_CONFIG_TYPE_STRING      = 0x0A,  /**< Null-terminated string */
    ESOCORE_CONFIG_TYPE_BINARY      = 0x0B,  /**< Binary data */
    ESOCORE_CONFIG_TYPE_ENUM        = 0x0C   /**< Enumeration value */
} esocore_config_type_t;

/**
 * @brief Configuration parameter access levels
 */
typedef enum {
    ESOCORE_ACCESS_READ_ONLY        = 0x01,  /**< Read-only parameter */
    ESOCORE_ACCESS_READ_WRITE       = 0x02,  /**< Read-write parameter */
    ESOCORE_ACCESS_ADMIN_ONLY       = 0x03,  /**< Admin-only access */
    ESOCORE_ACCESS_SYSTEM_ONLY      = 0x04   /**< System-only access */
} esocore_config_access_t;

/**
 * @brief Configuration parameter categories
 */
typedef enum {
    ESOCORE_CATEGORY_SYSTEM         = 0x01,  /**< System configuration */
    ESOCORE_CATEGORY_NETWORK        = 0x02,  /**< Network configuration */
    ESOCORE_CATEGORY_SENSOR         = 0x03,  /**< Sensor configuration */
    ESOCORE_CATEGORY_STORAGE        = 0x04,  /**< Storage configuration */
    ESOCORE_CATEGORY_POWER          = 0x05,  /**< Power management */
    ESOCORE_CATEGORY_SAFETY         = 0x06,  /**< Safety system */
    ESOCORE_CATEGORY_USER_INTERFACE = 0x07,  /**< User interface */
    ESOCORE_CATEGORY_DIAGNOSTIC     = 0x08,  /**< Diagnostic settings */
    ESOCORE_CATEGORY_MAX
} esocore_config_category_t;

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief Configuration parameter definition
 */
typedef struct {
    uint16_t parameter_id;                  /**< Unique parameter identifier */
    char name[ESOCORE_MAX_PARAMETER_NAME_LENGTH]; /**< Parameter name */
    esocore_config_type_t data_type;       /**< Parameter data type */
    esocore_config_access_t access_level;   /**< Access level */
    esocore_config_category_t category;     /**< Parameter category */
    uint16_t max_value_size;                /**< Maximum value size in bytes */
    uint32_t default_value;                 /**< Default value (32-bit) */
    uint32_t min_value;                     /**< Minimum allowed value */
    uint32_t max_value;                     /**< Maximum allowed value */
    bool requires_restart;                  /**< Requires system restart */
    bool is_persistent;                     /**< Persists across reboots */
    char description[128];                  /**< Parameter description */
    char units[16];                         /**< Units of measurement */
} esocore_config_parameter_t;

/**
 * @brief Configuration parameter value
 */
typedef struct {
    uint16_t parameter_id;                  /**< Parameter identifier */
    esocore_config_type_t data_type;       /**< Parameter data type */
    uint16_t value_size;                    /**< Size of value data */
    uint8_t value[ESOCORE_MAX_PARAMETER_VALUE_SIZE]; /**< Parameter value */
    uint32_t timestamp;                     /**< Value timestamp */
    uint8_t source;                         /**< Source of value change */
} esocore_config_value_t;

/**
 * @brief Configuration profile
 */
typedef struct {
    uint8_t profile_id;                     /**< Profile identifier */
    char profile_name[32];                  /**< Profile name */
    char profile_description[128];          /**< Profile description */
    uint16_t parameter_count;               /**< Number of parameters in profile */
    esocore_config_value_t *parameters;     /**< Parameter values */
    bool is_active;                         /**< Profile is currently active */
    uint32_t created_timestamp;             /**< Profile creation timestamp */
    uint32_t modified_timestamp;            /**< Profile modification timestamp */
} esocore_config_profile_t;

/**
 * @brief Configuration validation rule
 */
typedef struct {
    uint16_t parameter_id;                  /**< Parameter to validate */
    uint8_t validator_type;                 /**< Type of validation */
    uint32_t rule_value;                    /**< Validation rule value */
    char error_message[64];                 /**< Error message if validation fails */
    bool (*custom_validator)(const esocore_config_value_t *value, void *context); /**< Custom validator function */
    void *validator_context;                /**< Context for custom validator */
} esocore_config_validator_t;

/**
 * @brief Configuration system status
 */
typedef struct {
    uint16_t total_parameters;              /**< Total number of parameters */
    uint16_t active_parameters;             /**< Number of active parameters */
    uint8_t active_profile;                 /**< Currently active profile */
    uint32_t last_update_timestamp;         /**< Timestamp of last configuration update */
    uint32_t pending_changes;               /**< Number of pending changes */
    bool requires_restart;                  /**< System requires restart */
    uint8_t validation_errors;              /**< Number of validation errors */
    uint32_t backup_timestamp;              /**< Timestamp of last backup */
    bool remote_sync_enabled;               /**< Remote synchronization enabled */
} esocore_config_status_t;

/**
 * @brief Configuration update request
 */
typedef struct {
    uint16_t parameter_id;                  /**< Parameter to update */
    esocore_config_value_t new_value;       /**< New parameter value */
    uint8_t source;                         /**< Source of update request */
    uint32_t request_timestamp;             /**< Request timestamp */
    bool requires_validation;               /**< Requires validation */
    bool requires_restart;                  /**< Requires system restart */
} esocore_config_update_request_t;

/**
 * @brief Configuration backup structure
 */
typedef struct {
    uint8_t backup_slot;                    /**< Backup slot identifier */
    char backup_name[32];                   /**< Backup name */
    uint32_t backup_timestamp;              /**< Backup timestamp */
    uint16_t parameter_count;               /**< Number of parameters backed up */
    uint32_t data_size;                     /**< Size of backup data */
    bool is_compressed;                     /**< Backup is compressed */
    bool is_encrypted;                      /**< Backup is encrypted */
} esocore_config_backup_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize configuration manager
 *
 * @return true if initialization successful, false otherwise
 */
bool esocore_config_init(void);

/**
 * @brief Deinitialize configuration manager
 *
 * @return true if deinitialization successful, false otherwise
 */
bool esocore_config_deinit(void);

/**
 * @brief Register configuration parameter
 *
 * @param parameter Pointer to parameter definition
 * @return true if parameter registered successfully, false otherwise
 */
bool esocore_config_register_parameter(const esocore_config_parameter_t *parameter);

/**
 * @brief Unregister configuration parameter
 *
 * @param parameter_id Parameter identifier to unregister
 * @return true if parameter unregistered successfully, false otherwise
 */
bool esocore_config_unregister_parameter(uint16_t parameter_id);

/**
 * @brief Get parameter definition
 *
 * @param parameter_id Parameter identifier
 * @param parameter Pointer to parameter structure to fill
 * @return true if parameter definition retrieved successfully, false otherwise
 */
bool esocore_config_get_parameter_definition(uint16_t parameter_id,
                                           esocore_config_parameter_t *parameter);

/**
 * @brief Set parameter value
 *
 * @param parameter_id Parameter identifier
 * @param value Pointer to new parameter value
 * @param source Source of the value change
 * @return true if parameter value set successfully, false otherwise
 */
bool esocore_config_set_value(uint16_t parameter_id, const esocore_config_value_t *value,
                             uint8_t source);

/**
 * @brief Get parameter value
 *
 * @param parameter_id Parameter identifier
 * @param value Pointer to value structure to fill
 * @return true if parameter value retrieved successfully, false otherwise
 */
bool esocore_config_get_value(uint16_t parameter_id, esocore_config_value_t *value);

/**
 * @brief Get parameter value as integer
 *
 * @param parameter_id Parameter identifier
 * @param int_value Pointer to store integer value
 * @return true if integer value retrieved successfully, false otherwise
 */
bool esocore_config_get_int_value(uint16_t parameter_id, int32_t *int_value);

/**
 * @brief Get parameter value as string
 *
 * @param parameter_id Parameter identifier
 * @param str_value Pointer to string buffer
 * @param buffer_size Size of string buffer
 * @return true if string value retrieved successfully, false otherwise
 */
bool esocore_config_get_string_value(uint16_t parameter_id, char *str_value,
                                    uint16_t buffer_size);

/**
 * @brief Get parameter value as boolean
 *
 * @param parameter_id Parameter identifier
 * @param bool_value Pointer to store boolean value
 * @return true if boolean value retrieved successfully, false otherwise
 */
bool esocore_config_get_bool_value(uint16_t parameter_id, bool *bool_value);

/**
 * @brief Get parameter value as float
 *
 * @param parameter_id Parameter identifier
 * @param float_value Pointer to store float value
 * @return true if float value retrieved successfully, false otherwise
 */
bool esocore_config_get_float_value(uint16_t parameter_id, float *float_value);

/**
 * @brief Validate parameter value
 *
 * @param parameter_id Parameter identifier
 * @param value Pointer to value to validate
 * @return true if parameter value is valid, false otherwise
 */
bool esocore_config_validate_value(uint16_t parameter_id, const esocore_config_value_t *value);

/**
 * @brief Get configuration system status
 *
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool esocore_config_get_status(esocore_config_status_t *status);

/**
 * @brief Create configuration profile
 *
 * @param profile Pointer to profile definition
 * @return true if profile created successfully, false otherwise
 */
bool esocore_config_create_profile(const esocore_config_profile_t *profile);

/**
 * @brief Delete configuration profile
 *
 * @param profile_id Profile identifier to delete
 * @return true if profile deleted successfully, false otherwise
 */
bool esocore_config_delete_profile(uint8_t profile_id);

/**
 * @brief Load configuration profile
 *
 * @param profile_id Profile identifier to load
 * @return true if profile loaded successfully, false otherwise
 */
bool esocore_config_load_profile(uint8_t profile_id);

/**
 * @brief Get configuration profile
 *
 * @param profile_id Profile identifier
 * @param profile Pointer to profile structure to fill
 * @return true if profile retrieved successfully, false otherwise
 */
bool esocore_config_get_profile(uint8_t profile_id, esocore_config_profile_t *profile);

/**
 * @brief List available configuration profiles
 *
 * @param profiles Array to store profile IDs
 * @param max_profiles Maximum number of profiles to list
 * @param num_profiles Pointer to store number of profiles found
 * @return true if profiles listed successfully, false otherwise
 */
bool esocore_config_list_profiles(uint8_t *profiles, uint8_t max_profiles,
                                 uint8_t *num_profiles);

/**
 * @brief Add validator for parameter
 *
 * @param validator Pointer to validator definition
 * @return true if validator added successfully, false otherwise
 */
bool esocore_config_add_validator(const esocore_config_validator_t *validator);

/**
 * @brief Remove validator for parameter
 *
 * @param parameter_id Parameter identifier
 * @return true if validator removed successfully, false otherwise
 */
bool esocore_config_remove_validator(uint16_t parameter_id);

/**
 * @brief Validate all configuration parameters
 *
 * @param error_count Pointer to store number of validation errors
 * @return true if all parameters are valid, false otherwise
 */
bool esocore_config_validate_all(uint8_t *error_count);

/**
 * @brief Apply pending configuration changes
 *
 * @return true if changes applied successfully, false otherwise
 */
bool esocore_config_apply_changes(void);

/**
 * @brief Discard pending configuration changes
 *
 * @return true if changes discarded successfully, false otherwise
 */
bool esocore_config_discard_changes(void);

/**
 * @brief Save configuration to persistent storage
 *
 * @return true if configuration saved successfully, false otherwise
 */
bool esocore_config_save(void);

/**
 * @brief Load configuration from persistent storage
 *
 * @return true if configuration loaded successfully, false otherwise
 */
bool esocore_config_load(void);

/**
 * @brief Reset configuration to factory defaults
 *
 * @return true if configuration reset successfully, false otherwise
 */
bool esocore_config_reset_to_defaults(void);

/**
 * @brief Create configuration backup
 *
 * @param backup_name Name for the backup
 * @param slot Backup slot to use
 * @return true if backup created successfully, false otherwise
 */
bool esocore_config_create_backup(const char *backup_name, uint8_t slot);

/**
 * @brief Restore configuration from backup
 *
 * @param slot Backup slot to restore from
 * @return true if backup restored successfully, false otherwise
 */
bool esocore_config_restore_backup(uint8_t slot);

/**
 * @brief Export configuration to file
 *
 * @param filename Export filename
 * @param format Export format (0=JSON, 1=Binary)
 * @return true if configuration exported successfully, false otherwise
 */
bool esocore_config_export(const char *filename, uint8_t format);

/**
 * @brief Import configuration from file
 *
 * @param filename Import filename
 * @param format Import format
 * @return true if configuration imported successfully, false otherwise
 */
bool esocore_config_import(const char *filename, uint8_t format);

/**
 * @brief Enable remote configuration synchronization
 *
 * @param enable true to enable remote sync, false to disable
 * @param server_url Remote server URL
 * @param api_key API key for authentication
 * @return true if remote sync configured successfully, false otherwise
 */
bool esocore_config_enable_remote_sync(bool enable, const char *server_url,
                                      const char *api_key);

/**
 * @brief Synchronize configuration with remote server
 *
 * @return true if synchronization completed successfully, false otherwise
 */
bool esocore_config_sync_remote(void);

/**
 * @brief Get configuration change history
 *
 * @param history Array to store change history
 * @param max_entries Maximum number of entries to retrieve
 * @param num_entries Pointer to store number of entries retrieved
 * @return true if history retrieved successfully, false otherwise
 */
bool esocore_config_get_change_history(esocore_config_value_t *history,
                                      uint32_t max_entries, uint32_t *num_entries);

/**
 * @brief Register configuration change callback
 *
 * @param callback Callback function for configuration changes
 * @param context User context pointer
 * @return true if callback registered successfully, false otherwise
 */
bool esocore_config_register_callback(void (*callback)(uint16_t parameter_id,
                                                      const esocore_config_value_t *old_value,
                                                      const esocore_config_value_t *new_value,
                                                      void *context),
                                     void *context);

/**
 * @brief Unregister configuration change callback
 *
 * @return true if callback unregistered successfully, false otherwise
 */
bool esocore_config_unregister_callback(void);

/**
 * @brief Get parameter statistics
 *
 * @param parameter_id Parameter identifier
 * @param change_count Pointer to store number of changes
 * @param last_change Pointer to store last change timestamp
 * @return true if statistics retrieved successfully, false otherwise
 */
bool esocore_config_get_parameter_stats(uint16_t parameter_id, uint32_t *change_count,
                                       uint32_t *last_change);

/**
 * @brief Lock configuration parameter
 *
 * @param parameter_id Parameter identifier to lock
 * @return true if parameter locked successfully, false otherwise
 */
bool esocore_config_lock_parameter(uint16_t parameter_id);

/**
 * @brief Unlock configuration parameter
 *
 * @param parameter_id Parameter identifier to unlock
 * @return true if parameter unlocked successfully, false otherwise
 */
bool esocore_config_unlock_parameter(uint16_t parameter_id);

/**
 * @brief Check if parameter requires restart
 *
 * @param parameter_id Parameter identifier
 * @param requires_restart Pointer to store restart requirement
 * @return true if restart requirement checked successfully, false otherwise
 */
bool esocore_config_requires_restart(uint16_t parameter_id, bool *requires_restart);

/**
 * @brief Get configuration system memory usage
 *
 * @param used_memory Pointer to store used memory
 * @param available_memory Pointer to store available memory
 * @return true if memory usage retrieved successfully, false otherwise
 */
bool esocore_config_get_memory_usage(uint32_t *used_memory, uint32_t *available_memory);

/**
 * @brief Perform configuration system self-test
 *
 * @return true if self-test passed, false otherwise
 */
bool esocore_config_self_test(void);

/**
 * @brief Get configuration system version
 *
 * @param version_string Buffer to store version string
 * @param buffer_size Size of version buffer
 * @return true if version retrieved successfully, false otherwise
 */
bool esocore_config_get_version(char *version_string, uint16_t buffer_size);

/**
 * @brief Enable configuration change notifications
 *
 * @param enable true to enable notifications, false to disable
 * @return true if notification setting changed successfully, false otherwise
 */
bool esocore_config_enable_notifications(bool enable);

#endif /* CONFIG_MANAGER_H */
