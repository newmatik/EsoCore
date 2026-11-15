/**
 * @file test_sensor_bus.c
 * @brief Host unit tests for the timing of the Edge sensor bus manager (common/communication/sensor_bus.c)
 *
 * sensor_bus.c is compiled unchanged with the protocol frame codec. This file
 * stands in for the STM32H7 UART and GPIO HAL: transmitted frames are decoded
 * and recorded, received bytes go through the registered RX callback, and the
 * millisecond clock is a variable the tests advance. Built and run with
 * `make test` in firmware/.
 */

#include "sensor_bus.h"
#include "protocol.h"
#include "../../stm32/stm32h7/hal/hal_uart.h"
#include "../../stm32/stm32h7/hal/hal_gpio.h"

#include <stdio.h>
#include <string.h>

static int checks_run = 0;
static int checks_failed = 0;
static const char *current_test = "";

#define CHECK(condition)                                                              \
    do {                                                                              \
        checks_run++;                                                                 \
        if (!(condition)) {                                                           \
            checks_failed++;                                                          \
            printf("FAIL %s (%s:%d): %s\n", current_test, __FILE__, __LINE__, #condition); \
        }                                                                             \
    } while (0)

/* ---------------------------------------------------------------------------
 * Fake clock and HAL
 * ------------------------------------------------------------------------- */

static uint32_t fake_now_ms = 0;

static uint32_t fake_time(void) {
    return fake_now_ms;
}

static hal_uart_rx_callback_t fake_rx_callback = NULL;
static void *fake_rx_context = NULL;
static int discover_frames_sent = 0;

bool hal_uart_init(hal_uart_handle_t *handle, const hal_uart_config_t *config) {
    (void)config;
    handle->initialized = true;
    return true;
}

void hal_uart_deinit(hal_uart_handle_t *handle) {
    handle->initialized = false;
}

bool hal_uart_transmit(hal_uart_handle_t *handle, const uint8_t *data, uint16_t length,
                       uint32_t timeout_ms) {
    (void)handle;
    (void)timeout_ms;
    static esocore_message_t sent;
    uint32_t consumed = 0;
    if (esocore_protocol_decode_frame(data, length, &sent, &consumed) == ESOCORE_FRAME_OK &&
        sent.header.message_type == ESOCORE_MSG_DISCOVER) {
        discover_frames_sent++;
    }
    return true;
}

void hal_uart_rs485_tx_enable(hal_uart_handle_t *handle) { (void)handle; }
void hal_uart_rs485_rx_enable(hal_uart_handle_t *handle) { (void)handle; }
void hal_uart_enable_rx_interrupt(hal_uart_handle_t *handle) { (void)handle; }

void hal_uart_register_rx_callback(hal_uart_handle_t *handle, hal_uart_rx_callback_t callback,
                                   void *context) {
    (void)handle;
    fake_rx_callback = callback;
    fake_rx_context = context;
}

void hal_gpio_init_output(uint16_t pin) { (void)pin; }
void hal_gpio_write(uint16_t pin, bool state) {
    (void)pin;
    (void)state;
}

/* ---------------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------------- */

/* Device callback record */
static int connects = 0;
static int disconnects = 0;
static uint8_t last_event_address = 0;

static void on_device(const sensor_bus_device_t *device, bool connected, void *ctx) {
    (void)ctx;
    last_event_address = device->address;
    if (connected) {
        connects++;
    } else {
        disconnects++;
    }
}

static int data_frames = 0;

static void on_data(uint8_t address, uint8_t sequence, uint8_t flags, const uint8_t *data,
                    uint16_t length, void *ctx) {
    (void)address;
    (void)sequence;
    (void)flags;
    (void)data;
    (void)length;
    (void)ctx;
    data_frames++;
}

/* Feed bytes as the UART RX interrupt would */
static void receive_bytes(const uint8_t *bytes, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        fake_rx_callback(bytes[i], fake_rx_context);
    }
}

/* Encode a frame from a sensor module to the Edge; returns its length */
static uint16_t sensor_frame(uint8_t source, uint8_t type, const uint8_t *payload, uint16_t length,
                             uint8_t *frame) {
    static esocore_message_t message;
    memset(&message, 0, sizeof(message));
    message.header.start_byte = ESOCORE_PROTOCOL_START_BYTE;
    message.header.source_address = source;
    message.header.destination_address = ESOCORE_PROTOCOL_MASTER_ADDRESS;
    message.header.message_type = type;
    message.header.payload_length = length;
    if (length > 0) {
        memcpy(message.payload, payload, length);
    }
    return esocore_protocol_encode_frame(&message, frame, ESOCORE_PROTOCOL_FRAME_SIZE);
}

static void receive_discovery_response(uint8_t address) {
    esocore_device_info_t info;
    memset(&info, 0, sizeof(info));
    info.device_type = ESOCORE_DEVICE_TYPE_VIBRATION;
    info.firmware_version_major = 1;
    info.serial_number = 0x1234;
    uint8_t payload[ESOCORE_PROTOCOL_MAX_PAYLOAD_SIZE];
    uint16_t payload_length = esocore_protocol_encode_device_info(&info, payload);
    uint8_t frame[ESOCORE_PROTOCOL_FRAME_SIZE];
    uint16_t length = sensor_frame(address, ESOCORE_MSG_DISCOVER_RESPONSE, payload, payload_length, frame);
    receive_bytes(frame, length);
}

static void receive_heartbeat(uint8_t address) {
    uint8_t frame[ESOCORE_PROTOCOL_FRAME_SIZE];
    uint16_t length = sensor_frame(address, ESOCORE_MSG_HEARTBEAT, NULL, 0, frame);
    receive_bytes(frame, length);
}

static sensor_bus_config_t config_with_clock(uint32_t start_ms) {
    fake_now_ms = start_ms;
    sensor_bus_config_t config;
    memset(&config, 0, sizeof(config));
    config.discovery_interval_ms = 5000;
    config.get_time_ms = fake_time;
    return config;
}

/* Initialise and start the bus at start_ms, with the record reset */
static void start_bus(uint32_t start_ms) {
    sensor_bus_config_t config = config_with_clock(start_ms);
    CHECK(sensor_bus_init(&config));
    CHECK(sensor_bus_register_callbacks(on_device, on_data, NULL));
    CHECK(sensor_bus_start());
    connects = 0;
    disconnects = 0;
    data_frames = 0;
    discover_frames_sent = 0;
    last_event_address = 0;
}

static sensor_bus_device_t device_status(uint8_t address) {
    static sensor_bus_status_t status;
    sensor_bus_device_t none;
    memset(&none, 0, sizeof(none));
    if (!sensor_bus_get_status(&status)) {
        return none;
    }
    for (uint8_t i = 0; i < status.sensor_count; i++) {
        if (status.sensors[i].address == address) {
            return status.sensors[i];
        }
    }
    return none;
}

/* Heartbeat timeout: SBUS_HEARTBEAT_TIMEOUT_MULT * 1000 ms */
#define TIMEOUT_MS 3000U

/* ---------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

static void test_init_needs_clock(void) {
    current_test = "init_needs_clock";
    sensor_bus_config_t config;
    memset(&config, 0, sizeof(config));
    CHECK(!sensor_bus_init(&config));
    config.get_time_ms = fake_time;
    CHECK(sensor_bus_init(&config));
    CHECK(sensor_bus_deinit());
}

static void test_last_seen_uses_clock(void) {
    current_test = "last_seen_uses_clock";
    start_bus(100000);
    receive_discovery_response(5);
    CHECK(sensor_bus_poll());
    CHECK(connects == 1);
    sensor_bus_device_t device = device_status(5);
    CHECK(device.address == 5);
    CHECK(device.online);
    CHECK(device.last_seen_ms == 100000);

    fake_now_ms = 101500;
    receive_heartbeat(5);
    sensor_bus_poll();
    CHECK(device_status(5).last_seen_ms == 101500);
    CHECK(sensor_bus_deinit());
}

static void test_heartbeat_timeout(void) {
    current_test = "heartbeat_timeout";
    start_bus(1000);
    receive_discovery_response(7);
    sensor_bus_poll();
    CHECK(connects == 1);

    /* Exactly at the timeout the device is still online */
    fake_now_ms = 1000 + TIMEOUT_MS;
    sensor_bus_poll();
    CHECK(disconnects == 0);
    CHECK(device_status(7).online);

    /* One millisecond later it is reported offline, once */
    fake_now_ms = 1000 + TIMEOUT_MS + 1;
    sensor_bus_poll();
    CHECK(disconnects == 1);
    CHECK(last_event_address == 7);
    CHECK(!device_status(7).online);
    fake_now_ms += 10000;
    sensor_bus_poll();
    CHECK(disconnects == 1);

    /* A heartbeat brings it back */
    receive_heartbeat(7);
    sensor_bus_poll();
    CHECK(connects == 2);
    CHECK(device_status(7).online);
    CHECK(sensor_bus_deinit());
}

static void test_heartbeats_keep_device_online(void) {
    current_test = "heartbeats_keep_device_online";
    start_bus(0);
    receive_discovery_response(9);
    sensor_bus_poll();
    for (int i = 0; i < 20; i++) {
        fake_now_ms += 1000;
        receive_heartbeat(9);
        sensor_bus_poll();
    }
    CHECK(disconnects == 0);
    CHECK(device_status(9).online);
    CHECK(sensor_bus_deinit());
}

static void test_timeout_across_clock_wrap(void) {
    current_test = "timeout_across_clock_wrap";
    start_bus(0xFFFFFF00U);
    receive_discovery_response(11);
    sensor_bus_poll();
    CHECK(connects == 1);

    fake_now_ms = 0xFFFFFF00U + 2000U; /* wrapped past zero, 2 s later */
    sensor_bus_poll();
    CHECK(disconnects == 0);

    fake_now_ms = 0xFFFFFF00U + TIMEOUT_MS + 1U;
    sensor_bus_poll();
    CHECK(disconnects == 1);
    CHECK(sensor_bus_deinit());
}

static void test_discovery_timing(void) {
    current_test = "discovery_timing";
    /* The first discovery is on the first poll, whatever the clock reads */
    start_bus(1234);
    sensor_bus_poll();
    CHECK(discover_frames_sent == 1);

    fake_now_ms = 1234 + 4999;
    sensor_bus_poll();
    CHECK(discover_frames_sent == 1);

    fake_now_ms = 1234 + 5000;
    sensor_bus_poll();
    CHECK(discover_frames_sent == 2);

    fake_now_ms = 1234 + 5000 + 5000;
    sensor_bus_poll();
    CHECK(discover_frames_sent == 3);

    /* A restart discovers on its first poll again */
    CHECK(sensor_bus_stop());
    CHECK(sensor_bus_start());
    fake_now_ms += 1;
    sensor_bus_poll();
    CHECK(discover_frames_sent == 4);
    CHECK(sensor_bus_deinit());
}

static void test_partial_frame_gap_timeout(void) {
    current_test = "partial_frame_gap_timeout";
    start_bus(0);
    sensor_bus_poll(); /* initial discovery */

    uint8_t frame[ESOCORE_PROTOCOL_FRAME_SIZE];
    const uint8_t payload[] = {1, 2, 3, 4};
    uint16_t length = sensor_frame(3, ESOCORE_MSG_DATA_RESPONSE, payload, sizeof(payload), frame);

    /* Half a frame, then silence longer than the gap timeout */
    receive_bytes(frame, 5);
    sensor_bus_poll();
    fake_now_ms += ESOCORE_PROTOCOL_FRAME_GAP_TIMEOUT_MS + 1;
    sensor_bus_poll(); /* nothing new arrived: the partial frame is abandoned */

    /* A complete frame afterwards is delivered on its own */
    receive_bytes(frame, length);
    sensor_bus_poll();
    CHECK(data_frames == 1);

    sensor_bus_status_t status;
    CHECK(sensor_bus_get_status(&status));
    CHECK(status.bus_errors > 0);
    CHECK(sensor_bus_deinit());
}

static void test_partial_frame_within_gap(void) {
    current_test = "partial_frame_within_gap";
    start_bus(0);
    sensor_bus_poll();

    uint8_t frame[ESOCORE_PROTOCOL_FRAME_SIZE];
    const uint8_t payload[] = {9, 8, 7};
    uint16_t length = sensor_frame(4, ESOCORE_MSG_DATA_RESPONSE, payload, sizeof(payload), frame);

    /* The rest arrives within the gap timeout: one frame, no error */
    receive_bytes(frame, 4);
    sensor_bus_poll();
    fake_now_ms += ESOCORE_PROTOCOL_FRAME_GAP_TIMEOUT_MS;
    sensor_bus_poll();
    receive_bytes(frame + 4, (uint32_t)length - 4);
    sensor_bus_poll();
    CHECK(data_frames == 1);

    sensor_bus_status_t status;
    CHECK(sensor_bus_get_status(&status));
    CHECK(status.bus_errors == 0);
    CHECK(sensor_bus_deinit());
}

int main(void) {
    test_init_needs_clock();
    test_last_seen_uses_clock();
    test_heartbeat_timeout();
    test_heartbeats_keep_device_online();
    test_timeout_across_clock_wrap();
    test_discovery_timing();
    test_partial_frame_gap_timeout();
    test_partial_frame_within_gap();

    printf("sensor bus tests: %d checks, %d failed\n", checks_run, checks_failed);
    return checks_failed == 0 ? 0 : 1;
}
