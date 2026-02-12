/**
 * @file hal_eth.c
 * @brief Dual Ethernet MAC HAL Implementation for STM32H747
 *
 * The STM32H747 has one Ethernet MAC peripheral. For dual-port operation
 * with two independent KSZ8081RNACA PHYs, we use the single MAC with
 * multiplexed PHY access via MDIO. True dual-MAC requires external MAC
 * or software-level port separation (the preferred approach for v1.5.0.0
 * uses lwIP with two netif interfaces sharing one MAC, with VLAN or
 * MAC-address-based filtering).
 *
 * Note: If the PCB routes two independent RMII interfaces, one to the
 * internal MAC and one to an external MAC (e.g., via SPI Ethernet),
 * this driver would manage the internal MAC for Port A, and a separate
 * SPI-based Ethernet driver would handle Port B.
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "hal_eth.h"
#include "hal_gpio.h"
#include "../bsp/pin_mapping_v150.h"
#include "../bsp/system_clock.h"
#include "../cmsis/stm32h747xx.h"

/* Ethernet DMA descriptors -- must be in non-cacheable SRAM or use cache management */
typedef struct {
    volatile uint32_t DESC0;
    volatile uint32_t DESC1;
    volatile uint32_t DESC2;
    volatile uint32_t DESC3;
} eth_dma_desc_t;

/* Descriptor and buffer storage (aligned to 32-byte boundary for DMA) */
static eth_dma_desc_t tx_desc[ETH_TX_DESC_COUNT] __attribute__((aligned(32)));
static eth_dma_desc_t rx_desc[ETH_RX_DESC_COUNT] __attribute__((aligned(32)));
static uint8_t rx_buffers[ETH_RX_DESC_COUNT][ETH_RX_BUF_SIZE] __attribute__((aligned(32)));

/* Current descriptor indices */
static uint8_t tx_desc_idx = 0;
static uint8_t rx_desc_idx = 0;

/* Rx callback */
static hal_eth_rx_callback_t rx_callback[ETH_PORT_COUNT] = {0};
static void *rx_callback_ctx[ETH_PORT_COUNT] = {0};

/* Port configurations */
static hal_eth_config_t port_config[ETH_PORT_COUNT];
static bool port_initialized[ETH_PORT_COUNT] = {false, false};

/* ETH register shortcuts */
#define ETH_MACCR       (*(volatile uint32_t *)(ETH_BASE + 0x0000))
#define ETH_MACPFR      (*(volatile uint32_t *)(ETH_BASE + 0x0008))
#define ETH_MACMDIOAR   (*(volatile uint32_t *)(ETH_BASE + 0x0200))
#define ETH_MACMDIODR   (*(volatile uint32_t *)(ETH_BASE + 0x0204))
#define ETH_MACA0HR     (*(volatile uint32_t *)(ETH_BASE + 0x0300))
#define ETH_MACA0LR     (*(volatile uint32_t *)(ETH_BASE + 0x0304))
#define ETH_DMAMR       (*(volatile uint32_t *)(ETH_BASE + 0x1000))
#define ETH_DMASBMR     (*(volatile uint32_t *)(ETH_BASE + 0x1004))
#define ETH_DMAC0CR     (*(volatile uint32_t *)(ETH_BASE + 0x1100))
#define ETH_DMAC0TXCR   (*(volatile uint32_t *)(ETH_BASE + 0x1104))
#define ETH_DMAC0RXCR   (*(volatile uint32_t *)(ETH_BASE + 0x1108))
#define ETH_DMAC0TXDLAR (*(volatile uint32_t *)(ETH_BASE + 0x1114))
#define ETH_DMAC0RXDLAR (*(volatile uint32_t *)(ETH_BASE + 0x111C))
#define ETH_DMAC0TXRLR  (*(volatile uint32_t *)(ETH_BASE + 0x112C))
#define ETH_DMAC0RXRLR  (*(volatile uint32_t *)(ETH_BASE + 0x1130))
#define ETH_DMAC0SR     (*(volatile uint32_t *)(ETH_BASE + 0x1160))

/* MDIO clock divider (HCLK/102 for 240 MHz -> ~2.35 MHz, within 2.5 MHz limit) */
#define MDIO_CR_DIV     4  /* CSR clock range 150-250 MHz */

static void eth_init_gpio_port_a(void)
{
    /* Configure RMII pins for Ethernet Port A */
    hal_gpio_init_af(ETH1_RMII_REF_CLK, ETH1_RMII_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(ETH1_RMII_MDIO,    ETH1_RMII_AF, GPIO_SPEED_MEDIUM);
    hal_gpio_init_af(ETH1_RMII_CRS_DV,  ETH1_RMII_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(ETH1_RMII_MDC,     ETH1_RMII_AF, GPIO_SPEED_MEDIUM);
    hal_gpio_init_af(ETH1_RMII_RXD0,    ETH1_RMII_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(ETH1_RMII_RXD1,    ETH1_RMII_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(ETH1_RMII_TX_EN,   ETH1_RMII_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(ETH1_RMII_TXD0,    ETH1_RMII_AF, GPIO_SPEED_VERY_HIGH);
    hal_gpio_init_af(ETH1_RMII_TXD1,    ETH1_RMII_AF, GPIO_SPEED_VERY_HIGH);

    /* PHY reset and interrupt pins */
    hal_gpio_init_output(ETH1_PHY_RESET);
    hal_gpio_init_input(ETH1_PHY_INT, GPIO_PULL_UP);
}

static void eth_init_gpio_port_b(void)
{
    /* Port B PHY reset and interrupt pins */
    hal_gpio_init_output(ETH2_PHY_RESET);
    hal_gpio_init_input(ETH2_PHY_INT, GPIO_PULL_UP);

    /* Note: Port B RMII pins are configured for the second PHY.
     * Since STM32H747 has only one ETH MAC, Port B shares the MAC
     * with Port A. The second set of RMII pins would only be used
     * if an external Ethernet MAC/switch IC is present on the PCB. */
}

static void eth_reset_phy(hal_eth_port_t port)
{
    uint16_t reset_pin = (port == ETH_PORT_A) ? ETH1_PHY_RESET : ETH2_PHY_RESET;

    hal_gpio_reset(reset_pin);   /* Assert reset (active low) */
    /* Delay ~10 ms */
    volatile uint32_t delay = system_clock_get_hclk() / 100;
    while (delay--) { __asm volatile("nop"); }
    hal_gpio_set(reset_pin);     /* Release reset */
    /* Wait for PHY to stabilize */
    delay = system_clock_get_hclk() / 50;
    while (delay--) { __asm volatile("nop"); }
}

static void eth_init_dma_descriptors(void)
{
    /* Initialize TX descriptors */
    for (int i = 0; i < ETH_TX_DESC_COUNT; i++) {
        tx_desc[i].DESC0 = 0;
        tx_desc[i].DESC1 = 0;
        tx_desc[i].DESC2 = 0;
        tx_desc[i].DESC3 = 0;  /* OWN = 0 (CPU owns) */
    }

    /* Initialize RX descriptors */
    for (int i = 0; i < ETH_RX_DESC_COUNT; i++) {
        rx_desc[i].DESC0 = (uint32_t)rx_buffers[i];
        rx_desc[i].DESC1 = 0;
        rx_desc[i].DESC2 = 0;
        rx_desc[i].DESC3 = (1UL << 31) | (1UL << 30);  /* OWN=1, IOC=1 */
    }

    tx_desc_idx = 0;
    rx_desc_idx = 0;
}

bool hal_eth_init(hal_eth_port_t port, const hal_eth_config_t *config)
{
    if (port >= ETH_PORT_COUNT || !config) return false;

    port_config[port] = *config;

    if (port == ETH_PORT_A) {
        eth_init_gpio_port_a();
    } else {
        eth_init_gpio_port_b();
    }

    /* Reset PHY */
    eth_reset_phy(port);

    /* Only initialize MAC hardware for the primary port (Port A) */
    if (port == ETH_PORT_A) {
        /* Software reset ETH MAC */
        ETH_DMAMR |= (1UL << 0);  /* SWR */
        uint32_t timeout = 1000;
        while ((ETH_DMAMR & (1UL << 0)) && timeout--) { }

        /* Initialize DMA descriptors */
        eth_init_dma_descriptors();

        /* Configure DMA */
        ETH_DMAC0TXDLAR = (uint32_t)tx_desc;
        ETH_DMAC0RXDLAR = (uint32_t)rx_desc;
        ETH_DMAC0TXRLR = ETH_TX_DESC_COUNT - 1;
        ETH_DMAC0RXRLR = ETH_RX_DESC_COUNT - 1;
        ETH_DMAC0TXCR = (32UL << 16);  /* TX PBL = 32 */
        ETH_DMAC0RXCR = (32UL << 16) | ((ETH_RX_BUF_SIZE) << 1);

        /* Configure MAC */
        ETH_MACCR = (1UL << 14) |  /* FES: 100 Mbps */
                    (1UL << 13) |  /* DM: Full duplex */
                    (1UL << 0)  |  /* RE: Receiver enable */
                    (1UL << 1);    /* TE: Transmitter enable */

        /* Set MAC address */
        ETH_MACA0HR = ((uint32_t)config->mac_addr.addr[5] << 8) |
                      ((uint32_t)config->mac_addr.addr[4]);
        ETH_MACA0LR = ((uint32_t)config->mac_addr.addr[3] << 24) |
                      ((uint32_t)config->mac_addr.addr[2] << 16) |
                      ((uint32_t)config->mac_addr.addr[1] << 8) |
                      ((uint32_t)config->mac_addr.addr[0]);

        /* Configure MDIO clock */
        ETH_MACMDIOAR = (MDIO_CR_DIV << 8);  /* CSR clock divider */
    }

    port_initialized[port] = true;
    return true;
}

void hal_eth_deinit(hal_eth_port_t port)
{
    if (port >= ETH_PORT_COUNT) return;
    hal_eth_stop(port);
    port_initialized[port] = false;
}

bool hal_eth_start(hal_eth_port_t port)
{
    if (port != ETH_PORT_A || !port_initialized[port]) return false;

    /* Start DMA TX and RX */
    ETH_DMAC0TXCR |= (1UL << 0);  /* ST: Start TX */
    ETH_DMAC0RXCR |= (1UL << 0);  /* SR: Start RX */

    return true;
}

void hal_eth_stop(hal_eth_port_t port)
{
    if (port != ETH_PORT_A) return;

    ETH_DMAC0TXCR &= ~(1UL << 0);
    ETH_DMAC0RXCR &= ~(1UL << 0);
}

bool hal_eth_transmit(hal_eth_port_t port,
                      const uint8_t *data, uint16_t length)
{
    if (port != ETH_PORT_A || !data || length == 0 || length > ETH_MAX_FRAME_SIZE) {
        return false;
    }

    eth_dma_desc_t *desc = &tx_desc[tx_desc_idx];

    /* Check if descriptor is available (OWN bit must be 0) */
    if (desc->DESC3 & (1UL << 31)) {
        return false;  /* DMA still owns this descriptor */
    }

    /* Set buffer address and length */
    desc->DESC0 = (uint32_t)data;
    desc->DESC1 = 0;
    desc->DESC2 = length;  /* Buffer 1 length */
    desc->DESC3 = (1UL << 31) |  /* OWN */
                  (1UL << 29) |  /* FD: first descriptor */
                  (1UL << 28) |  /* LD: last descriptor */
                  length;        /* Packet length */

    /* Advance to next descriptor */
    tx_desc_idx = (tx_desc_idx + 1) % ETH_TX_DESC_COUNT;

    /* Issue TX poll demand by writing to tail pointer */
    ETH_DMAC0TXDLAR = (uint32_t)&tx_desc[tx_desc_idx];

    return true;
}

void hal_eth_register_rx_callback(hal_eth_port_t port,
                                  hal_eth_rx_callback_t callback,
                                  void *context)
{
    if (port < ETH_PORT_COUNT) {
        rx_callback[port] = callback;
        rx_callback_ctx[port] = context;
    }
}

bool hal_eth_mdio_read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *value)
{
    if (!value) return false;

    uint32_t timeout = 10000;

    /* Wait for MDIO not busy */
    while ((ETH_MACMDIOAR & (1UL << 0)) && timeout--) { }
    if (timeout == 0) return false;

    /* Configure read operation */
    ETH_MACMDIOAR = (MDIO_CR_DIV << 8) |
                    ((uint32_t)phy_addr << 21) |
                    ((uint32_t)reg_addr << 16) |
                    (3UL << 2) |    /* GOC: Read */
                    (1UL << 0);     /* GB: busy/start */

    /* Wait for completion */
    timeout = 10000;
    while ((ETH_MACMDIOAR & (1UL << 0)) && timeout--) { }
    if (timeout == 0) return false;

    *value = (uint16_t)(ETH_MACMDIODR & 0xFFFF);
    return true;
}

bool hal_eth_mdio_write(uint8_t phy_addr, uint8_t reg_addr, uint16_t value)
{
    uint32_t timeout = 10000;

    while ((ETH_MACMDIOAR & (1UL << 0)) && timeout--) { }
    if (timeout == 0) return false;

    ETH_MACMDIODR = (uint32_t)value;
    ETH_MACMDIOAR = (MDIO_CR_DIV << 8) |
                    ((uint32_t)phy_addr << 21) |
                    ((uint32_t)reg_addr << 16) |
                    (1UL << 2) |    /* GOC: Write */
                    (1UL << 0);     /* GB: busy/start */

    timeout = 10000;
    while ((ETH_MACMDIOAR & (1UL << 0)) && timeout--) { }

    return (timeout > 0);
}

hal_eth_link_status_t hal_eth_get_link_status(hal_eth_port_t port)
{
    if (port >= ETH_PORT_COUNT || !port_initialized[port]) {
        return ETH_LINK_DOWN;
    }

    uint16_t bsr;
    uint8_t phy_addr = port_config[port].phy_addr;

    if (!hal_eth_mdio_read(phy_addr, 0x01, &bsr)) {
        return ETH_LINK_DOWN;
    }

    if (!(bsr & (1U << 2))) {  /* Link status bit */
        return ETH_LINK_DOWN;
    }

    /* Read PHY-specific status register for speed/duplex */
    uint16_t ctrl;
    if (!hal_eth_mdio_read(phy_addr, 0x1F, &ctrl)) {
        return ETH_LINK_UP_100M_FD;  /* Assume best case */
    }

    /* KSZ8081 PHY Control 1 register (0x1E) bits [2:0] for speed/duplex */
    uint8_t speed_duplex = ctrl & 0x07;
    switch (speed_duplex) {
        case 1: return ETH_LINK_UP_10M_HD;
        case 5: return ETH_LINK_UP_10M_FD;
        case 2: return ETH_LINK_UP_100M_HD;
        case 6: return ETH_LINK_UP_100M_FD;
        default: return ETH_LINK_UP_100M_FD;
    }
}

void hal_eth_set_mac_addr(hal_eth_port_t port, const hal_eth_mac_addr_t *addr)
{
    if (!addr) return;
    port_config[port].mac_addr = *addr;

    if (port == ETH_PORT_A) {
        ETH_MACA0HR = ((uint32_t)addr->addr[5] << 8) | addr->addr[4];
        ETH_MACA0LR = ((uint32_t)addr->addr[3] << 24) |
                      ((uint32_t)addr->addr[2] << 16) |
                      ((uint32_t)addr->addr[1] << 8) |
                      addr->addr[0];
    }
}

void hal_eth_poll(hal_eth_port_t port)
{
    if (port != ETH_PORT_A) return;

    while (1) {
        eth_dma_desc_t *desc = &rx_desc[rx_desc_idx];

        /* Check if DMA has released this descriptor */
        if (desc->DESC3 & (1UL << 31)) {
            break;  /* Still owned by DMA */
        }

        /* Extract frame length */
        uint16_t length = (uint16_t)(desc->DESC3 & 0x7FFF);

        if (length > 0 && length <= ETH_RX_BUF_SIZE && rx_callback[port]) {
            rx_callback[port](port, rx_buffers[rx_desc_idx], length,
                             rx_callback_ctx[port]);
        }

        /* Return descriptor to DMA */
        desc->DESC0 = (uint32_t)rx_buffers[rx_desc_idx];
        desc->DESC3 = (1UL << 31) | (1UL << 30);

        rx_desc_idx = (rx_desc_idx + 1) % ETH_RX_DESC_COUNT;
    }
}
