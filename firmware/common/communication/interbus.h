/**
 * @file interbus.h
 * @brief Interbus Protocol Layer over RS-485 (ADM2582EBRWZ Transceiver)
 *
 * This file defines the Interbus protocol layer for the EsoCore Edge device.
 * Interbus is a cyclic fieldbus protocol originally developed by Phoenix Contact.
 * This implementation provides the RS-485 physical layer framework using the
 * isolated ADM2582EBRWZ transceiver on UART5, with a simplified summation-frame
 * protocol suitable for integration with legacy Bosch Rexroth controller machines.
 *
 * Features:
 * - Cyclic I/O data exchange via summation frames
 * - Configurable cycle time and baud rate
 * - CRC-16 frame integrity checking
 * - 120 ohm bus termination control
 * - Callback-based cyclic data handling
 * - Error detection and state management
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_INTERBUS_H
#define ESOCORE_INTERBUS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Interbus Constants
 * ============================================================================ */

#define INTERBUS_DEFAULT_BAUDRATE    500000  /* Default baud rate (500 kbps) */
#define INTERBUS_MAX_DATA_SIZE      16      /* Maximum cyclic I/O data size */
#define INTERBUS_SYNC_BYTE          0x7E    /* Frame synchronization byte */
#define INTERBUS_BROADCAST_ADDR     0xFF    /* Broadcast address */
#define INTERBUS_FRAME_MIN_SIZE     6       /* Sync + Addr + Len + CRC(2) + End */

/* ============================================================================
 * Interbus State Machine
 * ============================================================================ */

typedef enum {
    INTERBUS_STATE_IDLE    = 0,   /* Bus idle, not started */
    INTERBUS_STATE_INIT    = 1,   /* Initialization in progress */
    INTERBUS_STATE_RUNNING = 2,   /* Cyclic communication active */
    INTERBUS_STATE_ERROR   = 3,   /* Error condition detected */
} interbus_state_t;

/* ============================================================================
 * Interbus Configuration
 * ============================================================================ */

/**
 * @brief Interbus configuration structure
 */
typedef struct {
    uint32_t baudrate;              /* UART baud rate (default: 500000) */
    uint8_t  node_address;          /* This node's address on the bus */
    bool     enable_termination;    /* Enable 120 ohm bus termination resistor */
    uint16_t cycle_time_ms;         /* Target cycle time in milliseconds */
} interbus_config_t;

/* ============================================================================
 * Interbus Status and Data Structures
 * ============================================================================ */

/**
 * @brief Interbus runtime status
 */
typedef struct {
    interbus_state_t state;         /* Current bus state */
    uint32_t cycle_count;           /* Total completed cycles */
    uint32_t errors;                /* Total error count */
    uint32_t last_cycle_time_us;    /* Duration of last cycle in microseconds */
    bool     connected;             /* true if bus communication is active */
} interbus_status_t;

/**
 * @brief Interbus cyclic I/O data
 *
 * Each cycle exchanges input and output data between the controller
 * and this node. Input data is received from the controller; output
 * data is sent to the controller.
 */
typedef struct {
    uint8_t input_data[INTERBUS_MAX_DATA_SIZE];   /* Data received from controller */
    uint8_t input_length;                          /* Valid bytes in input_data */
    uint8_t output_data[INTERBUS_MAX_DATA_SIZE];  /* Data to send to controller */
    uint8_t output_length;                         /* Valid bytes in output_data */
} interbus_cyclic_data_t;

/* ============================================================================
 * Callback Types
 * ============================================================================ */

/**
 * @brief Cyclic data exchange callback
 *
 * Invoked each bus cycle after input data is received and before output
 * data is sent. The application should read input_data and update
 * output_data within this callback.
 *
 * @param data Pointer to cyclic data (input populated, output to be filled)
 * @param ctx  User-provided context pointer
 */
typedef void (*interbus_cycle_callback_t)(interbus_cyclic_data_t *data, void *ctx);

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize the Interbus protocol layer
 *
 * Configures UART5 and associated GPIOs (DE/RE direction, termination)
 * according to the supplied configuration. Does not start communication;
 * call interbus_start() after initialization.
 *
 * @param config Pointer to Interbus configuration
 * @return true if initialization successful, false otherwise
 */
bool interbus_init(const interbus_config_t *config);

/**
 * @brief Deinitialize the Interbus protocol layer
 *
 * Stops communication (if running), releases UART5, and resets all
 * internal state.
 *
 * @return true if deinitialization successful, false otherwise
 */
bool interbus_deinit(void);

/**
 * @brief Start Interbus cyclic communication
 *
 * Enables UART RX interrupt and transitions the state machine to
 * INTERBUS_STATE_RUNNING.
 *
 * @return true if started successfully, false otherwise
 */
bool interbus_start(void);

/**
 * @brief Stop Interbus communication
 *
 * Halts cyclic communication and returns to INTERBUS_STATE_IDLE.
 *
 * @return true if stopped successfully, false otherwise
 */
bool interbus_stop(void);

/**
 * @brief Poll the Interbus state machine
 *
 * Must be called periodically (e.g. from main loop). Checks for
 * received frames, processes summation-frame protocol, and invokes
 * the registered cyclic callback.
 *
 * @return true if a cycle was completed during this poll, false otherwise
 */
bool interbus_poll(void);

/**
 * @brief Get current Interbus status
 *
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool interbus_get_status(interbus_status_t *status);

/**
 * @brief Register cyclic data exchange callback
 *
 * @param callback Function to invoke each bus cycle
 * @param ctx      User context passed to callback
 * @return true if callback registered successfully, false otherwise
 */
bool interbus_register_callback(interbus_cycle_callback_t callback, void *ctx);

/**
 * @brief Set output data for the next bus cycle
 *
 * Copies the provided data into the output buffer. It will be
 * transmitted in the next summation-frame response.
 *
 * @param data   Pointer to output data
 * @param length Number of bytes (max INTERBUS_MAX_DATA_SIZE)
 * @return true if data set successfully, false otherwise
 */
bool interbus_set_output_data(const uint8_t *data, uint8_t length);

/**
 * @brief Get most recently received input data
 *
 * @param data       Pointer to buffer to receive input data
 * @param max_length Maximum number of bytes to copy
 * @return Number of bytes copied, 0 if no data available
 */
uint8_t interbus_get_input_data(uint8_t *data, uint8_t max_length);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_INTERBUS_H */
