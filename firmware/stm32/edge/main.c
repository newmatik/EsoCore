/**
 * @file main.c
 * @brief EsoCore Edge Device v1.5.0.0 Main Application
 *
 * Complete system initialization and runtime loop for the EsoCore Edge device
 * targeting the STM32H747IIT6 (Cortex-M7). Integrates all v1.5.0.0 subsystems:
 *
 *   - 24V DC power management with brownout detection and supercap backup
 *   - Dual Ethernet (Port A: Machine/OT, Port B: IT/Cloud)
 *   - 4-channel dual-mode analog inputs (IEPE + DC, 24-bit, 50 kSPS)
 *   - 6 dual-channel safety digital inputs (EN ISO 13849 Cat. 3)
 *   - RS-485 sensor bus with auto-discovery
 *   - Modbus RTU, PROFIBUS DP, Interbus fieldbus interfaces
 *   - ESP32-S3 WiFi/BT via UART AT commands
 *   - MicroSD storage, OLED display, TinyML edge intelligence
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* BSP and HAL */
#include "../../stm32/stm32h7/bsp/bsp_edge_v150.h"
#include "../../stm32/stm32h7/bsp/system_clock.h"
#include "../../stm32/stm32h7/cmsis/stm32h747xx.h"
#include "../../stm32/stm32h7/cmsis/system_stm32h7xx.h"
#include "../../stm32/stm32h7/hal/hal_gpio.h"
#include "../../stm32/stm32h7/hal/hal_spi.h"
#include "../../stm32/stm32h7/hal/hal_uart.h"
#include "../../stm32/stm32h7/hal/hal_i2c.h"
#include "../../stm32/stm32h7/hal/hal_eth.h"
#include "../../stm32/stm32h7/hal/hal_dma.h"
#include "../../stm32/stm32h7/hal/hal_timer.h"
#include "../../stm32/stm32h7/hal/hal_adc.h"

/* Hardware drivers */
#include "../../stm32/stm32h7/drivers/drv_ksz8081.h"
#include "../../stm32/stm32h7/drivers/drv_ads1274.h"
#include "../../stm32/stm32h7/drivers/drv_pga280.h"
#include "../../stm32/stm32h7/drivers/drv_tmux1101.h"
#include "../../stm32/stm32h7/drivers/drv_iepe.h"
#include "../../stm32/stm32h7/drivers/drv_ssr.h"

/* Subsystem modules */
#include "../../common/communication/ethernet_manager.h"
#include "../../common/analog/analog_input.h"
#include "../../common/communication/interbus.h"
#include "../../common/communication/sensor_bus.h"

/* Existing common modules */
#include "../../common/management/power_management.h"
#include "../../common/safety/safety_io.h"
#include "../../common/communication/wifi_manager.h"
#include "../../common/communication/http_client.h"
#include "../../common/communication/modbus_rtu.h"
#include "../../common/communication/protocol.h"
#include "../../common/storage/storage_system.h"
#include "../../common/management/config_manager.h"
#include "../../common/intelligence/event_system.h"
#include "../../common/intelligence/tinyml_engine.h"
#include "../../common/ui/oled_display.h"

/* ============================================================================
 * Firmware Version
 * ============================================================================ */

#define FW_VERSION_STRING   ESOCORE_FW_VERSION_STRING

/* ============================================================================
 * Timing Configuration (all in milliseconds)
 * ============================================================================ */

#define POWER_POLL_INTERVAL_MS      100     /* Power rail monitoring */
#define SAFETY_POLL_INTERVAL_MS     10      /* Safety inputs (fast for response time) */
#define ETHERNET_POLL_INTERVAL_MS   1       /* Ethernet RX polling */
#define SENSOR_BUS_POLL_INTERVAL_MS 10      /* RS-485 sensor bus */
#define INTERBUS_POLL_INTERVAL_MS   5       /* Interbus cyclic */
#define DISPLAY_UPDATE_INTERVAL_MS  2000    /* OLED update */
#define HEARTBEAT_INTERVAL_MS       30000   /* Cloud heartbeat */
#define TELEMETRY_INTERVAL_MS       5000    /* Cloud telemetry */
#define OTA_CHECK_INTERVAL_MS       3600000 /* OTA check (1 hour) */
#define WATCHDOG_FEED_INTERVAL_MS   500     /* IWDG feed */

/* Device identity */
#define DEVICE_ADDRESS              0x01
#define DEVICE_TYPE                 0x01    /* Edge device */

/* ============================================================================
 * Global SPI Handles (shared between drivers and HAL)
 * ============================================================================ */

hal_spi_handle_t g_spi3_handle;   /* ADS1274 ADC */
hal_spi_handle_t g_spi4_handle;   /* PGA280 amplifiers */

/* ============================================================================
 * Global State
 * ============================================================================ */

static volatile bool system_running = false;

/* Timing counters */
static uint32_t last_power_poll = 0;
static uint32_t last_safety_poll = 0;
static uint32_t last_ethernet_poll = 0;
static uint32_t last_sensor_bus_poll = 0;
static uint32_t last_interbus_poll = 0;
static uint32_t last_display_update = 0;
static uint32_t last_heartbeat = 0;
static uint32_t last_telemetry = 0;
static uint32_t last_watchdog_feed = 0;

/* Forward declarations */
extern void hal_gpio_init_all(void);
static bool init_hal_peripherals(void);
static bool init_subsystems(void);
static void runtime_loop(void);
static void graceful_shutdown(void);
static void feed_watchdog(void);

/* ============================================================================
 * IWDG (Independent Watchdog) Control
 * ============================================================================ */

#define IWDG_BASE       0x58004800UL
#define IWDG_KR         (*(volatile uint32_t *)(IWDG_BASE + 0x00))
#define IWDG_PR         (*(volatile uint32_t *)(IWDG_BASE + 0x04))
#define IWDG_RLR        (*(volatile uint32_t *)(IWDG_BASE + 0x08))
#define IWDG_SR         (*(volatile uint32_t *)(IWDG_BASE + 0x0C))

static void iwdg_init(uint32_t timeout_ms)
{
    /* LSI ~32 kHz, prescaler /64 -> 500 Hz counter */
    IWDG_KR = 0x5555;  /* Enable register access */
    IWDG_PR = 4;       /* Prescaler /64 */
    IWDG_RLR = (timeout_ms * 500) / 1000;  /* Reload value */
    IWDG_KR = 0xCCCC;  /* Start watchdog */
}

static void feed_watchdog(void)
{
    IWDG_KR = 0xAAAA;  /* Reload counter */
}

/* ============================================================================
 * Interrupt Handlers
 * ============================================================================ */

/**
 * @brief SysTick interrupt -- 1 ms tick
 */
void SysTick_Handler(void)
{
    system_clock_tick_increment();
}

/**
 * @brief Hard fault handler -- enter safe state and reset
 */
void HardFault_Handler(void)
{
    /* De-energize all safety outputs immediately */
    hal_gpio_reset(SAFETY_RELAY_CTRL);
    hal_gpio_reset(SSR_CTRL);

    /* Log fault (best effort) */
    /* TODO: Write fault info to backup SRAM for post-mortem */

    /* System reset */
    NVIC_SystemReset();
}

/**
 * @brief DMA1 Stream 0 -- ADS1274 SPI RX complete
 */
void DMA1_Stream0_IRQHandler(void)
{
    /* TODO: Handle ADC DMA transfer complete, invoke buffer swap */
}

/**
 * @brief Ethernet IRQ handler
 */
void ETH_IRQHandler(void)
{
    /* TODO: Handle Ethernet MAC interrupt */
}

/* ============================================================================
 * HAL Peripheral Initialization
 * ============================================================================ */

static bool init_hal_peripherals(void)
{
    /* SPI3: ADS1274 ADC (16 MHz, mode 1, 8-bit for byte-level DMA) */
    hal_spi_config_t spi3_cfg = {
        .instance    = SPI3,
        .mode        = SPI_MODE_1,
        .data_size   = SPI_DATA_8BIT,
        .prescaler   = 8,   /* 120 MHz / 8 = 15 MHz (close to 16 MHz target) */
        .msb_first   = true,
        .software_cs = true,
    };
    if (!hal_spi_init(&g_spi3_handle, &spi3_cfg)) return false;

    /* SPI4: PGA280 amplifiers (5 MHz, mode 0, 8-bit) */
    hal_spi_config_t spi4_cfg = {
        .instance    = SPI4,
        .mode        = SPI_MODE_0,
        .data_size   = SPI_DATA_8BIT,
        .prescaler   = 32,  /* 120 MHz / 32 = 3.75 MHz (under 5 MHz target) */
        .msb_first   = true,
        .software_cs = true,
    };
    if (!hal_spi_init(&g_spi4_handle, &spi4_cfg)) return false;

    /* Internal ADC for power monitoring */
    if (!hal_adc_init()) return false;

    return true;
}

/* ============================================================================
 * Subsystem Initialization
 * ============================================================================ */

static bool init_subsystems(void)
{
    /* --- Power Management (must be first, checks all rails) --- */
    esocore_power_config_t pwr_cfg = {
        .brownout_warning_mv   = ESOCORE_RAIL_24V_BROWNOUT_MV,
        .brownout_shutdown_mv  = ESOCORE_RAIL_24V_SHUTDOWN_MV,
        .overvoltage_mv        = ESOCORE_RAIL_24V_MAX_MV + 500,
        .temperature_limit_c   = 70,
        .brownout_holdoff_ms   = 50,
        .enable_supercap_backup = true,
        .enable_analog_sequencing = true,
    };
    if (!esocore_power_init(&pwr_cfg)) return false;

    /* Power-up sequencing: wait for all rails to stabilize */
    if (!esocore_power_sequence_up()) return false;

    /* --- Event System (needed for logging) --- */
    esocore_event_init(NULL);

    /* --- Safety I/O --- */
    safety_system_config_t safety_cfg = {
        .watchdog_timeout_ms        = SAFETY_WATCHDOG_TIMEOUT,
        .test_pulse_interval_ms     = SAFETY_TEST_PULSE_INTERVAL,
        .fault_reset_time_ms        = SAFETY_FAULT_RESET_TIME,
        .enable_cross_monitoring    = true,
        .enable_safe_state_enforcement = true,
        .safety_category            = 3,
        .system_description         = "EsoCore Edge v1.5.0 Safety I/O",
    };
    if (!safety_io_init(&safety_cfg)) return false;

    /* --- Dual Ethernet --- */
#if ESOCORE_FEATURE_DUAL_ETHERNET
    ethm_config_t eth_cfg = {
        .port_config = {
            [ETHM_PORT_A] = {
                .role     = ETHM_ROLE_MACHINE,
                .phy_addr = ETH1_PHY_ADDR,
                .mac_addr = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x01},
                .ip_config = {
                    .use_dhcp = false,
                    .ip_addr  = {192, 168, 10, 100},
                    .netmask  = {255, 255, 255, 0},
                    .gateway  = {192, 168, 10, 1},
                },
            },
            [ETHM_PORT_B] = {
                .role     = ETHM_ROLE_IT,
                .phy_addr = ETH2_PHY_ADDR,
                .mac_addr = {0x00, 0x80, 0xE1, 0x00, 0x00, 0x02},
                .ip_config = {
                    .use_dhcp = true,
                },
            },
        },
    };
    ethm_init(&eth_cfg);
#endif

    /* --- Analog Input Subsystem --- */
#if ESOCORE_FEATURE_ANALOG_INPUTS
    analog_config_t analog_cfg = {
        .sample_rate_hz    = ESOCORE_ADC_SAMPLE_RATE_HZ,
        .samples_per_block = 256,
        .channel_config    = {
            [0] = { .mode = ANALOG_MODE_IEPE, .gain = ANALOG_GAIN_1X,
                     .trigger_mode = ANALOG_TRIGGER_CONTINUOUS },
            [1] = { .mode = ANALOG_MODE_IEPE, .gain = ANALOG_GAIN_1X,
                     .trigger_mode = ANALOG_TRIGGER_CONTINUOUS },
            [2] = { .mode = ANALOG_MODE_DC,   .gain = ANALOG_GAIN_1X,
                     .trigger_mode = ANALOG_TRIGGER_CONTINUOUS },
            [3] = { .mode = ANALOG_MODE_DC,   .gain = ANALOG_GAIN_1X,
                     .trigger_mode = ANALOG_TRIGGER_CONTINUOUS },
        },
    };
    analog_input_init(&analog_cfg);
#endif

    /* --- RS-485 Sensor Bus --- */
#if ESOCORE_FEATURE_SENSOR_BUS
    sensor_bus_config_t sbus_cfg = {
        .baudrate            = ESOCORE_SBUS_BAUDRATE,
        .enable_termination  = true,
        .enable_12v_power    = true,
        .discovery_interval_ms = 5000,
        .max_sensors         = 32,
    };
    sensor_bus_init(&sbus_cfg);
#endif

    /* --- Interbus --- */
#if ESOCORE_FEATURE_INTERBUS
    interbus_config_t ibus_cfg = {
        .baudrate           = ESOCORE_INTERBUS_BAUDRATE,
        .node_address       = 1,
        .enable_termination = true,
        .cycle_time_ms      = 5,
    };
    interbus_init(&ibus_cfg);
#endif

    /* --- WiFi (ESP32-S3 via USART1) --- */
#if ESOCORE_FEATURE_WIFI_ESP32
    wifi_config_t wifi_cfg = {
        .mode         = WIFI_MODE_STATION,
        .dhcp_enabled = true,
        .max_retries  = 5,
    };
    wifi_manager_init(&wifi_cfg);
#endif

    /* --- Storage (microSD via SDMMC1) --- */
#if ESOCORE_FEATURE_MICROSD
    storage_system_init();
#endif

    /* --- TinyML Engine --- */
#if ESOCORE_FEATURE_TINYML
    tinyml_engine_init();
#endif

    /* --- OLED Display --- */
#if ESOCORE_FEATURE_OLED_DISPLAY
    oled_display_init();
    oled_display_status_screen("EsoCore Edge", FW_VERSION_STRING, "Booting...", 0);
#endif

    /* --- Protocol (RS-485 sensor communication) --- */
    esocore_protocol_init(DEVICE_ADDRESS, DEVICE_TYPE);

    return true;
}

/* ============================================================================
 * Runtime Loop
 * ============================================================================ */

static void runtime_loop(void)
{
    system_running = true;

    /* Start continuous acquisitions */
#if ESOCORE_FEATURE_ANALOG_INPUTS
    analog_input_start_acquisition();
#endif
#if ESOCORE_FEATURE_SENSOR_BUS
    sensor_bus_start();
#endif
#if ESOCORE_FEATURE_INTERBUS
    interbus_start();
#endif

    /* Main superloop */
    while (system_running) {
        uint32_t now = system_clock_get_tick();

        /* --- Power monitoring (100 Hz) --- */
        if (now - last_power_poll >= POWER_POLL_INTERVAL_MS) {
            esocore_power_poll();
            last_power_poll = now;

            /* Check for brownout -> initiate graceful shutdown */
            if (esocore_power_get_state() == ESOCORE_POWER_STATE_CRITICAL) {
                if (esocore_supercap_is_shutdown_safe()) {
                    graceful_shutdown();
                    return;
                }
            }
        }

        /* --- Safety I/O (100 Hz, <10 ms response requirement) --- */
        if (now - last_safety_poll >= SAFETY_POLL_INTERVAL_MS) {
            /* Safety polling is handled by safety_io internal test pulses.
             * Here we check for faults and react. */
            safety_system_status_t safety_status;
            if (safety_io_get_system_status(&safety_status)) {
                if (safety_status.fault_code != SAFETY_FAULT_NONE) {
                    esocore_event_log(ESOCORE_EVENT_SAFETY_FAULT,
                                     ESOCORE_EVENT_SEVERITY_CRITICAL,
                                     (uint8_t *)"Safety fault detected", 0);
                    /* Safety module auto-enters safe state */
                }
            }
            last_safety_poll = now;
        }

        /* --- Ethernet polling (1 kHz) --- */
#if ESOCORE_FEATURE_DUAL_ETHERNET
        if (now - last_ethernet_poll >= ETHERNET_POLL_INTERVAL_MS) {
            ethm_poll();
            last_ethernet_poll = now;
        }
#endif

        /* --- Sensor bus polling (100 Hz) --- */
#if ESOCORE_FEATURE_SENSOR_BUS
        if (now - last_sensor_bus_poll >= SENSOR_BUS_POLL_INTERVAL_MS) {
            sensor_bus_poll();
            last_sensor_bus_poll = now;
        }
#endif

        /* --- Interbus polling (200 Hz) --- */
#if ESOCORE_FEATURE_INTERBUS
        if (now - last_interbus_poll >= INTERBUS_POLL_INTERVAL_MS) {
            interbus_poll();
            last_interbus_poll = now;
        }
#endif

        /* --- Display update (0.5 Hz) --- */
#if ESOCORE_FEATURE_OLED_DISPLAY
        if (now - last_display_update >= DISPLAY_UPDATE_INTERVAL_MS) {
            esocore_power_status_t pwr_status;
            esocore_power_get_status(&pwr_status);

            oled_display_status_screen(
                "EsoCore Edge",
                FW_VERSION_STRING,
                (ethm_get_link_status(ETHM_PORT_A) == ETHM_LINK_UP) ? "ETH:OK" : "ETH:--",
                pwr_status.uptime_seconds);
            last_display_update = now;
        }
#endif

        /* --- Cloud heartbeat (via Port B / WiFi) --- */
        if (now - last_heartbeat >= HEARTBEAT_INTERVAL_MS) {
            /* TODO: Send heartbeat via IT network (Ethernet Port B or WiFi) */
            last_heartbeat = now;
        }

        /* --- Telemetry upload --- */
        if (now - last_telemetry >= TELEMETRY_INTERVAL_MS) {
            /* TODO: Collect and send telemetry data */
            last_telemetry = now;
        }

        /* --- Process incoming protocol messages --- */
        esocore_message_t msg;
        if (esocore_protocol_receive_message(&msg, 0)) {
            esocore_protocol_handle_message(&msg);
        }

        /* --- Flush event buffer --- */
        esocore_event_flush_buffer();

        /* --- Feed watchdog --- */
        if (now - last_watchdog_feed >= WATCHDOG_FEED_INTERVAL_MS) {
            feed_watchdog();
            last_watchdog_feed = now;
        }
    }
}

/* ============================================================================
 * Graceful Shutdown
 * ============================================================================ */

static void graceful_shutdown(void)
{
    system_running = false;

    /* Log shutdown event */
    esocore_event_log(ESOCORE_EVENT_SYSTEM_SHUTDOWN, ESOCORE_EVENT_SEVERITY_INFO,
                     (uint8_t *)"Graceful shutdown initiated", 0);

    /* Stop acquisitions */
#if ESOCORE_FEATURE_ANALOG_INPUTS
    analog_input_stop_acquisition();
#endif
#if ESOCORE_FEATURE_SENSOR_BUS
    sensor_bus_stop();
#endif
#if ESOCORE_FEATURE_INTERBUS
    interbus_stop();
#endif

    /* Enter safety mode (de-energize all outputs) */
    safety_io_enter_safety_mode();

    /* Flush events and data to microSD */
    esocore_event_flush_buffer();
    /* TODO: storage_system_flush() */

    /* Display shutdown message */
#if ESOCORE_FEATURE_OLED_DISPLAY
    oled_display_status_screen("EsoCore Edge", FW_VERSION_STRING, "SHUTDOWN", 0);
#endif

    /* Shut down subsystems in reverse order */
#if ESOCORE_FEATURE_OLED_DISPLAY
    oled_display_deinit();
#endif
    tinyml_engine_deinit();
#if ESOCORE_FEATURE_INTERBUS
    interbus_deinit();
#endif
#if ESOCORE_FEATURE_SENSOR_BUS
    sensor_bus_deinit();
#endif
#if ESOCORE_FEATURE_DUAL_ETHERNET
    ethm_deinit();
#endif
    analog_input_deinit();
    safety_io_deinit();

    /* Power down sequence */
    esocore_power_sequence_down();
    esocore_power_deinit();
}

/* ============================================================================
 * Main Entry Point
 * ============================================================================ */

/**
 * @brief Application entry point (called by Reset_Handler after SystemInit)
 *
 * Initialization sequence for EsoCore Edge v1.5.0.0:
 *   1. System clock configuration (480 MHz via PLL1)
 *   2. SysTick for 1 ms timing
 *   3. GPIO initialization (all pins per v1.5.0.0 mapping)
 *   4. HAL peripheral initialization (SPI, UART, I2C, etc.)
 *   5. Power rail sequencing and verification
 *   6. Subsystem initialization (Ethernet, analog, safety, fieldbus, ...)
 *   7. Watchdog start
 *   8. Enter runtime superloop
 */
int main(void)
{
    /* Step 1: Configure system clocks (25 MHz HSE -> 480 MHz SYSCLK) */
    if (!system_clock_init()) {
        /* Clock failure: system cannot operate. Hang with error LED. */
        /* Note: GPIO clocks may not be running; best effort */
        while (1) { __asm volatile("nop"); }
    }

    /* Update CMSIS SystemCoreClock variable */
    SystemCoreClockUpdate();

    /* Step 2: Start SysTick for 1 ms timing */
    system_clock_init_systick();

    /* Step 3: Initialize all GPIO pins per v1.5.0.0 board mapping */
    hal_gpio_init_all();

    /* Indicate boot in progress */
    hal_gpio_set(LED_STATUS_GREEN);

    /* Step 4: Initialize HAL peripherals (SPI, ADC) */
    if (!init_hal_peripherals()) {
        hal_gpio_set(LED_STATUS_RED);
        while (1) { bsp_delay_ms(1000); }
    }

    /* Step 5+6: Initialize all subsystems */
    if (!init_subsystems()) {
        hal_gpio_set(LED_STATUS_RED);
        esocore_event_log(ESOCORE_EVENT_SYSTEM_STARTUP, ESOCORE_EVENT_SEVERITY_CRITICAL,
                         (uint8_t *)"Subsystem initialization failed", 0);
        while (1) { bsp_delay_ms(1000); }
    }

    /* Step 7: Start independent watchdog (2 second timeout) */
    iwdg_init(2000);

    /* Log successful startup */
    esocore_event_log(ESOCORE_EVENT_SYSTEM_STARTUP, ESOCORE_EVENT_SEVERITY_INFO,
                     (uint8_t *)"EsoCore Edge v1.5.0.0 started", 0);

    /* Turn off boot LED, system is running */
    hal_gpio_reset(LED_STATUS_GREEN);

    /* Step 8: Enter main runtime loop */
    runtime_loop();

    /* If we exit the loop, perform graceful shutdown */
    graceful_shutdown();

    /* Should never reach here; reset */
    NVIC_SystemReset();
    return 0;
}

/* ============================================================================
 * BSP Helper Implementations
 * ============================================================================ */

/**
 * @brief bsp_delay_ms -- blocking delay using SysTick
 */
void bsp_delay_ms(uint32_t ms)
{
    uint32_t start = system_clock_get_tick();
    while ((system_clock_get_tick() - start) < ms) {
        /* Busy wait */
    }
}

/**
 * @brief bsp_get_tick_ms -- system tick in milliseconds
 */
uint32_t bsp_get_tick_ms(void)
{
    return system_clock_get_tick();
}
