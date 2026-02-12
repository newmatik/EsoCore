/**
 * @file drv_ksz8081.c
 * @brief KSZ8081RNACA Ethernet PHY Driver Implementation
 *
 * Implements MDIO-based control of the KSZ8081RNACA 10/100 Ethernet PHY.
 * Uses the HAL Ethernet layer for MDIO read/write access to PHY registers.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "drv_ksz8081.h"
#include "../hal/hal_eth.h"

/* Soft-reset timeout: maximum number of polling iterations.
 * At ~1 us per read cycle, 500000 iterations ~= 500 ms. */
#define KSZ8081_RESET_TIMEOUT_CYCLES    500000U

/* ============================================================================
 * Private Helpers
 * ============================================================================ */

/**
 * @brief Simple busy-wait delay (approximate microseconds)
 *
 * This is a coarse software delay used only during PHY initialisation.
 * In production firmware this should be replaced with an RTOS delay or
 * a hardware timer.
 */
static void delay_us(volatile uint32_t us)
{
    /* ~10 iterations per us at 480 MHz with pipeline stalls */
    volatile uint32_t count = us * 10;
    while (count--) {
        __asm volatile("nop");
    }
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================ */

bool drv_ksz8081_init(const ksz8081_config_t *config)
{
    if (!config) {
        return false;
    }

    uint8_t phy_addr = config->phy_addr;

    /* Step 1: Soft reset the PHY */
    if (!drv_ksz8081_soft_reset(phy_addr)) {
        return false;
    }

    /* Step 2: Verify PHY identity */
    uint16_t id1 = 0;
    uint16_t id2 = 0;

    if (!drv_ksz8081_read_id(phy_addr, &id1, &id2)) {
        return false;
    }

    if (id1 != KSZ8081_PHY_ID1 ||
        (id2 & KSZ8081_PHY_ID2_MASK) != (KSZ8081_PHY_ID2 & KSZ8081_PHY_ID2_MASK)) {
        return false;  /* PHY ID mismatch */
    }

    /* Step 3: Enable auto-negotiation for 10/100 half/full duplex */
    if (!drv_ksz8081_enable_auto_negotiation(phy_addr)) {
        return false;
    }

    /* Step 4: Enable Auto MDI/MDI-X via PHY_CTRL2 */
    uint16_t ctrl2 = 0;
    if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_PHY_CTRL2, &ctrl2)) {
        return false;
    }

    ctrl2 |= KSZ8081_CTRL2_HP_MDIX;  /* Enable HP Auto MDI/MDI-X */

    if (!hal_eth_mdio_write(phy_addr, KSZ8081_REG_PHY_CTRL2, ctrl2)) {
        return false;
    }

    return true;
}

bool drv_ksz8081_get_status(uint8_t phy_addr, ksz8081_status_t *status)
{
    if (!status) {
        return false;
    }

    /* Read BSR for link status.
     * IEEE 802.3 requires reading BSR twice to get the latched-low
     * link status to clear, then the current real-time status. */
    uint16_t bsr = 0;

    if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_BSR, &bsr)) {
        return false;
    }
    /* Second read for current (non-latched) status */
    if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_BSR, &bsr)) {
        return false;
    }

    status->link_up = (bsr & KSZ8081_BSR_LINK_STATUS) != 0;

    /* If link is down, remaining fields are not meaningful */
    if (!status->link_up) {
        status->speed_100m  = false;
        status->full_duplex = false;
        status->auto_mdix   = false;
        return true;
    }

    /* Read PHY Control 1 register for negotiated speed/duplex */
    uint16_t ctrl1 = 0;
    if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_PHY_CTRL1, &ctrl1)) {
        return false;
    }

    uint8_t op_mode = ctrl1 & KSZ8081_CTRL1_OP_MODE_MASK;

    switch (op_mode) {
        case KSZ8081_CTRL1_OP_10M_HD:
            status->speed_100m  = false;
            status->full_duplex = false;
            break;
        case KSZ8081_CTRL1_OP_10M_FD:
            status->speed_100m  = false;
            status->full_duplex = true;
            break;
        case KSZ8081_CTRL1_OP_100M_HD:
            status->speed_100m  = true;
            status->full_duplex = false;
            break;
        case KSZ8081_CTRL1_OP_100M_FD:
        default:
            status->speed_100m  = true;
            status->full_duplex = true;
            break;
    }

    /* Read PHY_CTRL2 for auto MDI/MDI-X state */
    uint16_t ctrl2 = 0;
    if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_PHY_CTRL2, &ctrl2)) {
        status->auto_mdix = false;
    } else {
        status->auto_mdix = (ctrl2 & KSZ8081_CTRL2_HP_MDIX) != 0;
    }

    return true;
}

bool drv_ksz8081_soft_reset(uint8_t phy_addr)
{
    /* Write BCR.RESET bit -- this bit self-clears when reset completes */
    uint16_t bcr = 0;

    if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_BCR, &bcr)) {
        return false;
    }

    bcr |= KSZ8081_BCR_RESET;

    if (!hal_eth_mdio_write(phy_addr, KSZ8081_REG_BCR, bcr)) {
        return false;
    }

    /* Poll BCR until RESET bit clears (timeout ~500 ms) */
    uint32_t timeout = KSZ8081_RESET_TIMEOUT_CYCLES;

    while (timeout--) {
        if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_BCR, &bcr)) {
            return false;
        }

        if (!(bcr & KSZ8081_BCR_RESET)) {
            /* Reset complete -- allow PHY to stabilize */
            delay_us(1000);
            return true;
        }

        delay_us(1);
    }

    return false;  /* Timeout: RESET bit did not self-clear */
}

bool drv_ksz8081_enable_auto_negotiation(uint8_t phy_addr)
{
    /* Configure advertisement: 10/100 Mbps, half and full duplex */
    uint16_t anar = KSZ8081_ANAR_SELECTOR_IEEE |
                    KSZ8081_ANAR_10BASE_T_HD   |
                    KSZ8081_ANAR_10BASE_T_FD   |
                    KSZ8081_ANAR_100BASE_TX_HD |
                    KSZ8081_ANAR_100BASE_TX_FD;

    if (!hal_eth_mdio_write(phy_addr, KSZ8081_REG_ANAR, anar)) {
        return false;
    }

    /* Enable auto-negotiation and restart the process */
    uint16_t bcr = 0;

    if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_BCR, &bcr)) {
        return false;
    }

    bcr |= KSZ8081_BCR_AN_ENABLE | KSZ8081_BCR_RESTART_AN;

    if (!hal_eth_mdio_write(phy_addr, KSZ8081_REG_BCR, bcr)) {
        return false;
    }

    return true;
}

bool drv_ksz8081_read_id(uint8_t phy_addr, uint16_t *id1, uint16_t *id2)
{
    if (!id1 || !id2) {
        return false;
    }

    if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_PHYID1, id1)) {
        return false;
    }

    if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_PHYID2, id2)) {
        return false;
    }

    return true;
}

bool drv_ksz8081_set_loopback(uint8_t phy_addr, bool enable)
{
    uint16_t bcr = 0;

    if (!hal_eth_mdio_read(phy_addr, KSZ8081_REG_BCR, &bcr)) {
        return false;
    }

    if (enable) {
        bcr |= KSZ8081_BCR_LOOPBACK;
    } else {
        bcr &= ~KSZ8081_BCR_LOOPBACK;
    }

    if (!hal_eth_mdio_write(phy_addr, KSZ8081_REG_BCR, bcr)) {
        return false;
    }

    return true;
}
