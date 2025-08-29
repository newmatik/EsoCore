/**
 * @file modbus_rtu.c
 * @brief Modbus RTU Protocol Implementation
 *
 * This file contains the implementation of the Modbus RTU protocol for industrial
 * communication over RS-485. It supports both master and slave modes with
 * comprehensive error handling and diagnostics.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "modbus_rtu.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Private Data Structures
 * ============================================================================ */

static modbus_config_t modbus_config;
static modbus_data_map_t modbus_data_map;
static modbus_statistics_t modbus_statistics;
static bool modbus_initialized = false;
static uint8_t modbus_rx_buffer[MODBUS_MAX_FRAME_SIZE];
static uint8_t modbus_tx_buffer[MODBUS_MAX_FRAME_SIZE];
static uint16_t modbus_rx_index = 0;

/* Function pointer for coil/register callbacks */
static bool (*coil_callback)(uint16_t address, bool *value, bool write) = NULL;
static bool (*register_callback)(uint16_t address, uint16_t *value, bool write) = NULL;

/* ============================================================================
 * CRC-16 Calculation (Modbus Standard)
 * ============================================================================ */

/**
 * @brief Calculate CRC-16 for Modbus
 *
 * @param data Pointer to data buffer
 * @param length Length of data
 * @return CRC-16 value
 */
uint16_t modbus_calculate_crc(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

/**
 * @brief Validate Modbus frame CRC
 *
 * @param frame Pointer to Modbus frame
 * @return true if CRC is valid, false otherwise
 */
bool modbus_validate_crc(const modbus_frame_t *frame) {
    if (!frame || !frame->data) {
        return false;
    }

    uint8_t crc_data[MODBUS_MAX_FRAME_SIZE];
    uint16_t crc_data_length = 0;

    /* Build data for CRC calculation */
    crc_data[crc_data_length++] = frame->address;
    crc_data[crc_data_length++] = frame->function_code;

    if (frame->data && frame->data_length > 0) {
        memcpy(&crc_data[crc_data_length], frame->data, frame->data_length);
        crc_data_length += frame->data_length;
    }

    uint16_t calculated_crc = modbus_calculate_crc(crc_data, crc_data_length);

    return (calculated_crc == frame->crc);
}

/* ============================================================================
 * Frame Building and Parsing
 * ============================================================================ */

/**
 * @brief Build Modbus RTU frame
 *
 * @param address Slave address
 * @param function_code Function code
 * @param data Pointer to data payload
 * @param data_length Data length
 * @param frame Pointer to frame structure to fill
 * @return true if frame built successfully, false otherwise
 */
static bool modbus_build_frame(uint8_t address, uint8_t function_code,
                              const uint8_t *data, uint16_t data_length,
                              modbus_frame_t *frame) {
    if (!frame || data_length > MODBUS_MAX_FRAME_SIZE - 4) {
        return false;
    }

    frame->address = address;
    frame->function_code = function_code;
    frame->data_length = data_length;
    frame->timestamp = 0; /* TODO: Add timestamp */

    if (data && data_length > 0) {
        frame->data = modbus_tx_buffer;
        memcpy(frame->data, data, data_length);
    } else {
        frame->data = NULL;
    }

    /* Calculate CRC */
    uint8_t crc_data[MODBUS_MAX_FRAME_SIZE];
    uint16_t crc_data_length = 0;

    crc_data[crc_data_length++] = address;
    crc_data[crc_data_length++] = function_code;

    if (data && data_length > 0) {
        memcpy(&crc_data[crc_data_length], data, data_length);
        crc_data_length += data_length;
    }

    frame->crc = modbus_calculate_crc(crc_data, crc_data_length);

    return true;
}

/**
 * @brief Parse Modbus RTU frame from raw data
 *
 * @param raw_data Pointer to raw frame data
 * @param data_length Length of raw data
 * @param frame Pointer to frame structure to fill
 * @return true if frame parsed successfully, false otherwise
 */
static bool modbus_parse_frame(const uint8_t *raw_data, uint16_t data_length,
                              modbus_frame_t *frame) {
    if (!raw_data || !frame || data_length < 4) {
        return false;
    }

    frame->address = raw_data[0];
    frame->function_code = raw_data[1];
    frame->data_length = data_length - 4; /* Subtract address, function, CRC */
    frame->timestamp = 0; /* TODO: Add timestamp */

    if (frame->data_length > 0) {
        frame->data = modbus_rx_buffer;
        memcpy(frame->data, &raw_data[2], frame->data_length);
    } else {
        frame->data = NULL;
    }

    /* Extract CRC */
    frame->crc = (raw_data[data_length - 1] << 8) | raw_data[data_length - 2];

    return modbus_validate_crc(frame);
}

/* ============================================================================
 * Modbus Function Code Handlers
 * ============================================================================ */

/**
 * @brief Handle Read Coils function (0x01)
 *
 * @param frame Pointer to received frame
 * @param response_buffer Pointer to response buffer
 * @param response_length Pointer to response length
 * @return true if handled successfully, false otherwise
 */
static bool modbus_handle_read_coils(const modbus_frame_t *frame,
                                    uint8_t *response_buffer, uint16_t *response_length) {
    if (!frame || frame->data_length != 4 || !response_buffer || !response_length) {
        return false;
    }

    uint16_t start_address = (frame->data[0] << 8) | frame->data[1];
    uint16_t quantity = (frame->data[2] << 8) | frame->data[3];

    /* Validate request */
    if (quantity < 1 || quantity > 2000 || start_address + quantity > modbus_data_map.num_coils) {
        return false;
    }

    /* Build response */
    uint8_t byte_count = (quantity + 7) / 8;
    response_buffer[0] = byte_count;

    /* Pack coil values into bytes */
    for (uint16_t i = 0; i < quantity; i++) {
        uint16_t coil_address = start_address + i;
        bool coil_value = false;

        if (coil_callback) {
            coil_callback(coil_address, &coil_value, false);
        } else if (coil_address < modbus_data_map.num_coils) {
            coil_value = modbus_data_map.coils[coil_address];
        }

        if (coil_value) {
            response_buffer[1 + (i / 8)] |= (1 << (i % 8));
        }
    }

    *response_length = 1 + byte_count;
    return true;
}

/**
 * @brief Handle Read Holding Registers function (0x03)
 *
 * @param frame Pointer to received frame
 * @param response_buffer Pointer to response buffer
 * @param response_length Pointer to response length
 * @return true if handled successfully, false otherwise
 */
static bool modbus_handle_read_holding_registers(const modbus_frame_t *frame,
                                                uint8_t *response_buffer, uint16_t *response_length) {
    if (!frame || frame->data_length != 4 || !response_buffer || !response_length) {
        return false;
    }

    uint16_t start_address = (frame->data[0] << 8) | frame->data[1];
    uint16_t quantity = (frame->data[2] << 8) | frame->data[3];

    /* Validate request */
    if (quantity < 1 || quantity > 125 || start_address + quantity > modbus_data_map.num_holding_registers) {
        return false;
    }

    /* Build response */
    uint8_t byte_count = quantity * 2;
    response_buffer[0] = byte_count;

    for (uint16_t i = 0; i < quantity; i++) {
        uint16_t reg_address = start_address + i;
        uint16_t reg_value = 0;

        if (register_callback) {
            register_callback(reg_address, &reg_value, false);
        } else if (reg_address < modbus_data_map.num_holding_registers) {
            reg_value = modbus_data_map.holding_registers[reg_address];
        }

        response_buffer[1 + (i * 2)] = (reg_value >> 8) & 0xFF;
        response_buffer[2 + (i * 2)] = reg_value & 0xFF;
    }

    *response_length = 1 + byte_count;
    return true;
}

/**
 * @brief Handle Write Single Coil function (0x05)
 *
 * @param frame Pointer to received frame
 * @param response_buffer Pointer to response buffer
 * @param response_length Pointer to response length
 * @return true if handled successfully, false otherwise
 */
static bool modbus_handle_write_single_coil(const modbus_frame_t *frame,
                                           uint8_t *response_buffer, uint16_t *response_length) {
    if (!frame || frame->data_length != 4 || !response_buffer || !response_length) {
        return false;
    }

    uint16_t coil_address = (frame->data[0] << 8) | frame->data[1];
    uint16_t coil_value = (frame->data[2] << 8) | frame->data[3];

    /* Validate coil value (should be 0x0000 or 0xFF00) */
    if (coil_value != 0x0000 && coil_value != 0xFF00) {
        return false;
    }

    bool coil_bool_value = (coil_value == 0xFF00);

    if (coil_callback) {
        if (!coil_callback(coil_address, &coil_bool_value, true)) {
            return false;
        }
    } else if (coil_address < modbus_data_map.num_coils) {
        modbus_data_map.coils[coil_address] = coil_bool_value;
    } else {
        return false;
    }

    /* Echo the request as response */
    memcpy(response_buffer, frame->data, frame->data_length);
    *response_length = frame->data_length;
    return true;
}

/**
 * @brief Handle Write Multiple Coils function (0x0F)
 *
 * @param frame Pointer to received frame
 * @param response_buffer Pointer to response buffer
 * @param response_length Pointer to response length
 * @return true if handled successfully, false otherwise
 */
static bool modbus_handle_write_multiple_coils(const modbus_frame_t *frame,
                                              uint8_t *response_buffer, uint16_t *response_length) {
    if (!frame || frame->data_length < 6 || !response_buffer || !response_length) {
        return false;
    }

    uint16_t start_address = (frame->data[0] << 8) | frame->data[1];
    uint16_t quantity = (frame->data[2] << 8) | frame->data[3];
    uint8_t byte_count = frame->data[4];

    /* Validate request */
    if (quantity < 1 || quantity > 1968 || start_address + quantity > modbus_data_map.num_coils) {
        return false;
    }

    if (byte_count != ((quantity + 7) / 8)) {
        return false;
    }

    /* Write coil values */
    for (uint16_t i = 0; i < quantity; i++) {
        uint16_t coil_address = start_address + i;
        bool coil_value = (frame->data[5 + (i / 8)] & (1 << (i % 8))) != 0;

        if (coil_callback) {
            if (!coil_callback(coil_address, &coil_value, true)) {
                return false;
            }
        } else if (coil_address < modbus_data_map.num_coils) {
            modbus_data_map.coils[coil_address] = coil_value;
        } else {
            return false;
        }
    }

    /* Return start address and quantity as response */
    response_buffer[0] = (start_address >> 8) & 0xFF;
    response_buffer[1] = start_address & 0xFF;
    response_buffer[2] = (quantity >> 8) & 0xFF;
    response_buffer[3] = quantity & 0xFF;
    *response_length = 4;
    return true;
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

/**
 * @brief Initialize Modbus RTU protocol
 */
bool modbus_rtu_init(const modbus_config_t *config) {
    if (!config || modbus_initialized) {
        return false;
    }

    memcpy(&modbus_config, config, sizeof(modbus_config_t));

    /* Initialize data map */
    memset(&modbus_data_map, 0, sizeof(modbus_data_map_t));
    memset(&modbus_statistics, 0, sizeof(modbus_statistics_t));

    /* Allocate memory for data arrays if sizes are specified */
    if (modbus_config.max_coils > 0) {
        modbus_data_map.coils = (bool *)malloc(modbus_config.max_coils * sizeof(bool));
        if (!modbus_data_map.coils) {
            return false;
        }
        modbus_data_map.num_coils = modbus_config.max_coils;
        memset(modbus_data_map.coils, 0, modbus_config.max_coils * sizeof(bool));
    }

    if (modbus_config.max_registers > 0) {
        modbus_data_map.holding_registers = (uint16_t *)malloc(modbus_config.max_registers * sizeof(uint16_t));
        if (!modbus_data_map.holding_registers) {
            return false;
        }
        modbus_data_map.num_holding_registers = modbus_config.max_registers;
        memset(modbus_data_map.holding_registers, 0, modbus_config.max_registers * sizeof(uint16_t));
    }

    modbus_initialized = true;
    return true;
}

/**
 * @brief Deinitialize Modbus RTU protocol
 */
bool modbus_rtu_deinit(void) {
    if (!modbus_initialized) {
        return false;
    }

    /* Free allocated memory */
    if (modbus_data_map.coils) {
        free(modbus_data_map.coils);
        modbus_data_map.coils = NULL;
    }

    if (modbus_data_map.holding_registers) {
        free(modbus_data_map.holding_registers);
        modbus_data_map.holding_registers = NULL;
    }

    if (modbus_data_map.discrete_inputs) {
        free(modbus_data_map.discrete_inputs);
        modbus_data_map.discrete_inputs = NULL;
    }

    if (modbus_data_map.input_registers) {
        free(modbus_data_map.input_registers);
        modbus_data_map.input_registers = NULL;
    }

    modbus_initialized = false;
    return true;
}

/**
 * @brief Initialize Modbus data map
 */
bool modbus_init_data_map(modbus_data_map_t *data_map) {
    if (!data_map || !modbus_initialized) {
        return false;
    }

    memcpy(&modbus_data_map, data_map, sizeof(modbus_data_map_t));
    return true;
}

/**
 * @brief Read coils from slave device
 */
bool modbus_read_coils(uint8_t slave_address, uint16_t start_address,
                      uint16_t quantity, bool *coil_values) {
    if (!modbus_initialized || modbus_config.mode != MODBUS_MODE_MASTER ||
        !coil_values || quantity < 1 || quantity > 2000) {
        return false;
    }

    uint8_t request_data[4];
    request_data[0] = (start_address >> 8) & 0xFF;
    request_data[1] = start_address & 0xFF;
    request_data[2] = (quantity >> 8) & 0xFF;
    request_data[3] = quantity & 0xFF;

    uint8_t response_buffer[MODBUS_MAX_FRAME_SIZE];
    uint16_t response_length;

    if (!modbus_send_request(slave_address, MODBUS_FC_READ_COILS,
                           request_data, 4, response_buffer, &response_length,
                           MODBUS_MAX_FRAME_SIZE)) {
        return false;
    }

    /* Parse response */
    if (response_length < 2) {
        return false;
    }

    uint8_t byte_count = response_buffer[0];
    uint16_t expected_bytes = (quantity + 7) / 8;

    if (byte_count != expected_bytes || response_length != (1 + byte_count)) {
        return false;
    }

    /* Unpack coil values */
    for (uint16_t i = 0; i < quantity; i++) {
        coil_values[i] = (response_buffer[1 + (i / 8)] & (1 << (i % 8))) != 0;
    }

    modbus_statistics.successful_requests++;
    return true;
}

/**
 * @brief Read holding registers from slave device
 */
bool modbus_read_holding_registers(uint8_t slave_address, uint16_t start_address,
                                  uint16_t quantity, uint16_t *register_values) {
    if (!modbus_initialized || modbus_config.mode != MODBUS_MODE_MASTER ||
        !register_values || quantity < 1 || quantity > 125) {
        return false;
    }

    uint8_t request_data[4];
    request_data[0] = (start_address >> 8) & 0xFF;
    request_data[1] = start_address & 0xFF;
    request_data[2] = (quantity >> 8) & 0xFF;
    request_data[3] = quantity & 0xFF;

    uint8_t response_buffer[MODBUS_MAX_FRAME_SIZE];
    uint16_t response_length;

    if (!modbus_send_request(slave_address, MODBUS_FC_READ_HOLDING_REGISTERS,
                           request_data, 4, response_buffer, &response_length,
                           MODBUS_MAX_FRAME_SIZE)) {
        return false;
    }

    /* Parse response */
    if (response_length < 2) {
        return false;
    }

    uint8_t byte_count = response_buffer[0];
    uint16_t expected_bytes = quantity * 2;

    if (byte_count != expected_bytes || response_length != (1 + byte_count)) {
        return false;
    }

    /* Extract register values */
    for (uint16_t i = 0; i < quantity; i++) {
        register_values[i] = (response_buffer[1 + (i * 2)] << 8) |
                            response_buffer[2 + (i * 2)];
    }

    modbus_statistics.successful_requests++;
    return true;
}

/**
 * @brief Write single coil to slave device
 */
bool modbus_write_single_coil(uint8_t slave_address, uint16_t coil_address, bool coil_value) {
    if (!modbus_initialized || modbus_config.mode != MODBUS_MODE_MASTER) {
        return false;
    }

    uint8_t request_data[4];
    request_data[0] = (coil_address >> 8) & 0xFF;
    request_data[1] = coil_address & 0xFF;
    request_data[2] = coil_value ? 0xFF : 0x00;
    request_data[3] = 0x00;

    uint8_t response_buffer[MODBUS_MAX_FRAME_SIZE];
    uint16_t response_length;

    if (!modbus_send_request(slave_address, MODBUS_FC_WRITE_SINGLE_COIL,
                           request_data, 4, response_buffer, &response_length,
                           MODBUS_MAX_FRAME_SIZE)) {
        return false;
    }

    /* Verify response matches request */
    if (response_length != 4 ||
        memcmp(request_data, response_buffer, 4) != 0) {
        return false;
    }

    modbus_statistics.successful_requests++;
    return true;
}

/**
 * @brief Write multiple coils to slave device
 */
bool modbus_write_multiple_coils(uint8_t slave_address, uint16_t start_address,
                                uint16_t quantity, const bool *coil_values) {
    if (!modbus_initialized || modbus_config.mode != MODBUS_MODE_MASTER ||
        !coil_values || quantity < 1 || quantity > 1968) {
        return false;
    }

    uint8_t byte_count = (quantity + 7) / 8;
    uint8_t request_data[5 + byte_count];

    request_data[0] = (start_address >> 8) & 0xFF;
    request_data[1] = start_address & 0xFF;
    request_data[2] = (quantity >> 8) & 0xFF;
    request_data[3] = quantity & 0xFF;
    request_data[4] = byte_count;

    /* Pack coil values into bytes */
    memset(&request_data[5], 0, byte_count);
    for (uint16_t i = 0; i < quantity; i++) {
        if (coil_values[i]) {
            request_data[5 + (i / 8)] |= (1 << (i % 8));
        }
    }

    uint8_t response_buffer[MODBUS_MAX_FRAME_SIZE];
    uint16_t response_length;

    if (!modbus_send_request(slave_address, MODBUS_FC_WRITE_MULTIPLE_COILS,
                           request_data, 5 + byte_count, response_buffer,
                           &response_length, MODBUS_MAX_FRAME_SIZE)) {
        return false;
    }

    /* Verify response contains start address and quantity */
    if (response_length != 4 ||
        response_buffer[0] != request_data[0] ||
        response_buffer[1] != request_data[1] ||
        response_buffer[2] != request_data[2] ||
        response_buffer[3] != request_data[3]) {
        return false;
    }

    modbus_statistics.successful_requests++;
    return true;
}

/**
 * @brief Write multiple registers to slave device
 */
bool modbus_write_multiple_registers(uint8_t slave_address, uint16_t start_address,
                                    uint16_t quantity, const uint16_t *register_values) {
    if (!modbus_initialized || modbus_config.mode != MODBUS_MODE_MASTER ||
        !register_values || quantity < 1 || quantity > 123) {
        return false;
    }

    uint8_t byte_count = quantity * 2;
    uint8_t request_data[5 + byte_count];

    request_data[0] = (start_address >> 8) & 0xFF;
    request_data[1] = start_address & 0xFF;
    request_data[2] = (quantity >> 8) & 0xFF;
    request_data[3] = quantity & 0xFF;
    request_data[4] = byte_count;

    /* Pack register values */
    for (uint16_t i = 0; i < quantity; i++) {
        request_data[5 + (i * 2)] = (register_values[i] >> 8) & 0xFF;
        request_data[6 + (i * 2)] = register_values[i] & 0xFF;
    }

    uint8_t response_buffer[MODBUS_MAX_FRAME_SIZE];
    uint16_t response_length;

    if (!modbus_send_request(slave_address, MODBUS_FC_WRITE_MULTIPLE_REGISTERS,
                           request_data, 5 + byte_count, response_buffer,
                           &response_length, MODBUS_MAX_FRAME_SIZE)) {
        return false;
    }

    /* Verify response contains start address and quantity */
    if (response_length != 4 ||
        response_buffer[0] != request_data[0] ||
        response_buffer[1] != request_data[1] ||
        response_buffer[2] != request_data[2] ||
        response_buffer[3] != request_data[3]) {
        return false;
    }

    modbus_statistics.successful_requests++;
    return true;
}

/**
 * @brief Send raw Modbus request (placeholder - needs UART implementation)
 */
bool modbus_send_request(uint8_t slave_address, uint8_t function_code,
                        const uint8_t *data, uint16_t data_length,
                        uint8_t *response_buffer, uint16_t *response_length,
                        uint16_t max_response_length) {
    if (!modbus_initialized || !data || !response_buffer || !response_length) {
        return false;
    }

    modbus_statistics.frames_sent++;

    /* TODO: Implement UART transmission and reception */
    /* This is a placeholder that would need to be implemented with actual UART hardware */

    modbus_statistics.frames_received++;
    *response_length = 0;

    return true;
}

/**
 * @brief Process incoming Modbus frame (slave mode)
 */
bool modbus_process_frame(const modbus_frame_t *frame, uint8_t *response_buffer,
                         uint16_t *response_length, uint16_t max_response_length) {
    if (!modbus_initialized || !frame || !response_buffer || !response_length ||
        modbus_config.mode != MODBUS_MODE_SLAVE) {
        return false;
    }

    /* Check if frame is addressed to this slave */
    if (frame->address != modbus_config.slave_address && frame->address != MODBUS_BROADCAST_ADDRESS) {
        return false;
    }

    bool success = false;
    uint8_t response_data[MODBUS_MAX_FRAME_SIZE];
    uint16_t response_data_length = 0;

    /* Handle different function codes */
    switch (frame->function_code) {
        case MODBUS_FC_READ_COILS:
            success = modbus_handle_read_coils(frame, response_data, &response_data_length);
            break;

        case MODBUS_FC_READ_HOLDING_REGISTERS:
            success = modbus_handle_read_holding_registers(frame, response_data, &response_data_length);
            break;

        case MODBUS_FC_WRITE_SINGLE_COIL:
            success = modbus_handle_write_single_coil(frame, response_data, &response_data_length);
            break;

        case MODBUS_FC_WRITE_MULTIPLE_COILS:
            success = modbus_handle_write_multiple_coils(frame, response_data, &response_data_length);
            break;

        default:
            /* Unsupported function code - send exception response */
            response_data[0] = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
            response_data_length = 1;
            success = true;
            break;
    }

    if (!success) {
        /* Send exception response for illegal data */
        response_data[0] = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        response_data_length = 1;
    }

    /* Build complete response frame */
    if (response_data_length > 0 && response_data_length <= max_response_length) {
        response_buffer[0] = modbus_config.slave_address;
        response_buffer[1] = frame->function_code | (response_data_length == 1 ? 0x80 : 0x00);

        if (response_data_length > 0) {
            memcpy(&response_buffer[2], response_data, response_data_length);
        }

        /* Calculate and append CRC */
        uint16_t crc = modbus_calculate_crc(response_buffer, 2 + response_data_length);
        response_buffer[2 + response_data_length] = crc & 0xFF;
        response_buffer[3 + response_data_length] = (crc >> 8) & 0xFF;

        *response_length = 4 + response_data_length;
        modbus_statistics.frames_sent++;
        return true;
    }

    return false;
}

/**
 * @brief Register coil read/write callback
 */
bool modbus_register_coil_callback(bool (*callback)(uint16_t address, bool *value, bool write)) {
    if (!modbus_initialized) {
        return false;
    }

    coil_callback = callback;
    return true;
}

/**
 * @brief Register register read/write callback
 */
bool modbus_register_register_callback(bool (*callback)(uint16_t address, uint16_t *value, bool write)) {
    if (!modbus_initialized) {
        return false;
    }

    register_callback = callback;
    return true;
}

/**
 * @brief Get Modbus statistics
 */
bool modbus_get_statistics(modbus_statistics_t *statistics) {
    if (!modbus_initialized || !statistics) {
        return false;
    }

    memcpy(statistics, &modbus_statistics, sizeof(modbus_statistics_t));
    return true;
}

/**
 * @brief Clear Modbus statistics
 */
bool modbus_clear_statistics(void) {
    if (!modbus_initialized) {
        return false;
    }

    memset(&modbus_statistics, 0, sizeof(modbus_statistics_t));
    return true;
}

/**
 * @brief Perform Modbus diagnostics
 */
bool modbus_diagnostics(uint16_t sub_function, uint16_t data,
                       uint8_t *response_buffer, uint16_t *response_length) {
    /* TODO: Implement Modbus diagnostics */
    return false;
}

/**
 * @brief Set slave address
 */
bool modbus_set_slave_address(uint8_t address) {
    if (!modbus_initialized || address < 1 || address > 247) {
        return false;
    }

    modbus_config.slave_address = address;
    return true;
}

/**
 * @brief Get current slave address
 */
uint8_t modbus_get_slave_address(void) {
    return modbus_initialized ? modbus_config.slave_address : 0;
}

/**
 * @brief Enable/disable broadcast messages
 */
bool modbus_enable_broadcast(bool enable) {
    if (!modbus_initialized) {
        return false;
    }

    modbus_config.enable_broadcast = enable;
    return true;
}

/**
 * @brief Set communication parameters
 */
bool modbus_set_comm_params(uint32_t baud_rate, uint8_t data_bits,
                           uint8_t stop_bits, char parity) {
    if (!modbus_initialized) {
        return false;
    }

    modbus_config.baud_rate = baud_rate;
    modbus_config.data_bits = data_bits;
    modbus_config.stop_bits = stop_bits;
    modbus_config.parity = parity;

    /* TODO: Reconfigure UART with new parameters */
    return true;
}

/**
 * @brief Get communication parameters
 */
bool modbus_get_comm_params(uint32_t *baud_rate, uint8_t *data_bits,
                           uint8_t *stop_bits, char *parity) {
    if (!modbus_initialized || !baud_rate || !data_bits || !stop_bits || !parity) {
        return false;
    }

    *baud_rate = modbus_config.baud_rate;
    *data_bits = modbus_config.data_bits;
    *stop_bits = modbus_config.stop_bits;
    *parity = modbus_config.parity;

    return true;
}

/**
 * @brief Enter Modbus test mode
 */
bool modbus_enter_test_mode(void) {
    /* TODO: Implement test mode */
    return false;
}

/**
 * @brief Exit Modbus test mode
 */
bool modbus_exit_test_mode(void) {
    /* TODO: Implement test mode */
    return false;
}
