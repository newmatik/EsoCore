/**
 * @file hal_eth.h
 * @brief Dual Ethernet MAC Hardware Abstraction Layer for STM32H747
 *
 * Provides low-level MAC control for both Ethernet ports.
 * Higher-level TCP/IP stack integration is in ethernet_manager.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_HAL_ETH_H
#define ESOCORE_HAL_ETH_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ETH_MTU             1500
#define ETH_HEADER_SIZE     14
#define ETH_MAX_FRAME_SIZE  (ETH_MTU + ETH_HEADER_SIZE + 4) /* +4 for CRC */

#define ETH_TX_DESC_COUNT   4
#define ETH_RX_DESC_COUNT   4
#define ETH_RX_BUF_SIZE     ETH_MAX_FRAME_SIZE

/* Ethernet port identifier */
typedef enum {
    ETH_PORT_A = 0,   /* Machine/OT network */
    ETH_PORT_B = 1,   /* IT network */
    ETH_PORT_COUNT = 2,
} hal_eth_port_t;

/* Link status */
typedef enum {
    ETH_LINK_DOWN = 0,
    ETH_LINK_UP_10M_HD,
    ETH_LINK_UP_10M_FD,
    ETH_LINK_UP_100M_HD,
    ETH_LINK_UP_100M_FD,
} hal_eth_link_status_t;

/* MAC address */
typedef struct {
    uint8_t addr[6];
} hal_eth_mac_addr_t;

/* Configuration for one Ethernet port */
typedef struct {
    hal_eth_port_t      port;
    hal_eth_mac_addr_t  mac_addr;
    uint8_t             phy_addr;   /* MDIO address of PHY */
} hal_eth_config_t;

/* Receive callback */
typedef void (*hal_eth_rx_callback_t)(hal_eth_port_t port,
                                      uint8_t *data, uint16_t length,
                                      void *context);

/**
 * @brief Initialize Ethernet MAC for the specified port
 */
bool hal_eth_init(hal_eth_port_t port, const hal_eth_config_t *config);

/**
 * @brief Deinitialize Ethernet MAC
 */
void hal_eth_deinit(hal_eth_port_t port);

/**
 * @brief Start Ethernet MAC (enable TX/RX)
 */
bool hal_eth_start(hal_eth_port_t port);

/**
 * @brief Stop Ethernet MAC
 */
void hal_eth_stop(hal_eth_port_t port);

/**
 * @brief Transmit an Ethernet frame
 */
bool hal_eth_transmit(hal_eth_port_t port,
                      const uint8_t *data, uint16_t length);

/**
 * @brief Register frame receive callback
 */
void hal_eth_register_rx_callback(hal_eth_port_t port,
                                  hal_eth_rx_callback_t callback,
                                  void *context);

/**
 * @brief Read a PHY register via MDIO (SMI)
 */
bool hal_eth_mdio_read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *value);

/**
 * @brief Write a PHY register via MDIO (SMI)
 */
bool hal_eth_mdio_write(uint8_t phy_addr, uint8_t reg_addr, uint16_t value);

/**
 * @brief Get link status for a port
 */
hal_eth_link_status_t hal_eth_get_link_status(hal_eth_port_t port);

/**
 * @brief Set MAC address for a port
 */
void hal_eth_set_mac_addr(hal_eth_port_t port, const hal_eth_mac_addr_t *addr);

/**
 * @brief Poll for received frames (call from main loop or RTOS task)
 */
void hal_eth_poll(hal_eth_port_t port);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_HAL_ETH_H */
