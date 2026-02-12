# Data Format Specification

This document defines the data formats, file structures, and event schemas used by the EsoCore edge-to-cloud
telemetry system (firmware v1.5.0).

---

## Local Data Storage Format

**Why standardize the format?** Ensures data integrity, enables audit trails, and simplifies cloud
synchronization across different firmware versions.

### File Organization

```text
SD Card Structure:
├── 202501/
│   ├── DEVICE001_20250115_143022_001.json.zst
│   ├── DEVICE001_20250115_150000_002.json.zst
│   └── ...
└── 202502/
    └── ...
```

### Technical Specifications

- **Data Format**: JSON (JavaScript Object Notation) - human-readable, widely supported, easier debugging and maintenance
- **Compression**: Zstandard (.zst) for ~70% size reduction
- **File Naming**: `YYYYMM/DEVICEID_YYYYMMDD_HHMMSS_SEQ.json.zst`
- **Rotation**: New file every 1MB or 1 hour (configurable)

---

## Data Record Schemas

### Sensor Data Record

Each sensor reading contains:

```json
{
  "ts": "2025-01-15T14:30:22.123Z", // Timestamp (UTC)
  "device_id": "DEVICE001", // Unique device identifier
  "site_id": "WAREHOUSE_A", // Installation location
  "asset_id": "ASSET_3", // Specific asset/machine
  "series": "temp_ambient", // Data type (sensor data or system event)
  "val": 23.5, // Measurement value (null for events)
  "unit": "°C", // Unit of measurement
  "meta": {
    // Additional context
    "firmware_ver": "1.5.0.0",
    "sensor_id": "SHT40_01",
    "sample_rate": 1000
  }
}
```

### System Event Records

In addition to sensor data, the system captures operational events:

#### Network Connectivity Event

```json
{
  "ts": "2025-01-15T14:35:10.456Z",
  "device_id": "DEVICE001",
  "site_id": "WAREHOUSE_A",
  "asset_id": "ASSET_3",
  "series": "connectivity_lost",
  "val": null,
  "unit": null,
  "meta": {
    "event_type": "network",
    "severity": "warning",
    "description": "WiFi connection lost - switching to offline mode",
    "duration_ms": 45000,
    "retry_count": 3
  }
}
```

#### Power Event

```json
{
  "ts": "2025-01-15T09:22:33.123Z",
  "device_id": "DEVICE001",
  "site_id": "WAREHOUSE_A",
  "asset_id": "ASSET_3",
  "series": "power_outage",
  "val": null,
  "unit": null,
  "meta": {
    "event_type": "power",
    "severity": "critical",
    "description": "24V DC input lost - running on supercap backup",
    "input_voltage_mv": 0,
    "rail_3v3_mv": 3280,
    "rail_1v2_mv": 1195,
    "supercap_voltage_mv": 3650,
    "supercap_charge_pct": 82,
    "estimated_runtime_sec": 25
  }
}
```

#### Safety Event

```json
{
  "ts": "2025-01-15T11:45:22.789Z",
  "device_id": "DEVICE001",
  "site_id": "WAREHOUSE_A",
  "asset_id": "ASSET_3",
  "series": "safety_input_triggered",
  "val": null,
  "unit": null,
  "meta": {
    "event_type": "safety",
    "severity": "critical",
    "description": "Emergency stop activated - dual-channel safety input triggered",
    "input_channel": "SI_1",
    "input_type": "emergency_stop",
    "channel_a_state": true,
    "channel_b_state": true,
    "cross_monitor_ok": true,
    "response_time_ms": 4
  }
}
```

#### Safe Shutdown Event

```json
{
  "ts": "2025-01-15T09:22:35.456Z",
  "device_id": "DEVICE001",
  "site_id": "WAREHOUSE_A",
  "asset_id": "ASSET_3",
  "series": "safe_shutdown_initiated",
  "val": null,
  "unit": null,
  "meta": {
    "event_type": "power",
    "severity": "critical",
    "description": "Emergency shutdown sequence started - graceful power-down",
    "supercap_voltage_mv": 3200,
    "supercap_charge_pct": 55,
    "data_flushed_mb": 2.4,
    "shutdown_reason": "brownout_critical"
  }
}
```

#### Analog Input Event

```json
{
  "ts": "2025-01-15T14:42:11.890Z",
  "device_id": "DEVICE001",
  "site_id": "WAREHOUSE_A",
  "asset_id": "ASSET_3",
  "series": "analog_acquisition",
  "val": null,
  "unit": null,
  "meta": {
    "event_type": "analog",
    "severity": "info",
    "channel": 0,
    "mode": "iepe",
    "gain": 10,
    "sample_rate_sps": 50000,
    "rms_uv": 4520,
    "peak_uv": 18200,
    "adc_bits": 24
  }
}
```

#### Ethernet Port Event

```json
{
  "ts": "2025-01-15T14:38:05.321Z",
  "device_id": "DEVICE001",
  "site_id": "WAREHOUSE_A",
  "asset_id": "ASSET_3",
  "series": "eth_link_change",
  "val": null,
  "unit": null,
  "meta": {
    "event_type": "network",
    "severity": "warning",
    "port": "A",
    "role": "OT",
    "link_state": "down",
    "phy": "KSZ8081",
    "speed_mbps": 100,
    "duplex": "full"
  }
}
```

---

## Event Categories

### Power Events

- `power_outage` - 24V DC input lost
- `power_restored` - 24V DC input restored
- `brownout_warning` - Input voltage below warning threshold
- `brownout_critical` - Input voltage below critical threshold, shutdown imminent
- `rail_fault` - Individual power rail (5V, 3.3V, 1.2V, 3.3VA) out of tolerance
- `backup_activated` - Supercapacitor backup engaged
- `supercap_low` - Supercapacitor charge below safe threshold
- `safe_shutdown_initiated` - Graceful shutdown sequence started

### Safety Events

- `safety_input_triggered` - Safety input activated (dual-channel confirmed)
- `safety_cross_monitor_fault` - Dual-channel cross-monitoring mismatch detected
- `safety_input_stuck` - Safety input stuck-at fault detected
- `safety_relay_activated` - Safety relay output energized
- `safety_relay_fault` - Safety relay feedback mismatch
- `safety_ssr_activated` - Solid-state relay (AQY212EHAZ) output activated
- `safety_test_pulse` - Periodic safety input test pulse executed

### Analog Input Events

- `analog_acquisition` - Analog channel acquisition started or reconfigured
- `analog_mode_change` - Channel mode changed (IEPE/DC)
- `analog_gain_change` - Channel gain reconfigured
- `analog_overrange` - ADC input overrange detected
- `analog_sensor_check` - IEPE sensor connectivity check result

### Network Events

- `eth_link_change` - Ethernet port link state change (up/down)
- `eth_port_a_up` - OT network port link established
- `eth_port_b_up` - IT network port link established
- `connectivity_lost` - Network connection lost
- `connectivity_restored` - Network connection restored
- `server_unreachable` - Cannot reach cloud server
- `sync_failed` - Data synchronization failed

### Fieldbus Events

- `interbus_connected` - Interbus master connection established
- `interbus_disconnected` - Interbus master disconnected
- `interbus_fault` - Interbus communication error
- `sensor_bus_device_found` - New sensor module discovered on RS-485 bus
- `sensor_bus_device_lost` - Sensor module heartbeat timeout
- `sensor_bus_fault` - Sensor bus communication error

### System Events

- `device_boot` - Device startup (v1.5.0 firmware)
- `watchdog_reset` - Independent watchdog (IWDG) reset
- `sd_error` - SD card error
- `sensor_fault` - Sensor malfunction
- `ota_started` - OTA update started
- `ota_completed` - OTA update completed
- `memory_error` - Memory allocation error
- `hardfault` - Cortex-M7 HardFault (logged on next boot)

### Security Events

- `auth_failed` - Authentication failure
- `tamper_detected` - Physical tampering detected
- `config_changed` - Configuration modified
- `enclosure_opened` - Device enclosure opened

---

## Data Integrity & Reliability

- **Power-safe writes**: Atomic file operations prevent corruption during power loss
- **Checksums**: CRC32 per record + SHA-256 per file to detect corruption
- **Audit trail**: Immutable append-only format for compliance requirements
- **Event persistence**: System events logged even during connectivity outages

---

## Data Semantics (Metrics & Rates)

- **Ambient**: temp_ambient (C), RH (%), 1 Hz default; averaged to 1 min for cloud
- **Drive stator temperature**: temp_stator (C), 1 Hz default; per-cycle min/avg/max for load indication
- **Asset cycles**: increment on open-closed or start-stop transitions; event objects carry `cycle_id` and durations
- **Torque proxy**: motor current RMS at 100-500 Hz; compute per-cycle stats (avg/max)
- **Vibration (analog input)**: 4 channels, 24-bit resolution, up to 50 kSPS via ADS1274. Rolling RMS computed
  per acquisition block; periodic high-rate windows for spectral bands (bearing, imbalance). Firmware computes
  RMS and peak values in real time with DMA double-buffered capture.
- **Acoustics**: rolling RMS and event detection; periodic windows in audible/ultrasound bands for spectral
  features; flag high-amplitude events (e.g., spring break)
- **Analog signal chain**: Each channel supports IEPE mode (4 mA current source for accelerometers) or DC mode
  (direct voltage measurement). Programmable gain from 1x to 128x (PGA280). 15 kHz anti-aliasing filter before
  ADC. All 4 channels sampled simultaneously.
- **Derived health KPIs**: over-torque rate, vibration trend slope, mean time between faults
- **Power monitoring**: Internal ADC3 reads 24V input, 5V, 3.3V digital, 3.3V analog, 1.2V core, and die
  temperature at 1 Hz. Brownout state machine transitions logged as events.
