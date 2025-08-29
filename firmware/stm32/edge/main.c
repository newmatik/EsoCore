/**
 * @file main.c
 * @brief EsoCore Edge Device Main Application
 *
 * This file contains the main application for the EsoCore Edge device, integrating
 * all system components including WiFi, HTTP, Safety I/O, Storage, TinyML,
 * Configuration Management, and Event Logging.
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
#include "wifi_manager.h"
#include "http_client.h"
#include "safety_io.h"
#include "storage_system.h"
#include "tinyml_engine.h"
#include "config_manager.h"
#include "event_system.h"
#include "power_management.h"
#include "oled_display.h"
#include "protocol.h"
#include "../../common/sensors/sensor_interface.h"

/* Vibration Sensor for demonstration */
#include "vibration_sensor.h"

/* ============================================================================
 * Configuration Macros
 * ============================================================================ */

/* WiFi Configuration */
#define WIFI_SSID                       "EsoCore_Network"
#define WIFI_PASSWORD                   "esocore2025"
#define WIFI_SECURITY_TYPE              WIFI_SECURITY_WPA2

/* Server Configuration */
#define SERVER_URL                      "https://api.esocore.com"
#define SERVER_PORT                     443
#define API_KEY                         "esocore_edge_device_key_2025"

/* Device Configuration */
#define DEVICE_ADDRESS                  0x01
#define DEVICE_TYPE                     ESOCORE_DEVICE_TYPE_MASTER

/* Timing Configuration */
#define HEARTBEAT_INTERVAL_MS           30000   /* 30 seconds */
#define TELEMETRY_INTERVAL_MS           5000    /* 5 seconds */
#define SENSOR_READ_INTERVAL_MS         1000    /* 1 second */
#define DISPLAY_UPDATE_INTERVAL_MS      2000    /* 2 seconds */
#define OTA_CHECK_INTERVAL_MS           3600000 /* 1 hour */
/* ============================================================================
 * Global Variables
 * ============================================================================ */

/* System state */
static bool system_initialized = false;
static bool system_running = false;
static uint32_t system_uptime_ms = 0;

/* Timing variables */
static uint32_t last_heartbeat_time = 0;
static uint32_t last_telemetry_time = 0;
static uint32_t last_sensor_read_time = 0;
static uint32_t last_display_update_time = 0;
static uint32_t last_ota_check_time = 0;

/* System status */
static struct {
    bool wifi_connected;
    bool server_reachable;
    bool sensors_active;
    bool storage_available;
    uint8_t active_sensors;
    uint32_t total_measurements;
    uint8_t system_health;
} system_status;

/* Demo sensor data */
static vibration_sensor_data_t vibration_data;

/* ============================================================================
 * System Initialization Functions
 * ============================================================================ */

/* ============================================================================
 * System Initialization Functions
 * ============================================================================ */

/**
 * @brief Initialize WiFi connectivity
 */
static bool initialize_wifi(void) {
    wifi_config_t wifi_config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASSWORD,
        .security = WIFI_SECURITY_TYPE,
        .mode = WIFI_MODE_STATION,
        .dhcp_enabled = true,
        .max_retries = 5
    };

    if (!wifi_manager_init(&wifi_config)) {
        esocore_event_log(ESOCORE_EVENT_NETWORK_ERROR, ESOCORE_EVENT_SEVERITY_ERROR,
                         (uint8_t *)"WiFi initialization failed", 0);
        return false;
    }

    if (!wifi_manager_connect()) {
        esocore_event_log(ESOCORE_EVENT_NETWORK_DISCONNECTED, ESOCORE_EVENT_SEVERITY_WARNING,
                         (uint8_t *)"WiFi connection failed", 0);
        return false;
    }

    system_status.wifi_connected = true;
    esocore_event_log(ESOCORE_EVENT_NETWORK_CONNECTED, ESOCORE_EVENT_SEVERITY_INFO,
                     (uint8_t *)"WiFi connected successfully", 0);

    return true;
}

/**
 * @brief Initialize HTTP client
 */
static bool initialize_http_client(void) {
    http_client_config_t http_config = {
        .server_url = SERVER_URL,
        .server_port = SERVER_PORT,
        .api_key = API_KEY,
        .use_https = true,
        .response_timeout_ms = 10000,
        .max_retries = 3
    };

    if (!http_client_init(&http_config)) {
        esocore_event_log(ESOCORE_EVENT_NETWORK_ERROR, ESOCORE_EVENT_SEVERITY_ERROR,
                         (uint8_t *)"HTTP client initialization failed", 0);
        return false;
    }

    return true;
}

/**
 * @brief Initialize demo vibration sensor
 */
static bool initialize_demo_sensor(void) {
    vibration_sensor_config_t vib_config = {
        .base_config = {
            .sensor_type = ESOCORE_SENSOR_VIBRATION,
            .sensor_id = 1,
            .sampling_rate_hz = 1000,
            .sample_count = 1024,
            .measurement_interval_ms = 1000
        },
        .accelerometer_type = 0,  /* ADXL355 */
        .sensitivity_mg_per_lsb = 2.0f,
        .measurement_range = 2,   /* ±2g */
        .enable_high_pass_filter = true,
        .high_pass_cutoff_hz = 10.0f,
        .enable_low_pass_filter = true,
        .low_pass_cutoff_hz = 500.0f,
        .enable_temperature_compensation = true,
        .fft_window_type = 0      /* Hanning */
    };

    if (!vibration_sensor_init(&vib_config)) {
        esocore_event_log(ESOCORE_EVENT_SENSOR_ERROR, ESOCORE_EVENT_SEVERITY_WARNING,
                         (uint8_t *)"Vibration sensor initialization failed", 0);
        return false;
    }

    if (!vibration_sensor_start_acquisition()) {
        esocore_event_log(ESOCORE_EVENT_SENSOR_ERROR, ESOCORE_EVENT_SEVERITY_WARNING,
                         (uint8_t *)"Vibration sensor acquisition start failed", 0);
        return false;
    }

    system_status.sensors_active = true;
    system_status.active_sensors = 1;

    return true;
}

/* ============================================================================
 * System Runtime Functions
 * ============================================================================ */

/**
 * @brief Send heartbeat to server
 */
static void send_heartbeat(void) {
    uint32_t current_time = system_uptime_ms;

    if (current_time - last_heartbeat_time >= HEARTBEAT_INTERVAL_MS) {
        http_response_t response;

        if (http_client_post("/api/heartbeat", (uint8_t *)"{\"status\":\"active\"}", 20,
                           "application/json", &response)) {
            if (response.status_code == 200) {
                system_status.server_reachable = true;
                esocore_event_log(ESOCORE_EVENT_NETWORK_CONNECTED, ESOCORE_EVENT_SEVERITY_DEBUG,
                                 (uint8_t *)"Heartbeat sent successfully", 0);
            } else {
                system_status.server_reachable = false;
                esocore_event_log(ESOCORE_EVENT_NETWORK_ERROR, ESOCORE_EVENT_SEVERITY_WARNING,
                                 (uint8_t *)"Heartbeat failed", 0);
            }
        }

        last_heartbeat_time = current_time;
    }
}

/**
 * @brief Send telemetry data to server
 */
static void send_telemetry(void) {
    uint32_t current_time = system_uptime_ms;

    if (current_time - last_telemetry_time >= TELEMETRY_INTERVAL_MS) {
        char telemetry_json[512];
        int len = snprintf(telemetry_json, sizeof(telemetry_json),
                          "{\"timestamp\":%lu,\"uptime\":%lu,\"sensors\":%d,\"measurements\":%lu,\"health\":%d}",
                          current_time, system_uptime_ms / 1000, system_status.active_sensors,
                          system_status.total_measurements, system_status.system_health);

        if (len > 0 && len < sizeof(telemetry_json)) {
            http_response_t response;
            if (http_client_post("/api/telemetry", (uint8_t *)telemetry_json, len,
                               "application/json", &response)) {
                if (response.status_code == 200) {
                    esocore_event_log(ESOCORE_EVENT_SYSTEM_STARTUP, ESOCORE_EVENT_SEVERITY_DEBUG,
                                     (uint8_t *)"Telemetry sent successfully", 0);
                }
            }
        }

        last_telemetry_time = current_time;
    }
}

/**
 * @brief Read sensor data
 */
static void read_sensor_data(void) {
    uint32_t current_time = system_uptime_ms;

    if (current_time - last_sensor_read_time >= SENSOR_READ_INTERVAL_MS) {
        if (system_status.sensors_active) {
            if (vibration_sensor_read_data(&vibration_data, 1000)) {
                system_status.total_measurements++;

                // Store data in storage system
                storage_write_record((storage_data_record_t *)&vibration_data);

                // Log significant events
                if (vibration_data.overall_condition < 50) {
                    esocore_event_log(ESOCORE_EVENT_SENSOR_ERROR, ESOCORE_EVENT_SEVERITY_WARNING,
                                     (uint8_t *)"Poor equipment condition detected", 0);
                }

                esocore_event_log(ESOCORE_EVENT_SENSOR_DATA_READY, ESOCORE_EVENT_SEVERITY_DEBUG,
                                 (uint8_t *)"Sensor data acquired", 0);
            }
        }

        last_sensor_read_time = current_time;
    }
}

/**
 * @brief Update OLED display
 */
static void update_display(void) {
    uint32_t current_time = system_uptime_ms;

    if (current_time - last_display_update_time >= DISPLAY_UPDATE_INTERVAL_MS) {
        char wifi_status[20] = "Disconnected";
        if (system_status.wifi_connected) {
            strcpy(wifi_status, "Connected");
        }

        char server_status[20] = "Offline";
        if (system_status.server_reachable) {
            strcpy(server_status, "Online");
        }

        oled_display_status_screen("EsoCore Edge",
                                  system_running ? "Running" : "Initializing",
                                  wifi_status,
                                  system_uptime_ms / 1000);

        last_display_update_time = current_time;
    }
}

/**
 * @brief Check for OTA updates
 */
static void check_ota_updates(void) {
    uint32_t current_time = system_uptime_ms;

    if (current_time - last_ota_check_time >= OTA_CHECK_INTERVAL_MS) {
        http_response_t response;

        if (http_client_get("/api/firmware/check?version=1.0.0", &response)) {
            if (response.status_code == 200 && response.body) {
                // Parse update information
                esocore_event_log(ESOCORE_EVENT_NETWORK_OTA, ESOCORE_EVENT_SEVERITY_INFO,
                                 (uint8_t *)"OTA update check completed", 0);
            }
        }

        last_ota_check_time = current_time;
    }
}

/**
 * @brief Handle system events and maintenance
 */
static void handle_system_events(void) {
    // Process incoming protocol messages
    esocore_message_t message;
    if (esocore_protocol_receive_message(&message, 10)) {
        esocore_protocol_handle_message(&message);
    }

    // Process safety system
    safety_system_status_t safety_status;
    if (safety_io_get_system_status(&safety_status)) {
        if (safety_status.fault_code != 0) {
            esocore_event_log(ESOCORE_EVENT_SAFETY_FAULT, ESOCORE_EVENT_SEVERITY_CRITICAL,
                             (uint8_t *)"Safety system fault detected", 0);
        }
    }

    // Process power management
    esocore_power_status_t power_status;
    if (esocore_power_get_status(&power_status)) {
        if (power_status.fault_flags != 0) {
            esocore_event_log(ESOCORE_EVENT_POWER_FAULT, ESOCORE_EVENT_SEVERITY_ERROR,
                             (uint8_t *)"Power system fault detected", 0);
        }
    }

    // Flush event buffer periodically
    esocore_event_flush_buffer();
}

/* ============================================================================
 * Main System Functions
 * ============================================================================ */

/**
 * @brief Initialize all system components
 */
static bool initialize_system(void) {
    printf("EsoCore Edge Device Initializing...\n");

    // Initialize core systems in order of dependency
    if (!esocore_event_init(NULL)) {
        printf("ERROR: Event system initialization failed\n");
        return false;
    }
    printf("✓ Event system initialized\n");

    if (!esocore_power_init()) {
        printf("ERROR: Power management initialization failed\n");
        return false;
    }
    printf("✓ Power management initialized\n");

    if (!esocore_config_init()) {
        printf("ERROR: Configuration manager initialization failed\n");
        return false;
    }
    printf("✓ Configuration manager initialized\n");

    if (!initialize_wifi()) {
        printf("ERROR: WiFi initialization failed\n");
        return false;
    }
    printf("✓ WiFi initialized\n");

    if (!initialize_http_client()) {
        printf("ERROR: HTTP client initialization failed\n");
        return false;
    }
    printf("✓ HTTP client initialized\n");

    if (!esocore_protocol_init(DEVICE_ADDRESS, DEVICE_TYPE)) {
        printf("ERROR: Protocol initialization failed\n");
        return false;
    }
    printf("✓ Protocol initialized\n");

    if (!esocore_sensor_init()) {
        printf("ERROR: Sensor interface initialization failed\n");
        return false;
    }
    printf("✓ Sensor interface initialized\n");

    if (!initialize_demo_sensor()) {
        printf("WARNING: Demo sensor initialization failed - continuing without sensors\n");
    } else {
        printf("✓ Demo sensor initialized\n");
    }

    system_initialized = true;
    esocore_event_log(ESOCORE_EVENT_SYSTEM_STARTUP, ESOCORE_EVENT_SEVERITY_INFO,
                     (uint8_t *)"EsoCore Edge device startup completed", 0);

    printf("EsoCore Edge Device initialization completed successfully!\n");
    return true;
}

/**
 * @brief Main system runtime loop
 */
static void system_runtime_loop(void) {
    printf("EsoCore Edge Device entering runtime mode...\n");

    system_running = true;

    while (system_running) {
        // Update system uptime
        system_uptime_ms += 100;  // Simulate 100ms tick

        // Core system functions
        send_heartbeat();
        send_telemetry();
        read_sensor_data();
        update_display();
        check_ota_updates();

        // System maintenance
        handle_system_events();

        // Small delay to prevent busy waiting
        // In real implementation, this would be handled by RTOS
        for (volatile int i = 0; i < 100000; i++);
    }
}

/**
 * @brief System shutdown sequence
 */
static void system_shutdown(void) {
    printf("EsoCore Edge Device shutting down...\n");

    // Stop sensors
    if (system_status.sensors_active) {
        vibration_sensor_stop_acquisition();
        vibration_sensor_deinit();
    }

    // Shutdown systems in reverse order
    esocore_event_log(ESOCORE_EVENT_SYSTEM_SHUTDOWN, ESOCORE_EVENT_SEVERITY_INFO,
                     (uint8_t *)"EsoCore Edge device shutting down", 0);

    tinyml_engine_deinit();
    sensor_interface_deinit();
    esocore_protocol_deinit();
    oled_display_deinit();
    esocore_power_deinit();
    esocore_config_deinit();
    storage_system_deinit();
    safety_io_deinit();
    http_client_deinit();
    wifi_manager_deinit();
    esocore_event_deinit();

    printf("EsoCore Edge Device shutdown complete\n");
}

/**
 * @brief Main application entry point
 */
int main(void) {
    printf("=== EsoCore Edge Device ===\n");
    printf("Industrial IoT Edge Computing Platform\n");
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

/* ============================================================================
 * Interrupt Handlers and System Hooks (Placeholder)
 * ============================================================================ */

/**
 * @brief System tick handler
 */
void SysTick_Handler(void) {
    // Update system timing
    system_uptime_ms++;
}

/**
 * @brief Hard fault handler
 */
void HardFault_Handler(void) {
    esocore_event_log(ESOCORE_EVENT_SYSTEM_RESET, ESOCORE_EVENT_SEVERITY_CRITICAL,
                     (uint8_t *)"Hard fault occurred", 0);
    system_running = false;
}

/**
 * @brief Watchdog handler
 */
void WWDG_IRQHandler(void) {
    esocore_event_log(ESOCORE_EVENT_SYSTEM_WATCHDOG, ESOCORE_EVENT_SEVERITY_CRITICAL,
                     (uint8_t *)"Watchdog timeout", 0);
    // Reset system
    NVIC_SystemReset();
}

    /* Main application loop */
    while (1) {
        /* Process sensor discovery */
        sensor_discovery_process();

        /* Collect data from sensors */
        data_collection_process();

        /* Handle safety I/O */
        safety_io_process();

        /* Manage WiFi connectivity */
        wifi_management_process();

        /* Handle API communication */
        api_communication_process();

        /* Check for OTA updates */
        ota_update_process();

        /* Transmit data over network */
        network_transmit_process();

        /* Monitor power systems */
        power_management_process();

        /* Small delay to prevent busy-waiting */
        HAL_Delay(10);
    }
}

/* ============================================================================
 * Initialization Functions
 * ============================================================================ */

static void edge_init(void) {
    /* Initialize protocol layer */
    if (!esocore_protocol_init(EDGE_DEVICE_ADDRESS, DEVICE_TYPE_EDGE)) {
        printf("Failed to initialize protocol layer\r\n");
        Error_Handler();
    }

    /* Initialize WiFi connectivity */
    wifi_init();

    /* Initialize API client */
    api_init();

    /* Initialize power management */
    esocore_power_thresholds_t power_thresholds = {
        .voltage_high_mv = 25000,    /* 25V */
        .voltage_low_mv = 10000,     /* 10V */
        .current_limit_ma = 2000,    /* 2A */
        .temperature_high_c = 70,    /* 70°C */
        .brownout_timeout_ms = 100,  /* 100ms */
        .shutdown_timeout_ms = 30000 /* 30 seconds */
    };

    if (!esocore_power_init(&power_thresholds)) {
        printf("Failed to initialize power management\r\n");
        Error_Handler();
    }

    /* Initialize PoE if available */
    if (!esocore_poe_init()) {
        printf("PoE not available, using DC input\r\n");
    }

    /* Initialize supercapacitor backup */
    if (!esocore_supercap_init(10.0f)) { /* 10F capacitor bank */
        printf("Failed to initialize supercapacitor backup\r\n");
    }

    /* Configure power source switching */
    esocore_power_configure_sources(POWER_SOURCE_POE, POWER_SOURCE_DC_INPUT, true);

    printf("EsoCore Edge initialization complete\r\n");
}

static void wifi_init(void) {
    /* Configure WiFi */
    wifi_config_t wifi_config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASSWORD,
        .security = WIFI_SECURITY,
        .mode = WIFI_MODE_STATION,
        .dhcp_enabled = true,
        .connection_timeout_ms = 30000,
        .max_retries = 3,
        .auto_reconnect = true,
        .reconnect_interval_ms = 5000
    };

    if (!wifi_manager_init(&wifi_config)) {
        printf("Failed to initialize WiFi manager\r\n");
        Error_Handler();
    }

    /* Attempt initial WiFi connection */
    if (!wifi_manager_connect()) {
        printf("Initial WiFi connection failed, will retry in background\r\n");
    }

    printf("WiFi manager initialized\r\n");
}

static void api_init(void) {
    /* Configure HTTP client */
    http_client_config_t http_config = {
        .server_url = API_SERVER_URL,
        .server_port = API_SERVER_PORT,
        .use_https = API_USE_HTTPS,
        .api_key = API_DEVICE_KEY,
        .connection_timeout_ms = 30000,
        .response_timeout_ms = 30000,
        .max_retries = 3,
        .retry_delay_ms = 5000,
        .keep_alive = true,
        .buffer_size = 2048
    };

    if (!http_client_init(&http_config)) {
        printf("Failed to initialize HTTP client\r\n");
        Error_Handler();
    }

    printf("HTTP client initialized\r\n");
}

/* ============================================================================
 * Process Functions
 * ============================================================================ */

static void sensor_discovery_process(void) {
    static uint32_t last_discovery = 0;
    uint32_t current_time = HAL_GetTick();

    /* Perform sensor discovery every 30 seconds */
    if (current_time - last_discovery >= 30000) {
        printf("Starting sensor discovery...\r\n");

        /* Send discovery request to all sensors */
        uint8_t discovery_payload[ESOCORE_MAX_PAYLOAD_SIZE];
        uint16_t payload_length = esocore_create_discovery_request(discovery_payload,
                                                                 sizeof(discovery_payload));

        if (payload_length > 0) {
            esocore_send_message(0xFF, MSG_TYPE_DISCOVERY_REQUEST,
                               discovery_payload, payload_length);
        }

        last_discovery = current_time;

        /* Reset discovered sensor count for new discovery cycle */
        num_discovered_sensors = 0;
    }

    /* Process discovery responses */
    uint8_t source_address;
    esocore_msg_type_t msg_type;
    uint8_t response_payload[ESOCORE_MAX_PAYLOAD_SIZE];

    int received_length = esocore_receive_message(&source_address, &msg_type,
                                                response_payload, sizeof(response_payload), 100);

    if (received_length > 0 && msg_type == MSG_TYPE_DISCOVERY_RESPONSE) {
        esocore_device_info_t device_info;

        if (esocore_parse_device_info(response_payload, received_length, &device_info)) {
            /* Check if sensor already discovered */
            bool already_discovered = false;
            for (uint8_t i = 0; i < num_discovered_sensors; i++) {
                if (discovered_sensors[i].device_type == device_info.device_type) {
                    already_discovered = true;
                    break;
                }
            }

            if (!already_discovered && num_discovered_sensors < MAX_DISCOVERED_SENSORS) {
                memcpy(&discovered_sensors[num_discovered_sensors++], &device_info,
                       sizeof(esocore_device_info_t));

                printf("Discovered sensor: %s (Type: 0x%02X, Address: 0x%02X)\r\n",
                       device_info.device_name, device_info.device_type, source_address);
            }
        }
    }
}

static void data_collection_process(void) {
    uint32_t current_time = HAL_GetTick();

    /* Collect data from all discovered sensors */
    for (uint8_t i = 0; i < num_discovered_sensors; i++) {
        esocore_device_info_t *sensor = &discovered_sensors[i];

        /* Request data from sensor */
        esocore_send_message(i + 1, MSG_TYPE_DATA_REQUEST, NULL, 0);

        /* Wait for response */
        uint8_t source_address;
        esocore_msg_type_t msg_type;
        uint8_t data_payload[ESOCORE_MAX_PAYLOAD_SIZE];

        int received_length = esocore_receive_message(&source_address, &msg_type,
                                                    data_payload, sizeof(data_payload), 500);

        if (received_length > 0 && msg_type == MSG_TYPE_DATA_RESPONSE) {
            /* Process sensor data */
            printf("Received data from sensor %s (%d bytes)\r\n",
                   sensor->device_name, received_length);

            /* Here you would parse the sensor data and store/forward it */
            /* Implementation depends on specific sensor data format */
        }
    }
}

static void safety_io_process(void) {
    /* Monitor safety inputs and control safety outputs */
    /* This is a placeholder for safety I/O processing */

    /* Check emergency stop inputs */
    /* Check safety gate sensors */
    /* Control safety relay outputs */

    /* If safety violation detected, trigger appropriate response */
}

static void network_transmit_process(void) {
    uint32_t current_time = HAL_GetTick();

    /* Transmit aggregated data every second */
    if (current_time - last_data_transmit >= DATA_TRANSMIT_INTERVAL_MS) {
        /* Prepare aggregated sensor data for transmission */
        /* Send data over Ethernet/WiFi */

        printf("Transmitting sensor data over network\r\n");
        last_data_transmit = current_time;
    }
}

static void power_management_process(void) {
    static uint32_t last_power_check = 0;
    uint32_t current_time = HAL_GetTick();

    /* Check power status every 5 seconds */
    if (current_time - last_power_check >= 5000) {
        esocore_power_monitoring_t power_data;

        if (esocore_power_get_monitoring_data(&power_data)) {
            printf("Power Status - Input: %d mV, Current: %d mA, Source: %d\r\n",
                   power_data.input_voltage_mv, power_data.input_current_ma,
                   power_data.active_source);

            /* Check for power faults */
            uint16_t faults = esocore_power_check_faults();
            if (faults != 0) {
                printf("Power faults detected: 0x%04X\r\n", faults);

                /* Handle power faults */
                if (faults & 0x0001) { /* Low voltage */
                    printf("Low voltage detected - switching to backup power\r\n");
                }
            }
        }

        last_power_check = current_time;
    }
}

static void wifi_management_process(void) {
    /* Manage WiFi connectivity */
    static uint32_t last_wifi_check = 0;
    uint32_t current_time = HAL_GetTick();

    /* Check WiFi status every 10 seconds */
    if (current_time - last_wifi_check >= 10000) {
        if (wifi_manager_get_status(&wifi_status)) {
            bool currently_connected = (wifi_status.status == WIFI_STATUS_CONNECTED);

            if (currently_connected != wifi_connected) {
                wifi_connected = currently_connected;

                if (wifi_connected) {
                    printf("WiFi connected: %s (%s)\r\n",
                           wifi_status.connected_ssid, wifi_status.ip_address);
                } else {
                    printf("WiFi disconnected\r\n");
                }
            }

            /* Log connection quality */
            if (wifi_connected) {
                printf("WiFi RSSI: %d dBm\r\n", wifi_status.rssi);
            }
        }

        last_wifi_check = current_time;
    }
}

static void api_communication_process(void) {
    /* Handle API communication when WiFi is connected */
    if (!wifi_connected) {
        return;
    }

    uint32_t current_time = HAL_GetTick();

    /* Send heartbeat every 60 seconds */
    if (current_time - last_heartbeat >= 60000) {
        if (send_heartbeat()) {
            printf("Heartbeat sent successfully\r\n");
        } else {
            printf("Failed to send heartbeat\r\n");
        }
        last_heartbeat = current_time;
    }

    /* Upload telemetry data every 30 seconds if buffer has data */
    if (current_time - last_data_transmit >= 30000 && telemetry_buffer_index > 0) {
        if (upload_telemetry_data()) {
            printf("Telemetry data uploaded (%d bytes)\r\n", telemetry_buffer_index);
            telemetry_buffer_index = 0; /* Clear buffer after successful upload */
        } else {
            printf("Failed to upload telemetry data\r\n");
        }
    }
}

static void ota_update_process(void) {
    /* Check for OTA updates */
    uint32_t current_time = HAL_GetTick();

    /* Check for updates every 24 hours */
    if (current_time - last_ota_check >= 86400000) { /* 24 hours in ms */
        if (wifi_connected && check_for_updates()) {
            printf("OTA update check completed\r\n");
        }
        last_ota_check = current_time;
    }
}

static bool send_auth_handshake(void) {
    /* Send authentication handshake to server */
    char device_id[32];
    sprintf(device_id, "%s_%08X", DEVICE_MODEL, HAL_GetUIDw0());

    http_response_t response;
    if (esocore_api_auth_handshake(device_id, &response)) {
        printf("Authentication successful\r\n");
        return true;
    } else {
        printf("Authentication failed: %d\r\n", response.status_code);
        return false;
    }
}

static bool upload_telemetry_data(void) {
    /* Upload telemetry data to server */
    http_response_t response;
    if (esocore_api_send_telemetry(telemetry_buffer, telemetry_buffer_index, &response)) {
        return true;
    } else {
        printf("Telemetry upload failed: %d\r\n", response.status_code);
        return false;
    }
}

static bool send_heartbeat(void) {
    /* Send heartbeat with device status */
    char status[128];
    sprintf(status, "model=%s,version=%s,sensors=%d,wifi=%s",
            DEVICE_MODEL, FIRMWARE_VERSION, num_discovered_sensors,
            wifi_connected ? "connected" : "disconnected");

    http_response_t response;
    if (esocore_api_send_heartbeat(status, &response)) {
        return true;
    } else {
        printf("Heartbeat failed: %d\r\n", response.status_code);
        return false;
    }
}

static bool check_for_updates(void) {
    /* Check for firmware updates */
    http_response_t response;
    if (esocore_api_check_ota(FIRMWARE_VERSION, &response)) {
        if (response.status_code == HTTP_STATUS_OK) {
            printf("Firmware is up to date\r\n");
        } else if (response.status_code == HTTP_STATUS_NO_CONTENT) {
            printf("Firmware update available!\r\n");
            /* Here you would initiate download and update process */
        }
        return true;
    } else {
        printf("OTA check failed: %d\r\n", response.status_code);
        return false;
    }
}

/* ============================================================================
 * STM32 HAL Configuration Functions
 * ============================================================================ */

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* Configure GPIO pins for RS-485 communication */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;  /* PA2: TX, PA3: RX */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure RS-485 direction control pin */
    GPIO_InitStruct.Pin = GPIO_PIN_1;  /* PA1: Direction control */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Configure safety I/O pins */
    /* Add safety input/output pin configurations here */
}

void MX_USART2_UART_Init(void) {
    /* USART2 configuration for RS-485 communication */
    /* Add UART initialization code here */
}

void MX_ETH_Init(void) {
    /* Ethernet configuration */
    /* Add Ethernet initialization code here */
}

void MX_RTC_Init(void) {
    /* Real-time clock configuration */
    /* Add RTC initialization code here */
}

void MX_TIM2_Init(void) {
    /* Timer configuration for periodic tasks */
    /* Add timer initialization code here */
}

void Error_Handler(void) {
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    /* User can add his own implementation to report the file name and line number */
}
#endif /* USE_FULL_ASSERT */
