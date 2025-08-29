/**
 * @file config_manager.h
 * @brief Advanced Configuration Management for EsoCore Edge Device
 *
 * This file implements a comprehensive configuration management system for the EsoCore Edge device,
 * providing remote parameter updates, configuration profiles, validation, and rollback capabilities.
 * The system ensures safe and reliable configuration changes in industrial environments.
 *
 * Features:
 * - Remote configuration updates via API
 * - Configuration profiles and versioning
 * - Parameter validation and constraints
 * - Automatic rollback on failure
 * - Configuration backup and restore
 * - Change tracking and audit trails
 * - Safe parameter application with staging
 * - Configuration synchronization across fleet
 * - Template-based configuration management
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_CONFIG_MANAGER_H
#define ESOCORE_CONFIG_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Configuration Management Constants
 * ============================================================================ */

#define CONFIG_MAX_PROFILES              16    /* Maximum configuration profiles */
#define CONFIG_MAX_PARAMETERS            256   /* Maximum configuration parameters */
#define CONFIG_MAX_PARAMETER_NAME        64    /* Maximum parameter name length */
#define CONFIG_MAX_PARAMETER_VALUE       128   /* Maximum parameter value length */
#define CONFIG_MAX_VALIDATION_RULES      64    /* Maximum validation rules */
#define CONFIG_BACKUP_SLOTS              5     /* Number of configuration backup slots */
#define CONFIG_CHANGE_LOG_SIZE           100   /* Configuration change log size */

/* ============================================================================
 * Configuration Parameter Types
 * ============================================================================ */

typedef enum {
    CONFIG_TYPE_INT32      = 0,    /* 32-bit signed integer */
    CONFIG_TYPE_UINT32     = 1,    /* 32-bit unsigned integer */
    CONFIG_TYPE_FLOAT      = 2,    /* 32-bit floating point */
    CONFIG_TYPE_DOUBLE     = 3,    /* 64-bit floating point */
    CONFIG_TYPE_BOOL       = 4,    /* Boolean value */
    CONFIG_TYPE_STRING     = 5,    /* String value */
    CONFIG_TYPE_ENUM       = 6,    /* Enumeration value */
    CONFIG_TYPE_ARRAY_INT  = 7,    /* Integer array */
    CONFIG_TYPE_ARRAY_FLOAT = 8,   /* Float array */
    CONFIG_TYPE_JSON       = 9,    /* JSON object */
} config_parameter_type_t;

/* ============================================================================
 * Configuration Profile Structures
 * ============================================================================ */

typedef struct {
    char name[CONFIG_MAX_PARAMETER_NAME];       /* Parameter name */
    config_parameter_type_t type;               /* Parameter type */
    void *value;                               /* Parameter value */
    void *default_value;                       /* Default value */
    void *min_value;                           /* Minimum allowed value */
    void *max_value;                           /* Maximum allowed value */
    char description[128];                     /* Parameter description */
    char unit[16];                             /* Parameter unit */
    uint32_t flags;                            /* Parameter flags */
    bool requires_restart;                     /* Requires device restart */
    uint32_t validation_rules;                 /* Validation rule indices */
} config_parameter_t;

typedef struct {
    char profile_name[32];                     /* Profile name */
    char profile_version[16];                  /* Profile version */
    char description[128];                     /* Profile description */
    uint32_t created_timestamp;                /* Creation timestamp */
    uint32_t modified_timestamp;               /* Last modification timestamp */
    config_parameter_t *parameters;            /* Array of parameters */
    uint32_t num_parameters;                   /* Number of parameters */
    uint32_t profile_id;                       /* Unique profile ID */
    bool is_active;                            /* Profile is currently active */
    bool is_template;                          /* Profile is a template */
    char target_hardware[32];                  /* Target hardware version */
    char target_firmware[16];                  /* Target firmware version */
} config_profile_t;

/* ============================================================================
 * Configuration Validation Structures
 * ============================================================================ */

typedef enum {
    CONFIG_VALIDATION_RANGE     = 0,    /* Range validation */
    CONFIG_VALIDATION_ENUM      = 1,    /* Enumeration validation */
    CONFIG_VALIDATION_REGEX     = 2,    /* Regular expression validation */
    CONFIG_VALIDATION_CUSTOM    = 3,    /* Custom validation function */
    CONFIG_VALIDATION_DEPENDENCY = 4,  /* Parameter dependency validation */
} config_validation_type_t;

typedef struct {
    config_validation_type_t type;             /* Validation type */
    char parameter_name[CONFIG_MAX_PARAMETER_NAME]; /* Parameter to validate */
    void *validation_data;                     /* Validation data (range, enum values, etc.) */
    char error_message[128];                   /* Error message for validation failure */
    bool (*custom_validator)(const void *value, const void *validation_data); /* Custom validator function */
} config_validation_rule_t;

/* ============================================================================
 * Configuration Change Tracking
 * ============================================================================ */

typedef enum {
    CONFIG_CHANGE_TYPE_UPDATE   = 0,    /* Parameter update */
    CONFIG_CHANGE_TYPE_PROFILE_SWITCH = 1, /* Profile switch */
    CONFIG_CHANGE_TYPE_ROLLBACK = 2,    /* Rollback operation */
    CONFIG_CHANGE_TYPE_RESET    = 3,    /* Reset to defaults */
    CONFIG_CHANGE_TYPE_IMPORT   = 4,    /* Configuration import */
} config_change_type_t;

typedef struct {
    uint32_t change_id;                         /* Unique change ID */
    config_change_type_t change_type;           /* Type of change */
    char parameter_name[CONFIG_MAX_PARAMETER_NAME]; /* Affected parameter */
    char old_value[CONFIG_MAX_PARAMETER_VALUE]; /* Old parameter value */
    char new_value[CONFIG_MAX_PARAMETER_VALUE]; /* New parameter value */
    uint32_t timestamp;                         /* Change timestamp */
    char user_id[32];                           /* User who made the change */
    char change_reason[128];                    /* Reason for change */
    bool requires_restart;                      /* Change requires restart */
    bool applied_successfully;                  /* Change applied successfully */
    char error_message[128];                    /* Error message if failed */
} config_change_log_entry_t;

/* ============================================================================
 * Configuration Management Status
 * ============================================================================ */

typedef enum {
    CONFIG_STATUS_IDLE          = 0,    /* Configuration system idle */
    CONFIG_STATUS_LOADING       = 1,    /* Loading configuration */
    CONFIG_STATUS_VALIDATING    = 2,    /* Validating configuration */
    CONFIG_STATUS_APPLYING      = 3,    /* Applying configuration changes */
    CONFIG_STATUS_ROLLBACK      = 4,    /* Performing rollback */
    CONFIG_STATUS_ERROR         = 5,    /* Configuration error */
    CONFIG_STATUS_PENDING_RESTART = 6,  /* Restart required */
} config_manager_status_t;

typedef struct {
    config_manager_status_t status;             /* Current status */
    config_profile_t *active_profile;           /* Currently active profile */
    uint32_t pending_changes;                   /* Number of pending changes */
    uint32_t failed_changes;                    /* Number of failed changes */
    bool restart_required;                      /* System restart required */
    uint32_t last_backup_timestamp;             /* Last backup timestamp */
    uint32_t config_version;                    /* Configuration version */
    uint32_t change_log_entries;                /* Number of change log entries */
    float validation_progress;                  /* Validation progress (0-100) */
    char last_error[128];                       /* Last error message */
} config_manager_status_t;

/* ============================================================================
 * Remote Configuration Structures
 * ============================================================================ */

typedef enum {
    CONFIG_UPDATE_METHOD_IMMEDIATE = 0,  /* Apply immediately */
    CONFIG_UPDATE_METHOD_SCHEDULED = 1,  /* Apply at scheduled time */
    CONFIG_UPDATE_METHOD_MAINTENANCE = 2, /* Apply during maintenance window */
    CONFIG_UPDATE_METHOD_GRADUAL = 3,    /* Apply gradually (canary deployment) */
} config_update_method_t;

typedef struct {
    char target_device_id[32];                  /* Target device ID */
    config_profile_t *new_profile;              /* New configuration profile */
    config_update_method_t update_method;       /* Update method */
    uint32_t scheduled_time;                    /* Scheduled update time */
    uint32_t rollback_timeout_minutes;          /* Rollback timeout */
    bool enable_validation;                     /* Enable validation before apply */
    bool create_backup;                         /* Create backup before update */
    char update_reason[128];                    /* Reason for update */
    char requested_by[32];                      /* User who requested update */
} config_remote_update_request_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize configuration manager
 *
 * @param max_profiles Maximum number of configuration profiles
 * @param max_parameters Maximum number of parameters per profile
 * @return true if initialization successful, false otherwise
 */
bool config_manager_init(uint32_t max_profiles, uint32_t max_parameters);

/**
 * @brief Deinitialize configuration manager
 *
 * @return true if deinitialization successful, false otherwise
 */
bool config_manager_deinit(void);

/**
 * @brief Create new configuration profile
 *
 * @param profile_name Name of the new profile
 * @param description Profile description
 * @param profile Pointer to profile structure to initialize
 * @return true if profile created successfully, false otherwise
 */
bool config_create_profile(const char *profile_name, const char *description,
                          config_profile_t *profile);

/**
 * @brief Load configuration profile from storage
 *
 * @param profile_name Name of the profile to load
 * @param profile Pointer to profile structure to fill
 * @return true if profile loaded successfully, false otherwise
 */
bool config_load_profile(const char *profile_name, config_profile_t *profile);

/**
 * @brief Save configuration profile to storage
 *
 * @param profile Pointer to profile to save
 * @return true if profile saved successfully, false otherwise
 */
bool config_save_profile(const config_profile_t *profile);

/**
 * @brief Delete configuration profile
 *
 * @param profile_name Name of the profile to delete
 * @return true if profile deleted successfully, false otherwise
 */
bool config_delete_profile(const char *profile_name);

/**
 * @brief Activate configuration profile
 *
 * @param profile_name Name of the profile to activate
 * @return true if profile activated successfully, false otherwise
 */
bool config_activate_profile(const char *profile_name);

/**
 * @brief Get currently active profile
 *
 * @param profile Pointer to profile structure to fill
 * @return true if active profile retrieved successfully, false otherwise
 */
bool config_get_active_profile(config_profile_t *profile);

/**
 * @brief Add parameter to configuration profile
 *
 * @param profile Pointer to profile
 * @param parameter Pointer to parameter to add
 * @return true if parameter added successfully, false otherwise
 */
bool config_add_parameter(config_profile_t *profile, const config_parameter_t *parameter);

/**
 * @brief Update parameter value
 *
 * @param profile_name Name of the profile containing the parameter
 * @param parameter_name Name of the parameter to update
 * @param new_value Pointer to new parameter value
 * @param change_reason Reason for the change
 * @param user_id User making the change
 * @return true if parameter updated successfully, false otherwise
 */
bool config_update_parameter(const char *profile_name, const char *parameter_name,
                            const void *new_value, const char *change_reason,
                            const char *user_id);

/**
 * @brief Get parameter value
 *
 * @param profile_name Name of the profile
 * @param parameter_name Name of the parameter
 * @param value_buffer Buffer to store parameter value
 * @param buffer_size Size of buffer
 * @return true if parameter value retrieved successfully, false otherwise
 */
bool config_get_parameter_value(const char *profile_name, const char *parameter_name,
                               void *value_buffer, uint32_t buffer_size);

/**
 * @brief Validate configuration profile
 *
 * @param profile Pointer to profile to validate
 * @param error_message Buffer to store validation error message
 * @param buffer_size Size of buffer
 * @return true if profile is valid, false otherwise
 */
bool config_validate_profile(const config_profile_t *profile, char *error_message,
                            uint32_t buffer_size);

/**
 * @brief Add validation rule
 *
 * @param rule Pointer to validation rule to add
 * @return true if validation rule added successfully, false otherwise
 */
bool config_add_validation_rule(const config_validation_rule_t *rule);

/**
 * @brief Apply configuration changes
 *
 * @param profile_name Name of the profile to apply
 * @return true if configuration applied successfully, false otherwise
 */
bool config_apply_changes(const char *profile_name);

/**
 * @brief Rollback configuration to previous version
 *
 * @param rollback_reason Reason for rollback
 * @param user_id User performing rollback
 * @return true if rollback successful, false otherwise
 */
bool config_rollback_changes(const char *rollback_reason, const char *user_id);

/**
 * @brief Create configuration backup
 *
 * @param backup_name Name for the backup
 * @return true if backup created successfully, false otherwise
 */
bool config_create_backup(const char *backup_name);

/**
 * @brief Restore configuration from backup
 *
 * @param backup_name Name of the backup to restore
 * @return true if backup restored successfully, false otherwise
 */
bool config_restore_backup(const char *backup_name);

/**
 * @brief Get configuration change log
 *
 * @param log_entries Array to store log entries
 * @param max_entries Maximum number of entries to retrieve
 * @param num_entries Pointer to store number of entries retrieved
 * @return true if log retrieved successfully, false otherwise
 */
bool config_get_change_log(config_change_log_entry_t *log_entries, uint32_t max_entries,
                          uint32_t *num_entries);

/**
 * @brief Export configuration profile to JSON
 *
 * @param profile_name Name of the profile to export
 * @param json_buffer Buffer to store JSON data
 * @param buffer_size Size of buffer
 * @return true if export successful, false otherwise
 */
bool config_export_profile_json(const char *profile_name, char *json_buffer,
                               uint32_t buffer_size);

/**
 * @brief Import configuration profile from JSON
 *
 * @param json_data JSON data containing profile
 * @param profile_name Name for the imported profile
 * @return true if import successful, false otherwise
 */
bool config_import_profile_json(const char *json_data, const char *profile_name);

/**
 * @brief Get configuration manager status
 *
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool config_get_manager_status(config_manager_status_t *status);

/**
 * @brief Process remote configuration update request
 *
 * @param request Pointer to remote update request
 * @return true if update request processed successfully, false otherwise
 */
bool config_process_remote_update(const config_remote_update_request_t *request);

/**
 * @brief Get pending configuration updates
 *
 * @param updates Array to store pending updates
 * @param max_updates Maximum number of updates to retrieve
 * @param num_updates Pointer to store number of updates retrieved
 * @return true if pending updates retrieved successfully, false otherwise
 */
bool config_get_pending_updates(config_remote_update_request_t *updates,
                               uint32_t max_updates, uint32_t *num_updates);

/**
 * @brief Approve configuration update
 *
 * @param update_id ID of the update to approve
 * @param approver_id ID of the approver
 * @return true if update approved successfully, false otherwise
 */
bool config_approve_update(uint32_t update_id, const char *approver_id);

/**
 * @brief Reject configuration update
 *
 * @param update_id ID of the update to reject
 * @param reject_reason Reason for rejection
 * @param rejector_id ID of the rejector
 * @return true if update rejected successfully, false otherwise
 */
bool config_reject_update(uint32_t update_id, const char *reject_reason,
                         const char *rejector_id);

/**
 * @brief Get configuration templates
 *
 * @param templates Array to store template names
 * @param max_templates Maximum number of templates to retrieve
 * @param num_templates Pointer to store number of templates retrieved
 * @return true if templates retrieved successfully, false otherwise
 */
bool config_get_templates(char *templates[][32], uint32_t max_templates,
                         uint32_t *num_templates);

/**
 * @brief Create configuration template
 *
 * @param template_name Name of the template
 * @param base_profile_name Name of the base profile
 * @param description Template description
 * @return true if template created successfully, false otherwise
 */
bool config_create_template(const char *template_name, const char *base_profile_name,
                           const char *description);

/**
 * @brief Apply template to create new profile
 *
 * @param template_name Name of the template to apply
 * @param new_profile_name Name for the new profile
 * @param custom_parameters JSON string with custom parameter values
 * @return true if template applied successfully, false otherwise
 */
bool config_apply_template(const char *template_name, const char *new_profile_name,
                          const char *custom_parameters);

/**
 * @brief Get configuration parameter constraints
 *
 * @param parameter_name Name of the parameter
 * @param constraints JSON buffer to store constraints
 * @param buffer_size Size of buffer
 * @return true if constraints retrieved successfully, false otherwise
 */
bool config_get_parameter_constraints(const char *parameter_name, char *constraints,
                                     uint32_t buffer_size);

/**
 * @brief Set configuration parameter constraints
 *
 * @param parameter_name Name of the parameter
 * @param constraints JSON string with parameter constraints
 * @return true if constraints set successfully, false otherwise
 */
bool config_set_parameter_constraints(const char *parameter_name, const char *constraints);

/**
 * @brief Check if configuration change requires restart
 *
 * @param profile_name Name of the profile
 * @param parameter_name Name of the parameter being changed
 * @return true if restart is required, false otherwise
 */
bool config_requires_restart(const char *profile_name, const char *parameter_name);

/**
 * @brief Get configuration health status
 *
 * @param health_score Pointer to store health score (0-100)
 * @param issues Pointer to store number of configuration issues
 * @return true if health status retrieved successfully, false otherwise
 */
bool config_get_health_status(float *health_score, uint32_t *issues);

/**
 * @brief Perform configuration consistency check
 *
 * @param issues_buffer Buffer to store consistency issues
 * @param buffer_size Size of buffer
 * @return true if consistency check completed successfully, false otherwise
 */
bool config_check_consistency(char *issues_buffer, uint32_t buffer_size);

/**
 * @brief Reset configuration to factory defaults
 *
 * @param reset_reason Reason for reset
 * @param user_id User performing reset
 * @return true if reset successful, false otherwise
 */
bool config_reset_to_defaults(const char *reset_reason, const char *user_id);

/**
 * @brief Get configuration version information
 *
 * @param version_string Buffer to store version string
 * @param buffer_size Size of buffer
 * @return true if version information retrieved successfully, false otherwise
 */
bool config_get_version_info(char *version_string, uint16_t buffer_size);

/**
 * @brief Enable/disable configuration change notifications
 *
 * @param enable true to enable notifications, false to disable
 * @param notification_endpoint Endpoint for notifications
 * @return true if configuration successful, false otherwise
 */
bool config_enable_change_notifications(bool enable, const char *notification_endpoint);

/**
 * @brief Synchronize configuration with remote server
 *
 * @return true if synchronization successful, false otherwise
 */
bool config_sync_with_remote(void);

/**
 * @brief Get configuration update progress
 *
 * @param progress Pointer to store progress percentage (0-100)
 * @param status_message Buffer to store status message
 * @param buffer_size Size of buffer
 * @return true if progress retrieved successfully, false otherwise
 */
bool config_get_update_progress(float *progress, char *status_message, uint16_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_CONFIG_MANAGER_H */
