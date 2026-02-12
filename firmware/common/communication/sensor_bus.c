/**
 * @file sensor_bus.c
 * @brief RS-485 Sensor Bus Manager Implementation
 *
 * This file implements the sensor bus management layer using USART2 with the
 * SN65HVD72D half-duplex RS-485 transceiver. It builds on top of the EsoCore
 * protocol layer (protocol.h) to provide automatic device discovery,
 * hot-plug detection, health monitoring, and 12V bus power management.
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

/* ============================================================================
 * Protocol Message Handling
 * ============================================================================ */

/**
 * @brief Process incoming data from the RX buffer
 *
 * Scans for EsoCore protocol frames (start delimiter ESOCORE_FRAME_START),
 * parses complete frames, and dispatches them based on message type.
 *
 * Handled message types:
 * - MSG_TYPE_DISCOVERY_RESPONSE: update device table
 * - MSG_TYPE_HEARTBEAT: refresh device last_seen timestamp
 * - MSG_TYPE_DATA_STREAM / MSG_TYPE_DATA_RESPONSE: invoke data callback
 *
 * TODO: This currently uses a simplified byte-by-byte parser. For
 * production, integrate with the full esocore_protocol_receive_message()
 * path once the protocol layer has proper USART2 hardware bindings.
 */
static void sbus_process_rx(void) {
    uint8_t byte;

    /*
     * Simplified frame processing loop.
     *
     * The EsoCore protocol frame starts with ESOCORE_FRAME_START (0xAA).
     * We scan for it, then attempt to read a complete frame header to
     * determine the payload length, and finally validate + process.
     *
     * TODO: Replace with ring-buffer-aware frame parser that handles
     * partial frames across multiple poll() calls. Current implementation
     * processes one byte at a time and relies on frames arriving intact
     * within a single poll interval.
     */

    static uint8_t  frame_buf[ESOCORE_MAX_PAYLOAD_SIZE + 32];
    static uint16_t frame_idx = 0;
    static bool     in_frame = false;
    static uint16_t expected_len = 0;

    while (sbus_rx_read(&byte)) {
        if (!in_frame) {
            if (byte == ESOCORE_FRAME_START) {
                frame_buf[0] = byte;
                frame_idx = 1;
                in_frame = true;
                expected_len = 0;
            }
            continue;
        }

        if (frame_idx < sizeof(frame_buf)) {
            frame_buf[frame_idx++] = byte;
        } else {
            /* Frame too long, discard */
            in_frame = false;
            frame_idx = 0;
            sb_status.bus_errors++;
            continue;
        }

        /*
         * Once we have the header (sizeof esocore_frame_header_t bytes),
         * we can determine the total expected frame length.
         * Header: start(1) + version(1) + src(1) + dst(1) + type(1) +
         *         payload_len(2) + header_crc(2) = 9 bytes
         * Then payload + payload_crc(2) + end_delimiter(1)
         */
        if (frame_idx == 9 && expected_len == 0) {
            uint16_t payload_len = (uint16_t)frame_buf[5] |
                                   ((uint16_t)frame_buf[6] << 8);

            if (payload_len > ESOCORE_MAX_PAYLOAD_SIZE) {
                /* Invalid payload length */
                in_frame = false;
                frame_idx = 0;
                sb_status.bus_errors++;
                continue;
            }

            /* Total: header(9) + payload + payload_crc(2) + end(1) */
            expected_len = 9 + payload_len + 2 + 1;
        }

        /* Check if we have a complete frame */
        if (expected_len > 0 && frame_idx >= expected_len) {
            /* Extract key fields for dispatch */
            uint8_t src_addr = frame_buf[2];
            uint8_t msg_type = frame_buf[4];
            uint16_t payload_len = (uint16_t)frame_buf[5] |
                                   ((uint16_t)frame_buf[6] << 8);
            uint8_t *payload = &frame_buf[9];

            /* Verify end delimiter */
            if (frame_buf[expected_len - 1] != ESOCORE_FRAME_END) {
                sb_status.bus_errors++;
                in_frame = false;
                frame_idx = 0;
                continue;
            }

            /* TODO: Validate header CRC and payload CRC using
             * esocore_crc16() for production use */

            /* Dispatch based on message type */
            switch (msg_type) {
                case MSG_TYPE_DISCOVERY_RESPONSE:
                    sbus_handle_discovery_response(src_addr, payload,
                                                   payload_len);
                    break;

                case MSG_TYPE_HEARTBEAT:
                    sbus_handle_heartbeat(src_addr);
                    break;

                case MSG_TYPE_DATA_RESPONSE:
                case MSG_TYPE_DATA_STREAM:
                    if (sb_data_cb) {
                        sb_data_cb(src_addr, payload, payload_len,
                                   sb_cb_ctx);
                    }
                    break;

                default:
                    /* Unknown or unhandled message type */
                    break;
            }

            in_frame = false;
            frame_idx = 0;
            expected_len = 0;
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
        /* New device -- parse info and add to table */
        uint8_t device_type = 0;
        if (payload_len >= 1) {
            device_type = payload[0];
        }

        dev = sbus_add_device(address, device_type);
        if (!dev) {
            return; /* Table full */
        }

        /* Parse firmware version (bytes 1-3) */
        if (payload_len >= 4) {
            dev->firmware_version[0] = payload[1]; /* major */
            dev->firmware_version[1] = payload[2]; /* minor */
            dev->firmware_version[2] = payload[3]; /* patch */
        }

        /* Parse serial number (bytes 4-11) */
        if (payload_len >= 4 + SBUS_SERIAL_SIZE) {
            memcpy(dev->serial, &payload[4], SBUS_SERIAL_SIZE);
        }

        /* Parse device name (bytes 12+) */
        if (payload_len > 4 + SBUS_SERIAL_SIZE) {
            uint16_t name_len = payload_len - 4 - SBUS_SERIAL_SIZE;
            if (name_len >= SBUS_DEVICE_NAME_SIZE) {
                name_len = SBUS_DEVICE_NAME_SIZE - 1;
            }
            memcpy(dev->name, &payload[4 + SBUS_SERIAL_SIZE], name_len);
            dev->name[name_len] = '\0';
        }

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

    /*
     * Build and send a discovery request broadcast.
     *
     * Uses the EsoCore protocol frame format defined in protocol.h:
     * - Start delimiter: ESOCORE_FRAME_START (0xAA)
     * - Message type: MSG_TYPE_DISCOVERY_REQUEST (0x01)
     * - Destination: broadcast (0xFF)
     *
     * TODO: Use esocore_send_message() once it has proper USART2 bindings.
     * For now, we build the frame manually and transmit via sbus_transmit().
     */

    uint8_t discovery_frame[12];
    uint16_t idx = 0;

    discovery_frame[idx++] = ESOCORE_FRAME_START;       /* Start delimiter */
    discovery_frame[idx++] = ESOCORE_PROTOCOL_VERSION;   /* Protocol version */
    discovery_frame[idx++] = 0x00;                       /* Source: Edge (addr 0) */
    discovery_frame[idx++] = 0xFF;                       /* Destination: broadcast */
    discovery_frame[idx++] = MSG_TYPE_DISCOVERY_REQUEST;  /* Message type */
    discovery_frame[idx++] = 0x00;                       /* Payload length low */
    discovery_frame[idx++] = 0x00;                       /* Payload length high */

    /* Header CRC (placeholder -- TODO: compute with esocore_crc16) */
    uint16_t hdr_crc = esocore_crc16(&discovery_frame[0], idx);
    discovery_frame[idx++] = (uint8_t)(hdr_crc & 0xFF);
    discovery_frame[idx++] = (uint8_t)((hdr_crc >> 8) & 0xFF);

    /* No payload, so payload CRC is 0x0000 */
    discovery_frame[idx++] = 0x00;
    discovery_frame[idx++] = 0x00;

    discovery_frame[idx++] = ESOCORE_FRAME_END;          /* End delimiter */

    bool ok = sbus_transmit(discovery_frame, idx);

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

    if (length > ESOCORE_MAX_PAYLOAD_SIZE - 1) {
        return false; /* Command byte + data must fit in payload */
    }

    /*
     * Build command frame using EsoCore protocol format.
     *
     * Payload: [CMD_BYTE] [DATA...]
     *
     * TODO: Use esocore_send_message() with MSG_TYPE_COMMAND once
     * the protocol layer supports USART2 hardware bindings.
     */

    uint8_t frame[ESOCORE_MAX_PAYLOAD_SIZE + 32];
    uint16_t idx = 0;
    uint16_t payload_len = 1 + length; /* cmd byte + data */

    frame[idx++] = ESOCORE_FRAME_START;
    frame[idx++] = ESOCORE_PROTOCOL_VERSION;
    frame[idx++] = 0x00;               /* Source: Edge */
    frame[idx++] = address;             /* Destination */
    frame[idx++] = MSG_TYPE_COMMAND;    /* Message type */
    frame[idx++] = (uint8_t)(payload_len & 0xFF);
    frame[idx++] = (uint8_t)((payload_len >> 8) & 0xFF);

    /* Header CRC */
    uint16_t hdr_crc = esocore_crc16(&frame[0], idx);
    frame[idx++] = (uint8_t)(hdr_crc & 0xFF);
    frame[idx++] = (uint8_t)((hdr_crc >> 8) & 0xFF);

    /* Payload: command byte + data */
    frame[idx++] = cmd;
    if (data && length > 0) {
        memcpy(&frame[idx], data, length);
        idx += length;
    }

    /* Payload CRC */
    uint16_t pay_crc = esocore_crc16(&frame[9], payload_len);
    frame[idx++] = (uint8_t)(pay_crc & 0xFF);
    frame[idx++] = (uint8_t)((pay_crc >> 8) & 0xFF);

    frame[idx++] = ESOCORE_FRAME_END;

    return sbus_transmit(frame, idx);
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
