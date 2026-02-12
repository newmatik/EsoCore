/**
 * @file drv_ksz8081.h
 * @brief KSZ8081RNACA Ethernet PHY Driver for EsoCore Edge
 *
 * Driver for the Microchip KSZ8081RNACA 10/100 Ethernet PHY used on both
 * Ethernet ports of the EsoCore Edge board. Communicates with the PHY via
 * MDIO (SMI) through the STM32H747 Ethernet MAC HAL layer.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#ifndef ESOCORE_DRV_KSZ8081_H
#define ESOCORE_DRV_KSZ8081_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * IEEE 802.3 Standard PHY Register Addresses
 * ============================================================================ */

#define KSZ8081_REG_BCR         0x00    /* Basic Control Register */
#define KSZ8081_REG_BSR         0x01    /* Basic Status Register */
#define KSZ8081_REG_PHYID1      0x02    /* PHY Identifier 1 */
#define KSZ8081_REG_PHYID2      0x03    /* PHY Identifier 2 */
#define KSZ8081_REG_ANAR        0x04    /* Auto-Negotiation Advertisement Register */
#define KSZ8081_REG_ANLPAR      0x05    /* Auto-Negotiation Link Partner Ability */
#define KSZ8081_REG_PHY_CTRL1   0x1E    /* PHY Control 1 (vendor-specific) */
#define KSZ8081_REG_PHY_CTRL2   0x1F    /* PHY Control 2 (vendor-specific) */

/* ============================================================================
 * BCR (Basic Control Register, 0x00) Bit Definitions
 * ============================================================================ */

#define KSZ8081_BCR_RESET           (1U << 15)  /* Soft reset, self-clearing */
#define KSZ8081_BCR_LOOPBACK        (1U << 14)  /* Enable loopback mode */
#define KSZ8081_BCR_SPEED_SELECT    (1U << 13)  /* 1=100 Mbps, 0=10 Mbps */
#define KSZ8081_BCR_AN_ENABLE       (1U << 12)  /* Enable auto-negotiation */
#define KSZ8081_BCR_POWER_DOWN      (1U << 11)  /* Power-down mode */
#define KSZ8081_BCR_ISOLATE         (1U << 10)  /* Electrically isolate PHY */
#define KSZ8081_BCR_RESTART_AN      (1U << 9)   /* Restart auto-negotiation */
#define KSZ8081_BCR_DUPLEX          (1U << 8)   /* 1=full duplex, 0=half */

/* ============================================================================
 * BSR (Basic Status Register, 0x01) Bit Definitions
 * ============================================================================ */

#define KSZ8081_BSR_AN_COMPLETE     (1U << 5)   /* Auto-negotiation complete */
#define KSZ8081_BSR_LINK_STATUS     (1U << 2)   /* Link is up */

/* ============================================================================
 * ANAR (Auto-Negotiation Advertisement Register, 0x04) Bit Definitions
 * ============================================================================ */

#define KSZ8081_ANAR_100BASE_TX_FD  (1U << 8)   /* Advertise 100BASE-TX full duplex */
#define KSZ8081_ANAR_100BASE_TX_HD  (1U << 7)   /* Advertise 100BASE-TX half duplex */
#define KSZ8081_ANAR_10BASE_T_FD    (1U << 6)   /* Advertise 10BASE-T full duplex */
#define KSZ8081_ANAR_10BASE_T_HD    (1U << 5)   /* Advertise 10BASE-T half duplex */
#define KSZ8081_ANAR_SELECTOR_IEEE  0x0001       /* IEEE 802.3 selector field */

/* ============================================================================
 * PHY_CTRL1 (0x1E) -- Operation Mode Indication
 * Bits [2:0] indicate negotiated speed/duplex after link-up
 * ============================================================================ */

#define KSZ8081_CTRL1_OP_MODE_MASK  0x0007
#define KSZ8081_CTRL1_OP_10M_HD    0x01   /* 10BASE-T half duplex */
#define KSZ8081_CTRL1_OP_10M_FD    0x05   /* 10BASE-T full duplex */
#define KSZ8081_CTRL1_OP_100M_HD   0x02   /* 100BASE-TX half duplex */
#define KSZ8081_CTRL1_OP_100M_FD   0x06   /* 100BASE-TX full duplex */

/* ============================================================================
 * PHY_CTRL2 (0x1F) -- Speed/Duplex and MDI/MDI-X Status
 * ============================================================================ */

#define KSZ8081_CTRL2_HP_MDIX      (1U << 15)  /* HP Auto MDI/MDI-X mode */
#define KSZ8081_CTRL2_MDIX_SEL     (1U << 14)  /* MDI-X select */
#define KSZ8081_CTRL2_PAIR_SWAP    (1U << 13)  /* Pair swap disable */
#define KSZ8081_CTRL2_FORCE_LINK   (1U << 11)  /* Force link pass */
#define KSZ8081_CTRL2_POWER_SAVE   (1U << 10)  /* Power saving enable */
#define KSZ8081_CTRL2_INT_LEVEL    (1U << 9)   /* Interrupt level */
#define KSZ8081_CTRL2_JABBER_EN    (1U << 8)   /* Jabber counter enable */
#define KSZ8081_CTRL2_RMII_REF_CLK (1U << 7)   /* RMII reference clock select */
#define KSZ8081_CTRL2_LED_MODE     (1U << 4)   /* LED mode */
#define KSZ8081_CTRL2_AUTO_MDIX_EN (1U << 14)  /* Auto MDI/MDI-X enable */

/* ============================================================================
 * KSZ8081RNACA Expected PHY Identifier Values
 * ============================================================================ */

#define KSZ8081_PHY_ID1             0x0022  /* Microchip OUI upper */
#define KSZ8081_PHY_ID2             0x1560  /* KSZ8081RNACA model/revision */
#define KSZ8081_PHY_ID2_MASK        0xFFF0  /* Mask revision nibble */

/* ============================================================================
 * Data Structures
 * ============================================================================ */

/** Configuration for KSZ8081 PHY instance */
typedef struct {
    uint8_t phy_addr;   /**< MDIO bus address (0 or 1 on EsoCore Edge) */
} ksz8081_config_t;

/** PHY link status */
typedef struct {
    bool link_up;       /**< Link is established */
    bool speed_100m;    /**< true=100 Mbps, false=10 Mbps */
    bool full_duplex;   /**< true=full duplex, false=half duplex */
    bool auto_mdix;     /**< Auto MDI/MDI-X is active */
} ksz8081_status_t;

/* ============================================================================
 * Public API
 * ============================================================================ */

/**
 * @brief Initialize the KSZ8081 PHY
 *
 * Performs a soft reset, verifies the PHY identifier, enables auto-negotiation,
 * and configures auto MDI/MDI-X.
 *
 * @param config PHY configuration (MDIO address)
 * @return true on success, false on ID mismatch or timeout
 */
bool drv_ksz8081_init(const ksz8081_config_t *config);

/**
 * @brief Read current PHY link status
 *
 * @param phy_addr MDIO address of the PHY
 * @param status   Pointer to status structure to fill
 * @return true on success, false on MDIO communication error
 */
bool drv_ksz8081_get_status(uint8_t phy_addr, ksz8081_status_t *status);

/**
 * @brief Perform a PHY soft reset
 *
 * Writes the RESET bit in BCR and waits for it to self-clear (up to 500 ms).
 *
 * @param phy_addr MDIO address of the PHY
 * @return true if reset completed, false on timeout
 */
bool drv_ksz8081_soft_reset(uint8_t phy_addr);

/**
 * @brief Enable auto-negotiation
 *
 * Configures the ANAR to advertise 10/100 half/full duplex and enables
 * auto-negotiation in the BCR.
 *
 * @param phy_addr MDIO address of the PHY
 * @return true on success
 */
bool drv_ksz8081_enable_auto_negotiation(uint8_t phy_addr);

/**
 * @brief Read PHY identifier registers
 *
 * @param phy_addr MDIO address of the PHY
 * @param id1      Pointer to receive PHYID1 value
 * @param id2      Pointer to receive PHYID2 value
 * @return true on success
 */
bool drv_ksz8081_read_id(uint8_t phy_addr, uint16_t *id1, uint16_t *id2);

/**
 * @brief Enable or disable loopback mode
 *
 * @param phy_addr MDIO address of the PHY
 * @param enable   true to enable loopback, false to disable
 * @return true on success
 */
bool drv_ksz8081_set_loopback(uint8_t phy_addr, bool enable);

#ifdef __cplusplus
}
#endif

#endif /* ESOCORE_DRV_KSZ8081_H */
