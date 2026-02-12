/**
 * @file sensor_bus.h
 * @brief RS-485 Sensor Bus Manager (SN65HVD72D Transceiver)
 *
 * This file defines the sensor bus management layer for the EsoCore Edge device.
 * The sensor bus is a half-duplex RS-485 network connecting the Edge to multiple
 * EsoCore sensor modules (vibration, acoustic, current, temperature, etc.).
 *
 * This module extends the lower-level protocol defined in protocol.h by adding:
 * - Automatic device discovery and enumeration
 * - Hot-plug detection for sensor modules
 * - Device health monitoring via heartbeat tracking
 * - 12V sensor bus power supply management
 * - 120 ohm bus termination control
 *
 * Hardware:
 * - USART2 (PA3 TX, PA0 RX) via SN65HVD72D transceiver
 * - DE/RE direction control on PA4
 * - 120 ohm termination enable on PA5
 * - 12V sensor bus PSU enable on PA6
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_SENSOR_BUS_H
#define ESOCORE_SENSOR_BUS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Sensor Bus Constants
 * ============================================================================ */

#define SBUS_DEFAULT_BAUDRATE        115200  /* Default baud rate */
#define SBUS_MAX_SENSORS             32      /* Maximum sensors on the bus */
#define SBUS_DEVICE_NAME_SIZE        16      /* Max device name length */
#define SBUS_SERIAL_SIZE             8       /* Device serial number length */
#define SBUS_FW_VERSION_SIZE         3       /* Firmware version: major.minor.patch */
#define SBUS_DEFAULT_DISCOVERY_MS    5000    /* Default discovery interval (5s) */
#define SBUS_HEARTBEAT_TIMEOUT_MULT  3       /* Mark offline after 3x heartbeat miss */

/* ============================================================================
 * Sensor Bus State Machine
 * ============================================================================ */

typedef enum {
    SBUS_STATE_IDLE        = 0,   /* Bus idle, not started */
    SBUS_STATE_DISCOVERING = 1,   /* Device discovery in progress */
    SBUS_STATE_RUNNING     = 2,   /* Normal operation */
    SBUS_STATE_ERROR       = 3,   /* Error condition */
} sensor_bus_state_t;

/* ============================================================================
 * Sensor Bus Configuration
 * ============================================================================ */

/**
 * @brief Sensor bus configuration structure
 */
typedef struct {
    uint32_t baudrate;              /* UART baud rate (default: 115200) */
    bool     enable_termination;    /* Enable 120 ohm bus termination */
    bool     enable_12v_power;      /* Enable 12V sensor bus power on start */
    uint32_t discovery_interval_ms; /* Periodic discovery interval (ms) */
    uint8_t  max_sensors;           /* Maximum number of sensors to track */
} sensor_bus_config_t;

/* ============================================================================
 * Sensor Bus Device Information
 * ============================================================================ */

/**
 * @brief Information about a discovered sensor device
 */
typedef struct {
    uint8_t  address;                          /* Device bus address */
    uint8_t  device_type;                      /* Device type (esocore_device_type_t) */
    uint8_t  firmware_version[SBUS_FW_VERSION_SIZE]; /* FW version: [major, minor, patch] */
    uint8_t  serial[SBUS_SERIAL_SIZE];         /* Device serial number */
    char     name[SBUS_DEVICE_NAME_SIZE];      /* Human-readable device name */
    bool     online;                           /* true if device is responsive */
    uint32_t last_seen_ms;                     /* Timestamp of last communication */
} sensor_bus_device_t;

/* ============================================================================
 * Sensor Bus Status
 * ============================================================================ */

/**
 * @brief Overall sensor bus status
 */
typedef struct {
    sensor_bus_state_t  state;                         /* Current bus state */
    uint8_t             sensor_count;                  /* Number of discovered sensors */
    sensor_bus_device_t sensors[SBUS_MAX_SENSORS];     /* Device table */
    uint16_t            bus_voltage_mv;                /* Measured bus voltage in mV */
    uint32_t            bus_errors;                    /* Total bus error count */
} sensor_bus_status_t;

/* ============================================================================
 * Callback Types
 * ============================================================================ */

/**
 * @brief Device connect/disconnect event callback
 *
 * Invoked when a sensor device is discovered (connected) or has timed out
 * (disconnected). Enables the application to react to hot-plug events.
 *
 * @param device    Pointer to device information
 * @param connected true if device just connected, false if disconnected
 * @param ctx       User-provided context pointer
 */
typedef void (*sensor_bus_device_callback_t)(const sensor_bus_device_t *device,
                                            bool connected, void *ctx);

/**
 * @brief Incoming sensor data callback
 *
 * Invoked when a data frame is received from a sensor device. The
 * application should process the data within this callback or copy
 * it for deferred processing.
 *
 * @param address Source device address
 * @param data    Pointer to received data payload
 * @param length  Length of data payload
 * @param ctx     User-provided context pointer
 */
typedef void (*sensor_bus_data_callback_t)(uint8_t address,
                                          const uint8_t *data,
                                          uint16_t length, void *ctx);

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize the sensor bus manager
 *
 * Configures USART2 and associated GPIOs (DE/RE direction, termination,
 * 12V PSU enable) according to the supplied configuration. Does not start
 * the bus; call sensor_bus_start() after initialization.
 *
 * @param config Pointer to sensor bus configuration
 * @return true if initialization successful, false otherwise
 */
bool sensor_bus_init(const sensor_bus_config_t *config);

/**
 * @brief Deinitialize the sensor bus manager
 *
 * Stops the bus, powers down 12V supply, releases USART2, and resets
 * all internal state.
 *
 * @return true if deinitialization successful, false otherwise
 */
bool sensor_bus_deinit(void);

/**
 * @brief Start the sensor bus
 *
 * Enables 12V power (if configured), begins periodic device discovery,
 * and transitions to the RUNNING state.
 *
 * @return true if started successfully, false otherwise
 */
bool sensor_bus_start(void);

/**
 * @brief Stop the sensor bus
 *
 * Halts communication and optionally powers down the 12V supply.
 *
 * @return true if stopped successfully, false otherwise
 */
bool sensor_bus_stop(void);

/**
 * @brief Poll the sensor bus state machine
 *
 * Must be called periodically (e.g. from main loop). Handles incoming
 * messages, processes heartbeats, detects device timeouts, and triggers
 * periodic discovery scans.
 *
 * @return true if any activity was processed, false otherwise
 */
bool sensor_bus_poll(void);

/**
 * @brief Get current sensor bus status
 *
 * @param status Pointer to status structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool sensor_bus_get_status(sensor_bus_status_t *status);

/**
 * @brief Trigger a device discovery scan
 *
 * Sends a broadcast discovery request using the EsoCore protocol
 * (MSG_TYPE_DISCOVERY_REQUEST) and collects responses to build or
 * update the device table.
 *
 * @return true if discovery initiated successfully, false otherwise
 */
bool sensor_bus_discover(void);

/**
 * @brief Send a command to a specific sensor device
 *
 * Handles RS-485 direction switching (DE/RE to TX, transmit, back to RX).
 *
 * @param address Destination device address
 * @param cmd     Command byte
 * @param data    Pointer to command data payload (may be NULL)
 * @param length  Length of data payload
 * @return true if command sent successfully, false otherwise
 */
bool sensor_bus_send_command(uint8_t address, uint8_t cmd,
                            const uint8_t *data, uint16_t length);

/**
 * @brief Register device and data callbacks
 *
 * @param device_cb Callback for device connect/disconnect events (may be NULL)
 * @param data_cb   Callback for incoming sensor data (may be NULL)
 * @param ctx       User context passed to both callbacks
 * @return true if callbacks registered, false otherwise
 */
bool sensor_bus_register_callbacks(sensor_bus_device_callback_t device_cb,
                                  sensor_bus_data_callback_t data_cb,
                                  void *ctx);

/**
 * @brief Control the 12V sensor bus power supply
 *
 * @param enable true to enable 12V output, false to disable
 * @return true if power state changed successfully, false otherwise
 */
bool sensor_bus_set_12v_power(bool enable);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_SENSOR_BUS_H */
