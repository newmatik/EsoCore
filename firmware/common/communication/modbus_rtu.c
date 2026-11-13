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
#include <stdlib.h>
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

/* True while the data map arrays were allocated by modbus_rtu_init(). Arrays
 * handed in through modbus_init_data_map() belong to the caller. */
static bool modbus_data_map_owned = false;

/* RTU receiver state (slave mode), see modbus_rtu_receive_byte(). */
static bool modbus_rx_in_frame = false;     /* A frame is being received or awaits modbus_rtu_poll() */
static bool modbus_rx_frame_bad = false;    /* The current frame must be discarded */
static uint32_t modbus_rx_last_byte_us = 0; /* Time the last character was completely received */
static uint32_t modbus_char_us = 0;         /* Duration of one character on the line */
static uint32_t modbus_t1_5_us = 0;         /* Maximum silence between two characters of a frame */
static uint32_t modbus_t3_5_us = 0;         /* Minimum silence between two frames */

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
    if (!frame || frame->data_length > MODBUS_MAX_FRAME_SIZE - 4 ||
        (!frame->data && frame->data_length > 0)) {
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
 * The frame's data pointer refers to the raw buffer, so the frame is valid
 * only as long as that buffer is unchanged.
 *
 * @param raw_data Pointer to raw frame data (address, function, data, CRC)
 * @param data_length Length of raw data
 * @param frame Pointer to frame structure to fill
 * @return true if frame parsed successfully and its CRC is valid, false otherwise
 */
static bool modbus_parse_frame(uint8_t *raw_data, uint16_t data_length,
                              modbus_frame_t *frame) {
    if (!raw_data || !frame || data_length < 4 || data_length > MODBUS_MAX_FRAME_SIZE) {
        return false;
    }

    frame->address = raw_data[0];
    frame->function_code = raw_data[1];
    frame->data_length = data_length - 4; /* Subtract address, function, CRC */
    frame->timestamp = 0; /* TODO: Add timestamp */
    frame->data = (frame->data_length > 0) ? &raw_data[2] : NULL;

    /* Extract CRC (low byte first on the wire) */
    frame->crc = (uint16_t)((raw_data[data_length - 1] << 8) | raw_data[data_length - 2]);

    return modbus_validate_crc(frame);
}

/* ============================================================================
 * Modbus Function Code Handlers (slave mode)
 *
 * Each handler returns 0 and fills the response data, or returns the Modbus
 * exception code to answer with. The checks follow the order of the state
 * diagrams in the Modbus Application Protocol Specification V1.1b3: quantity
 * and value first (exception 03), then the address range (exception 02), then
 * the access itself (exception 04).
 * ============================================================================ */

#define MODBUS_NO_EXCEPTION 0

static uint16_t modbus_get_u16(const uint8_t *bytes) {
    return (uint16_t)((bytes[0] << 8) | bytes[1]);
}

static void modbus_put_u16(uint8_t *bytes, uint16_t value) {
    bytes[0] = (uint8_t)(value >> 8);
    bytes[1] = (uint8_t)(value & 0xFF);
}

/** @brief true if [start, start + quantity) lies inside a table of table_size entries */
static bool modbus_range_ok(uint16_t start, uint16_t quantity, uint16_t table_size) {
    return ((uint32_t)start + quantity) <= table_size;
}

/**
 * @brief Read one coil through the callback, else from the data map
 */
static bool modbus_coil_read(uint16_t address, bool *value) {
    if (coil_callback) {
        return coil_callback(address, value, false);
    }
    if (!modbus_data_map.coils) {
        return false;
    }
    *value = modbus_data_map.coils[address];
    return true;
}

/**
 * @brief Write one coil through the callback, else to the data map
 */
static bool modbus_coil_write(uint16_t address, bool value) {
    if (coil_callback) {
        return coil_callback(address, &value, true);
    }
    if (!modbus_data_map.coils) {
        return false;
    }
    modbus_data_map.coils[address] = value;
    return true;
}

/**
 * @brief Read one holding register through the callback, else from the data map
 */
static bool modbus_holding_register_read(uint16_t address, uint16_t *value) {
    if (register_callback) {
        return register_callback(address, value, false);
    }
    if (!modbus_data_map.holding_registers) {
        return false;
    }
    *value = modbus_data_map.holding_registers[address];
    return true;
}

/**
 * @brief Write one holding register through the callback, else to the data map
 */
static bool modbus_holding_register_write(uint16_t address, uint16_t value) {
    if (register_callback) {
        return register_callback(address, &value, true);
    }
    if (!modbus_data_map.holding_registers) {
        return false;
    }
    modbus_data_map.holding_registers[address] = value;
    return true;
}

/**
 * @brief Handle Read Coils (0x01) and Read Discrete Inputs (0x02)
 *
 * @param frame Pointer to received frame
 * @param discrete_inputs true for function 0x02, false for function 0x01
 * @param response_buffer Pointer to response data buffer
 * @param response_length Pointer to response data length
 * @return 0 on success, else the Modbus exception code
 */
static uint8_t modbus_handle_read_bits(const modbus_frame_t *frame, bool discrete_inputs,
                                      uint8_t *response_buffer, uint16_t *response_length) {
    if (frame->data_length != 4) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    uint16_t start_address = modbus_get_u16(&frame->data[0]);
    uint16_t quantity = modbus_get_u16(&frame->data[2]);
    uint16_t table_size = discrete_inputs ? modbus_data_map.num_discrete_inputs
                                          : modbus_data_map.num_coils;

    if (quantity < 1 || quantity > 2000) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    if (!modbus_range_ok(start_address, quantity, table_size)) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    /* Build response: byte count, then the bits, LSB of the first byte first.
     * Unused bits of the last byte stay zero. */
    uint8_t byte_count = (uint8_t)((quantity + 7) / 8);
    response_buffer[0] = byte_count;
    memset(&response_buffer[1], 0, byte_count);

    for (uint16_t i = 0; i < quantity; i++) {
        uint16_t bit_address = (uint16_t)(start_address + i);
        bool bit_value = false;

        if (discrete_inputs) {
            if (!modbus_data_map.discrete_inputs) {
                return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
            }
            bit_value = modbus_data_map.discrete_inputs[bit_address];
        } else if (!modbus_coil_read(bit_address, &bit_value)) {
            return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
        }

        if (bit_value) {
            response_buffer[1 + (i / 8)] |= (uint8_t)(1 << (i % 8));
        }
    }

    *response_length = (uint16_t)(1 + byte_count);
    return MODBUS_NO_EXCEPTION;
}

/**
 * @brief Handle Read Holding Registers (0x03) and Read Input Registers (0x04)
 *
 * @param frame Pointer to received frame
 * @param input_registers true for function 0x04, false for function 0x03
 * @param response_buffer Pointer to response data buffer
 * @param response_length Pointer to response data length
 * @return 0 on success, else the Modbus exception code
 */
static uint8_t modbus_handle_read_registers(const modbus_frame_t *frame, bool input_registers,
                                           uint8_t *response_buffer, uint16_t *response_length) {
    if (frame->data_length != 4) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    uint16_t start_address = modbus_get_u16(&frame->data[0]);
    uint16_t quantity = modbus_get_u16(&frame->data[2]);
    uint16_t table_size = input_registers ? modbus_data_map.num_input_registers
                                          : modbus_data_map.num_holding_registers;

    if (quantity < 1 || quantity > 125) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    if (!modbus_range_ok(start_address, quantity, table_size)) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    /* Build response: byte count, then the registers, high byte first */
    uint8_t byte_count = (uint8_t)(quantity * 2);
    response_buffer[0] = byte_count;

    for (uint16_t i = 0; i < quantity; i++) {
        uint16_t reg_address = (uint16_t)(start_address + i);
        uint16_t reg_value = 0;

        if (input_registers) {
            if (!modbus_data_map.input_registers) {
                return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
            }
            reg_value = modbus_data_map.input_registers[reg_address];
        } else if (!modbus_holding_register_read(reg_address, &reg_value)) {
            return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
        }

        modbus_put_u16(&response_buffer[1 + (i * 2)], reg_value);
    }

    *response_length = (uint16_t)(1 + byte_count);
    return MODBUS_NO_EXCEPTION;
}

/**
 * @brief Handle Write Single Coil function (0x05)
 *
 * @param frame Pointer to received frame
 * @param response_buffer Pointer to response data buffer
 * @param response_length Pointer to response data length
 * @return 0 on success, else the Modbus exception code
 */
static uint8_t modbus_handle_write_single_coil(const modbus_frame_t *frame,
                                              uint8_t *response_buffer, uint16_t *response_length) {
    if (frame->data_length != 4) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    uint16_t coil_address = modbus_get_u16(&frame->data[0]);
    uint16_t coil_value = modbus_get_u16(&frame->data[2]);

    /* Only 0x0000 (off) and 0xFF00 (on) are legal */
    if (coil_value != 0x0000 && coil_value != 0xFF00) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    if (!modbus_range_ok(coil_address, 1, modbus_data_map.num_coils)) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    if (!modbus_coil_write(coil_address, coil_value == 0xFF00)) {
        return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
    }

    /* The response echoes the request */
    memcpy(response_buffer, frame->data, 4);
    *response_length = 4;
    return MODBUS_NO_EXCEPTION;
}

/**
 * @brief Handle Write Single Register function (0x06)
 *
 * @param frame Pointer to received frame
 * @param response_buffer Pointer to response data buffer
 * @param response_length Pointer to response data length
 * @return 0 on success, else the Modbus exception code
 */
static uint8_t modbus_handle_write_single_register(const modbus_frame_t *frame,
                                                  uint8_t *response_buffer, uint16_t *response_length) {
    if (frame->data_length != 4) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    uint16_t reg_address = modbus_get_u16(&frame->data[0]);
    uint16_t reg_value = modbus_get_u16(&frame->data[2]);

    if (!modbus_range_ok(reg_address, 1, modbus_data_map.num_holding_registers)) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    if (!modbus_holding_register_write(reg_address, reg_value)) {
        return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
    }

    /* The response echoes the request */
    memcpy(response_buffer, frame->data, 4);
    *response_length = 4;
    return MODBUS_NO_EXCEPTION;
}

/**
 * @brief Handle Write Multiple Coils function (0x0F)
 *
 * @param frame Pointer to received frame
 * @param response_buffer Pointer to response data buffer
 * @param response_length Pointer to response data length
 * @return 0 on success, else the Modbus exception code
 */
static uint8_t modbus_handle_write_multiple_coils(const modbus_frame_t *frame,
                                                 uint8_t *response_buffer, uint16_t *response_length) {
    if (frame->data_length < 6) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    uint16_t start_address = modbus_get_u16(&frame->data[0]);
    uint16_t quantity = modbus_get_u16(&frame->data[2]);
    uint8_t byte_count = frame->data[4];

    if (quantity < 1 || quantity > 1968 || byte_count != ((quantity + 7) / 8) ||
        frame->data_length != (uint16_t)(5 + byte_count)) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    if (!modbus_range_ok(start_address, quantity, modbus_data_map.num_coils)) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    for (uint16_t i = 0; i < quantity; i++) {
        bool coil_value = (frame->data[5 + (i / 8)] & (1 << (i % 8))) != 0;

        if (!modbus_coil_write((uint16_t)(start_address + i), coil_value)) {
            return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
        }
    }

    /* The response returns start address and quantity */
    memcpy(response_buffer, frame->data, 4);
    *response_length = 4;
    return MODBUS_NO_EXCEPTION;
}

/**
 * @brief Handle Write Multiple Registers function (0x10)
 *
 * @param frame Pointer to received frame
 * @param response_buffer Pointer to response data buffer
 * @param response_length Pointer to response data length
 * @return 0 on success, else the Modbus exception code
 */
static uint8_t modbus_handle_write_multiple_registers(const modbus_frame_t *frame,
                                                     uint8_t *response_buffer, uint16_t *response_length) {
    if (frame->data_length < 7) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    uint16_t start_address = modbus_get_u16(&frame->data[0]);
    uint16_t quantity = modbus_get_u16(&frame->data[2]);
    uint8_t byte_count = frame->data[4];

    if (quantity < 1 || quantity > 123 || byte_count != (quantity * 2) ||
        frame->data_length != (uint16_t)(5 + byte_count)) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    if (!modbus_range_ok(start_address, quantity, modbus_data_map.num_holding_registers)) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
    }

    for (uint16_t i = 0; i < quantity; i++) {
        uint16_t reg_value = modbus_get_u16(&frame->data[5 + (i * 2)]);

        if (!modbus_holding_register_write((uint16_t)(start_address + i), reg_value)) {
            return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
        }
    }

    /* The response returns start address and quantity */
    memcpy(response_buffer, frame->data, 4);
    *response_length = 4;
    return MODBUS_NO_EXCEPTION;
}

/**
 * @brief Free the data map arrays that modbus_rtu_init() allocated
 */
static void modbus_release_data_map(void) {
    if (modbus_data_map_owned) {
        free(modbus_data_map.coils);
        free(modbus_data_map.discrete_inputs);
        free(modbus_data_map.holding_registers);
        free(modbus_data_map.input_registers);
    }

    memset(&modbus_data_map, 0, sizeof(modbus_data_map_t));
    modbus_data_map_owned = false;
}

/**
 * @brief Discard the frame being received
 */
static void modbus_rx_reset(void) {
    modbus_rx_index = 0;
    modbus_rx_in_frame = false;
    modbus_rx_frame_bad = false;
}

/** @brief Bits per character: start bit, data bits, parity bit if any, stop bits */
static uint32_t modbus_bits_per_char(uint8_t data_bits, uint8_t stop_bits, char parity) {
    return 1u + data_bits + ((parity == 'N' || parity == 'n') ? 0u : 1u) + stop_bits;
}

/**
 * @brief Set the receiver timing (character time, T1.5, T3.5) for a character format
 *
 * @return true if set, false if the timing cannot be derived (baud rate 0)
 */
static bool modbus_apply_frame_timing(uint32_t baud_rate, uint8_t data_bits, uint8_t stop_bits,
                                      char parity) {
    uint32_t t1_5_us = 0;
    uint32_t t3_5_us = 0;

    if (!modbus_rtu_get_frame_timing(baud_rate, data_bits, stop_bits, parity, &t1_5_us, &t3_5_us)) {
        return false;
    }

    /* One character time in microseconds, rounded up */
    modbus_char_us = (uint32_t)((modbus_bits_per_char(data_bits, stop_bits, parity) * 1000000ull +
                                 baud_rate - 1) / baud_rate);
    modbus_t1_5_us = t1_5_us;
    modbus_t3_5_us = t3_5_us;
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

    /* The receiver timing follows from the character format */
    if (!modbus_apply_frame_timing(config->baud_rate, config->data_bits, config->stop_bits,
                                   config->parity)) {
        return false;
    }

    memcpy(&modbus_config, config, sizeof(modbus_config_t));

    /* Initialize data map */
    memset(&modbus_data_map, 0, sizeof(modbus_data_map_t));
    memset(&modbus_statistics, 0, sizeof(modbus_statistics_t));
    modbus_data_map_owned = true;

    /* Allocate the four data tables if sizes are specified. max_coils sizes
     * both bit tables (coils, discrete inputs), max_registers both register
     * tables (holding registers, input registers). */
    if (modbus_config.max_coils > 0) {
        modbus_data_map.coils = (bool *)calloc(modbus_config.max_coils, sizeof(bool));
        modbus_data_map.discrete_inputs = (bool *)calloc(modbus_config.max_coils, sizeof(bool));
        if (!modbus_data_map.coils || !modbus_data_map.discrete_inputs) {
            modbus_release_data_map();
            return false;
        }
        modbus_data_map.num_coils = modbus_config.max_coils;
        modbus_data_map.num_discrete_inputs = modbus_config.max_coils;
    }

    if (modbus_config.max_registers > 0) {
        modbus_data_map.holding_registers = (uint16_t *)calloc(modbus_config.max_registers, sizeof(uint16_t));
        modbus_data_map.input_registers = (uint16_t *)calloc(modbus_config.max_registers, sizeof(uint16_t));
        if (!modbus_data_map.holding_registers || !modbus_data_map.input_registers) {
            modbus_release_data_map();
            return false;
        }
        modbus_data_map.num_holding_registers = modbus_config.max_registers;
        modbus_data_map.num_input_registers = modbus_config.max_registers;
    }

    modbus_rx_reset();

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

    /* Frees only what modbus_rtu_init() allocated; a data map handed in
     * through modbus_init_data_map() stays with its owner. */
    modbus_release_data_map();
    modbus_rx_reset();

    coil_callback = NULL;
    register_callback = NULL;

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

    /* The caller's arrays replace the ones from modbus_rtu_init() */
    modbus_release_data_map();
    memcpy(&modbus_data_map, data_map, sizeof(modbus_data_map_t));
    return true;
}

/**
 * @brief Set a discrete input in the data map
 */
bool modbus_set_discrete_input(uint16_t address, bool value) {
    if (!modbus_initialized || !modbus_data_map.discrete_inputs ||
        address >= modbus_data_map.num_discrete_inputs) {
        return false;
    }

    modbus_data_map.discrete_inputs[address] = value;
    return true;
}

/**
 * @brief Set an input register in the data map
 */
bool modbus_set_input_register(uint16_t address, uint16_t value) {
    if (!modbus_initialized || !modbus_data_map.input_registers ||
        address >= modbus_data_map.num_input_registers) {
        return false;
    }

    modbus_data_map.input_registers[address] = value;
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
 * @brief Read discrete inputs from slave device
 */
bool modbus_read_discrete_inputs(uint8_t slave_address, uint16_t start_address,
                                uint16_t quantity, bool *input_values) {
    if (!modbus_initialized || modbus_config.mode != MODBUS_MODE_MASTER ||
        !input_values || quantity < 1 || quantity > 2000) {
        return false;
    }

    uint8_t request_data[4];
    request_data[0] = (start_address >> 8) & 0xFF;
    request_data[1] = start_address & 0xFF;
    request_data[2] = (quantity >> 8) & 0xFF;
    request_data[3] = quantity & 0xFF;

    uint8_t response_buffer[MODBUS_MAX_FRAME_SIZE];
    uint16_t response_length;

    if (!modbus_send_request(slave_address, MODBUS_FC_READ_DISCRETE_INPUTS,
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

    /* Unpack input values */
    for (uint16_t i = 0; i < quantity; i++) {
        input_values[i] = (response_buffer[1 + (i / 8)] & (1 << (i % 8))) != 0;
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
 * @brief Read input registers from slave device
 */
bool modbus_read_input_registers(uint8_t slave_address, uint16_t start_address,
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

    if (!modbus_send_request(slave_address, MODBUS_FC_READ_INPUT_REGISTERS,
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
 * @brief Write single register to slave device
 */
bool modbus_write_single_register(uint8_t slave_address, uint16_t register_address,
                                 uint16_t register_value) {
    if (!modbus_initialized || modbus_config.mode != MODBUS_MODE_MASTER) {
        return false;
    }

    uint8_t request_data[4];
    request_data[0] = (register_address >> 8) & 0xFF;
    request_data[1] = register_address & 0xFF;
    request_data[2] = (register_value >> 8) & 0xFF;
    request_data[3] = register_value & 0xFF;

    uint8_t response_buffer[MODBUS_MAX_FRAME_SIZE];
    uint16_t response_length;

    if (!modbus_send_request(slave_address, MODBUS_FC_WRITE_SINGLE_REGISTER,
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

    *response_length = 0;

    if (frame->data_length > MODBUS_MAX_FRAME_SIZE - 4 || (!frame->data && frame->data_length > 0)) {
        return false;
    }

    /* Check if frame is addressed to this slave */
    bool broadcast = (frame->address == MODBUS_BROADCAST_ADDRESS);

    if (broadcast ? !modbus_config.enable_broadcast : (frame->address != modbus_config.slave_address)) {
        return false;
    }

    modbus_statistics.frames_received++;

    uint8_t exception = MODBUS_NO_EXCEPTION;
    uint8_t response_data[MODBUS_MAX_FRAME_SIZE];
    uint16_t response_data_length = 0;

    /* Handle different function codes */
    switch (frame->function_code) {
        case MODBUS_FC_READ_COILS:
            exception = modbus_handle_read_bits(frame, false, response_data, &response_data_length);
            break;

        case MODBUS_FC_READ_DISCRETE_INPUTS:
            exception = modbus_handle_read_bits(frame, true, response_data, &response_data_length);
            break;

        case MODBUS_FC_READ_HOLDING_REGISTERS:
            exception = modbus_handle_read_registers(frame, false, response_data, &response_data_length);
            break;

        case MODBUS_FC_READ_INPUT_REGISTERS:
            exception = modbus_handle_read_registers(frame, true, response_data, &response_data_length);
            break;

        case MODBUS_FC_WRITE_SINGLE_COIL:
            exception = modbus_handle_write_single_coil(frame, response_data, &response_data_length);
            break;

        case MODBUS_FC_WRITE_SINGLE_REGISTER:
            exception = modbus_handle_write_single_register(frame, response_data, &response_data_length);
            break;

        case MODBUS_FC_WRITE_MULTIPLE_COILS:
            exception = modbus_handle_write_multiple_coils(frame, response_data, &response_data_length);
            break;

        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            exception = modbus_handle_write_multiple_registers(frame, response_data, &response_data_length);
            break;

        default:
            exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
            break;
    }

    /* A broadcast request is executed but never answered */
    if (broadcast) {
        return true;
    }

    uint8_t function_code = frame->function_code;

    if (exception != MODBUS_NO_EXCEPTION) {
        /* Exception response: function code with the top bit set, then the code */
        function_code |= 0x80;
        response_data[0] = exception;
        response_data_length = 1;
    }

    /* Build complete response frame: address, function, data, CRC */
    if ((uint32_t)response_data_length + 4 > max_response_length) {
        return false;
    }

    response_buffer[0] = modbus_config.slave_address;
    response_buffer[1] = function_code;
    memcpy(&response_buffer[2], response_data, response_data_length);

    /* Calculate and append CRC, low byte first */
    uint16_t crc = modbus_calculate_crc(response_buffer, (uint16_t)(2 + response_data_length));
    response_buffer[2 + response_data_length] = (uint8_t)(crc & 0xFF);
    response_buffer[3 + response_data_length] = (uint8_t)(crc >> 8);

    *response_length = (uint16_t)(4 + response_data_length);
    modbus_statistics.frames_sent++;
    if (exception != MODBUS_NO_EXCEPTION) {
        modbus_statistics.exceptions_sent++;
    }
    return true;
}

/* ============================================================================
 * RTU Frame Timing and Receiver (slave mode)
 * ============================================================================ */

/**
 * @brief Calculate the RTU inter-character (T1.5) and inter-frame (T3.5) times
 */
bool modbus_rtu_get_frame_timing(uint32_t baud_rate, uint8_t data_bits, uint8_t stop_bits,
                                char parity, uint32_t *t1_5_us, uint32_t *t3_5_us) {
    if (baud_rate == 0 || !t1_5_us || !t3_5_us) {
        return false;
    }

    if (baud_rate > 19200) {
        /* Fixed values above 19200 Bd (Modbus over Serial Line V1.02, 2.5.1.1) */
        *t1_5_us = 750;
        *t3_5_us = 1750;
        return true;
    }

    uint32_t bits_per_char = modbus_bits_per_char(data_bits, stop_bits, parity);

    /* 1.5 and 3.5 character times in microseconds, rounded up */
    *t1_5_us = (uint32_t)((bits_per_char * 1500000ull + baud_rate - 1) / baud_rate);
    *t3_5_us = (uint32_t)((bits_per_char * 3500000ull + baud_rate - 1) / baud_rate);
    return true;
}

/**
 * @brief Feed one received character into the RTU receiver
 */
void modbus_rtu_receive_byte(uint8_t byte, uint32_t now_us) {
    if (!modbus_initialized || modbus_config.mode != MODBUS_MODE_SLAVE) {
        return;
    }

    if (modbus_rx_in_frame) {
        /* Both timestamps mark the end of a character, so their distance holds
         * this character's own duration; the rest is the silence before it.
         * Unsigned subtraction keeps the distance correct across timer wrap-around. */
        uint32_t distance_us = now_us - modbus_rx_last_byte_us;
        uint32_t gap_us = (distance_us > modbus_char_us) ? (distance_us - modbus_char_us) : 0;

        if (gap_us >= modbus_t3_5_us) {
            /* The previous frame had ended but modbus_rtu_poll() never took
             * it: it is lost, and this character starts a new frame. */
            if (!modbus_rx_frame_bad) {
                modbus_statistics.overrun_errors++;
            } else {
                modbus_statistics.framing_errors++;
            }
            modbus_rx_reset();
        } else if (gap_us > modbus_t1_5_us) {
            /* Silence of more than T1.5 inside a frame: the frame is
             * incomplete and is discarded as a whole. */
            modbus_rx_frame_bad = true;
        }
    }

    if (!modbus_rx_frame_bad) {
        if (modbus_rx_index < MODBUS_MAX_FRAME_SIZE) {
            modbus_rx_buffer[modbus_rx_index++] = byte;
        } else {
            /* Longer than the largest RTU frame */
            modbus_rx_frame_bad = true;
        }
    }

    modbus_rx_in_frame = true;
    modbus_rx_last_byte_us = now_us;
}

/**
 * @brief Close the received frame after T3.5 of silence and answer it
 */
bool modbus_rtu_poll(uint32_t now_us, uint8_t *response_buffer,
                    uint16_t *response_length, uint16_t max_response_length) {
    if (response_length) {
        *response_length = 0;
    }

    if (!modbus_initialized || modbus_config.mode != MODBUS_MODE_SLAVE ||
        !response_buffer || !response_length) {
        return false;
    }

    /* The frame ends only after T3.5 of silence. A character that started
     * within T3.5 is reported only once it is complete, one character time
     * later, so the frame is closed when nothing has arrived for T3.5 plus
     * one character time. */
    if (!modbus_rx_in_frame ||
        (uint32_t)(now_us - modbus_rx_last_byte_us) < modbus_t3_5_us + modbus_char_us) {
        return false;
    }

    uint16_t frame_length = modbus_rx_index;
    bool frame_bad = modbus_rx_frame_bad;

    modbus_rx_reset();

    /* Too short, too long, or broken by a gap of more than T1.5 */
    if (frame_bad || frame_length < 4) {
        modbus_statistics.framing_errors++;
        return false;
    }

    modbus_frame_t frame;

    if (!modbus_parse_frame(modbus_rx_buffer, frame_length, &frame)) {
        modbus_statistics.crc_errors++;
        return false;
    }

    frame.timestamp = now_us;

    return modbus_process_frame(&frame, response_buffer, response_length, max_response_length) &&
           *response_length > 0;
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

    /* The receiver timing depends on the character format; a frame received
     * with the old format is discarded. */
    if (!modbus_apply_frame_timing(baud_rate, data_bits, stop_bits, parity)) {
        return false;
    }
    modbus_rx_reset();

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
