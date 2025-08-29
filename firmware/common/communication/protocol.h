/**
 * @file protocol.h
 * @brief EsoCore RS-485 Communication Protocol Definitions
 *
 * This file defines the communication protocol used between the Edge device
 * and sensor modules over the RS-485 bus.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_PROTOCOL_H
#define ESOCORE_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Protocol Constants
 * ============================================================================ */

#define ESOCORE_PROTOCOL_VERSION        0x01
#define ESOCORE_MAX_PAYLOAD_SIZE        256
#define ESOCORE_MAX_SENSORS             254
#define ESOCORE_DEFAULT_BAUDRATE        115200
#define ESOCORE_PROTOCOL_TIMEOUT_MS     100

/* Frame delimiters */
#define ESOCORE_FRAME_START             0xAA
#define ESOCORE_FRAME_END               0x55
#define ESOCORE_FRAME_ESCAPE            0x1B

/* ============================================================================
 * Message Types
 * ============================================================================ */

typedef enum {
    /* System messages */
    MSG_TYPE_DISCOVERY_REQUEST  = 0x01,   /* Edge -> Sensors: Discover devices */
    MSG_TYPE_DISCOVERY_RESPONSE = 0x02,   /* Sensors -> Edge: Device info */
    MSG_TYPE_HEARTBEAT         = 0x03,   /* Bidirectional: Keep-alive */
    MSG_TYPE_STATUS_REQUEST    = 0x04,   /* Edge -> Sensors: Request status */
    MSG_TYPE_STATUS_RESPONSE   = 0x05,   /* Sensors -> Edge: Status info */

    /* Configuration messages */
    MSG_TYPE_CONFIG_REQUEST    = 0x10,   /* Edge -> Sensors: Request config */
    MSG_TYPE_CONFIG_RESPONSE   = 0x11,   /* Sensors -> Edge: Send config */
    MSG_TYPE_CONFIG_SET        = 0x12,   /* Edge -> Sensors: Set config */
    MSG_TYPE_CONFIG_ACK        = 0x13,   /* Sensors -> Edge: Config ACK */

    /* Data messages */
    MSG_TYPE_DATA_REQUEST      = 0x20,   /* Edge -> Sensors: Request data */
    MSG_TYPE_DATA_RESPONSE     = 0x21,   /* Sensors -> Edge: Send data */
    MSG_TYPE_DATA_STREAM       = 0x22,   /* Sensors -> Edge: Stream data */

    /* Control messages */
    MSG_TYPE_COMMAND           = 0x30,   /* Edge -> Sensors: Execute command */
    MSG_TYPE_COMMAND_ACK       = 0x31,   /* Sensors -> Edge: Command ACK */

    /* Error messages */
    MSG_TYPE_ERROR             = 0xFF,   /* Bidirectional: Error notification */
} esocore_msg_type_t;

/* ============================================================================
 * Device Types
 * ============================================================================ */

typedef enum {
    DEVICE_TYPE_EDGE           = 0x01,   /* Main Edge device */
    DEVICE_TYPE_SENSOR_VIBRATION = 0x10, /* Vibration sensor */
    DEVICE_TYPE_SENSOR_ACOUSTIC = 0x11,  /* Acoustic sensor */
    DEVICE_TYPE_SENSOR_CURRENT = 0x12,   /* Current sensor */
    DEVICE_TYPE_SENSOR_AIR_QUALITY = 0x13, /* Air quality sensor */
    DEVICE_TYPE_SENSOR_LIGHT   = 0x14,   /* Light sensor */
    DEVICE_TYPE_SENSOR_OIL_QUALITY = 0x15, /* Oil quality sensor */
    DEVICE_TYPE_SENSOR_PRESSURE = 0x16,  /* Pressure sensor */
    DEVICE_TYPE_SENSOR_TEMPERATURE = 0x17, /* Temperature sensor */
    DEVICE_TYPE_SENSOR_PROXIMITY = 0x18,  /* Proximity sensor */
} esocore_device_type_t;

/* ============================================================================
 * Error Codes
 * ============================================================================ */

typedef enum {
    ERROR_NONE                 = 0x00,   /* No error */
    ERROR_CRC_MISMATCH         = 0x01,   /* CRC check failed */
    ERROR_INVALID_FRAME        = 0x02,   /* Invalid frame format */
    ERROR_TIMEOUT              = 0x03,   /* Communication timeout */
    ERROR_BUFFER_OVERFLOW      = 0x04,   /* Buffer overflow */
    ERROR_INVALID_COMMAND      = 0x05,   /* Invalid command */
    ERROR_DEVICE_BUSY          = 0x06,   /* Device is busy */
    ERROR_HARDWARE_FAULT       = 0x07,   /* Hardware fault detected */
    ERROR_CONFIG_INVALID       = 0x08,   /* Invalid configuration */
    ERROR_SENSOR_FAULT         = 0x09,   /* Sensor fault */
    ERROR_POWER_LOW            = 0x0A,   /* Low power condition */
} esocore_error_t;

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/**
 * @brief Protocol frame header
 */
typedef struct {
    uint8_t start_delimiter;             /* Frame start (0xAA) */
    uint8_t version;                     /* Protocol version */
    uint8_t source_address;              /* Source device address */
    uint8_t destination_address;         /* Destination device address */
    uint8_t message_type;                /* Message type */
    uint16_t payload_length;             /* Payload length */
    uint16_t crc;                        /* CRC checksum of header */
} __attribute__((packed)) esocore_frame_header_t;

/**
 * @brief Complete protocol frame
 */
typedef struct {
    esocore_frame_header_t header;       /* Frame header */
    uint8_t payload[ESOCORE_MAX_PAYLOAD_SIZE]; /* Payload data */
    uint16_t payload_crc;                /* CRC of payload */
    uint8_t end_delimiter;               /* Frame end (0x55) */
} __attribute__((packed)) esocore_frame_t;

/**
 * @brief Device information structure
 */
typedef struct {
    uint8_t device_type;                 /* Device type */
    uint8_t hardware_version;            /* Hardware version */
    uint8_t firmware_version;            /* Firmware version */
    uint8_t capabilities;                /* Device capabilities flags */
    char device_name[32];                /* Human-readable device name */
    char serial_number[16];              /* Device serial number */
} __attribute__((packed)) esocore_device_info_t;

/**
 * @brief Sensor data header
 */
typedef struct {
    uint32_t timestamp;                  /* Unix timestamp */
    uint8_t sensor_count;                /* Number of sensors in this frame */
    uint8_t data_format;                 /* Data format version */
    uint16_t total_samples;              /* Total number of samples */
} __attribute__((packed)) esocore_sensor_data_header_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize the protocol layer
 *
 * @param device_address Device address (0-254, 255 = broadcast)
 * @param device_type Device type
 * @return true if initialization successful, false otherwise
 */
bool esocore_protocol_init(uint8_t device_address, esocore_device_type_t device_type);

/**
 * @brief Send a message to a specific device
 *
 * @param dest_address Destination device address
 * @param msg_type Message type
 * @param payload Pointer to payload data
 * @param payload_length Length of payload data
 * @return true if message sent successfully, false otherwise
 */
bool esocore_send_message(uint8_t dest_address, esocore_msg_type_t msg_type,
                         const uint8_t *payload, uint16_t payload_length);

/**
 * @brief Receive a message (non-blocking)
 *
 * @param source_address Pointer to store source address
 * @param msg_type Pointer to store message type
 * @param payload Pointer to payload buffer
 * @param max_payload_length Maximum payload buffer size
 * @param timeout_ms Timeout in milliseconds
 * @return Number of bytes received, 0 if no message, negative on error
 */
int esocore_receive_message(uint8_t *source_address, esocore_msg_type_t *msg_type,
                           uint8_t *payload, uint16_t max_payload_length,
                           uint32_t timeout_ms);

/**
 * @brief Calculate CRC16 checksum
 *
 * @param data Pointer to data
 * @param length Length of data
 * @return CRC16 checksum
 */
uint16_t esocore_crc16(const uint8_t *data, uint16_t length);

/**
 * @brief Validate a received frame
 *
 * @param frame Pointer to frame
 * @return true if frame is valid, false otherwise
 */
bool esocore_validate_frame(const esocore_frame_t *frame);

/**
 * @brief Create a discovery request message
 *
 * @param payload Pointer to payload buffer
 * @param max_length Maximum payload length
 * @return Actual payload length, 0 on error
 */
uint16_t esocore_create_discovery_request(uint8_t *payload, uint16_t max_length);

/**
 * @brief Create a discovery response message
 *
 * @param device_info Pointer to device information
 * @param payload Pointer to payload buffer
 * @param max_length Maximum payload length
 * @return Actual payload length, 0 on error
 */
uint16_t esocore_create_discovery_response(const esocore_device_info_t *device_info,
                                          uint8_t *payload, uint16_t max_length);

/**
 * @brief Parse device information from discovery response
 *
 * @param payload Pointer to payload data
 * @param payload_length Length of payload data
 * @param device_info Pointer to device info structure to fill
 * @return true if parsing successful, false otherwise
 */
bool esocore_parse_device_info(const uint8_t *payload, uint16_t payload_length,
                              esocore_device_info_t *device_info);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_PROTOCOL_H */
