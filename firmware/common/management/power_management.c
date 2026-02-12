/**
 * @file power_management.c
 * @brief Power Management Implementation for EsoCore Edge v1.5.0.0
 *
 * 24V DC input with brownout detection, multi-rail monitoring, supercapacitor
 * backup, and power sequencing. All PoE functionality has been removed for
 * v1.5.0.0 (dedicated 24V DC input replaces PoE).
 *
 * @author EsoCore Development Team
 * @copyright Copyright (c) 2026 Newmatik. All rights reserved.
 * @license Apache License, Version 2.0
 */

#include "power_management.h"
#include <string.h>

/* ============================================================================
 * Default Thresholds (from bsp_edge_v150.h values)
 * ============================================================================ */

#define DEFAULT_BROWNOUT_WARNING_MV     21000
#define DEFAULT_BROWNOUT_SHUTDOWN_MV    19000
#define DEFAULT_OVERVOLTAGE_MV          28500
#define DEFAULT_TEMP_LIMIT_C            70
#define DEFAULT_BROWNOUT_HOLDOFF_MS     50

/* Rail nominal/min/max (from BSP) */
static const uint16_t rail_nominal_mv[ESOCORE_RAIL_COUNT] = {
    24000, 5000, 3300, 3300, 1200, 12000, 3800
};
static const uint16_t rail_min_mv[ESOCORE_RAIL_COUNT] = {
    20000, 4750, 3135, 3135, 1140, 11400, 2000
};
static const uint16_t rail_max_mv[ESOCORE_RAIL_COUNT] = {
    28000, 5250, 3465, 3465, 1260, 12600, 3800
};

/* Fault flag for each rail */
static const uint16_t rail_fault_flag[ESOCORE_RAIL_COUNT] = {
    ESOCORE_POWER_FAULT_UNDERVOLTAGE,
    ESOCORE_POWER_FAULT_RAIL_5V,
    ESOCORE_POWER_FAULT_RAIL_3V3D,
    ESOCORE_POWER_FAULT_RAIL_3V3A,
    ESOCORE_POWER_FAULT_RAIL_1V2,
    ESOCORE_POWER_FAULT_RAIL_12V,
    ESOCORE_POWER_FAULT_SUPERCAP_LOW,
};

/* ============================================================================
 * Private State
 * ============================================================================ */

static bool power_initialized = false;
static esocore_power_config_t config;
static esocore_power_status_t status;
static esocore_power_callback_t event_callback = NULL;
static void *callback_context = NULL;

/* Brownout state machine */
static uint32_t brownout_start_tick = 0;
static bool brownout_active = false;

/* Uptime tracking */
static uint32_t last_poll_tick = 0;

/* ============================================================================
 * Hardware Abstraction (to be replaced with actual HAL calls)
 * ============================================================================ */

/* Forward declaration -- implemented by BSP using hal_adc.h */
extern void hal_adc_read_all_rails(uint32_t rail_mv[5]);
extern int8_t hal_adc_read_temperature(void);
extern uint32_t system_clock_get_tick(void);

/**
 * @brief Read all power rail voltages via internal ADC
 */
static void power_hw_read_rails(void)
{
    uint32_t adc_rails[5];
    hal_adc_read_all_rails(adc_rails);

    status.rail_voltage_mv[ESOCORE_RAIL_24V]      = (uint16_t)adc_rails[0];
    status.rail_voltage_mv[ESOCORE_RAIL_5V]        = (uint16_t)adc_rails[1];
    status.rail_voltage_mv[ESOCORE_RAIL_3V3_DIG]   = (uint16_t)adc_rails[2];
    status.rail_voltage_mv[ESOCORE_RAIL_1V2]        = (uint16_t)adc_rails[3];
    status.rail_voltage_mv[ESOCORE_RAIL_12V_SBUS]   = (uint16_t)adc_rails[4];

    /* 3.3V analog rail -- read separately if on a different ADC channel */
    /* For now, approximate from digital rail (same LDO input) */
    status.rail_voltage_mv[ESOCORE_RAIL_3V3_ANA] =
        status.rail_voltage_mv[ESOCORE_RAIL_3V3_DIG];

    /* Supercap voltage -- read from dedicated ADC channel */
    /* TODO: Read PWR_MON_SUPERCAP pin via hal_adc */
    status.rail_voltage_mv[ESOCORE_RAIL_SUPERCAP] = 3800; /* Placeholder */

    /* Temperature */
    status.temperature_c = hal_adc_read_temperature();
}

/**
 * @brief Notify registered callback of a power event
 */
static void power_notify(esocore_power_event_t event)
{
    if (event_callback) {
        event_callback(event, status.fault_flags, callback_context);
    }
}

/* ============================================================================
 * Brownout State Machine
 * ============================================================================ */

static void power_update_brownout(void)
{
    uint16_t vin = status.rail_voltage_mv[ESOCORE_RAIL_24V];
    uint32_t now = system_clock_get_tick();

    if (vin < config.brownout_shutdown_mv) {
        /* Critical: input below shutdown threshold */
        if (!brownout_active || (now - brownout_start_tick) > config.brownout_holdoff_ms) {
            status.state = ESOCORE_POWER_STATE_CRITICAL;
            status.active_source = ESOCORE_POWER_SOURCE_SUPERCAP;
            status.fault_flags |= ESOCORE_POWER_FAULT_BROWNOUT;
            power_notify(ESOCORE_POWER_EVENT_BROWNOUT_SHUTDOWN);
        }
        if (!brownout_active) {
            brownout_active = true;
            brownout_start_tick = now;
            status.brownout_count++;
        }
    } else if (vin < config.brownout_warning_mv) {
        /* Warning: input dropping */
        if (!brownout_active) {
            brownout_active = true;
            brownout_start_tick = now;
        }
        if ((now - brownout_start_tick) > config.brownout_holdoff_ms) {
            status.state = ESOCORE_POWER_STATE_BROWNOUT;
            status.fault_flags |= ESOCORE_POWER_FAULT_BROWNOUT;
            power_notify(ESOCORE_POWER_EVENT_BROWNOUT_WARNING);
        }
    } else {
        /* Normal input voltage */
        if (brownout_active) {
            brownout_active = false;
            status.fault_flags &= ~ESOCORE_POWER_FAULT_BROWNOUT;
            if (status.state == ESOCORE_POWER_STATE_BROWNOUT ||
                status.state == ESOCORE_POWER_STATE_CRITICAL) {
                status.state = ESOCORE_POWER_STATE_NORMAL;
                status.active_source = ESOCORE_POWER_SOURCE_DC_INPUT;
                power_notify(ESOCORE_POWER_EVENT_RECOVERED);
            }
        }
    }
}

/* ============================================================================
 * Rail Monitoring
 * ============================================================================ */

static void power_check_rails(void)
{
    /* Clear rail-specific faults (will be re-asserted if still present) */
    status.fault_flags &= (ESOCORE_POWER_FAULT_BROWNOUT |
                           ESOCORE_POWER_FAULT_OVERTEMPERATURE);

    for (int i = 0; i < ESOCORE_RAIL_COUNT; i++) {
        uint16_t v = status.rail_voltage_mv[i];
        if (v < rail_min_mv[i] || v > rail_max_mv[i]) {
            status.fault_flags |= rail_fault_flag[i];
            if (i <= ESOCORE_RAIL_12V_SBUS) {
                power_notify(ESOCORE_POWER_EVENT_RAIL_FAULT);
            }
        }
    }

    /* Overvoltage on 24V input */
    if (status.rail_voltage_mv[ESOCORE_RAIL_24V] > config.overvoltage_mv) {
        status.fault_flags |= ESOCORE_POWER_FAULT_OVERVOLTAGE;
    }

    /* Temperature */
    if (status.temperature_c > config.temperature_limit_c) {
        status.fault_flags |= ESOCORE_POWER_FAULT_OVERTEMPERATURE;
        power_notify(ESOCORE_POWER_EVENT_OVERTEMPERATURE);
    }

    /* Supercap low */
    if (status.rail_voltage_mv[ESOCORE_RAIL_SUPERCAP] < 2500) {
        status.fault_flags |= ESOCORE_POWER_FAULT_SUPERCAP_LOW;
        power_notify(ESOCORE_POWER_EVENT_SUPERCAP_LOW);
    }
}

/* ============================================================================
 * Supercapacitor
 * ============================================================================ */

static void power_update_supercap(void)
{
    uint16_t v = status.rail_voltage_mv[ESOCORE_RAIL_SUPERCAP];
    /* Charge percentage: linear map from 2.0V (0%) to 3.8V (100%) */
    if (v <= 2000) {
        status.supercap_charge_percent = 0;
    } else if (v >= 3800) {
        status.supercap_charge_percent = 100;
    } else {
        status.supercap_charge_percent = (uint8_t)(((uint32_t)(v - 2000) * 100) / 1800);
    }
}

/* ============================================================================
 * Public API
 * ============================================================================ */

bool esocore_power_init(const esocore_power_config_t *user_config)
{
    if (power_initialized) return false;

    /* Apply configuration or defaults */
    if (user_config) {
        config = *user_config;
    } else {
        config.brownout_warning_mv = DEFAULT_BROWNOUT_WARNING_MV;
        config.brownout_shutdown_mv = DEFAULT_BROWNOUT_SHUTDOWN_MV;
        config.overvoltage_mv = DEFAULT_OVERVOLTAGE_MV;
        config.temperature_limit_c = DEFAULT_TEMP_LIMIT_C;
        config.brownout_holdoff_ms = DEFAULT_BROWNOUT_HOLDOFF_MS;
        config.enable_supercap_backup = true;
        config.enable_analog_sequencing = true;
    }

    /* Initialize status */
    memset(&status, 0, sizeof(status));
    status.state = ESOCORE_POWER_STATE_NORMAL;
    status.active_source = ESOCORE_POWER_SOURCE_DC_INPUT;

    brownout_active = false;
    last_poll_tick = system_clock_get_tick();

    power_initialized = true;
    return true;
}

void esocore_power_deinit(void)
{
    power_initialized = false;
}

void esocore_power_poll(void)
{
    if (!power_initialized) return;

    /* Read all rail voltages */
    power_hw_read_rails();

    /* Update supercap state */
    power_update_supercap();

    /* Run brownout state machine */
    power_update_brownout();

    /* Check all rails */
    power_check_rails();

    /* Update uptime */
    uint32_t now = system_clock_get_tick();
    if (now - last_poll_tick >= 1000) {
        status.uptime_seconds += (now - last_poll_tick) / 1000;
        last_poll_tick = now;
    }
}

bool esocore_power_get_status(esocore_power_status_t *out)
{
    if (!power_initialized || !out) return false;
    *out = status;
    return true;
}

esocore_power_state_t esocore_power_get_state(void)
{
    return status.state;
}

uint16_t esocore_power_read_rail(esocore_power_rail_t rail)
{
    if (rail >= ESOCORE_RAIL_COUNT) return 0;
    return status.rail_voltage_mv[rail];
}

uint16_t esocore_power_check_faults(void)
{
    return status.fault_flags;
}

void esocore_power_register_callback(esocore_power_callback_t callback, void *context)
{
    event_callback = callback;
    callback_context = context;
}

bool esocore_power_emergency_shutdown(void)
{
    if (!power_initialized) return false;

    status.state = ESOCORE_POWER_STATE_SHUTDOWN;

    /* TODO: Save critical state to microSD */
    /* TODO: De-energize safety outputs via safety_io */
    /* TODO: Disable non-essential peripherals */
    /* TODO: Enter lowest power state */

    return true;
}

bool esocore_power_sequence_up(void)
{
    if (!power_initialized) return false;

    /* Step 1: Verify 24V input is present */
    power_hw_read_rails();
    if (status.rail_voltage_mv[ESOCORE_RAIL_24V] < rail_min_mv[ESOCORE_RAIL_24V]) {
        return false;
    }

    /* Step 2: Wait for 5V buck to stabilize (should be automatic from hardware) */
    /* In hardware, the TPS54331 starts as soon as VIN is present */
    uint32_t timeout = system_clock_get_tick() + 100; /* 100 ms timeout */
    while (system_clock_get_tick() < timeout) {
        power_hw_read_rails();
        if (status.rail_voltage_mv[ESOCORE_RAIL_5V] >= rail_min_mv[ESOCORE_RAIL_5V]) {
            break;
        }
    }
    if (status.rail_voltage_mv[ESOCORE_RAIL_5V] < rail_min_mv[ESOCORE_RAIL_5V]) {
        status.fault_flags |= ESOCORE_POWER_FAULT_RAIL_5V;
        return false;
    }

    /* Step 3: Verify 3.3V digital and 1.2V core (LDOs from 5V) */
    timeout = system_clock_get_tick() + 50;
    while (system_clock_get_tick() < timeout) {
        power_hw_read_rails();
        if (status.rail_voltage_mv[ESOCORE_RAIL_3V3_DIG] >= rail_min_mv[ESOCORE_RAIL_3V3_DIG] &&
            status.rail_voltage_mv[ESOCORE_RAIL_1V2] >= rail_min_mv[ESOCORE_RAIL_1V2]) {
            break;
        }
    }

    /* Step 4: If analog sequencing enabled, verify 3.3V analog rail */
    if (config.enable_analog_sequencing) {
        timeout = system_clock_get_tick() + 50;
        while (system_clock_get_tick() < timeout) {
            power_hw_read_rails();
            if (status.rail_voltage_mv[ESOCORE_RAIL_3V3_ANA] >=
                rail_min_mv[ESOCORE_RAIL_3V3_ANA]) {
                break;
            }
        }
        if (status.rail_voltage_mv[ESOCORE_RAIL_3V3_ANA] <
            rail_min_mv[ESOCORE_RAIL_3V3_ANA]) {
            status.fault_flags |= ESOCORE_POWER_FAULT_RAIL_3V3A;
            return false;
        }
    }

    /* Step 5: 12V sensor bus is enabled on demand, not at startup */

    status.state = ESOCORE_POWER_STATE_NORMAL;
    return true;
}

void esocore_power_sequence_down(void)
{
    /* Reverse order: 12V sensor bus -> analog -> digital -> 5V */
    /* In hardware, LDOs auto-disable when 5V drops. We just need to
     * disable the 12V sensor bus enable GPIO. */

    /* TODO: hal_gpio_reset(SBUS_12V_EN) to disable 12V bus */

    status.state = ESOCORE_POWER_STATE_SHUTDOWN;
}

/* ============================================================================
 * Supercapacitor API
 * ============================================================================ */

uint8_t esocore_supercap_get_charge_level(void)
{
    return status.supercap_charge_percent;
}

uint16_t esocore_supercap_get_voltage(void)
{
    return status.rail_voltage_mv[ESOCORE_RAIL_SUPERCAP];
}

uint32_t esocore_supercap_estimate_runtime(uint16_t load_ma)
{
    if (load_ma == 0) return 0xFFFFFFFF;

    /* E = 0.5 * C * (V^2 - Vmin^2) */
    /* Runtime = E / P */
    uint16_t v = status.rail_voltage_mv[ESOCORE_RAIL_SUPERCAP];
    uint16_t vmin = 2000;  /* Minimum usable voltage */
    if (v <= vmin) return 0;

    /* Energy in millijoules */
    /* C = 20F, V in volts: E = 0.5 * 20 * (v^2 - vmin^2) */
    float v_f = (float)v / 1000.0f;
    float vmin_f = (float)vmin / 1000.0f;
    float energy_j = 0.5f * 20.0f * (v_f * v_f - vmin_f * vmin_f);

    /* Power in watts: P = V_avg * I */
    float v_avg = (v_f + vmin_f) / 2.0f;
    float power_w = v_avg * ((float)load_ma / 1000.0f);

    if (power_w <= 0.0f) return 0xFFFFFFFF;
    return (uint32_t)(energy_j / power_w);
}

bool esocore_supercap_is_shutdown_safe(void)
{
    /* Need at least 5 seconds of runtime at ~500 mA for safe shutdown */
    return esocore_supercap_estimate_runtime(500) >= 5;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

float esocore_voltage_mv_to_v(uint16_t mv)
{
    return (float)mv / 1000.0f;
}

float esocore_current_ma_to_a(int16_t ma)
{
    return (float)ma / 1000.0f;
}

float esocore_calculate_power(uint16_t voltage_mv, int16_t current_ma)
{
    return ((float)voltage_mv * (float)current_ma) / 1000000.0f;
}
