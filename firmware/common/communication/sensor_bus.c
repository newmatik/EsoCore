/**
 * @file sensor_bus.c
 * @brief RS-485 Sensor Bus Manager Implementation
 *
 * This file implements the sensor bus management layer using USART2 with the
 * SN65HVD72D half-duplex RS-485 transceiver. It provides automatic device
 * discovery, hot-plug detection, health monitoring, and 12V bus power
 * management. Frames are built and parsed by the protocol frame codec
 * (protocol.h, protocol_frame.c), the same code the sensor modules use.
 *
 * Hardware:
 * - USART2 (PA3 TX, PA0 RX) at configurable baud rate (default 115200)
 * - SN65HVD72D half-duplex RS-485 transceiver
 * - DE/RE direction control on PA4
 * - 120 ohm termination switch on PA5
 * - 12V sensor bus PSU enable on PA6
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "sensor_bus.h"
#include "protocol.h"
#include "../../stm32/stm32h7/hal/hal_uart.h"
#include "../../stm32/stm32h7/hal/hal_gpio.h"
#include "../../stm32/stm32h7/bsp/pin_mapping_v150.h"
#include <string.h>

/* ============================================================================
 * Private Constants
 * ============================================================================ */

#define SBUS_RX_BUFFER_SIZE        512    /* Receive ring buffer size */
#define SBUS_HEARTBEAT_INTERVAL_MS 1000   /* Expected heartbeat interval from sensors */
#define SBUS_DISCOVERY_WAIT_MS     200    /* Time to wait for discovery responses */
#define SBUS_TX_TIMEOUT_MS         50     /* UART transmit timeout */

/* Bus address of the Edge (master) */
#define SBUS_EDGE_ADDRESS          ESOCORE_PROTOCOL_MASTER_ADDRESS

/* ============================================================================
 * Private Data Structures
 * ============================================================================ */

/* Module state */
static sensor_bus_config_t  sb_config;
static sensor_bus_status_t  sb_status;
static bool                 sb_initialized  = false;
static bool                 sb_12v_enabled  = false;

/* UART handle for USART2 */
static hal_uart_handle_t sb_uart;

/* Receive buffer (filled by RX interrupt callback) */
static volatile uint8_t  sb_rx_buf[SBUS_RX_BUFFER_SIZE];
static volatile uint16_t sb_rx_head = 0;
static volatile uint16_t sb_rx_tail = 0;

/* Timing */
static uint32_t sb_last_discovery_ms  = 0;
static uint32_t sb_current_time_ms    = 0;  /* Updated externally or via SysTick */

/* Sequence number of the next frame the Edge sends */
static uint8_t sb_tx_sequence = 0;

/* User callbacks */
static sensor_bus_device_callback_t sb_device_cb  = NULL;
static sensor_bus_data_callback_t   sb_data_cb    = NULL;
static void                        *sb_cb_ctx     = NULL;

/* ============================================================================
 * Timing Helper
 * ============================================================================ */

/**
 * @brief Get current system time in milliseconds
 *
 * TODO: Replace with actual SysTick or HAL timer query.
 * For now returns a module-internal counter that must be updated
 * externally or hooked to the system tick.
 *
 * @return Current time in milliseconds
 */
static uint32_t sbus_get_time_ms(void) {
    /* TODO: Implement using HAL timer or SysTick */
    return sb_current_time_ms;
}

/* ============================================================================
 * UART RX Interrupt Callback
 * ============================================================================ */

/**
 * @brief USART2 receive interrupt handler
 *
 * Called from ISR context. Stores received bytes into the ring buffer
 * for later processing in sensor_bus_poll().
 */
static void sbus_rx_isr(uint8_t data, void *context) {
    (void)context;

    uint16_t next = (sb_rx_head + 1) % SBUS_RX_BUFFER_SIZE;
    if (next != sb_rx_tail) {
        sb_rx_buf[sb_rx_head] = data;
        sb_rx_head = next;
    } else {
        /* Buffer overrun */
        sb_status.bus_errors++;
    }
}

/**
 * @brief Read one byte from the receive ring buffer
 *
 * @param byte Pointer to store received byte
 * @return true if a byte was available, false if buffer empty
 */
static bool sbus_rx_read(uint8_t *byte) {
    if (sb_rx_tail == sb_rx_head) {
        return false;
    }
    *byte = sb_rx_buf[sb_rx_tail];
    sb_rx_tail = (sb_rx_tail + 1) % SBUS_RX_BUFFER_SIZE;
    return true;
}

/**
 * @brief Get number of bytes available in the RX ring buffer
 *
 * @return Number of bytes available
 */
static uint16_t sbus_rx_available(void) {
    uint16_t head = sb_rx_head;
    uint16_t tail = sb_rx_tail;
    if (head >= tail) {
        return head - tail;
    }
    return SBUS_RX_BUFFER_SIZE - tail + head;
}

/* ============================================================================
 * Device Table Management
 * ============================================================================ */

/**
 * @brief Find a device in the device table by address
 *
 * @param address Device bus address
 * @return Pointer to device entry, or NULL if not found
 */
static sensor_bus_device_t *sbus_find_device(uint8_t address) {
    for (uint8_t i = 0; i < sb_status.sensor_count; i++) {
        if (sb_status.sensors[i].address == address) {
            return &sb_status.sensors[i];
        }
    }
    return NULL;
}

/**
 * @brief Add a new device to the device table
 *
 * @param address     Device bus address
 * @param device_type Device type identifier
 * @return Pointer to new device entry, or NULL if table is full
 */
static sensor_bus_device_t *sbus_add_device(uint8_t address, uint8_t device_type) {
    uint8_t max = sb_config.max_sensors;
    if (max == 0 || max > SBUS_MAX_SENSORS) {
        max = SBUS_MAX_SENSORS;
    }

    if (sb_status.sensor_count >= max) {
        return NULL; /* Table full */
    }

    sensor_bus_device_t *dev = &sb_status.sensors[sb_status.sensor_count];
    memset(dev, 0, sizeof(sensor_bus_device_t));
    dev->address = address;
    dev->device_type = device_type;
    dev->online = true;
    dev->last_seen_ms = sbus_get_time_ms();
    sb_status.sensor_count++;

    return dev;
}

/**
 * @brief Check all devices for heartbeat timeout and mark offline
 *
 * A device is marked offline if it has not been seen for
 * SBUS_HEARTBEAT_TIMEOUT_MULT * SBUS_HEARTBEAT_INTERVAL_MS.
 */
static void sbus_check_timeouts(void) {
    uint32_t now = sbus_get_time_ms();
    uint32_t timeout = (uint32_t)SBUS_HEARTBEAT_TIMEOUT_MULT *
                       SBUS_HEARTBEAT_INTERVAL_MS;

    for (uint8_t i = 0; i < sb_status.sensor_count; i++) {
        sensor_bus_device_t *dev = &sb_status.sensors[i];
        if (!dev->online) {
            continue;
        }

        if ((now - dev->last_seen_ms) > timeout) {
            dev->online = false;

            /* Notify application of disconnect */
            if (sb_device_cb) {
                sb_device_cb(dev, false, sb_cb_ctx);
            }
        }
    }
}

/* ============================================================================
 * Forward Declarations (static helpers used before definition)
 * ============================================================================ */

static void sbus_handle_discovery_response(uint8_t address,
                                           const uint8_t *payload,
                                           uint16_t payload_len);
static void sbus_handle_heartbeat(uint8_t address);

/* ============================================================================
 * Frame Transmission Helpers
 * ============================================================================ */

/**
 * @brief Transmit raw data on the sensor bus
 *
 * Handles RS-485 direction switching: sets DE/RE to TX mode, transmits
 * the data, then switches back to RX mode.
 *
 * @param data   Pointer to data to transmit
 * @param length Length of data
 * @return true if transmission successful, false otherwise
 */
static bool sbus_transmit(const uint8_t *data, uint16_t length) {
    if (!data || length == 0) {
        return false;
    }

    /* Set transceiver to TX mode */
    hal_uart_rs485_tx_enable(&sb_uart);

    /* Transmit data */
    bool ok = hal_uart_transmit(&sb_uart, data, length, SBUS_TX_TIMEOUT_MS);

    /* Switch back to RX mode */
    hal_uart_rs485_rx_enable(&sb_uart);

    if (!ok) {
        sb_status.bus_errors++;
    }

    return ok;
}

/**
 * @brief Encode and transmit one protocol frame from the Edge
 *
 * @param destination Destination address (ESOCORE_PROTOCOL_BROADCAST_ADDRESS for all)
 * @param type        Message type (esocore_message_type_t)
 * @param payload     Payload bytes (may be NULL when length is 0)
 * @param length      Payload length
 * @param flags       ESOCORE_FLAG_* bits
 * @return true if the frame was transmitted
 */
static bool sbus_send_frame(uint8_t destination, esocore_message_type_t type,
                            const uint8_t *payload, uint16_t length, uint8_t flags) {
    if (length > ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE || (length > 0 && !payload)) {
        return false;
    }

    static esocore_message_t message;
    static uint8_t frame[ESOCORE_PROTOCOL_FRAME_SIZE];

    message.header.start_byte = ESOCORE_PROTOCOL_START_BYTE;
    message.header.source_address = SBUS_EDGE_ADDRESS;
    message.header.destination_address = destination;
    message.header.message_type = (uint8_t)type;
    message.header.sequence_number = sb_tx_sequence++;
    message.header.flags = flags;
    message.header.payload_length = length;
    if (length > 0) {
        memcpy(message.payload, payload, length);
    }

    uint16_t frame_length = esocore_protocol_encode_frame(&message, frame, sizeof(frame));
    return frame_length > 0 && sbus_transmit(frame, frame_length);
}

/* ============================================================================
 * Protocol Message Handling
 * ============================================================================ */

/**
 * @brief Process incoming data from the RX buffer
 *
 * Collects received bytes, takes complete frames out with
 * esocore_protocol_decode_frame() (start byte, length and CRC checked) and
 * dispatches those addressed to the Edge by message type:
 * - ESOCORE_MSG_DISCOVER_RESPONSE: update device table
 * - ESOCORE_MSG_HEARTBEAT: refresh device last_seen timestamp
 * - ESOCORE_MSG_DATA_RESPONSE / ESOCORE_MSG_DATA_BURST: invoke data callback
 *   (one call per frame, with sequence number and flags for reassembly)
 *
 * A frame that is still incomplete stays in the buffer for the next poll.
 */
static void sbus_process_rx(void) {
    static uint8_t           frame_buf[ESOCORE_PROTOCOL_FRAME_SIZE];
    static uint32_t          frame_len = 0;
    static esocore_message_t message;
    uint8_t byte;

    while (sbus_rx_read(&byte)) {
        frame_buf[frame_len++] = byte;

        /* Take every complete frame out of the buffer */
        while (frame_len > 0) {
            uint32_t consumed = 0;
            esocore_frame_status_t status = esocore_protocol_decode_frame(
                frame_buf, frame_len, &message, &consumed);

            if (status == ESOCORE_FRAME_INCOMPLETE) {
                break;
            }

            memmove(frame_buf, frame_buf + consumed, frame_len - consumed);
            frame_len -= consumed;

            if (status == ESOCORE_FRAME_CRC_ERROR || status == ESOCORE_FRAME_INVALID) {
                sb_status.bus_errors++;
                continue;
            }
            if (status != ESOCORE_FRAME_OK) {
                continue; /* ESOCORE_FRAME_NO_START: a byte between frames */
            }

            if (message.header.destination_address != SBUS_EDGE_ADDRESS &&
                message.header.destination_address != ESOCORE_PROTOCOL_BROADCAST_ADDRESS) {
                continue; /* Frame for another node */
            }

            uint8_t src_addr = message.header.source_address;

            switch (message.header.message_type) {
                case ESOCORE_MSG_DISCOVER_RESPONSE:
                    sbus_handle_discovery_response(src_addr, message.payload,
                                                   message.header.payload_length);
                    break;

                case ESOCORE_MSG_HEARTBEAT:
                    sbus_handle_heartbeat(src_addr);
                    break;

                case ESOCORE_MSG_DATA_RESPONSE:
                case ESOCORE_MSG_DATA_BURST:
                    if (sb_data_cb) {
                        sb_data_cb(src_addr, message.header.sequence_number,
                                   message.header.flags, message.payload,
                                   message.header.payload_length, sb_cb_ctx);
                    }
                    break;

                default:
                    /* Unknown or unhandled message type */
                    break;
            }
        }
    }
}

/**
 * @brief Handle a discovery response from a sensor device
 *
 * Parses the device information payload and adds the device to the
 * device table if not already present. If the device was previously
 * known but offline, marks it as online again (hot-plug re-detection).
 *
 * @param address     Source device address
 * @param payload     Pointer to discovery response payload
 * @param payload_len Length of payload
 */
static void sbus_handle_discovery_response(uint8_t address,
                                           const uint8_t *payload,
                                           uint16_t payload_len) {
    if (!payload || payload_len == 0) {
        return;
    }

    sensor_bus_device_t *dev = sbus_find_device(address);

    if (dev) {
        /* Device already known -- update timestamp and mark online */
        bool was_offline = !dev->online;
        dev->online = true;
        dev->last_seen_ms = sbus_get_time_ms();

        /* Hot-plug re-detection: notify if device came back online */
        if (was_offline && sb_device_cb) {
            sb_device_cb(dev, true, sb_cb_ctx);
        }
    } else {
        /* New device -- parse the device information and add it to the table */
        esocore_device_info_t info;
        if (!esocore_protocol_decode_device_info(payload, payload_len, address, &info)) {
            sb_status.bus_errors++;
            return;
        }

        dev = sbus_add_device(address, info.device_type);
        if (!dev) {
            return; /* Table full */
        }

        /* The device info carries major and minor version; no patch level */
        dev->firmware_version[0] = info.firmware_version_major;
        dev->firmware_version[1] = info.firmware_version_minor;
        dev->firmware_version[2] = 0;

        /* 16-bit serial number, little endian, in the first two bytes */
        memset(dev->serial, 0, SBUS_SERIAL_SIZE);
        dev->serial[0] = (uint8_t)(info.serial_number & 0xFFU);
        dev->serial[1] = (uint8_t)(info.serial_number >> 8);

        /* Notify application of new device */
        if (sb_device_cb) {
            sb_device_cb(dev, true, sb_cb_ctx);
        }
    }
}

/**
 * @brief Handle a heartbeat message from a sensor device
 *
 * Updates the device's last_seen timestamp. If the device was offline,
 * marks it as online and invokes the device callback.
 *
 * @param address Source device address
 */
static void sbus_handle_heartbeat(uint8_t address) {
    sensor_bus_device_t *dev = sbus_find_device(address);

    if (dev) {
        bool was_offline = !dev->online;
        dev->online = true;
        dev->last_seen_ms = sbus_get_time_ms();

        if (was_offline && sb_device_cb) {
            sb_device_cb(dev, true, sb_cb_ctx);
        }
    }
    /*
     * If we receive a heartbeat from an unknown device, we don't add it
     * to the table here. The next discovery cycle will pick it up with
     * full device information.
     */
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

/**
 * @brief Initialize the sensor bus manager
 */
bool sensor_bus_init(const sensor_bus_config_t *config) {
    if (!config || sb_initialized) {
        return false;
    }

    /* Store configuration */
    memcpy(&sb_config, config, sizeof(sensor_bus_config_t));

    /* Apply defaults */
    if (sb_config.baudrate == 0) {
        sb_config.baudrate = SBUS_DEFAULT_BAUDRATE;
    }
    if (sb_config.discovery_interval_ms == 0) {
        sb_config.discovery_interval_ms = SBUS_DEFAULT_DISCOVERY_MS;
    }
    if (sb_config.max_sensors == 0 || sb_config.max_sensors > SBUS_MAX_SENSORS) {
        sb_config.max_sensors = SBUS_MAX_SENSORS;
    }

    /* Initialize USART2 for RS-485 communication */
    hal_uart_config_t uart_cfg = {
        .instance    = USART2,
        .baudrate    = sb_config.baudrate,
        .word_length = UART_WORDLEN_8,
        .stop_bits   = UART_STOP_1,
        .parity      = UART_PARITY_NONE,
        .rx_enable   = true,
        .tx_enable   = true,
        .de_re_pin   = SBUS_DE_RE,
        .rs485_mode  = true,
    };

    if (!hal_uart_init(&sb_uart, &uart_cfg)) {
        return false;
    }

    /* Configure DE/RE direction pin */
    hal_gpio_init_output(SBUS_DE_RE);
    hal_gpio_write(SBUS_DE_RE, false); /* Default to RX mode */

    /* Configure 120 ohm termination resistor */
    hal_gpio_init_output(SBUS_TERM_EN);
    hal_gpio_write(SBUS_TERM_EN, sb_config.enable_termination);

    /* Configure 12V sensor bus PSU enable */
    hal_gpio_init_output(SBUS_12V_EN);
    hal_gpio_write(SBUS_12V_EN, false); /* Start with 12V off */
    sb_12v_enabled = false;

    /* Register RX interrupt callback */
    hal_uart_register_rx_callback(&sb_uart, sbus_rx_isr, NULL);

    /* Initialize internal state */
    memset((void *)&sb_status, 0, sizeof(sensor_bus_status_t));
    sb_rx_head = 0;
    sb_rx_tail = 0;
    sb_last_discovery_ms = 0;
    sb_current_time_ms = 0;

    sb_status.state = SBUS_STATE_IDLE;
    sb_initialized = true;

    return true;
}

/**
 * @brief Deinitialize the sensor bus manager
 */
bool sensor_bus_deinit(void) {
    if (!sb_initialized) {
        return false;
    }

    /* Stop bus operation */
    sensor_bus_stop();

    /* Power down 12V supply */
    sensor_bus_set_12v_power(false);

    /* Release USART2 */
    hal_uart_deinit(&sb_uart);

    /* Disable termination */
    hal_gpio_write(SBUS_TERM_EN, false);

    /* Reset state */
    sb_device_cb = NULL;
    sb_data_cb = NULL;
    sb_cb_ctx = NULL;
    sb_initialized = false;
    sb_status.state = SBUS_STATE_IDLE;

    return true;
}

/**
 * @brief Start the sensor bus
 */
bool sensor_bus_start(void) {
    if (!sb_initialized) {
        return false;
    }

    if (sb_status.state == SBUS_STATE_RUNNING) {
        return true; /* Already running */
    }

    /* Enable 12V sensor power if configured */
    if (sb_config.enable_12v_power) {
        sensor_bus_set_12v_power(true);

        /*
         * TODO: Add a short delay (e.g. 100 ms) after enabling 12V power
         * to allow sensor modules to boot before starting discovery.
         * For now, the first discovery cycle handles this implicitly since
         * the discovery interval provides sufficient startup time.
         */
    }

    /* Flush RX buffer */
    sb_rx_head = 0;
    sb_rx_tail = 0;

    /* Enable UART RX interrupt */
    hal_uart_enable_rx_interrupt(&sb_uart);

    /* Set transceiver to receive mode */
    hal_uart_rs485_rx_enable(&sb_uart);

    /* Clear device table */
    memset(sb_status.sensors, 0, sizeof(sb_status.sensors));
    sb_status.sensor_count = 0;
    sb_status.bus_errors = 0;

    sb_status.state = SBUS_STATE_RUNNING;

    /* Trigger initial discovery */
    sb_last_discovery_ms = 0; /* Force immediate discovery on first poll */

    return true;
}

/**
 * @brief Stop the sensor bus
 */
bool sensor_bus_stop(void) {
    if (!sb_initialized) {
        return false;
    }

    sb_status.state = SBUS_STATE_IDLE;

    /* Mark all devices offline */
    for (uint8_t i = 0; i < sb_status.sensor_count; i++) {
        sb_status.sensors[i].online = false;
    }

    return true;
}

/**
 * @brief Poll the sensor bus state machine
 */
bool sensor_bus_poll(void) {
    if (!sb_initialized || sb_status.state != SBUS_STATE_RUNNING) {
        return false;
    }

    bool activity = false;
    uint32_t now = sbus_get_time_ms();

    /* Process incoming data from RX buffer */
    if (sbus_rx_available() > 0) {
        sbus_process_rx();
        activity = true;
    }

    /* Check for device heartbeat timeouts */
    sbus_check_timeouts();

    /* Periodic discovery for hot-plug detection */
    if (sb_config.discovery_interval_ms > 0) {
        if ((now - sb_last_discovery_ms) >= sb_config.discovery_interval_ms) {
            sensor_bus_discover();
            sb_last_discovery_ms = now;
            activity = true;
        }
    }

    return activity;
}

/**
 * @brief Get current sensor bus status
 */
bool sensor_bus_get_status(sensor_bus_status_t *status) {
    if (!status || !sb_initialized) {
        return false;
    }

    memcpy(status, &sb_status, sizeof(sensor_bus_status_t));
    return true;
}

/**
 * @brief Trigger a device discovery scan
 */
bool sensor_bus_discover(void) {
    if (!sb_initialized) {
        return false;
    }

    sensor_bus_state_t prev_state = sb_status.state;
    sb_status.state = SBUS_STATE_DISCOVERING;

    /* Discovery request: broadcast, no payload */
    bool ok = sbus_send_frame(ESOCORE_PROTOCOL_BROADCAST_ADDRESS, ESOCORE_MSG_DISCOVER,
                              NULL, 0, ESOCORE_FLAG_BROADCAST);

    /* Restore previous state (responses will be processed in poll) */
    sb_status.state = prev_state;

    return ok;
}

/**
 * @brief Send a command to a specific sensor device
 */
bool sensor_bus_send_command(uint8_t address, uint8_t cmd,
                            const uint8_t *data, uint16_t length) {
    if (!sb_initialized) {
        return false;
    }

    if (length > ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE - 1 || (length > 0 && !data)) {
        return false; /* Command byte + data must fit in payload */
    }

    /* Payload: command byte (esocore_command_t), then its data */
    uint8_t payload[ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE];
    payload[0] = cmd;
    if (length > 0) {
        memcpy(&payload[1], data, length);
    }

    return sbus_send_frame(address, ESOCORE_MSG_COMMAND, payload, (uint16_t)(1 + length),
                           ESOCORE_FLAG_ACK_REQUIRED);
}

/**
 * @brief Register device and data callbacks
 */
bool sensor_bus_register_callbacks(sensor_bus_device_callback_t device_cb,
                                  sensor_bus_data_callback_t data_cb,
                                  void *ctx) {
    if (!sb_initialized) {
        return false;
    }

    sb_device_cb = device_cb;
    sb_data_cb = data_cb;
    sb_cb_ctx = ctx;
    return true;
}

/**
 * @brief Control the 12V sensor bus power supply
 */
bool sensor_bus_set_12v_power(bool enable) {
    if (!sb_initialized) {
        return false;
    }

    hal_gpio_write(SBUS_12V_EN, enable);
    sb_12v_enabled = enable;

    return true;
}
