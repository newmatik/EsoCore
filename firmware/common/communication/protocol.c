/**
 * @file protocol.c
 * @brief EsoCore Communication Protocol Implementation
 *
 * This file contains the implementation of the EsoCore communication protocol for RS-485 based
 * communication between the Edge device and sensor modules.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "protocol.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Private Data Structures
 * ============================================================================ */

static uint8_t device_address = 0;
static esocore_device_type_t device_type = ESOCORE_DEVICE_TYPE_MASTER;
static bool protocol_initialized = false;

/* Protocol statistics */
static uint32_t messages_sent = 0;
static uint32_t messages_received = 0;
static uint32_t crc_errors = 0;
static uint32_t timeout_errors = 0;
static uint32_t protocol_errors = 0;

/* Message buffer */
static uint8_t receive_buffer[ESOCORE_PROTOCOL_FRAME_SIZE];
static uint32_t receive_buffer_index = 0;
static bool receiving_message = false;

/* Sequence number for outgoing messages */
static uint8_t sequence_number = 0;

/* ============================================================================
 * Hardware Abstraction Layer (HAL)
 * ============================================================================ */

/**
 * @brief Initialize RS-485 communication hardware
 *
 * @return true if hardware initialization successful, false otherwise
 */
static bool protocol_hw_init(void) {
    /* TODO: Implement hardware-specific RS-485 initialization */
    /* This would typically involve:
     * - Configuring UART/USART peripheral
     * - Setting up GPIO pins for RS-485 direction control
     * - Configuring baud rate, parity, stop bits
     * - Enabling interrupts for receive/transmit
     */
    return true;
}

/**
 * @brief Send data over RS-485
 *
 * @param data Pointer to data buffer
 * @param length Length of data to send
 * @return true if data sent successfully, false otherwise
 */
static bool protocol_hw_send(const uint8_t *data, uint32_t length) {
    /* TODO: Implement hardware data transmission */
    /* This would typically involve:
     * - Setting RS-485 transceiver to transmit mode
     * - Sending data via UART
     * - Waiting for transmission complete
     * - Setting transceiver back to receive mode
     */
    return true;
}

/**
 * @brief Receive data from RS-485
 *
 * @param data Pointer to receive buffer
 * @param max_length Maximum length to receive
 * @param timeout_ms Timeout in milliseconds
 * @return Number of bytes received, or negative on error
 */
static int32_t protocol_hw_receive(uint8_t *data, uint32_t max_length, uint32_t timeout_ms) {
    /* TODO: Implement hardware data reception */
    /* This would typically involve:
     * - Reading from UART receive buffer
     * - Handling timeouts
     * - Error detection and recovery
     */
    return 0;
}

/**
 * @brief Check if data is available for reception
 *
 * @return true if data is available, false otherwise
 */
static bool protocol_hw_data_available(void) {
    /* TODO: Check UART receive buffer status */
    return false;
}

/**
 * @brief Flush transmit and receive buffers
 *
 * @return true if buffers flushed successfully, false otherwise
 */
static bool protocol_hw_flush_buffers(void) {
    /* TODO: Flush UART buffers */
    return true;
}

/**
 * @brief Get current timestamp in milliseconds
 *
 * @return Current timestamp
 */
static uint32_t protocol_get_timestamp_ms(void) {
    /* TODO: Get current system timestamp */
    return 0;
}

/* ============================================================================
 * Protocol Core Functions
 * ============================================================================ */

/**
 * @brief Calculate CRC-16 for data integrity
 *
 * @param data Pointer to data buffer
 * @param length Length of data
 * @return CRC-16 checksum
 */
uint16_t esocore_protocol_calculate_crc16(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

/**
 * @brief Validate message CRC
 *
 * @param message Pointer to message to validate
 * @return true if CRC is valid, false otherwise
 */
bool esocore_protocol_validate_crc(const esocore_message_t *message) {
    if (!message) {
        return false;
    }

    uint16_t calculated_crc = esocore_protocol_calculate_crc16(
        (const uint8_t *)message,
        sizeof(esocore_message_header_t) + message->header.payload_length
    );

    return (calculated_crc == message->crc);
}

/**
 * @brief Build protocol message from components
 *
 * @param destination_address Destination device address
 * @param message_type Type of message
 * @param payload Pointer to message payload
 * @param payload_length Length of payload data
 * @param flags Message flags
 * @param message Pointer to message structure to fill
 * @return true if message built successfully, false otherwise
 */
static bool protocol_build_message(uint8_t destination_address, esocore_message_type_t message_type,
                                  const uint8_t *payload, uint16_t payload_length,
                                  uint8_t flags, esocore_message_t *message) {
    if (!message || payload_length > ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE) {
        return false;
    }

    /* Build message header */
    message->header.start_byte = ESOCORE_PROTOCOL_START_BYTE;
    message->header.source_address = device_address;
    message->header.destination_address = destination_address;
    message->header.message_type = message_type;
    message->header.sequence_number = sequence_number++;
    message->header.flags = flags;
    message->header.payload_length = payload_length;

    /* Copy payload */
    if (payload && payload_length > 0) {
        memcpy(message->payload, payload, payload_length);
    }

    /* Calculate and set CRC */
    uint16_t crc_data_length = sizeof(esocore_message_header_t) + payload_length;
    message->crc = esocore_protocol_calculate_crc16((const uint8_t *)message, crc_data_length);

    return true;
}

/**
 * @brief Parse received data into message structure
 *
 * @param data Pointer to received data
 * @param data_length Length of received data
 * @param message Pointer to message structure to fill
 * @return true if message parsed successfully, false otherwise
 */
static bool protocol_parse_message(const uint8_t *data, uint32_t data_length,
                                  esocore_message_t *message) {
    if (!data || !message || data_length < sizeof(esocore_message_header_t)) {
        return false;
    }

    /* Copy header */
    memcpy(&message->header, data, sizeof(esocore_message_header_t));

    /* Validate start byte */
    if (message->header.start_byte != ESOCORE_PROTOCOL_START_BYTE) {
        protocol_errors++;
        return false;
    }

    /* Check payload length */
    if (message->header.payload_length > ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE) {
        protocol_errors++;
        return false;
    }

    /* Check if we have enough data for complete message */
    uint32_t expected_length = sizeof(esocore_message_header_t) +
                              message->header.payload_length +
                              ESOCORE_PROTOCOL_CRC_SIZE;

    if (data_length < expected_length) {
        return false; /* Incomplete message */
    }

    /* Copy payload */
    if (message->header.payload_length > 0) {
        memcpy(message->payload, data + sizeof(esocore_message_header_t),
               message->header.payload_length);
    }

    /* Copy CRC */
    memcpy(&message->crc,
           data + sizeof(esocore_message_header_t) + message->header.payload_length,
           ESOCORE_PROTOCOL_CRC_SIZE);

    return true;
}

/**
 * @brief Send acknowledgement for received message
 *
 * @param original_message Pointer to original message
 * @param success true for ACK, false for NACK
 * @return true if acknowledgement sent successfully, false otherwise
 */
static bool protocol_send_ack(const esocore_message_t *original_message, bool success) {
    uint8_t ack_payload[2];
    ack_payload[0] = original_message->header.sequence_number;
    ack_payload[1] = success ? 0x01 : 0x00;

    return esocore_protocol_send_message(
        original_message->header.source_address,
        success ? ESOCORE_MSG_DATA_ACK : ESOCORE_MSG_NACK,
        ack_payload, sizeof(ack_payload),
        ESOCORE_FLAG_ACK_REQUIRED
    );
}

/**
 * @brief Handle system messages
 *
 * @param message Pointer to received message
 * @return true if message handled successfully, false otherwise
 */
static bool protocol_handle_system_message(const esocore_message_t *message) {
    switch (message->header.message_type) {
        case ESOCORE_MSG_DISCOVER:
            /* Send discovery response */
            {
                esocore_device_info_t device_info;
                if (esocore_protocol_get_device_info(&device_info)) {
                    return esocore_protocol_send_message(
                        message->header.source_address,
                        ESOCORE_MSG_DISCOVER_RESPONSE,
                        (const uint8_t *)&device_info, sizeof(device_info),
                        ESOCORE_FLAG_ACK_REQUIRED
                    );
                }
            }
            break;

        case ESOCORE_MSG_HEARTBEAT:
            /* Send heartbeat response */
            return esocore_protocol_send_message(
                message->header.source_address,
                ESOCORE_MSG_HEARTBEAT_RESPONSE,
                NULL, 0,
                0
            );

        case ESOCORE_MSG_STATUS_REQUEST:
            /* Send status response - would need to implement status collection */
            return esocore_protocol_send_message(
                message->header.source_address,
                ESOCORE_MSG_STATUS_RESPONSE,
                NULL, 0, /* TODO: Add actual status data */
                0
            );

        case ESOCORE_MSG_RESET:
            /* Handle reset command */
            /* TODO: Implement device reset */
            return true;

        case ESOCORE_MSG_DATA_ACK:
        case ESOCORE_MSG_NACK:
            /* Handle acknowledgements */
            return true;

        default:
            return false;
    }

    return false;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

/**
 * @brief Initialize the EsoCore protocol
 */
bool esocore_protocol_init(uint8_t device_address_param, esocore_device_type_t device_type_param) {
    if (protocol_initialized) {
        return false;
    }

    device_address = device_address_param;
    device_type = device_type_param;

    /* Initialize hardware */
    if (!protocol_hw_init()) {
        return false;
    }

    /* Reset statistics */
    messages_sent = 0;
    messages_received = 0;
    crc_errors = 0;
    timeout_errors = 0;
    protocol_errors = 0;

    /* Reset sequence number */
    sequence_number = 0;

    /* Reset receive state */
    receive_buffer_index = 0;
    receiving_message = false;

    protocol_initialized = true;
    return true;
}

/**
 * @brief Send a protocol message
 */
bool esocore_protocol_send_message(uint8_t destination_address,
                                  esocore_message_type_t message_type,
                                  const uint8_t *payload,
                                  uint16_t payload_length,
                                  uint8_t flags) {
    if (!protocol_initialized) {
        return false;
    }

    /* Build message */
    esocore_message_t message;
    if (!protocol_build_message(destination_address, message_type, payload,
                               payload_length, flags, &message)) {
        protocol_errors++;
        return false;
    }

    /* Send message */
    uint32_t message_size = sizeof(esocore_message_header_t) + payload_length + ESOCORE_PROTOCOL_CRC_SIZE;
    if (!protocol_hw_send((const uint8_t *)&message, message_size)) {
        protocol_errors++;
        return false;
    }

    messages_sent++;

    /* Wait for acknowledgement if required */
    if (flags & ESOCORE_FLAG_ACK_REQUIRED) {
        /* TODO: Implement acknowledgement waiting */
    }

    return true;
}

/**
 * @brief Receive a protocol message
 */
bool esocore_protocol_receive_message(esocore_message_t *message, uint32_t timeout_ms) {
    if (!protocol_initialized || !message) {
        return false;
    }

    uint32_t start_time = protocol_get_timestamp_ms();
    uint32_t bytes_received = 0;

    /* Receive data with timeout */
    while (bytes_received < sizeof(esocore_message_t)) {
        if (protocol_get_timestamp_ms() - start_time > timeout_ms) {
            timeout_errors++;
            return false;
        }

        int32_t received = protocol_hw_receive(
            receive_buffer + bytes_received,
            sizeof(receive_buffer) - bytes_received,
            10 /* Small timeout for polling */
        );

        if (received > 0) {
            bytes_received += received;
        } else if (received < 0) {
            protocol_errors++;
            return false;
        }

        /* Check if we have a complete message */
        if (bytes_received >= sizeof(esocore_message_header_t)) {
            esocore_message_header_t temp_header;
            memcpy(&temp_header, receive_buffer, sizeof(esocore_message_header_t));

            if (temp_header.start_byte == ESOCORE_PROTOCOL_START_BYTE) {
                uint32_t expected_size = sizeof(esocore_message_header_t) +
                                        temp_header.payload_length +
                                        ESOCORE_PROTOCOL_CRC_SIZE;

                if (bytes_received >= expected_size) {
                    /* Parse the complete message */
                    if (protocol_parse_message(receive_buffer, bytes_received, message)) {
                        /* Validate CRC */
                        if (esocore_protocol_validate_crc(message)) {
                            messages_received++;

                            /* Check if message is for us */
                            if (message->header.destination_address == device_address ||
                                message->header.destination_address == ESOCORE_PROTOCOL_BROADCAST_ADDRESS) {

                                /* Send acknowledgement if required */
                                if (message->header.flags & ESOCORE_FLAG_ACK_REQUIRED) {
                                    protocol_send_ack(message, true);
                                }

                                return true;
                            }
                        } else {
                            crc_errors++;
                            /* Send NACK for CRC error */
                            protocol_send_ack(message, false);
                        }
                    } else {
                        protocol_errors++;
                    }

                    /* Reset receive buffer */
                    bytes_received = 0;
                }
            } else {
                /* Not a valid start byte, continue receiving */
            }
        }
    }

    return false;
}

/**
 * @brief Handle incoming protocol message
 */
bool esocore_protocol_handle_message(const esocore_message_t *message) {
    if (!message) {
        return false;
    }

    /* Route message based on type */
    switch (message->header.message_type) {
        case ESOCORE_MSG_DISCOVER:
        case ESOCORE_MSG_HEARTBEAT:
        case ESOCORE_MSG_STATUS_REQUEST:
        case ESOCORE_MSG_RESET:
        case ESOCORE_MSG_DATA_ACK:
        case ESOCORE_MSG_NACK:
            return protocol_handle_system_message(message);

        /* TODO: Handle other message types */
        default:
            return false;
    }
}

/**
 * @brief Send discovery message to find devices on network
 */
bool esocore_protocol_send_discovery(void) {
    return esocore_protocol_send_message(
        ESOCORE_PROTOCOL_BROADCAST_ADDRESS,
        ESOCORE_MSG_DISCOVER,
        NULL, 0,
        ESOCORE_FLAG_BROADCAST
    );
}

/**
 * @brief Send heartbeat message
 */
bool esocore_protocol_send_heartbeat(void) {
    /* Include basic device status in heartbeat */
    uint8_t heartbeat_data[4];
    heartbeat_data[0] = (uint8_t)device_type;
    heartbeat_data[1] = 0; /* TODO: Add device status */
    heartbeat_data[2] = 0; /* TODO: Add uptime high byte */
    heartbeat_data[3] = 0; /* TODO: Add uptime low byte */

    return esocore_protocol_send_message(
        ESOCORE_PROTOCOL_MASTER_ADDRESS,
        ESOCORE_MSG_HEARTBEAT,
        heartbeat_data, sizeof(heartbeat_data),
        0
    );
}

/**
 * @brief Send sensor data message
 */
bool esocore_protocol_send_sensor_data(const esocore_sensor_data_t *sensor_data,
                                      uint16_t data_size) {
    if (!sensor_data || data_size > ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE) {
        return false;
    }

    return esocore_protocol_send_message(
        ESOCORE_PROTOCOL_MASTER_ADDRESS,
        ESOCORE_MSG_DATA_RESPONSE,
        (const uint8_t *)sensor_data, data_size,
        ESOCORE_FLAG_ACK_REQUIRED
    );
}

/**
 * @brief Request configuration from master device
 */
bool esocore_protocol_request_configuration(void) {
    return esocore_protocol_send_message(
        ESOCORE_PROTOCOL_MASTER_ADDRESS,
        ESOCORE_MSG_CONFIG_REQUEST,
        NULL, 0,
        ESOCORE_FLAG_ACK_REQUIRED
    );
}

/**
 * @brief Send error message
 */
bool esocore_protocol_send_error(uint8_t error_code, const char *error_message) {
    uint8_t error_payload[65]; /* Error code + message */
    error_payload[0] = error_code;

    uint16_t message_len = 1;
    if (error_message) {
        uint16_t copy_len = strlen(error_message);
        if (copy_len > 64) {
            copy_len = 64;
        }
        memcpy(&error_payload[1], error_message, copy_len);
        message_len += copy_len;
    }

    return esocore_protocol_send_message(
        ESOCORE_PROTOCOL_MASTER_ADDRESS,
        ESOCORE_MSG_ERROR,
        error_payload, message_len,
        0
    );
}

/**
 * @brief Get device information for this node
 */
bool esocore_protocol_get_device_info(esocore_device_info_t *device_info) {
    if (!device_info) {
        return false;
    }

    memset(device_info, 0, sizeof(esocore_device_info_t));

    device_info->address = device_address;
    device_info->device_type = device_type;
    device_info->firmware_version_major = 1;
    device_info->firmware_version_minor = 0;
    device_info->hardware_version = 1;
    device_info->serial_number = 12345; /* TODO: Use actual serial number */
    device_info->capabilities = ESOCORE_CAPABILITY_SENSOR; /* TODO: Set based on actual capabilities */
    device_info->status_flags = ESOCORE_STATUS_READY; /* TODO: Set based on actual status */
    device_info->uptime_seconds = 0; /* TODO: Track actual uptime */

    return true;
}

/**
 * @brief Set device capabilities flags
 */
bool esocore_protocol_set_capabilities(uint8_t capabilities) {
    /* TODO: Store capabilities for device info */
    return true;
}

/**
 * @brief Get current protocol statistics
 */
bool esocore_protocol_get_statistics(uint32_t *messages_sent_out,
                                    uint32_t *messages_received_out,
                                    uint32_t *errors_count_out) {
    if (messages_sent_out) {
        *messages_sent_out = messages_sent;
    }
    if (messages_received_out) {
        *messages_received_out = messages_received;
    }
    if (errors_count_out) {
        *errors_count_out = crc_errors + timeout_errors + protocol_errors;
    }

    return true;
}

/**
 * @brief Reset protocol statistics
 */
bool esocore_protocol_reset_statistics(void) {
    messages_sent = 0;
    messages_received = 0;
    crc_errors = 0;
    timeout_errors = 0;
    protocol_errors = 0;

    return true;
}

/**
 * @brief Set protocol timeout values
 */
bool esocore_protocol_set_timeouts(uint32_t response_timeout_ms, uint8_t retry_count) {
    /* TODO: Store timeout values for use in message transmission */
    return true;
}

/**
 * @brief Enable/disable protocol debugging
 */
bool esocore_protocol_enable_debug(bool enable) {
    /* TODO: Enable/disable debug output */
    return true;
}