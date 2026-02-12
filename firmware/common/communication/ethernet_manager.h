/**
 * @file ethernet_manager.h
 * @brief Dual Ethernet Port Manager for EsoCore Edge Device
 *
 * High-level manager for dual Ethernet connectivity on the EsoCore Edge,
 * sitting above the HAL/PHY layer (hal_eth + KSZ8081 driver). Manages
 * two independent network interfaces:
 *
 *   Port A (Machine / OT Network):
 *     - PROFINET RT, Modbus TCP, local web UI
 *     - Static IP (default 192.168.10.100/24)
 *     - Deterministic, low-latency industrial traffic
 *
 *   Port B (IT Network):
 *     - HTTPS cloud sync, REST API, NTP, OTA updates
 *     - DHCP (default)
 *     - Standard enterprise/internet traffic
 *
 * Network Isolation:
 *   Port A and Port B operate as completely independent network interfaces.
 *   No IP routing is performed between the two ports. This ensures that
 *   machine-level (OT) traffic is never exposed to the IT network, and
 *   vice versa. The isolation is enforced at the software layer; each port
 *   maintains its own IP configuration, routing table, and socket bindings.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_ETHERNET_MANAGER_H
#define ESOCORE_ETHERNET_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Ethernet Port Identifiers
 * ============================================================================ */

/** Physical Ethernet port selection */
typedef enum {
    ETHM_PORT_A     = 0,    /* Machine / OT network */
    ETHM_PORT_B     = 1,    /* IT network */
    ETHM_PORT_COUNT = 2,
} ethm_port_t;

/* ============================================================================
 * Port Role Definitions
 * ============================================================================ */

/** Logical role assigned to each port */
typedef enum {
    ETHM_ROLE_MACHINE = 0,  /* PROFINET RT, Modbus TCP, web UI, static IP */
    ETHM_ROLE_IT      = 1,  /* HTTPS cloud sync, REST API, NTP, OTA, DHCP */
} ethm_port_role_t;

/* ============================================================================
 * Link Status
 * ============================================================================ */

/** Ethernet link state */
typedef enum {
    ETHM_LINK_DOWN = 0,     /* No link detected */
    ETHM_LINK_UP   = 1,     /* Link established */
} ethm_link_status_t;

/* ============================================================================
 * Configuration Structures
 * ============================================================================ */

/** IP configuration for a single port */
typedef struct {
    bool    use_dhcp;       /* true = DHCP, false = static IP */
    uint8_t ip_addr[4];    /* IPv4 address */
    uint8_t netmask[4];    /* Subnet mask */
    uint8_t gateway[4];    /* Default gateway */
    uint8_t dns[4];        /* Primary DNS server */
} ethm_ip_config_t;

/** Configuration for a single Ethernet port */
typedef struct {
    ethm_port_role_t role;          /* Port role (machine or IT) */
    ethm_ip_config_t ip_config;     /* IP configuration */
    uint8_t          mac_addr[6];   /* MAC address */
    uint8_t          phy_addr;      /* MDIO address of the KSZ8081 PHY */
} ethm_port_config_t;

/** Runtime status for a single Ethernet port */
typedef struct {
    ethm_link_status_t link_status;     /* Current link state */
    bool               speed_100m;      /* true = 100 Mbps, false = 10 Mbps */
    bool               full_duplex;     /* true = full duplex, false = half */
    uint8_t            ip_addr[4];      /* Currently active IP address */
    uint32_t           packets_tx;      /* Total transmitted packets */
    uint32_t           packets_rx;      /* Total received packets */
    uint32_t           errors;          /* Cumulative error count */
} ethm_port_status_t;

/** System-level configuration for both Ethernet ports */
typedef struct {
    ethm_port_config_t port_config[ETHM_PORT_COUNT];
} ethm_config_t;

/* ============================================================================
 * Receive Callback
 * ============================================================================ */

/**
 * @brief Callback invoked when a frame is received on a port
 *
 * @param port   Port that received the frame
 * @param data   Pointer to frame payload
 * @param length Length of the frame in bytes
 * @param ctx    User-provided context pointer
 */
typedef void (*ethm_rx_callback_t)(ethm_port_t port,
                                   const uint8_t *data, uint16_t length,
                                   void *ctx);

/* ============================================================================
 * Public API
 * ============================================================================ */

/**
 * @brief Initialize both Ethernet ports
 *
 * Configures MAC addresses, initializes the HAL ETH peripheral and both
 * KSZ8081 PHYs, and applies IP configuration for each port.
 *
 * @param config Pointer to system configuration
 * @return true on success, false on initialization failure
 */
bool ethm_init(const ethm_config_t *config);

/**
 * @brief Shut down both Ethernet ports
 *
 * Stops MAC operation, powers down PHYs, and releases resources.
 *
 * @return true on success, false on error
 */
bool ethm_deinit(void);

/**
 * @brief Get runtime status of a specific port
 *
 * @param port   Port to query
 * @param status Pointer to status structure to populate
 * @return true on success, false if port is invalid or not initialized
 */
bool ethm_get_port_status(ethm_port_t port, ethm_port_status_t *status);

/**
 * @brief Poll both Ethernet ports (call from main loop or RTOS task)
 *
 * Checks link status on both PHYs, processes received frames, and
 * updates internal statistics. Should be called periodically.
 */
void ethm_poll(void);

/**
 * @brief Transmit an Ethernet frame on a specific port
 *
 * @param port   Port to transmit on
 * @param data   Pointer to frame data
 * @param length Length of the frame in bytes
 * @return true if the frame was queued for transmission, false on error
 */
bool ethm_transmit(ethm_port_t port, const uint8_t *data, uint16_t length);

/**
 * @brief Register a receive callback for a specific port
 *
 * @param port     Port to register the callback for
 * @param callback Function to call when a frame is received
 * @param ctx      User-provided context passed to the callback
 * @return true on success, false if port is invalid
 */
bool ethm_register_rx_callback(ethm_port_t port,
                                ethm_rx_callback_t callback, void *ctx);

/**
 * @brief Get the current link status of a port
 *
 * Lightweight check without populating the full status structure.
 *
 * @param port Port to query
 * @return ETHM_LINK_UP or ETHM_LINK_DOWN
 */
ethm_link_status_t ethm_get_link_status(ethm_port_t port);

/**
 * @brief Update IP configuration for a port at runtime
 *
 * Applies a new IP configuration (static or DHCP) without reinitializing
 * the underlying MAC/PHY hardware.
 *
 * @param port      Port to reconfigure
 * @param ip_config Pointer to new IP configuration
 * @return true on success, false on error
 */
bool ethm_set_ip_config(ethm_port_t port, const ethm_ip_config_t *ip_config);

/**
 * @brief Verify network isolation between Port A and Port B
 *
 * Checks that no IP routing or bridging exists between the two ports.
 * This is a diagnostic function that confirms OT/IT separation.
 *
 * @return true if isolation is intact, false if cross-routing is detected
 */
bool ethm_network_isolation_check(void);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_ETHERNET_MANAGER_H */
