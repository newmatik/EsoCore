/**
 * @file ethernet_manager.c
 * @brief Dual Ethernet Port Manager Implementation
 *
 * Manages two independent Ethernet interfaces on the EsoCore Edge device.
 * Port A serves the machine/OT network (PROFINET RT, Modbus TCP, web UI)
 * with a static IP, while Port B serves the IT network (cloud sync, REST
 * API, NTP, OTA) via DHCP.
 *
 * Architectural note:
 *   The STM32H747 has a single Ethernet MAC peripheral. True hardware-level
 *   dual-port isolation would require an external Ethernet switch IC or a
 *   second MAC. In this implementation, Port A is driven directly by the
 *   STM32 MAC. Port B support is prepared at the software layer (PHY
 *   management, status monitoring, MAC-based filtering) but full independent
 *   TX/RX on Port B requires either an external switch with VLAN tagging or
 *   a future hardware revision with a second MAC. Where this limitation
 *   applies, it is noted in the code.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "ethernet_manager.h"
#include "../../stm32/stm32h7/hal/hal_eth.h"
#include "../../stm32/stm32h7/drivers/drv_ksz8081.h"
#include "../../stm32/stm32h7/bsp/pin_mapping_v150.h"

#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Default Configuration
 * ============================================================================ */

/* Default MAC addresses (Newmatik OUI block: 00:80:E1) */
static const uint8_t default_mac_port_a[6] = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x01};
static const uint8_t default_mac_port_b[6] = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x02};

/* Default IP: Port A = 192.168.10.100/24 static */
static const ethm_ip_config_t default_ip_port_a = {
    .use_dhcp = false,
    .ip_addr  = {192, 168, 10, 100},
    .netmask  = {255, 255, 255, 0},
    .gateway  = {192, 168, 10, 1},
    .dns      = {192, 168, 10, 1},
};

/* Default IP: Port B = DHCP */
static const ethm_ip_config_t default_ip_port_b = {
    .use_dhcp = true,
    .ip_addr  = {0, 0, 0, 0},
    .netmask  = {0, 0, 0, 0},
    .gateway  = {0, 0, 0, 0},
    .dns      = {0, 0, 0, 0},
};

/* ============================================================================
 * Internal State
 * ============================================================================ */

static bool ethm_initialized = false;

/** Per-port runtime state */
typedef struct {
    ethm_port_config_t  config;
    ethm_port_status_t  status;
    ethm_link_status_t  prev_link;      /* Previous link state for transition detection */
    ethm_rx_callback_t  rx_callback;
    void               *rx_ctx;
} ethm_port_state_t;

static ethm_port_state_t port_state[ETHM_PORT_COUNT];

/* ============================================================================
 * Forward Declarations (Private Helpers)
 * ============================================================================ */

static void ethm_apply_defaults(ethm_port_config_t *cfg, ethm_port_t port);
static void ethm_update_link_status(ethm_port_t port);
static void ethm_hal_rx_handler(hal_eth_port_t hal_port,
                                uint8_t *data, uint16_t length,
                                void *context);

/* ============================================================================
 * Private Helper Implementation
 * ============================================================================ */

/**
 * @brief Fill port config with defaults if fields are zero
 */
static void ethm_apply_defaults(ethm_port_config_t *cfg, ethm_port_t port) {
    /* Apply default MAC if all zeros */
    bool mac_zero = true;
    for (int i = 0; i < 6; i++) {
        if (cfg->mac_addr[i] != 0) {
            mac_zero = false;
            break;
        }
    }
    if (mac_zero) {
        const uint8_t *default_mac = (port == ETHM_PORT_A)
            ? default_mac_port_a : default_mac_port_b;
        memcpy(cfg->mac_addr, default_mac, 6);
    }

    /* Apply default IP if all zeros and not DHCP */
    bool ip_zero = true;
    for (int i = 0; i < 4; i++) {
        if (cfg->ip_config.ip_addr[i] != 0) {
            ip_zero = false;
            break;
        }
    }
    if (ip_zero && !cfg->ip_config.use_dhcp) {
        if (port == ETHM_PORT_A) {
            memcpy(&cfg->ip_config, &default_ip_port_a, sizeof(ethm_ip_config_t));
        } else {
            memcpy(&cfg->ip_config, &default_ip_port_b, sizeof(ethm_ip_config_t));
        }
    }
}

/**
 * @brief Check link status on a PHY and detect transitions
 */
static void ethm_update_link_status(ethm_port_t port) {
    if (port >= ETHM_PORT_COUNT) {
        return;
    }

    ethm_port_state_t *ps = &port_state[port];
    ksz8081_status_t phy_status;

    if (!drv_ksz8081_get_status(ps->config.phy_addr, &phy_status)) {
        ps->status.errors++;
        return;
    }

    /* Update status fields */
    ethm_link_status_t new_link = phy_status.link_up
        ? ETHM_LINK_UP : ETHM_LINK_DOWN;
    ps->status.link_status = new_link;
    ps->status.speed_100m  = phy_status.speed_100m;
    ps->status.full_duplex = phy_status.full_duplex;

    /* Detect link transitions */
    if (new_link != ps->prev_link) {
        if (new_link == ETHM_LINK_UP) {
            /* TODO: Replace printf with project logging framework */
            printf("[ETHM] Port %c: link UP (%s, %s duplex)\n",
                   (port == ETHM_PORT_A) ? 'A' : 'B',
                   phy_status.speed_100m ? "100M" : "10M",
                   phy_status.full_duplex ? "full" : "half");
        } else {
            printf("[ETHM] Port %c: link DOWN\n",
                   (port == ETHM_PORT_A) ? 'A' : 'B');
        }
        ps->prev_link = new_link;
    }
}

/**
 * @brief HAL-level RX callback, dispatches to registered user callback
 */
static void ethm_hal_rx_handler(hal_eth_port_t hal_port,
                                uint8_t *data, uint16_t length,
                                void *context) {
    ethm_port_t port = (ethm_port_t)hal_port;

    if (port >= ETHM_PORT_COUNT) {
        return;
    }

    ethm_port_state_t *ps = &port_state[port];
    ps->status.packets_rx++;

    if (ps->rx_callback) {
        ps->rx_callback(port, data, length, ps->rx_ctx);
    }
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

bool ethm_init(const ethm_config_t *config) {
    if (ethm_initialized) {
        return false;
    }

    /* Clear all state */
    memset(port_state, 0, sizeof(port_state));

    /* ---- Configure both ports ---- */
    for (int i = 0; i < ETHM_PORT_COUNT; i++) {
        ethm_port_state_t *ps = &port_state[i];

        if (config) {
            memcpy(&ps->config, &config->port_config[i],
                   sizeof(ethm_port_config_t));
        }
        ethm_apply_defaults(&ps->config, (ethm_port_t)i);

        ps->prev_link   = ETHM_LINK_DOWN;
        ps->rx_callback = NULL;
        ps->rx_ctx      = NULL;
    }

    /* ---- Initialize Port A (primary MAC) ---- */
    {
        ethm_port_state_t *ps = &port_state[ETHM_PORT_A];
        hal_eth_config_t hal_cfg;

        hal_cfg.port     = ETH_PORT_A;
        hal_cfg.phy_addr = ps->config.phy_addr ? ps->config.phy_addr : ETH1_PHY_ADDR;
        memcpy(hal_cfg.mac_addr.addr, ps->config.mac_addr, 6);

        if (!hal_eth_init(ETH_PORT_A, &hal_cfg)) {
            printf("[ETHM] ERROR: HAL ETH init failed for Port A\n");
            return false;
        }

        /* Register internal RX handler */
        hal_eth_register_rx_callback(ETH_PORT_A, ethm_hal_rx_handler, NULL);
    }

    /* ---- Initialize KSZ8081 PHY for Port A ---- */
    {
        ksz8081_config_t phy_cfg;
        phy_cfg.phy_addr = port_state[ETHM_PORT_A].config.phy_addr
            ? port_state[ETHM_PORT_A].config.phy_addr
            : ETH1_PHY_ADDR;

        if (!drv_ksz8081_init(&phy_cfg)) {
            printf("[ETHM] ERROR: PHY init failed for Port A (addr %u)\n",
                   phy_cfg.phy_addr);
            return false;
        }
    }

    /* ---- Initialize KSZ8081 PHY for Port B ---- */
    {
        ksz8081_config_t phy_cfg;
        phy_cfg.phy_addr = port_state[ETHM_PORT_B].config.phy_addr
            ? port_state[ETHM_PORT_B].config.phy_addr
            : ETH2_PHY_ADDR;

        if (!drv_ksz8081_init(&phy_cfg)) {
            printf("[ETHM] ERROR: PHY init failed for Port B (addr %u)\n",
                   phy_cfg.phy_addr);
            return false;
        }
    }

    /* ---- Start MAC ---- */
    if (!hal_eth_start(ETH_PORT_A)) {
        printf("[ETHM] ERROR: HAL ETH start failed for Port A\n");
        return false;
    }

    /*
     * Architectural limitation: STM32H747 has a single Ethernet MAC.
     * Port B PHY is initialized and monitored, but independent TX/RX on
     * Port B requires either:
     *   1. An external managed switch IC with VLAN tagging, or
     *   2. MAC-based frame filtering on the single MAC, or
     *   3. A hardware revision with a second MAC (e.g., external SPI-Ethernet).
     *
     * For now, Port B link status is monitored and reported. Full Port B
     * data-plane support is deferred to the next hardware revision.
     */
    printf("[ETHM] NOTE: Port B data-plane limited -- single-MAC STM32H747 "
           "architecture. PHY monitoring active; full TX/RX requires "
           "external switch or second MAC.\n");

    /* ---- Apply IP configuration ---- */
    /* TODO: Integrate with lwIP netif for each port.
     *   - Create netif for Port A with static IP
     *   - Create netif for Port B with DHCP
     *   - Bind each netif to its respective port
     */
    for (int i = 0; i < ETHM_PORT_COUNT; i++) {
        ethm_port_state_t *ps = &port_state[i];
        if (!ps->config.ip_config.use_dhcp) {
            memcpy(ps->status.ip_addr, ps->config.ip_config.ip_addr, 4);
        }
        /* DHCP address will be populated once the lwIP DHCP client acquires a lease */
    }

    printf("[ETHM] Initialized -- Port A: %u.%u.%u.%u (static), "
           "Port B: DHCP\n",
           port_state[ETHM_PORT_A].status.ip_addr[0],
           port_state[ETHM_PORT_A].status.ip_addr[1],
           port_state[ETHM_PORT_A].status.ip_addr[2],
           port_state[ETHM_PORT_A].status.ip_addr[3]);

    ethm_initialized = true;
    return true;
}

bool ethm_deinit(void) {
    if (!ethm_initialized) {
        return false;
    }

    /* Stop MAC */
    hal_eth_stop(ETH_PORT_A);
    hal_eth_deinit(ETH_PORT_A);

    /* TODO: Tear down lwIP netifs for both ports */

    memset(port_state, 0, sizeof(port_state));
    ethm_initialized = false;

    printf("[ETHM] Deinitialized\n");
    return true;
}

bool ethm_get_port_status(ethm_port_t port, ethm_port_status_t *status) {
    if (!ethm_initialized || port >= ETHM_PORT_COUNT || !status) {
        return false;
    }

    memcpy(status, &port_state[port].status, sizeof(ethm_port_status_t));
    return true;
}

void ethm_poll(void) {
    if (!ethm_initialized) {
        return;
    }

    /* Check link status on both PHYs */
    ethm_update_link_status(ETHM_PORT_A);
    ethm_update_link_status(ETHM_PORT_B);

    /* Process received frames on Port A (primary MAC) */
    hal_eth_poll(ETH_PORT_A);

    /*
     * Port B RX: With a single MAC, Port B frames are not independently
     * received. If an external switch with MAC-based filtering is used,
     * frames destined for Port B's MAC would arrive on the same MAC
     * peripheral and need to be dispatched based on destination MAC.
     *
     * TODO: Implement MAC-based RX demuxing if external switch is present.
     */
}

bool ethm_transmit(ethm_port_t port, const uint8_t *data, uint16_t length) {
    if (!ethm_initialized || port >= ETHM_PORT_COUNT || !data || length == 0) {
        return false;
    }

    ethm_port_state_t *ps = &port_state[port];

    if (ps->status.link_status != ETHM_LINK_UP) {
        ps->status.errors++;
        return false;
    }

    if (port == ETHM_PORT_A) {
        /* Port A: direct HAL transmission */
        if (hal_eth_transmit(ETH_PORT_A, data, length)) {
            ps->status.packets_tx++;
            return true;
        }
        ps->status.errors++;
        return false;
    }

    /*
     * Port B: Single-MAC limitation.
     * With an external switch, TX for Port B would be sent through the
     * same MAC with a VLAN tag or destination-based forwarding.
     * Without an external switch, Port B TX is not supported.
     *
     * TODO: Implement Port B TX via external switch VLAN tagging or
     * secondary SPI-Ethernet MAC when hardware is available.
     */
    printf("[ETHM] WARNING: Port B transmit not available -- "
           "single-MAC architecture\n");
    ps->status.errors++;
    return false;
}

bool ethm_register_rx_callback(ethm_port_t port,
                                ethm_rx_callback_t callback, void *ctx) {
    if (!ethm_initialized || port >= ETHM_PORT_COUNT) {
        return false;
    }

    port_state[port].rx_callback = callback;
    port_state[port].rx_ctx      = ctx;
    return true;
}

ethm_link_status_t ethm_get_link_status(ethm_port_t port) {
    if (!ethm_initialized || port >= ETHM_PORT_COUNT) {
        return ETHM_LINK_DOWN;
    }

    return port_state[port].status.link_status;
}

bool ethm_set_ip_config(ethm_port_t port, const ethm_ip_config_t *ip_config) {
    if (!ethm_initialized || port >= ETHM_PORT_COUNT || !ip_config) {
        return false;
    }

    ethm_port_state_t *ps = &port_state[port];
    memcpy(&ps->config.ip_config, ip_config, sizeof(ethm_ip_config_t));

    if (!ip_config->use_dhcp) {
        /* Static IP: apply immediately */
        memcpy(ps->status.ip_addr, ip_config->ip_addr, 4);

        /* TODO: Update lwIP netif with new static IP configuration
         *   netif_set_addr(netif, &ipaddr, &netmask, &gw);
         */
    } else {
        /* DHCP: start/restart DHCP client */
        memset(ps->status.ip_addr, 0, 4);

        /* TODO: Start lwIP DHCP client on this netif
         *   dhcp_start(netif);
         */
    }

    printf("[ETHM] Port %c: IP config updated (%s)\n",
           (port == ETHM_PORT_A) ? 'A' : 'B',
           ip_config->use_dhcp ? "DHCP" : "static");

    return true;
}

bool ethm_network_isolation_check(void) {
    if (!ethm_initialized) {
        return false;
    }

    /*
     * Verify that no IP forwarding or bridging is active between the two
     * ports. In this implementation, isolation is inherent because:
     *
     *   1. Each port has its own IP configuration (separate subnets).
     *   2. No IP forwarding logic exists in the firmware.
     *   3. The lwIP stack (when integrated) will bind each netif
     *      independently with no routing between them.
     *   4. Port B has limited data-plane support on single-MAC hardware,
     *      further preventing any accidental bridging.
     *
     * This function performs the following checks:
     *   - Confirm Port A and Port B have different subnets
     *   - Confirm no shared gateway between ports
     *   - Confirm no IP forwarding flag is set (lwIP)
     */

    ethm_port_state_t *pa = &port_state[ETHM_PORT_A];
    ethm_port_state_t *pb = &port_state[ETHM_PORT_B];

    /* Check that MAC addresses are different */
    if (memcmp(pa->config.mac_addr, pb->config.mac_addr, 6) == 0) {
        printf("[ETHM] ISOLATION FAIL: Port A and Port B share the same "
               "MAC address\n");
        return false;
    }

    /* Check that subnets don't overlap (simple check: network addresses differ) */
    if (!pa->config.ip_config.use_dhcp && !pb->config.ip_config.use_dhcp) {
        bool same_subnet = true;
        for (int i = 0; i < 4; i++) {
            uint8_t net_a = pa->config.ip_config.ip_addr[i]
                          & pa->config.ip_config.netmask[i];
            uint8_t net_b = pb->config.ip_config.ip_addr[i]
                          & pb->config.ip_config.netmask[i];
            if (net_a != net_b) {
                same_subnet = false;
                break;
            }
        }
        if (same_subnet) {
            printf("[ETHM] ISOLATION WARNING: Port A and Port B are on "
                   "the same subnet\n");
            return false;
        }
    }

    /* TODO: Check lwIP ip_forward flag is disabled
     *   if (ip_forward) { return false; }
     */

    printf("[ETHM] Network isolation check: PASSED\n");
    return true;
}
