/**
 * @file modbus_rtu.h
 * @brief Modbus RTU Protocol Implementation for EsoCore
 *
 * This file implements the Modbus RTU protocol over RS-485 for industrial
 * communication. The implementation supports both master and slave modes
 * with comprehensive error handling and diagnostics.
 *
 * Standards Compliance:
 * - Modbus Application Protocol Specification V1.1b3
 * - Modbus over Serial Line Specification V1.02
 * - RTU transmission mode
 *
 * Features:
 * - Master and slave mode support
 * - Standard Modbus function codes (1, 2, 3, 4, 5, 6, 15, 16)
 * - Exception handling and error recovery
 * - CRC-16 error detection
 * - Configurable timeouts and retries
 * - Broadcast message support
 * - Diagnostic and statistics collection
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_MODBUS_RTU_H
#define ESOCORE_MODBUS_RTU_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Modbus RTU Configuration
 * ============================================================================ */

#define MODBUS_MAX_FRAME_SIZE     256   /* Maximum Modbus frame size */
#define MODBUS_MAX_COILS         2000   /* Maximum coil addresses */
#define MODBUS_MAX_REGISTERS     125    /* Maximum register addresses */
#define MODBUS_BROADCAST_ADDRESS 0x00   /* Broadcast address */
#define MODBUS_MAX_SLAVES        247    /* Maximum slave addresses */

/* Modbus Function Codes */
typedef enum {
    MODBUS_FC_READ_COILS            = 0x01, /* Read Coils */
    MODBUS_FC_READ_DISCRETE_INPUTS  = 0x02, /* Read Discrete Inputs */
    MODBUS_FC_READ_HOLDING_REGISTERS = 0x03, /* Read Holding Registers */
    MODBUS_FC_READ_INPUT_REGISTERS  = 0x04, /* Read Input Registers */
    MODBUS_FC_WRITE_SINGLE_COIL     = 0x05, /* Write Single Coil */
    MODBUS_FC_WRITE_SINGLE_REGISTER = 0x06, /* Write Single Register */
    MODBUS_FC_READ_EXCEPTION_STATUS = 0x07, /* Read Exception Status */
    MODBUS_FC_DIAGNOSTICS           = 0x08, /* Diagnostics */
    MODBUS_FC_GET_COMM_EVENT_COUNTER = 0x0B, /* Get Comm Event Counter */
    MODBUS_FC_GET_COMM_EVENT_LOG    = 0x0C, /* Get Comm Event Log */
    MODBUS_FC_WRITE_MULTIPLE_COILS  = 0x0F, /* Write Multiple Coils */
    MODBUS_FC_WRITE_MULTIPLE_REGISTERS = 0x10, /* Write Multiple Registers */
    MODBUS_FC_REPORT_SLAVE_ID       = 0x11, /* Report Slave ID */
    MODBUS_FC_READ_FILE_RECORD      = 0x14, /* Read File Record */
    MODBUS_FC_WRITE_FILE_RECORD     = 0x15, /* Write File Record */
    MODBUS_FC_MASK_WRITE_REGISTER   = 0x16, /* Mask Write Register */
    MODBUS_FC_READ_WRITE_MULTIPLE_REGISTERS = 0x17, /* Read/Write Multiple Registers */
    MODBUS_FC_READ_FIFO_QUEUE       = 0x18, /* Read FIFO Queue */
} modbus_function_code_t;

/* Modbus Exception Codes */
typedef enum {
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01, /* Illegal function */
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS = 0x02, /* Illegal data address */
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE = 0x03, /* Illegal data value */
    MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE = 0x04, /* Slave device failure */
    MODBUS_EXCEPTION_ACKNOWLEDGE = 0x05, /* Acknowledge */
    MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY = 0x06, /* Slave device busy */
    MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE = 0x07, /* Negative acknowledge */
    MODBUS_EXCEPTION_MEMORY_PARITY_ERROR = 0x08, /* Memory parity error */
    MODBUS_EXCEPTION_GATEWAY_PATH_UNAVAILABLE = 0x0A, /* Gateway path unavailable */
    MODBUS_EXCEPTION_GATEWAY_TARGET_DEVICE_FAILED = 0x0B, /* Gateway target device failed */
} modbus_exception_code_t;

/* Modbus Operating Modes */
typedef enum {
    MODBUS_MODE_MASTER = 0,    /* Master mode */
    MODBUS_MODE_SLAVE  = 1,    /* Slave mode */
} modbus_mode_t;

/* Modbus Configuration */
typedef struct {
    modbus_mode_t mode;                   /* Operating mode (master/slave) */
    uint8_t slave_address;                /* Slave address (1-247 for slave mode) */
    uint32_t baud_rate;                   /* Baud rate (9600, 19200, 38400, 57600, 115200) */
    uint8_t data_bits;                    /* Data bits (7 or 8) */
    uint8_t stop_bits;                    /* Stop bits (1 or 2) */
    char parity;                          /* Parity ('N', 'E', 'O') */
    uint32_t response_timeout_ms;         /* Response timeout for master */
    uint32_t turn_around_delay_us;        /* Turn-around delay between requests */
    uint8_t max_retries;                  /* Maximum retry attempts */
    bool enable_broadcast;                /* Enable broadcast messages */
    uint16_t max_coils;                   /* Maximum coil addresses */
    uint16_t max_registers;               /* Maximum register addresses */
} modbus_config_t;

/* ============================================================================
 * Modbus Data Structures
 * ============================================================================ */

/* Modbus Frame Structure */
typedef struct {
    uint8_t address;                      /* Slave address */
    uint8_t function_code;                /* Function code */
    uint8_t *data;                        /* Data payload */
    uint16_t data_length;                 /* Data length */
    uint16_t crc;                         /* CRC-16 checksum */
    uint32_t timestamp;                   /* Frame timestamp */
} modbus_frame_t;

/* Modbus Exception Response */
typedef struct {
    uint8_t address;                      /* Slave address */
    uint8_t function_code;                /* Function code with error bit set */
    uint8_t exception_code;               /* Exception code */
    uint16_t crc;                         /* CRC-16 checksum */
} modbus_exception_response_t;

/* Modbus Data Map */
typedef struct {
    bool *coils;                          /* Coil status array */
    bool *discrete_inputs;                /* Discrete input array */
    uint16_t *holding_registers;          /* Holding register array */
    uint16_t *input_registers;            /* Input register array */
    uint16_t num_coils;                   /* Number of coils */
    uint16_t num_discrete_inputs;         /* Number of discrete inputs */
    uint16_t num_holding_registers;       /* Number of holding registers */
    uint16_t num_input_registers;         /* Number of input registers */
} modbus_data_map_t;

/* ============================================================================
 * Modbus Statistics and Diagnostics
 * ============================================================================ */

typedef struct {
    uint32_t frames_sent;                 /* Total frames sent */
    uint32_t frames_received;             /* Total frames received */
    uint32_t exceptions_sent;             /* Exception responses sent */
    uint32_t crc_errors;                  /* CRC errors detected */
    uint32_t timeouts;                    /* Response timeouts */
    uint32_t retries;                     /* Retry attempts */
    uint32_t successful_requests;         /* Successful requests */
    uint32_t failed_requests;             /* Failed requests */
    uint32_t broadcast_frames;            /* Broadcast frames sent */
    uint32_t overrun_errors;              /* Buffer overrun errors */
    uint32_t parity_errors;               /* Parity errors */
    uint32_t framing_errors;              /* Framing errors */
} modbus_statistics_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize Modbus RTU protocol
 *
 * @param config Pointer to Modbus configuration
 * @return true if initialization successful, false otherwise
 */
bool modbus_rtu_init(const modbus_config_t *config);

/**
 * @brief Deinitialize Modbus RTU protocol
 *
 * @return true if deinitialization successful, false otherwise
 */
bool modbus_rtu_deinit(void);

/**
 * @brief Initialize Modbus data map
 *
 * @param data_map Pointer to data map structure
 * @return true if initialization successful, false otherwise
 */
bool modbus_init_data_map(modbus_data_map_t *data_map);

/**
 * @brief Read coils from slave device
 *
 * @param slave_address Slave address (1-247)
 * @param start_address Starting coil address
 * @param quantity Number of coils to read
 * @param coil_values Pointer to array to store coil values
 * @return true if read successful, false otherwise
 */
bool modbus_read_coils(uint8_t slave_address, uint16_t start_address,
                      uint16_t quantity, bool *coil_values);

/**
 * @brief Read discrete inputs from slave device
 *
 * @param slave_address Slave address (1-247)
 * @param start_address Starting discrete input address
 * @param quantity Number of discrete inputs to read
 * @param input_values Pointer to array to store input values
 * @return true if read successful, false otherwise
 */
bool modbus_read_discrete_inputs(uint8_t slave_address, uint16_t start_address,
                                uint16_t quantity, bool *input_values);

/**
 * @brief Read holding registers from slave device
 *
 * @param slave_address Slave address (1-247)
 * @param start_address Starting register address
 * @param quantity Number of registers to read
 * @param register_values Pointer to array to store register values
 * @return true if read successful, false otherwise
 */
bool modbus_read_holding_registers(uint8_t slave_address, uint16_t start_address,
                                  uint16_t quantity, uint16_t *register_values);

/**
 * @brief Read input registers from slave device
 *
 * @param slave_address Slave address (1-247)
 * @param start_address Starting register address
 * @param quantity Number of registers to read
 * @param register_values Pointer to array to store register values
 * @return true if read successful, false otherwise
 */
bool modbus_read_input_registers(uint8_t slave_address, uint16_t start_address,
                                uint16_t quantity, uint16_t *register_values);

/**
 * @brief Write single coil to slave device
 *
 * @param slave_address Slave address (1-247)
 * @param coil_address Coil address
 * @param coil_value Coil value to write
 * @return true if write successful, false otherwise
 */
bool modbus_write_single_coil(uint8_t slave_address, uint16_t coil_address, bool coil_value);

/**
 * @brief Write single register to slave device
 *
 * @param slave_address Slave address (1-247)
 * @param register_address Register address
 * @param register_value Register value to write
 * @return true if write successful, false otherwise
 */
bool modbus_write_single_register(uint8_t slave_address, uint16_t register_address,
                                 uint16_t register_value);

/**
 * @brief Write multiple coils to slave device
 *
 * @param slave_address Slave address (1-247)
 * @param start_address Starting coil address
 * @param quantity Number of coils to write
 * @param coil_values Pointer to array of coil values
 * @return true if write successful, false otherwise
 */
bool modbus_write_multiple_coils(uint8_t slave_address, uint16_t start_address,
                                uint16_t quantity, const bool *coil_values);

/**
 * @brief Write multiple registers to slave device
 *
 * @param slave_address Slave address (1-247)
 * @param start_address Starting register address
 * @param quantity Number of registers to write
 * @param register_values Pointer to array of register values
 * @return true if write successful, false otherwise
 */
bool modbus_write_multiple_registers(uint8_t slave_address, uint16_t start_address,
                                    uint16_t quantity, const uint16_t *register_values);

/**
 * @brief Send raw Modbus request
 *
 * @param slave_address Slave address (1-247)
 * @param function_code Function code
 * @param data Pointer to request data
 * @param data_length Length of request data
 * @param response_buffer Pointer to response buffer
 * @param response_length Pointer to store response length
 * @param max_response_length Maximum response buffer length
 * @return true if request successful, false otherwise
 */
bool modbus_send_request(uint8_t slave_address, uint8_t function_code,
                        const uint8_t *data, uint16_t data_length,
                        uint8_t *response_buffer, uint16_t *response_length,
                        uint16_t max_response_length);

/**
 * @brief Process incoming Modbus frame (slave mode)
 *
 * @param frame Pointer to received frame
 * @param response_buffer Pointer to response buffer
 * @param response_length Pointer to store response length
 * @param max_response_length Maximum response buffer length
 * @return true if frame processed successfully, false otherwise
 */
bool modbus_process_frame(const modbus_frame_t *frame, uint8_t *response_buffer,
                         uint16_t *response_length, uint16_t max_response_length);

/**
 * @brief Register coil read/write callback
 *
 * @param callback Function pointer to coil callback
 * @return true if callback registered successfully, false otherwise
 */
bool modbus_register_coil_callback(bool (*callback)(uint16_t address, bool *value, bool write));

/**
 * @brief Register register read/write callback
 *
 * @param callback Function pointer to register callback
 * @return true if callback registered successfully, false otherwise
 */
bool modbus_register_register_callback(bool (*callback)(uint16_t address, uint16_t *value, bool write));

/**
 * @brief Get Modbus statistics
 *
 * @param statistics Pointer to statistics structure to fill
 * @return true if statistics retrieved successfully, false otherwise
 */
bool modbus_get_statistics(modbus_statistics_t *statistics);

/**
 * @brief Clear Modbus statistics
 *
 * @return true if statistics cleared successfully, false otherwise
 */
bool modbus_clear_statistics(void);

/**
 * @brief Perform Modbus diagnostics
 *
 * @param sub_function Diagnostic sub-function code
 * @param data Diagnostic data
 * @param response_buffer Pointer to response buffer
 * @param response_length Pointer to store response length
 * @return true if diagnostics successful, false otherwise
 */
bool modbus_diagnostics(uint16_t sub_function, uint16_t data,
                       uint8_t *response_buffer, uint16_t *response_length);

/**
 * @brief Calculate CRC-16 for Modbus frame
 *
 * @param data Pointer to data buffer
 * @param length Length of data
 * @return CRC-16 value
 */
uint16_t modbus_calculate_crc(const uint8_t *data, uint16_t length);

/**
 * @brief Validate Modbus frame CRC
 *
 * @param frame Pointer to Modbus frame
 * @return true if CRC is valid, false otherwise
 */
bool modbus_validate_crc(const modbus_frame_t *frame);

/**
 * @brief Set slave address
 *
 * @param address New slave address (1-247)
 * @return true if address set successfully, false otherwise
 */
bool modbus_set_slave_address(uint8_t address);

/**
 * @brief Get current slave address
 *
 * @return Current slave address
 */
uint8_t modbus_get_slave_address(void);

/**
 * @brief Enable/disable broadcast messages
 *
 * @param enable true to enable broadcast, false to disable
 * @return true if operation successful, false otherwise
 */
bool modbus_enable_broadcast(bool enable);

/**
 * @brief Set communication parameters
 *
 * @param baud_rate Baud rate
 * @param data_bits Data bits (7 or 8)
 * @param stop_bits Stop bits (1 or 2)
 * @param parity Parity ('N', 'E', 'O')
 * @return true if parameters set successfully, false otherwise
 */
bool modbus_set_comm_params(uint32_t baud_rate, uint8_t data_bits,
                           uint8_t stop_bits, char parity);

/**
 * @brief Get communication parameters
 *
 * @param baud_rate Pointer to store baud rate
 * @param data_bits Pointer to store data bits
 * @param stop_bits Pointer to store stop bits
 * @param parity Pointer to store parity
 * @return true if parameters retrieved successfully, false otherwise
 */
bool modbus_get_comm_params(uint32_t *baud_rate, uint8_t *data_bits,
                           uint8_t *stop_bits, char *parity);

/**
 * @brief Enter Modbus test mode
 *
 * @return true if test mode entered successfully, false otherwise
 */
bool modbus_enter_test_mode(void);

/**
 * @brief Exit Modbus test mode
 *
 * @return true if test mode exited successfully, false otherwise
 */
bool modbus_exit_test_mode(void);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_MODBUS_RTU_H */
