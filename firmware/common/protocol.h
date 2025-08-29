/**
 * @file protocol.h
 * @brief EsoCore Communication Protocol
 *
 * This header defines the EsoCore communication protocol for RS-485 based
 * communication between the Edge device and sensor modules.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Protocol Configuration
 * ============================================================================ */

#define ESOCORE_PROTOCOL_VERSION              0x01
#define ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE     256
#define ESOCORE_PROTOCOL_HEADER_SIZE          8
#define ESOCORE_PROTOCOL_CRC_SIZE             2
#define ESOCORE_PROTOCOL_FRAME_SIZE           (ESOCORE_PROTOCOL_HEADER_SIZE + ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE + ESOCORE_PROTOCOL_CRC_SIZE)

#define ESOCORE_PROTOCOL_START_BYTE           0xAA
#define ESOCORE_PROTOCOL_ESCAPE_BYTE          0xBB
#define ESOCORE_PROTOCOL_END_BYTE             0xCC

#define ESOCORE_PROTOCOL_BROADCAST_ADDRESS    0xFF
#define ESOCORE_PROTOCOL_MASTER_ADDRESS       0x00

#define ESOCORE_PROTOCOL_DEFAULT_TIMEOUT_MS   1000
#define ESOCORE_PROTOCOL_RETRY_COUNT          3

/* ============================================================================
 * Protocol Data Types
 * ============================================================================ */

/**
 * @brief Device types in the EsoCore network
 */
typedef enum {
    ESOCORE_DEVICE_TYPE_MASTER     = 0x00,  /**< Edge device (master) */
    ESOCORE_DEVICE_TYPE_VIBRATION  = 0x01,  /**< Vibration sensor */
    ESOCORE_DEVICE_TYPE_ACOUSTIC   = 0x02,  /**< Acoustic sensor */
    ESOCORE_DEVICE_TYPE_CURRENT    = 0x03,  /**< Current sensor */
    ESOCORE_DEVICE_TYPE_AIR_QUALITY= 0x04,  /**< Air quality sensor */
    ESOCORE_DEVICE_TYPE_LIGHT      = 0x05,  /**< Light sensor */
    ESOCORE_DEVICE_TYPE_OIL_QUALITY= 0x06,  /**< Oil quality sensor */
    ESOCORE_DEVICE_TYPE_PRESSURE   = 0x07,  /**< Pressure sensor */
    ESOCORE_DEVICE_TYPE_TEMPERATURE= 0x08,  /**< Temperature sensor */
    ESOCORE_DEVICE_TYPE_PROXIMITY  = 0x09,  /**< Proximity sensor */
    ESOCORE_DEVICE_TYPE_MAX
} esocore_device_type_t;

/**
 * @brief Message types for protocol communication
 */
typedef enum {
    /* System messages */
    ESOCORE_MSG_DISCOVER           = 0x01,  /**< Device discovery */
    ESOCORE_MSG_DISCOVER_RESPONSE  = 0x02,  /**< Discovery response */
    ESOCORE_MSG_HEARTBEAT          = 0x03,  /**< Heartbeat message */
    ESOCORE_MSG_HEARTBEAT_RESPONSE = 0x04,  /**< Heartbeat response */
    ESOCORE_MSG_STATUS_REQUEST     = 0x05,  /**< Status request */
    ESOCORE_MSG_STATUS_RESPONSE    = 0x06,  /**< Status response */
    ESOCORE_MSG_CONFIG_REQUEST     = 0x07,  /**< Configuration request */
    ESOCORE_MSG_CONFIG_RESPONSE    = 0x08,  /**< Configuration response */
    ESOCORE_MSG_CONFIG_UPDATE      = 0x09,  /**< Configuration update */
    ESOCORE_MSG_CONFIG_ACK         = 0x0A,  /**< Configuration acknowledge */

    /* Data messages */
    ESOCORE_MSG_DATA_REQUEST       = 0x10,  /**< Data request */
    ESOCORE_MSG_DATA_RESPONSE      = 0x11,  /**< Data response */
    ESOCORE_MSG_DATA_BURST         = 0x12,  /**< Data burst transmission */
    ESOCORE_MSG_DATA_ACK           = 0x13,  /**< Data acknowledge */

    /* Control messages */
    ESOCORE_MSG_RESET              = 0x20,  /**< Reset device */
    ESOCORE_MSG_SLEEP              = 0x21,  /**< Enter sleep mode */
    ESOCORE_MSG_WAKE               = 0x22,  /**< Wake from sleep */
    ESOCORE_MSG_CALIBRATE          = 0x23,  /**< Start calibration */
    ESOCORE_MSG_CALIBRATE_STATUS   = 0x24,  /**< Calibration status */
    ESOCORE_MSG_SELF_TEST          = 0x25,  /**< Run self-test */
    ESOCORE_MSG_SELF_TEST_RESULT   = 0x26,  /**< Self-test result */

    /* Error messages */
    ESOCORE_MSG_ERROR              = 0x30,  /**< Error message */
    ESOCORE_MSG_NACK               = 0x31,  /**< Negative acknowledge */
    ESOCORE_MSG_TIMEOUT            = 0x32,  /**< Timeout indication */

    /* Firmware update messages */
    ESOCORE_MSG_FIRMWARE_UPDATE    = 0x40,  /**< Firmware update request */
    ESOCORE_MSG_FIRMWARE_DATA      = 0x41,  /**< Firmware data packet */
    ESOCORE_MSG_FIRMWARE_ACK       = 0x42,  /**< Firmware data acknowledge */
    ESOCORE_MSG_FIRMWARE_COMPLETE  = 0x43,  /**< Firmware update complete */
} esocore_message_type_t;

/**
 * @brief Command codes for sensor control
 */
typedef enum {
    /* Sensor commands */
    ESOCORE_CMD_SENSOR_START       = 0x01,  /**< Start sensor acquisition */
    ESOCORE_CMD_SENSOR_STOP        = 0x02,  /**< Stop sensor acquisition */
    ESOCORE_CMD_SENSOR_CONFIGURE   = 0x03,  /**< Configure sensor parameters */
    ESOCORE_CMD_SENSOR_CALIBRATE   = 0x04,  /**< Calibrate sensor */
    ESOCORE_CMD_SENSOR_RESET       = 0x05,  /**< Reset sensor to defaults */
    ESOCORE_CMD_SENSOR_GET_DATA    = 0x06,  /**< Get sensor data */
    ESOCORE_CMD_SENSOR_SET_THRESHOLD= 0x07, /**< Set detection threshold */
    ESOCORE_CMD_SENSOR_GET_STATUS  = 0x08,  /**< Get sensor status */

    /* Safety commands */
    ESOCORE_CMD_SAFETY_ENABLE      = 0x10,  /**< Enable safety functions */
    ESOCORE_CMD_SAFETY_DISABLE     = 0x11,  /**< Disable safety functions */
    ESOCORE_CMD_SAFETY_RESET       = 0x12,  /**< Reset safety state */
    ESOCORE_CMD_SAFETY_TEST        = 0x13,  /**< Test safety functions */

    /* System commands */
    ESOCORE_CMD_SYSTEM_INFO        = 0x20,  /**< Get system information */
    ESOCORE_CMD_SYSTEM_REBOOT      = 0x21,  /**< Reboot device */
    ESOCORE_CMD_SYSTEM_SHUTDOWN    = 0x22,  /**< Shutdown device */
    ESOCORE_CMD_SYSTEM_UPGRADE     = 0x23,  /**< Upgrade firmware */
} esocore_command_t;

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief Device information structure
 */
typedef struct {
    uint8_t address;                    /**< Device address (1-254) */
    esocore_device_type_t device_type;  /**< Type of device */
    uint8_t firmware_version_major;     /**< Major firmware version */
    uint8_t firmware_version_minor;     /**< Minor firmware version */
    uint8_t hardware_version;           /**< Hardware version */
    uint16_t serial_number;             /**< Device serial number */
    uint8_t capabilities;               /**< Device capabilities flags */
    uint8_t status_flags;               /**< Current status flags */
    uint32_t uptime_seconds;            /**< Device uptime in seconds */
} esocore_device_info_t;

/**
 * @brief Protocol message header
 */
typedef struct {
    uint8_t start_byte;                 /**< Start of frame marker */
    uint8_t source_address;             /**< Source device address */
    uint8_t destination_address;        /**< Destination device address */
    esocore_message_type_t message_type;/**< Type of message */
    uint8_t sequence_number;            /**< Sequence number for ordering */
    uint8_t flags;                      /**< Message flags */
    uint16_t payload_length;            /**< Length of payload data */
} __attribute__((packed)) esocore_message_header_t;

/**
 * @brief Complete protocol message
 */
typedef struct {
    esocore_message_header_t header;    /**< Message header */
    uint8_t payload[ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE]; /**< Message payload */
    uint16_t crc;                       /**< CRC-16 checksum */
} __attribute__((packed)) esocore_message_t;

/**
 * @brief Sensor data payload structure
 */
typedef struct {
    uint32_t timestamp;                 /**< Data timestamp (Unix time) */
    uint16_t data_points;               /**< Number of data points */
    uint8_t data_format;                /**< Data format identifier */
    uint8_t compression_type;           /**< Compression type used */
    uint8_t quality_flags;              /**< Data quality indicators */
    uint8_t reserved;                   /**< Reserved for future use */
    uint8_t data[];                     /**< Variable-length sensor data */
} __attribute__((packed)) esocore_sensor_data_t;

/**
 * @brief Configuration payload structure
 */
typedef struct {
    uint16_t parameter_id;              /**< Configuration parameter ID */
    uint8_t parameter_type;             /**< Parameter data type */
    uint8_t flags;                      /**< Configuration flags */
    uint32_t value;                     /**< Parameter value (32-bit) */
} __attribute__((packed)) esocore_config_payload_t;

/**
 * @brief Error information structure
 */
typedef struct {
    uint8_t error_code;                 /**< Error code identifier */
    uint8_t severity;                   /**< Error severity level */
    uint16_t error_info;                /**< Additional error information */
    char error_message[64];             /**< Human-readable error message */
} esocore_error_info_t;

/* ============================================================================
 * Protocol Constants and Flags
 * ============================================================================ */

/**
 * @brief Message flags
 */
#define ESOCORE_FLAG_ACK_REQUIRED       0x01   /**< Acknowledgment required */
#define ESOCORE_FLAG_HIGH_PRIORITY      0x02   /**< High priority message */
#define ESOCORE_FLAG_COMPRESSED         0x04   /**< Payload is compressed */
#define ESOCORE_FLAG_ENCRYPTED          0x08   /**< Payload is encrypted */
#define ESOCORE_FLAG_FRAGMENTED         0x10   /**< Message is fragmented */
#define ESOCORE_FLAG_LAST_FRAGMENT      0x20   /**< Last fragment of message */
#define ESOCORE_FLAG_BROADCAST          0x40   /**< Broadcast message */
#define ESOCORE_FLAG_SYSTEM_MESSAGE     0x80   /**< System-level message */

/**
 * @brief Device capability flags
 */
#define ESOCORE_CAPABILITY_SENSOR       0x01   /**< Device has sensor capability */
#define ESOCORE_CAPABILITY_ACTUATOR     0x02   /**< Device has actuator capability */
#define ESOCORE_CAPABILITY_STORAGE      0x04   /**< Device has storage capability */
#define ESOCORE_CAPABILITY_NETWORK      0x08   /**< Device has network capability */
#define ESOCORE_CAPABILITY_DISPLAY      0x10   /**< Device has display capability */
#define ESOCORE_CAPABILITY_AUDIO        0x20   /**< Device has audio capability */
#define ESOCORE_CAPABILITY_SAFETY       0x40   /**< Device supports safety features */
#define ESOCORE_CAPABILITY_EXTENDED     0x80   /**< Extended capabilities available */

/**
 * @brief Device status flags
 */
#define ESOCORE_STATUS_INITIALIZING     0x01   /**< Device is initializing */
#define ESOCORE_STATUS_READY            0x02   /**< Device is ready for operation */
#define ESOCORE_STATUS_ACTIVE           0x04   /**< Device is actively collecting data */
#define ESOCORE_STATUS_ERROR            0x08   /**< Device has an error condition */
#define ESOCORE_STATUS_CALIBRATING      0x10   /**< Device is calibrating */
#define ESOCORE_STATUS_LOW_POWER        0x20   /**< Device is in low-power mode */
#define ESOCORE_STATUS_MAINTENANCE      0x40   /**< Device is in maintenance mode */
#define ESOCORE_STATUS_FAULT            0x80   /**< Device has a fault condition */

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize the EsoCore protocol
 *
 * @param device_address Device address for this node
 * @param device_type Type of device being initialized
 * @return true if initialization successful, false otherwise
 */
bool esocore_protocol_init(uint8_t device_address, esocore_device_type_t device_type);

/**
 * @brief Send a protocol message
 *
 * @param destination_address Destination device address
 * @param message_type Type of message to send
 * @param payload Pointer to message payload
 * @param payload_length Length of payload data
 * @param flags Message flags
 * @return true if message sent successfully, false otherwise
 */
bool esocore_protocol_send_message(uint8_t destination_address,
                                  esocore_message_type_t message_type,
                                  const uint8_t *payload,
                                  uint16_t payload_length,
                                  uint8_t flags);

/**
 * @brief Receive a protocol message
 *
 * @param message Pointer to message structure to fill
 * @param timeout_ms Timeout in milliseconds
 * @return true if message received successfully, false otherwise
 */
bool esocore_protocol_receive_message(esocore_message_t *message, uint32_t timeout_ms);

/**
 * @brief Handle incoming protocol message
 *
 * @param message Pointer to received message
 * @return true if message handled successfully, false otherwise
 */
bool esocore_protocol_handle_message(const esocore_message_t *message);

/**
 * @brief Send discovery message to find devices on network
 *
 * @return true if discovery sent successfully, false otherwise
 */
bool esocore_protocol_send_discovery(void);

/**
 * @brief Send heartbeat message
 *
 * @return true if heartbeat sent successfully, false otherwise
 */
bool esocore_protocol_send_heartbeat(void);

/**
 * @brief Send sensor data message
 *
 * @param sensor_data Pointer to sensor data structure
 * @param data_size Size of sensor data
 * @return true if data sent successfully, false otherwise
 */
bool esocore_protocol_send_sensor_data(const esocore_sensor_data_t *sensor_data,
                                      uint16_t data_size);

/**
 * @brief Request configuration from master device
 *
 * @return true if request sent successfully, false otherwise
 */
bool esocore_protocol_request_configuration(void);

/**
 * @brief Send error message
 *
 * @param error_code Error code identifier
 * @param error_message Human-readable error message
 * @return true if error sent successfully, false otherwise
 */
bool esocore_protocol_send_error(uint8_t error_code, const char *error_message);

/**
 * @brief Calculate CRC-16 for message integrity
 *
 * @param data Pointer to data buffer
 * @param length Length of data
 * @return CRC-16 checksum
 */
uint16_t esocore_protocol_calculate_crc16(const uint8_t *data, uint16_t length);

/**
 * @brief Validate message CRC
 *
 * @param message Pointer to message to validate
 * @return true if CRC is valid, false otherwise
 */
bool esocore_protocol_validate_crc(const esocore_message_t *message);

/**
 * @brief Get device information for this node
 *
 * @param device_info Pointer to device info structure to fill
 * @return true if information retrieved successfully, false otherwise
 */
bool esocore_protocol_get_device_info(esocore_device_info_t *device_info);

/**
 * @brief Set device capabilities flags
 *
 * @param capabilities Capability flags to set
 * @return true if capabilities set successfully, false otherwise
 */
bool esocore_protocol_set_capabilities(uint8_t capabilities);

/**
 * @brief Get current protocol statistics
 *
 * @param messages_sent Pointer to store number of messages sent
 * @param messages_received Pointer to store number of messages received
 * @param errors_count Pointer to store number of protocol errors
 * @return true if statistics retrieved successfully, false otherwise
 */
bool esocore_protocol_get_statistics(uint32_t *messages_sent,
                                    uint32_t *messages_received,
                                    uint32_t *errors_count);

/**
 * @brief Reset protocol statistics
 *
 * @return true if statistics reset successfully, false otherwise
 */
bool esocore_protocol_reset_statistics(void);

/**
 * @brief Set protocol timeout values
 *
 * @param response_timeout_ms Response timeout in milliseconds
 * @param retry_count Number of retries for failed messages
 * @return true if timeouts set successfully, false otherwise
 */
bool esocore_protocol_set_timeouts(uint32_t response_timeout_ms, uint8_t retry_count);

/**
 * @brief Enable/disable protocol debugging
 *
 * @param enable true to enable debugging, false to disable
 * @return true if debug setting changed successfully, false otherwise
 */
bool esocore_protocol_enable_debug(bool enable);

#endif /* PROTOCOL_H */
