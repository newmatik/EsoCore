/**
 * @file main.c
 * @brief EsoCore Sensor Module Main Application
 *
 * This file contains the main application for EsoCore sensor modules, integrating
 * sensor data acquisition, RS-485 communication, and power management.
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Core System Includes */
#include "protocol.h"
#include "../../common/sensors/sensor_interface.h"
#include "power_management.h"
#include "event_system.h"

/* Demo Sensors */
#include "vibration_sensor.h"
#include "acoustic_sensor.h"
#include "current_sensor.h"
#include "air_quality_sensor.h"

/* ============================================================================
 * Configuration Macros
 * ============================================================================ */

/* Device Configuration */
#define SENSOR_DEVICE_ADDRESS       0x02    /* Default sensor address */
#define SENSOR_DEVICE_TYPE          ESOCORE_DEVICE_TYPE_VIBRATION /* Default type */

/* Communication Configuration */
#define HEARTBEAT_INTERVAL_MS       15000   /* 15 seconds */
#define DATA_TRANSMIT_INTERVAL_MS   2000    /* 2 seconds */
#define CONFIG_CHECK_INTERVAL_MS    60000   /* 1 minute */

/* Power Configuration */
#define LOW_POWER_THRESHOLD_MA      50      /* Enter low power below this current */

/* ============================================================================
 * Global Variables
 * ============================================================================ */

/* System state */
static bool system_initialized = false;
static bool system_running = false;
static uint32_t system_uptime_ms = 0;

/* Device configuration */
static uint8_t device_address = SENSOR_DEVICE_ADDRESS;
static esocore_device_type_t device_type = SENSOR_DEVICE_TYPE;

/* Timing variables */
static uint32_t last_heartbeat_time = 0;
static uint32_t last_data_transmit_time = 0;
static uint32_t last_config_check_time = 0;

/* System status */
static struct {
    bool master_connected;
    bool sensors_active;
    uint8_t active_sensors;
    uint32_t total_measurements;
    uint8_t system_health;
    bool low_power_mode;
} system_status;

/* Sensor data buffers */
static vibration_sensor_data_t vibration_data;
static acoustic_sensor_data_t acoustic_data;
static current_sensor_data_t current_data;
static air_quality_sensor_data_t air_quality_data;

/* ============================================================================
 * System Initialization Functions
 * ============================================================================ */

/**
 * @brief Initialize demo sensors based on device type
 */
static bool initialize_demo_sensors(void) {
    bool success = false;

    switch (device_type) {
        case ESOCORE_DEVICE_TYPE_VIBRATION:
            {
                vibration_sensor_config_t config = {
                    .base_config = {
                        .sensor_type = ESOCORE_SENSOR_VIBRATION,
                        .sensor_id = 1,
                        .sampling_rate_hz = 1000,
                        .sample_count = 1024,
                        .measurement_interval_ms = 1000
                    },
                    .accelerometer_type = 0,
                    .sensitivity_mg_per_lsb = 2.0f,
                    .measurement_range = 2,
                    .enable_high_pass_filter = true,
                    .high_pass_cutoff_hz = 10.0f,
                    .enable_low_pass_filter = true,
                    .low_pass_cutoff_hz = 500.0f,
                    .enable_temperature_compensation = true,
                    .fft_window_type = 0
                };

                if (vibration_sensor_init(&config) && vibration_sensor_start_acquisition()) {
                    success = true;
                    system_status.active_sensors = 1;
                }
            }
            break;

        case ESOCORE_DEVICE_TYPE_CURRENT:
            {
                current_sensor_config_t config = {
                    .base_config = {
                        .sensor_type = ESOCORE_SENSOR_CURRENT,
                        .sensor_id = 1,
                        .sampling_rate_hz = 5000,
                        .sample_count = 512,
                        .measurement_interval_ms = 1000
                    },
                    .sensor_type = 1,
                    .sensitivity_mv_a = 40.0f,
                    .nominal_current_a = 5.0f,
                    .enable_temperature_compensation = true,
                    .enable_harmonic_analysis = true,
                    .harmonic_order_max = 15,
                    .enable_power_analysis = true,
                    .fft_window_type = 0,
                    .filter_cutoff_hz = 1000.0f
                };

                if (current_sensor_init(&config) && current_sensor_start_acquisition()) {
                    success = true;
                    system_status.active_sensors = 1;
                }
            }
            break;

        default:
            // For other sensor types, just initialize sensor interface
            if (esocore_sensor_init()) {
                success = true;
            }
            break;
    }

    if (success) {
        system_status.sensors_active = true;
    }

    return success;
}

/* ============================================================================
 * Communication and Data Functions
 * ============================================================================ */

/**
 * @brief Send heartbeat to master device
 */
static void send_heartbeat(void) {
    uint32_t current_time = system_uptime_ms;

    if (current_time - last_heartbeat_time >= HEARTBEAT_INTERVAL_MS) {
        if (esocore_protocol_send_heartbeat()) {
            system_status.master_connected = true;
            esocore_event_log(ESOCORE_EVENT_NETWORK_CONNECTED, ESOCORE_EVENT_SEVERITY_DEBUG,
                             (uint8_t *)"Heartbeat sent to master", 0);
        } else {
            system_status.master_connected = false;
            esocore_event_log(ESOCORE_EVENT_NETWORK_ERROR, ESOCORE_EVENT_SEVERITY_WARNING,
                             (uint8_t *)"Heartbeat failed", 0);
        }

        last_heartbeat_time = current_time;
    }
}

/**
 * @brief Send sensor data to master device
 */
static void send_sensor_data(void) {
    uint32_t current_time = system_uptime_ms;

    if (current_time - last_data_transmit_time >= DATA_TRANSMIT_INTERVAL_MS) {
        if (system_status.sensors_active && system_status.master_connected) {
            bool data_sent = false;

            switch (device_type) {
                case ESOCORE_DEVICE_TYPE_VIBRATION:
                    if (vibration_sensor_read_data(&vibration_data, 500)) {
                        esocore_protocol_send_sensor_data((esocore_sensor_data_t *)&vibration_data, sizeof(vibration_data));
                        data_sent = true;
                        system_status.total_measurements++;
                    }
                    break;

                case ESOCORE_DEVICE_TYPE_CURRENT:
                    if (current_sensor_read_data(&current_data, 500)) {
                        esocore_protocol_send_sensor_data((esocore_sensor_data_t *)&current_data, sizeof(current_data));
                        data_sent = true;
                        system_status.total_measurements++;
                    }
                    break;

                default:
                    break;
            }

            if (data_sent) {
                esocore_event_log(ESOCORE_EVENT_SENSOR_DATA_READY, ESOCORE_EVENT_SEVERITY_DEBUG,
                                 (uint8_t *)"Sensor data sent to master", 0);
            }
        }

        last_data_transmit_time = current_time;
    }
}

/**
 * @brief Process incoming protocol messages
 */
static void process_protocol_messages(void) {
    esocore_message_t message;

    if (esocore_protocol_receive_message(&message, 50)) {
        esocore_protocol_handle_message(&message);
    }
}

/**
 * @brief Check and manage power consumption
 */
static void manage_power(void) {
    esocore_power_status_t power_status;

    if (esocore_power_get_status(&power_status)) {
        // Enter low power mode if current is too low
        if (power_status.total_current_ma < LOW_POWER_THRESHOLD_MA && !system_status.low_power_mode) {
            esocore_power_enter_low_power();
            system_status.low_power_mode = true;
            esocore_event_log(ESOCORE_EVENT_POWER_LOW_VOLTAGE, ESOCORE_EVENT_SEVERITY_INFO,
                             (uint8_t *)"Entered low power mode", 0);
        }
        // Exit low power mode if current is sufficient
        else if (power_status.total_current_ma >= LOW_POWER_THRESHOLD_MA * 1.2f && system_status.low_power_mode) {
            esocore_power_exit_low_power();
            system_status.low_power_mode = false;
            esocore_event_log(ESOCORE_EVENT_POWER_SOURCE_CHANGE, ESOCORE_EVENT_SEVERITY_INFO,
                             (uint8_t *)"Exited low power mode", 0);
        }

        // Check for power faults
        if (power_status.fault_flags != 0) {
            esocore_event_log(ESOCORE_EVENT_POWER_FAULT, ESOCORE_EVENT_SEVERITY_ERROR,
                             (uint8_t *)"Power fault detected", 0);
        }
    }
}

/**
 * @brief Update system health status
 */
static void update_system_health(void) {
    uint8_t health_score = 100;

    // Check master connectivity
    if (!system_status.master_connected) {
        health_score -= 30;
    }

    // Check sensor status
    if (!system_status.sensors_active) {
        health_score -= 25;
    }

    // Check power status
    esocore_power_status_t power_status;
    if (esocore_power_get_status(&power_status)) {
        if (power_status.fault_flags != 0) {
            health_score -= 20;
        }
    }

    system_status.system_health = health_score;
}

/* ============================================================================
 * Main System Functions
 * ============================================================================ */

/**
 * @brief Initialize all system components
 */
static bool initialize_system(void) {
    printf("EsoCore Sensor Module Initializing...\n");
    printf("Device Type: %d, Address: 0x%02X\n", device_type, device_address);

    // Initialize core systems
    esocore_event_config_t event_config = {
        .enable_buffering = true,
        .max_buffer_size = 32,
        .flush_interval_ms = 30000,
        .max_subscribers = 1,
        .enable_remote_logging = false
    };

    if (!esocore_event_init(&event_config)) {
        printf("ERROR: Event system initialization failed\n");
        return false;
    }
    printf("✓ Event system initialized\n");

    if (!esocore_power_init()) {
        printf("ERROR: Power management initialization failed\n");
        return false;
    }
    printf("✓ Power management initialized\n");

    if (!esocore_protocol_init(device_address, device_type)) {
        printf("ERROR: Protocol initialization failed\n");
        return false;
    }
    printf("✓ Protocol initialized\n");

    if (!esocore_sensor_init()) {
        printf("ERROR: Sensor interface initialization failed\n");
        return false;
    }
    printf("✓ Sensor interface initialized\n");

    if (!initialize_demo_sensors()) {
        printf("ERROR: Sensor initialization failed\n");
        return false;
    }
    printf("✓ Sensors initialized\n");

    system_initialized = true;
    esocore_event_log(ESOCORE_EVENT_SYSTEM_STARTUP, ESOCORE_EVENT_SEVERITY_INFO,
                     (uint8_t *)"EsoCore sensor module startup completed", 0);

    printf("EsoCore Sensor Module initialization completed successfully!\n");
    return true;
}

/**
 * @brief Main system runtime loop
 */
static void system_runtime_loop(void) {
    printf("EsoCore Sensor Module entering runtime mode...\n");

    system_running = true;

    while (system_running) {
        // Update system uptime
        system_uptime_ms += 50;  // Simulate 50ms tick

        // Core communication functions
        send_heartbeat();
        send_sensor_data();

        // System maintenance
        process_protocol_messages();
        manage_power();
        update_system_health();

        // Small delay to prevent busy waiting
        for (volatile int i = 0; i < 50000; i++);
    }
}

/**
 * @brief System shutdown sequence
 */
static void system_shutdown(void) {
    printf("EsoCore Sensor Module shutting down...\n");

    // Stop sensors
    if (system_status.sensors_active) {
        switch (device_type) {
            case ESOCORE_DEVICE_TYPE_VIBRATION:
                vibration_sensor_stop_acquisition();
                vibration_sensor_deinit();
                break;
            case ESOCORE_DEVICE_TYPE_CURRENT:
                current_sensor_stop_acquisition();
                current_sensor_deinit();
                break;
            default:
                break;
        }
    }

    // Shutdown systems
    esocore_event_log(ESOCORE_EVENT_SYSTEM_SHUTDOWN, ESOCORE_EVENT_SEVERITY_INFO,
                     (uint8_t *)"EsoCore sensor module shutting down", 0);

    esocore_protocol_deinit();
    esocore_power_deinit();
    esocore_event_deinit();

    printf("EsoCore Sensor Module shutdown complete\n");
}

/**
 * @brief Set device type and address
 */
void sensor_set_device_config(uint8_t address, esocore_device_type_t type) {
    device_address = address;
    device_type = type;
}

/**
 * @brief Main application entry point
 */
int main(void) {
    printf("=== EsoCore Sensor Module ===\n");
    printf("Industrial IoT Sensor Node\n");
    printf("Version 1.0.0 - Newmatik 2025\n\n");

    // Initialize all system components
    if (!initialize_system()) {
        printf("CRITICAL: System initialization failed!\n");
        return -1;
    }

    // Enter main runtime loop
    system_runtime_loop();

    // Shutdown system
    system_shutdown();

    return 0;
}
