/**
 * @file interbus.c
 * @brief Interbus Protocol Layer Implementation
 *
 * This file implements the Interbus RS-485 physical layer framework using UART5
 * with the ADM2582EBRWZ isolated transceiver. It provides a simplified
 * summation-frame protocol for cyclic I/O data exchange.
 *
 * Hardware:
 * - UART5 (PC12 TX, PD2 RX) at configurable baud rate (default 500 kbps)
 * - ADM2582EBRWZ isolated RS-485 transceiver
 * - DE/RE direction control on PD14
 * - 120 ohm termination switch on PD15
 *
 * Frame format (simplified summation frame):
 *   [SYNC 0x7E] [ADDR] [LENGTH] [DATA...] [CRC16_LO] [CRC16_HI]
 *
 * Note: The actual Interbus specification (IEC 61158) is proprietary.
 * This implementation provides the RS-485 transport framework with a
 * compatible physical layer. Full Interbus protocol compliance requires
 * licensing and additional protocol stack implementation.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "interbus.h"
#include "../../stm32/stm32h7/hal/hal_uart.h"
#include "../../stm32/stm32h7/hal/hal_gpio.h"
#include "../../stm32/stm32h7/bsp/pin_mapping_v150.h"
#include <string.h>

/* ============================================================================
 * Private Constants
 * ============================================================================ */

#define INTERBUS_RX_BUFFER_SIZE     64     /* Receive ring buffer size */
#define INTERBUS_FRAME_MAX_SIZE     36     /* Sync + Addr + Len + 16 data + 2 CRC */
#define INTERBUS_FRAME_TIMEOUT_MS   10     /* Frame reception timeout */

/* ============================================================================
 * Private Data Structures
 * ============================================================================ */

/* Module state */
static interbus_config_t      ib_config;
static interbus_status_t      ib_status;
static interbus_cyclic_data_t ib_cyclic;
static bool                   ib_initialized = false;

/* UART handle for UART5 */
static hal_uart_handle_t ib_uart;

/* Receive buffer (filled by RX interrupt callback) */
static volatile uint8_t  ib_rx_buf[INTERBUS_RX_BUFFER_SIZE];
static volatile uint16_t ib_rx_head = 0;
static volatile uint16_t ib_rx_tail = 0;

/* Frame assembly buffer */
static uint8_t  ib_frame_buf[INTERBUS_FRAME_MAX_SIZE];
static uint16_t ib_frame_idx = 0;
static bool     ib_frame_receiving = false;

/* User callback */
static interbus_cycle_callback_t ib_callback     = NULL;
static void                     *ib_callback_ctx = NULL;

/* ============================================================================
 * CRC-16 Calculation
 * ============================================================================ */

/**
 * @brief Calculate CRC-16 (CCITT polynomial) for Interbus frames
 *
 * Uses the same CRC-16 polynomial (0xA001 reflected) as Modbus for
 * consistency across the EsoCore RS-485 bus implementations.
 *
 * @param data   Pointer to data buffer
 * @param length Length of data
 * @return CRC-16 value
 */
static uint16_t interbus_crc16(const uint8_t *data, uint16_t length) {
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

/* ============================================================================
 * UART RX Interrupt Callback
 * ============================================================================ */

/**
 * @brief UART5 receive interrupt handler
 *
 * Called from ISR context. Stores received bytes into the ring buffer
 * for later processing in interbus_poll().
 */
static void interbus_rx_isr(uint8_t data, void *context) {
    (void)context;

    uint16_t next = (ib_rx_head + 1) % INTERBUS_RX_BUFFER_SIZE;
    if (next != ib_rx_tail) {
        ib_rx_buf[ib_rx_head] = data;
        ib_rx_head = next;
    } else {
        /* Buffer overrun -- discard byte */
        ib_status.errors++;
    }
}

/**
 * @brief Read one byte from the receive ring buffer
 *
 * @param byte Pointer to store received byte
 * @return true if a byte was available, false if buffer empty
 */
static bool interbus_rx_read(uint8_t *byte) {
    if (ib_rx_tail == ib_rx_head) {
        return false;
    }
    *byte = ib_rx_buf[ib_rx_tail];
    ib_rx_tail = (ib_rx_tail + 1) % INTERBUS_RX_BUFFER_SIZE;
    return true;
}

/* ============================================================================
 * Frame Building and Transmission
 * ============================================================================ */

/**
 * @brief Build and transmit a summation-frame response
 *
 * Frame layout: [0x7E] [ADDR] [LEN] [DATA...] [CRC_LO] [CRC_HI]
 *
 * @param address Destination address
 * @param data    Pointer to payload data
 * @param length  Payload length (max INTERBUS_MAX_DATA_SIZE)
 * @return true if frame transmitted, false on error
 */
static bool interbus_send_frame(uint8_t address, const uint8_t *data,
                                uint8_t length) {
    if (length > INTERBUS_MAX_DATA_SIZE) {
        return false;
    }

    uint8_t frame[INTERBUS_FRAME_MAX_SIZE];
    uint16_t idx = 0;

    /* Build frame */
    frame[idx++] = INTERBUS_SYNC_BYTE;
    frame[idx++] = address;
    frame[idx++] = length;

    if (data && length > 0) {
        memcpy(&frame[idx], data, length);
        idx += length;
    }

    /* Calculate CRC over address + length + data (exclude sync byte) */
    uint16_t crc = interbus_crc16(&frame[1], idx - 1);
    frame[idx++] = (uint8_t)(crc & 0xFF);
    frame[idx++] = (uint8_t)((crc >> 8) & 0xFF);

    /* Switch transceiver to TX mode, transmit, switch back to RX */
    hal_uart_rs485_tx_enable(&ib_uart);
    bool ok = hal_uart_transmit(&ib_uart, frame, idx, 50);
    hal_uart_rs485_rx_enable(&ib_uart);

    return ok;
}

/* ============================================================================
 * Frame Reception and Processing
 * ============================================================================ */

/**
 * @brief Process a completely received frame
 *
 * Validates CRC, extracts input data, invokes the cyclic callback, and
 * sends a response frame with the current output data.
 *
 * @param frame  Pointer to frame buffer (starting with sync byte)
 * @param length Total frame length including sync and CRC
 * @return true if frame was valid and processed, false otherwise
 */
static bool interbus_process_frame(const uint8_t *frame, uint16_t length) {
    if (length < INTERBUS_FRAME_MIN_SIZE) {
        ib_status.errors++;
        return false;
    }

    /* Parse frame fields */
    uint8_t addr      = frame[1];
    uint8_t data_len  = frame[2];

    /* Validate data length against frame size */
    uint16_t expected_len = 3 + data_len + 2; /* sync + addr + len + data + crc */
    if (expected_len != length) {
        ib_status.errors++;
        return false;
    }

    /* CRC check: calculated over addr + len + data */
    uint16_t crc_calc = interbus_crc16(&frame[1], 2 + data_len);
    uint16_t crc_recv = (uint16_t)frame[3 + data_len] |
                        ((uint16_t)frame[4 + data_len] << 8);

    if (crc_calc != crc_recv) {
        ib_status.errors++;
        return false;
    }

    /* Check if frame is addressed to us or broadcast */
    if (addr != ib_config.node_address && addr != INTERBUS_BROADCAST_ADDR) {
        return false; /* Not for us, silently ignore */
    }

    /* Copy input data from frame */
    if (data_len > 0 && data_len <= INTERBUS_MAX_DATA_SIZE) {
        memcpy(ib_cyclic.input_data, &frame[3], data_len);
        ib_cyclic.input_length = data_len;
    }

    /* Invoke cyclic callback so application can process input and set output */
    if (ib_callback) {
        ib_callback(&ib_cyclic, ib_callback_ctx);
    }

    /* Send response with current output data (skip for broadcast frames) */
    if (addr != INTERBUS_BROADCAST_ADDR) {
        interbus_send_frame(ib_config.node_address,
                            ib_cyclic.output_data,
                            ib_cyclic.output_length);
    }

    /* Update status */
    ib_status.cycle_count++;
    ib_status.connected = true;

    return true;
}

/**
 * @brief Attempt to assemble and process frames from the RX buffer
 *
 * Scans for the sync byte, assembles frames byte-by-byte, and hands
 * complete frames to interbus_process_frame().
 */
static void interbus_process_rx(void) {
    uint8_t byte;

    while (interbus_rx_read(&byte)) {
        if (!ib_frame_receiving) {
            /* Waiting for sync byte */
            if (byte == INTERBUS_SYNC_BYTE) {
                ib_frame_buf[0] = byte;
                ib_frame_idx = 1;
                ib_frame_receiving = true;
            }
            continue;
        }

        /* Accumulate frame bytes */
        if (ib_frame_idx < INTERBUS_FRAME_MAX_SIZE) {
            ib_frame_buf[ib_frame_idx++] = byte;
        } else {
            /* Frame too long -- discard and resync */
            ib_frame_receiving = false;
            ib_frame_idx = 0;
            ib_status.errors++;
            continue;
        }

        /*
         * Check if we have enough bytes to determine frame completeness.
         * After receiving sync + addr + length (3 bytes), we know the
         * total expected frame size.
         */
        if (ib_frame_idx >= 3) {
            uint8_t data_len = ib_frame_buf[2];
            uint16_t expected_total = 3 + data_len + 2; /* sync+addr+len + data + crc */

            if (data_len > INTERBUS_MAX_DATA_SIZE) {
                /* Invalid length field -- discard frame */
                ib_frame_receiving = false;
                ib_frame_idx = 0;
                ib_status.errors++;
                continue;
            }

            if (ib_frame_idx >= expected_total) {
                /* Complete frame received -- process it */
                interbus_process_frame(ib_frame_buf, expected_total);
                ib_frame_receiving = false;
                ib_frame_idx = 0;
            }
        }
    }
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

/**
 * @brief Initialize the Interbus protocol layer
 */
bool interbus_init(const interbus_config_t *config) {
    if (!config || ib_initialized) {
        return false;
    }

    /* Store configuration */
    memcpy(&ib_config, config, sizeof(interbus_config_t));

    /* Apply defaults if needed */
    if (ib_config.baudrate == 0) {
        ib_config.baudrate = INTERBUS_DEFAULT_BAUDRATE;
    }

    /* Initialize UART5 for RS-485 communication */
    hal_uart_config_t uart_cfg = {
        .instance    = UART5,
        .baudrate    = ib_config.baudrate,
        .word_length = UART_WORDLEN_8,
        .stop_bits   = UART_STOP_1,
        .parity      = UART_PARITY_NONE,
        .rx_enable   = true,
        .tx_enable   = true,
        .de_re_pin   = INTERBUS_DE_RE,
        .rs485_mode  = true,
    };

    if (!hal_uart_init(&ib_uart, &uart_cfg)) {
        return false;
    }

    /* Configure DE/RE direction pin as output (managed by HAL RS-485 mode) */
    hal_gpio_init_output(INTERBUS_DE_RE);
    hal_gpio_write(INTERBUS_DE_RE, false); /* Default to RX mode (DE=LOW) */

    /* Configure 120 ohm termination GPIO */
    hal_gpio_init_output(INTERBUS_TERM_EN);
    hal_gpio_write(INTERBUS_TERM_EN, ib_config.enable_termination);

    /* Register RX interrupt callback */
    hal_uart_register_rx_callback(&ib_uart, interbus_rx_isr, NULL);

    /* Initialize internal state */
    memset((void *)&ib_status, 0, sizeof(interbus_status_t));
    memset(&ib_cyclic, 0, sizeof(interbus_cyclic_data_t));
    ib_rx_head = 0;
    ib_rx_tail = 0;
    ib_frame_idx = 0;
    ib_frame_receiving = false;

    ib_status.state = INTERBUS_STATE_INIT;
    ib_initialized = true;

    return true;
}

/**
 * @brief Deinitialize the Interbus protocol layer
 */
bool interbus_deinit(void) {
    if (!ib_initialized) {
        return false;
    }

    /* Stop communication first */
    interbus_stop();

    /* Release UART5 */
    hal_uart_deinit(&ib_uart);

    /* Disable termination */
    hal_gpio_write(INTERBUS_TERM_EN, false);

    /* Reset state */
    ib_callback = NULL;
    ib_callback_ctx = NULL;
    ib_initialized = false;
    ib_status.state = INTERBUS_STATE_IDLE;

    return true;
}

/**
 * @brief Start Interbus cyclic communication
 */
bool interbus_start(void) {
    if (!ib_initialized) {
        return false;
    }

    if (ib_status.state == INTERBUS_STATE_RUNNING) {
        return true; /* Already running */
    }

    /* Flush RX buffer */
    ib_rx_head = 0;
    ib_rx_tail = 0;
    ib_frame_idx = 0;
    ib_frame_receiving = false;

    /* Enable UART RX interrupt to begin receiving frames */
    hal_uart_enable_rx_interrupt(&ib_uart);

    /* Set transceiver to receive mode */
    hal_uart_rs485_rx_enable(&ib_uart);

    ib_status.state = INTERBUS_STATE_RUNNING;
    ib_status.connected = false;
    ib_status.cycle_count = 0;
    ib_status.errors = 0;

    /*
     * TODO: Implement Interbus initialization sequence
     *
     * Full Interbus compliance (IEC 61158 Type 8) requires:
     * - ID telegram exchange with the bus master
     * - Parameter assignment and configuration phase
     * - Loopword-based ring topology verification
     * - Summation frame counter synchronization
     *
     * This simplified implementation assumes the controller handles
     * ring management and we only respond to addressed data frames.
     */

    return true;
}

/**
 * @brief Stop Interbus communication
 */
bool interbus_stop(void) {
    if (!ib_initialized) {
        return false;
    }

    ib_status.state = INTERBUS_STATE_IDLE;
    ib_status.connected = false;

    return true;
}

/**
 * @brief Poll the Interbus state machine
 */
bool interbus_poll(void) {
    if (!ib_initialized || ib_status.state != INTERBUS_STATE_RUNNING) {
        return false;
    }

    uint32_t prev_cycle_count = ib_status.cycle_count;

    /* Process any received bytes and assemble/handle frames */
    interbus_process_rx();

    /*
     * TODO: Implement full Interbus polling logic
     *
     * A complete implementation would include:
     * - Watchdog: detect controller timeout (no frame received within
     *   N x cycle_time_ms) and transition to ERROR state
     * - Cycle timing measurement using a hardware timer to populate
     *   last_cycle_time_us accurately
     * - Automatic re-initialization after error recovery
     * - Diagnostics frame handling (Interbus Layer 2 management)
     * - Summation frame loopback verification
     */

    return (ib_status.cycle_count > prev_cycle_count);
}

/**
 * @brief Get current Interbus status
 */
bool interbus_get_status(interbus_status_t *status) {
    if (!status || !ib_initialized) {
        return false;
    }

    memcpy(status, (const void *)&ib_status, sizeof(interbus_status_t));
    return true;
}

/**
 * @brief Register cyclic data exchange callback
 */
bool interbus_register_callback(interbus_cycle_callback_t callback, void *ctx) {
    if (!ib_initialized) {
        return false;
    }

    ib_callback = callback;
    ib_callback_ctx = ctx;
    return true;
}

/**
 * @brief Set output data for the next bus cycle
 */
bool interbus_set_output_data(const uint8_t *data, uint8_t length) {
    if (!data || length == 0 || length > INTERBUS_MAX_DATA_SIZE) {
        return false;
    }

    memcpy(ib_cyclic.output_data, data, length);
    ib_cyclic.output_length = length;
    return true;
}

/**
 * @brief Get most recently received input data
 */
uint8_t interbus_get_input_data(uint8_t *data, uint8_t max_length) {
    if (!data || max_length == 0 || ib_cyclic.input_length == 0) {
        return 0;
    }

    uint8_t copy_len = ib_cyclic.input_length;
    if (copy_len > max_length) {
        copy_len = max_length;
    }

    memcpy(data, ib_cyclic.input_data, copy_len);
    return copy_len;
}
