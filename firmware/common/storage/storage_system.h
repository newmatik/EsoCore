/**
 * @file storage_system.h
 * @brief MicroSD Card Storage System for EsoCore
 *
 * This file implements a comprehensive storage system for the EsoCore Edge device
 * using microSD cards with industrial-grade reliability features including
 * Zstandard compression, power-safe operations, and automatic data management.
 *
 * Features:
 * - Industrial microSD card support (SLC/MLC with wear leveling)
 * - Zstandard compression for efficient storage utilization
 * - Power-safe write operations with atomic file handling
 * - Automatic file rotation and cleanup
 * - CRC-32 and SHA-256 integrity verification
 * - FAT32 filesystem with custom optimizations
 * - Data buffering during connectivity outages
 * - Automatic compression and decompression
 * - Wear leveling and bad block management
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_STORAGE_SYSTEM_H
#define ESOCORE_STORAGE_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Storage System Configuration
 * ============================================================================ */

#define STORAGE_MAX_FILENAME_LENGTH  64    /* Maximum filename length */
#define STORAGE_MAX_PATH_LENGTH      128   /* Maximum path length */
#define STORAGE_BUFFER_SIZE          4096  /* Internal buffer size */
#define STORAGE_MAX_FILES            1000  /* Maximum number of files */
#define STORAGE_COMPRESSION_LEVEL    3     /* Zstandard compression level (1-22) */
#define STORAGE_FILE_ROTATION_SIZE   (1024 * 1024) /* 1MB file rotation size */

/* Storage Configuration */
typedef struct {
    uint32_t max_file_size_bytes;          /* Maximum file size before rotation */
    uint32_t max_storage_usage_percent;    /* Maximum storage usage percentage */
    uint32_t compression_level;            /* Zstandard compression level */
    uint32_t buffer_size_bytes;            /* Internal buffer size */
    bool enable_compression;               /* Enable Zstandard compression */
    bool enable_integrity_check;           /* Enable SHA-256 integrity checks */
    bool enable_power_safe_writes;         /* Enable atomic write operations */
    bool enable_auto_cleanup;              /* Enable automatic file cleanup */
    uint32_t cleanup_threshold_percent;    /* Cleanup threshold percentage */
    char mount_point[32];                  /* Mount point path */
} storage_config_t;

/* ============================================================================
 * File Management Structures
 * ============================================================================ */

typedef enum {
    STORAGE_FILE_TYPE_DATA     = 0,    /* Sensor data file */
    STORAGE_FILE_TYPE_EVENT    = 1,    /* System event file */
    STORAGE_FILE_TYPE_CONFIG   = 2,    /* Configuration file */
    STORAGE_FILE_TYPE_LOG      = 3,    /* Log file */
    STORAGE_FILE_TYPE_FIRMWARE = 4,    /* Firmware file */
} storage_file_type_t;

/* File Information */
typedef struct {
    char filename[STORAGE_MAX_FILENAME_LENGTH];  /* Filename */
    storage_file_type_t file_type;               /* File type */
    uint32_t file_size;                          /* File size in bytes */
    uint32_t compressed_size;                    /* Compressed size in bytes */
    float compression_ratio;                     /* Compression ratio */
    uint32_t crc32;                              /* CRC-32 checksum */
    uint8_t sha256[32];                          /* SHA-256 hash */
    uint32_t creation_timestamp;                 /* Creation timestamp */
    uint32_t modification_timestamp;             /* Last modification timestamp */
    bool is_compressed;                          /* Compression status */
    bool is_power_safe;                          /* Power-safe status */
} storage_file_info_t;

/* ============================================================================
 * Data Buffering and Queuing
 * ============================================================================ */

typedef enum {
    STORAGE_PRIORITY_LOW      = 0,    /* Low priority data */
    STORAGE_PRIORITY_NORMAL   = 1,    /* Normal priority data */
    STORAGE_PRIORITY_HIGH     = 2,    /* High priority data */
    STORAGE_PRIORITY_CRITICAL = 3,    /* Critical data (events, alarms) */
} storage_priority_t;

/* Data Record */
typedef struct {
    storage_priority_t priority;               /* Data priority */
    uint8_t *data;                             /* Data buffer */
    uint32_t data_length;                      /* Data length */
    uint32_t timestamp;                        /* Record timestamp */
    uint32_t sequence_number;                  /* Sequence number */
    char metadata[128];                        /* Metadata string */
} storage_data_record_t;

/* ============================================================================
 * Storage System Status and Statistics
 * ============================================================================ */

typedef enum {
    STORAGE_STATUS_UNINITIALIZED = 0,    /* Storage not initialized */
    STORAGE_STATUS_INITIALIZING  = 1,    /* Storage initializing */
    STORAGE_STATUS_READY         = 2,    /* Storage ready for use */
    STORAGE_STATUS_ERROR         = 3,    /* Storage error */
    STORAGE_STATUS_FULL          = 4,    /* Storage full */
    STORAGE_STATUS_READONLY      = 5,    /* Storage read-only mode */
} storage_status_t;

/* Storage Statistics */
typedef struct {
    uint32_t total_capacity_bytes;            /* Total storage capacity */
    uint32_t used_capacity_bytes;             /* Used storage capacity */
    uint32_t free_capacity_bytes;             /* Free storage capacity */
    float usage_percentage;                   /* Usage percentage */
    uint32_t total_files;                     /* Total number of files */
    uint32_t data_files;                      /* Number of data files */
    uint32_t event_files;                     /* Number of event files */
    uint32_t log_files;                       /* Number of log files */
    uint32_t bytes_written;                   /* Total bytes written */
    uint32_t bytes_read;                      /* Total bytes read */
    uint32_t compression_savings_bytes;       /* Compression savings */
    float average_compression_ratio;          /* Average compression ratio */
    uint32_t write_operations;                /* Total write operations */
    uint32_t read_operations;                 /* Total read operations */
    uint32_t errors_count;                    /* Total errors */
    uint32_t power_cycles;                    /* Power cycle count */
} storage_statistics_t;

/* ============================================================================
 * Storage System Status
 * ============================================================================ */

typedef struct {
    storage_status_t status;                   /* Current storage status */
    bool sd_card_present;                      /* SD card presence */
    bool sd_card_initialized;                 /* SD card initialization status */
    bool filesystem_mounted;                  /* Filesystem mount status */
    bool compression_enabled;                 /* Compression enabled */
    bool integrity_check_enabled;             /* Integrity check enabled */
    bool power_safe_mode;                     /* Power-safe mode active */
    uint32_t last_error_code;                 /* Last error code */
    char last_error_message[128];             /* Last error message */
    uint32_t uptime_seconds;                  /* Storage system uptime */
    uint32_t buffer_usage_percent;            /* Internal buffer usage */
    uint32_t pending_operations;              /* Pending write operations */
} storage_system_status_t;

/* ============================================================================
 * Error Codes
 * ============================================================================ */

#define STORAGE_ERROR_NONE              0x0000
#define STORAGE_ERROR_SD_NOT_PRESENT    0x0001  /* SD card not present */
#define STORAGE_ERROR_SD_INIT_FAILED    0x0002  /* SD card initialization failed */
#define STORAGE_ERROR_FILESYSTEM_ERROR  0x0003  /* Filesystem error */
#define STORAGE_ERROR_DISK_FULL         0x0004  /* Storage full */
#define STORAGE_ERROR_FILE_NOT_FOUND    0x0005  /* File not found */
#define STORAGE_ERROR_READ_FAILED       0x0006  /* Read operation failed */
#define STORAGE_ERROR_WRITE_FAILED      0x0007  /* Write operation failed */
#define STORAGE_ERROR_COMPRESSION_FAILED 0x0008 /* Compression failed */
#define STORAGE_ERROR_DECOMPRESSION_FAILED 0x0009 /* Decompression failed */
#define STORAGE_ERROR_INTEGRITY_CHECK_FAILED 0x000A /* Integrity check failed */
#define STORAGE_ERROR_BUFFER_OVERFLOW   0x000B  /* Buffer overflow */
#define STORAGE_ERROR_INVALID_PARAMETER 0x000C  /* Invalid parameter */
#define STORAGE_ERROR_TIMEOUT           0x000D  /* Operation timeout */
#define STORAGE_ERROR_BUSY              0x000E  /* Storage system busy */

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize storage system
 *
 * @param config Pointer to storage configuration
 * @return true if initialization successful, false otherwise
 */
bool storage_system_init(const storage_config_t *config);

/**
 * @brief Deinitialize storage system
 *
 * @return true if deinitialization successful, false otherwise
 */
bool storage_system_deinit(void);

/**
 * @brief Get storage system status
 *
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool storage_system_get_status(storage_system_status_t *status);

/**
 * @brief Get storage statistics
 *
 * @param statistics Pointer to statistics structure to fill
 * @return true if statistics retrieved successfully, false otherwise
 */
bool storage_system_get_statistics(storage_statistics_t *statistics);

/**
 * @brief Write data record to storage
 *
 * @param record Pointer to data record
 * @return true if write successful, false otherwise
 */
bool storage_write_record(const storage_data_record_t *record);

/**
 * @brief Write data buffer to storage
 *
 * @param data Pointer to data buffer
 * @param length Data length
 * @param filename Target filename
 * @param file_type File type
 * @return true if write successful, false otherwise
 */
bool storage_write_file(const uint8_t *data, uint32_t length,
                       const char *filename, storage_file_type_t file_type);

/**
 * @brief Read data from storage
 *
 * @param filename Source filename
 * @param buffer Pointer to read buffer
 * @param max_length Maximum buffer length
 * @param bytes_read Pointer to store actual bytes read
 * @return true if read successful, false otherwise
 */
bool storage_read_file(const char *filename, uint8_t *buffer,
                      uint32_t max_length, uint32_t *bytes_read);

/**
 * @brief Delete file from storage
 *
 * @param filename Filename to delete
 * @return true if deletion successful, false otherwise
 */
bool storage_delete_file(const char *filename);

/**
 * @brief List files in storage
 *
 * @param file_list Pointer to array of file info structures
 * @param max_files Maximum number of files to list
 * @param num_files Pointer to store number of files found
 * @param file_type_filter File type filter (STORAGE_FILE_TYPE_* or -1 for all)
 * @return true if listing successful, false otherwise
 */
bool storage_list_files(storage_file_info_t *file_list, uint32_t max_files,
                       uint32_t *num_files, int file_type_filter);

/**
 * @brief Get file information
 *
 * @param filename Target filename
 * @param file_info Pointer to file info structure to fill
 * @return true if information retrieved successfully, false otherwise
 */
bool storage_get_file_info(const char *filename, storage_file_info_t *file_info);

/**
 * @brief Compress data using Zstandard
 *
 * @param input_data Pointer to input data
 * @param input_length Input data length
 * @param output_buffer Pointer to output buffer
 * @param output_buffer_size Output buffer size
 * @param compressed_length Pointer to store compressed length
 * @return true if compression successful, false otherwise
 */
bool storage_compress_data(const uint8_t *input_data, uint32_t input_length,
                          uint8_t *output_buffer, uint32_t output_buffer_size,
                          uint32_t *compressed_length);

/**
 * @brief Decompress data using Zstandard
 *
 * @param compressed_data Pointer to compressed data
 * @param compressed_length Compressed data length
 * @param output_buffer Pointer to output buffer
 * @param output_buffer_size Output buffer size
 * @param decompressed_length Pointer to store decompressed length
 * @return true if decompression successful, false otherwise
 */
bool storage_decompress_data(const uint8_t *compressed_data, uint32_t compressed_length,
                            uint8_t *output_buffer, uint32_t output_buffer_size,
                            uint32_t *decompressed_length);

/**
 * @brief Verify file integrity using SHA-256
 *
 * @param filename Target filename
 * @param expected_hash Pointer to expected SHA-256 hash (32 bytes)
 * @return true if integrity check passed, false otherwise
 */
bool storage_verify_integrity(const char *filename, const uint8_t *expected_hash);

/**
 * @brief Perform storage system maintenance
 *
 * @return true if maintenance completed successfully, false otherwise
 */
bool storage_system_maintenance(void);

/**
 * @brief Clean up old files based on age and priority
 *
 * @param max_age_days Maximum file age in days
 * @param priority_threshold Minimum priority to keep
 * @return true if cleanup completed successfully, false otherwise
 */
bool storage_cleanup_old_files(uint32_t max_age_days, storage_priority_t priority_threshold);

/**
 * @brief Format storage device (destructive operation)
 *
 * @return true if format successful, false otherwise
 */
bool storage_format_device(void);

/**
 * @brief Enter power-safe mode
 *
 * @return true if power-safe mode entered successfully, false otherwise
 */
bool storage_enter_power_safe_mode(void);

/**
 * @brief Exit power-safe mode
 *
 * @return true if power-safe mode exited successfully, false otherwise
 */
bool storage_exit_power_safe_mode(void);

/**
 * @brief Create data record from sensor data
 *
 * @param sensor_data Pointer to sensor data
 * @param data_length Sensor data length
 * @param priority Data priority
 * @param metadata Metadata string
 * @param record Pointer to record structure to fill
 * @return true if record created successfully, false otherwise
 */
bool storage_create_record(const uint8_t *sensor_data, uint32_t data_length,
                          storage_priority_t priority, const char *metadata,
                          storage_data_record_t *record);

/**
 * @brief Flush all pending write operations
 *
 * @return true if flush successful, false otherwise
 */
bool storage_flush_pending_operations(void);

/**
 * @brief Get storage error description
 *
 * @param error_code Error code
 * @param buffer Pointer to buffer for error description
 * @param buffer_size Size of buffer
 * @return true if description retrieved successfully, false otherwise
 */
bool storage_get_error_description(uint32_t error_code, char *buffer, uint16_t buffer_size);

/**
 * @brief Check if storage device is healthy
 *
 * @return true if device is healthy, false otherwise
 */
bool storage_device_health_check(void);

/**
 * @brief Get storage device information
 *
 * @param manufacturer Pointer to manufacturer string buffer
 * @param product Pointer to product string buffer
 * @param serial Pointer to serial string buffer
 * @param capacity_bytes Pointer to capacity in bytes
 * @param buffer_size Size of string buffers
 * @return true if information retrieved successfully, false otherwise
 */
bool storage_get_device_info(char *manufacturer, char *product, char *serial,
                            uint32_t *capacity_bytes, uint16_t buffer_size);

/**
 * @brief Set storage event callback
 *
 * @param callback Function pointer to event callback
 * @return true if callback set successfully, false otherwise
 */
bool storage_set_event_callback(void (*callback)(uint32_t event_code, const char *message));

/**
 * @brief Enable/disable storage compression
 *
 * @param enable true to enable compression, false to disable
 * @return true if operation successful, false otherwise
 */
bool storage_enable_compression(bool enable);

/**
 * @brief Set compression level
 *
 * @param level Compression level (1-22)
 * @return true if level set successfully, false otherwise
 */
bool storage_set_compression_level(uint32_t level);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_STORAGE_SYSTEM_H */
