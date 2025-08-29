/**
 * @file wifi_manager.h
 * @brief WiFi Manager for EsoCore Edge Device (ESP32-WROOM-32)
 *
 * This file provides WiFi connectivity management for the EsoCore Edge device,
 * using ESP32-WROOM-32 module with UART AT command interface.
 *
 * Features:
 * - ESP32 AT command protocol implementation
 * - WiFi connection management (802.11b/g/n)
 * - Bluetooth 5.0 support
 * - Network scanning and selection
 * - DHCP client functionality
 * - Static IP configuration
 * - Connection status monitoring
 * - Security (WPA2/3) support
 * - Power management modes
 * - Fallback and roaming capabilities
 *
 * ESP32-Specific Features:
 * - Dual-core Xtensa LX6 processor (240MHz each)
 * - Hardware encryption acceleration
 * - Ultra-low power sleep modes
 * - Rich peripheral set (SPI, I2C, ADC, etc.)
 *
 * @author EsoCore Development Team
 * @copyright Copyright © 2025 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_WIFI_MANAGER_H
#define ESOCORE_WIFI_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * WiFi Configuration
 * ============================================================================ */

/* ESP32-WROOM-32 Configuration */
#define WIFI_SSID_MAX_LENGTH     32
#define WIFI_PASSWORD_MAX_LENGTH 64
#define WIFI_MAX_NETWORKS        16
#define ESP32_DEFAULT_BAUDRATE   115200
#define ESP32_COMMAND_TIMEOUT    5000   /* 5 seconds */
#define ESP32_CONNECT_TIMEOUT    20000  /* 20 seconds for connection */
#define ESP32_SCAN_TIMEOUT       10000  /* 10 seconds for scanning */

typedef enum {
    WIFI_SECURITY_OPEN      = 0,    /* Open network */
    WIFI_SECURITY_WEP       = 1,    /* WEP security */
    WIFI_SECURITY_WPA       = 2,    /* WPA security */
    WIFI_SECURITY_WPA2      = 3,    /* WPA2 security */
    WIFI_SECURITY_WPA3      = 4,    /* WPA3 security */
    WIFI_SECURITY_UNKNOWN   = 5,    /* Unknown security */
} wifi_security_t;

typedef enum {
    WIFI_MODE_STATION       = 0,    /* Station mode (client) */
    WIFI_MODE_ACCESS_POINT  = 1,    /* Access point mode */
    WIFI_MODE_MIXED         = 2,    /* Station + Access point */
} wifi_mode_t;

typedef struct {
    char ssid[WIFI_SSID_MAX_LENGTH + 1];        /* WiFi network SSID */
    char password[WIFI_PASSWORD_MAX_LENGTH + 1]; /* WiFi password */
    wifi_security_t security;                    /* Security type */
    wifi_mode_t mode;                            /* WiFi operation mode */
    uint8_t channel;                             /* WiFi channel (1-13) */
    bool dhcp_enabled;                           /* Enable DHCP client */
    char static_ip[16];                          /* Static IP address (if DHCP disabled) */
    char subnet_mask[16];                        /* Subnet mask */
    char gateway[16];                            /* Gateway IP */
    char dns_server[16];                        /* DNS server IP */
    uint32_t connection_timeout_ms;              /* Connection timeout */
    uint8_t max_retries;                         /* Maximum connection retries */
    bool auto_reconnect;                         /* Enable automatic reconnection */
    uint32_t reconnect_interval_ms;              /* Reconnection interval */
} wifi_config_t;

/* ============================================================================
 * WiFi Network Information
 * ============================================================================ */

typedef struct {
    char ssid[WIFI_SSID_MAX_LENGTH + 1];        /* Network SSID */
    wifi_security_t security;                    /* Security type */
    int8_t rssi;                                 /* Signal strength (dBm) */
    uint8_t channel;                             /* WiFi channel */
    bool connected;                              /* Connection status */
} wifi_network_info_t;

/* ============================================================================
 * WiFi Status and Diagnostics
 * ============================================================================ */

typedef enum {
    WIFI_STATUS_DISCONNECTED   = 0,    /* Not connected */
    WIFI_STATUS_CONNECTING     = 1,    /* Connecting to network */
    WIFI_STATUS_CONNECTED      = 2,    /* Connected to network */
    WIFI_STATUS_CONNECTION_LOST = 3,   /* Connection lost */
    WIFI_STATUS_ERROR          = 4,    /* Connection error */
} wifi_status_t;

typedef struct {
    wifi_status_t status;                         /* Current WiFi status */
    char connected_ssid[WIFI_SSID_MAX_LENGTH + 1]; /* Connected network SSID */
    char ip_address[16];                          /* Assigned IP address */
    char mac_address[18];                         /* MAC address */
    int8_t rssi;                                  /* Signal strength (dBm) */
    uint8_t channel;                              /* Connected channel */
    uint32_t connected_time_ms;                   /* Connection duration */
    uint32_t bytes_sent;                          /* Total bytes sent */
    uint32_t bytes_received;                      /* Total bytes received */
    uint16_t connection_errors;                   /* Connection error count */
    uint16_t reconnect_count;                     /* Reconnection attempts */
} wifi_status_info_t;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/**
 * @brief Initialize WiFi manager
 *
 * @param config Pointer to WiFi configuration
 * @return true if initialization successful, false otherwise
 */
bool wifi_manager_init(const wifi_config_t *config);

/**
 * @brief Deinitialize WiFi manager
 *
 * @return true if deinitialization successful, false otherwise
 */
bool wifi_manager_deinit(void);

/**
 * @brief Connect to WiFi network
 *
 * @return true if connection successful, false otherwise
 */
bool wifi_manager_connect(void);

/**
 * @brief Disconnect from WiFi network
 *
 * @return true if disconnection successful, false otherwise
 */
bool wifi_manager_disconnect(void);

/**
 * @brief Reconnect to WiFi network
 *
 * @return true if reconnection successful, false otherwise
 */
bool wifi_manager_reconnect(void);

/**
 * @brief Get current WiFi status
 *
 * @param status_info Pointer to status info structure to fill
 * @return true if status retrieved successfully, false otherwise
 */
bool wifi_manager_get_status(wifi_status_info_t *status_info);

/**
 * @brief Scan for available WiFi networks
 *
 * @param networks Pointer to array of network info structures
 * @param max_networks Maximum number of networks to scan
 * @param num_found Pointer to store number of networks found
 * @return true if scan successful, false otherwise
 */
bool wifi_manager_scan_networks(wifi_network_info_t *networks, uint8_t max_networks,
                               uint8_t *num_found);

/**
 * @brief Update WiFi configuration
 *
 * @param config Pointer to new configuration
 * @return true if configuration updated successfully, false otherwise
 */
bool wifi_manager_update_config(const wifi_config_t *config);

/**
 * @brief Get current WiFi configuration
 *
 * @param config Pointer to configuration structure to fill
 * @return true if configuration retrieved successfully, false otherwise
 */
bool wifi_manager_get_config(wifi_config_t *config);

/**
 * @brief Set WiFi operation mode
 *
 * @param mode WiFi operation mode
 * @return true if mode set successfully, false otherwise
 */
bool wifi_manager_set_mode(wifi_mode_t mode);

/**
 * @brief Enable/disable DHCP
 *
 * @param enable true to enable DHCP, false for static IP
 * @return true if operation successful, false otherwise
 */
bool wifi_manager_set_dhcp(bool enable);

/**
 * @brief Configure static IP settings
 *
 * @param ip_address IP address string
 * @param subnet_mask Subnet mask string
 * @param gateway Gateway IP string
 * @param dns_server DNS server IP string
 * @return true if configuration successful, false otherwise
 */
bool wifi_manager_set_static_ip(const char *ip_address, const char *subnet_mask,
                               const char *gateway, const char *dns_server);

/**
 * @brief Get signal strength (RSSI)
 *
 * @return RSSI value in dBm, 0 if not connected
 */
int8_t wifi_manager_get_rssi(void);

/**
 * @brief Check if WiFi is connected
 *
 * @return true if connected, false otherwise
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Get connection quality metrics
 *
 * @param link_quality Pointer to store link quality (0-100)
 * @param signal_strength Pointer to store signal strength (-100 to 0 dBm)
 * @param noise_level Pointer to store noise level (dBm)
 * @return true if metrics retrieved successfully, false otherwise
 */
bool wifi_manager_get_quality_metrics(uint8_t *link_quality, int8_t *signal_strength,
                                     int8_t *noise_level);

/**
 * @brief Perform WiFi diagnostics
 *
 * @return Diagnostic result code (0 = OK, non-zero = error)
 */
uint16_t wifi_manager_run_diagnostics(void);

/**
 * @brief Reset WiFi module
 *
 * @return true if reset successful, false otherwise
 */
bool wifi_manager_reset(void);

/**
 * @brief Enter low-power WiFi mode
 *
 * @return true if low-power mode entered successfully, false otherwise
 */
bool wifi_manager_enter_low_power(void);

/**
 * @brief Exit low-power WiFi mode
 *
 * @return true if low-power mode exited successfully, false otherwise
 */
bool wifi_manager_exit_low_power(void);

/**
 * @brief Get WiFi statistics
 *
 * @param status_info Pointer to status info structure to fill
 * @return true if statistics retrieved successfully, false otherwise
 */
bool wifi_manager_get_statistics(wifi_status_info_t *status_info);

/**
 * @brief Clear WiFi statistics
 *
 * @return true if statistics cleared successfully, false otherwise
 */
bool wifi_manager_clear_statistics(void);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_WIFI_MANAGER_H */
