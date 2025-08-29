# Data Format Specification

This document defines the data formats, file structures, and event schemas used by the EsoCore edge-to-cloud telemetry system.

---

## Local Data Storage Format

**Why standardize the format?** Ensures data integrity, enables audit trails, and simplifies cloud synchronization across different firmware versions.

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
    "firmware_ver": "1.2.3",
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
    "description": "Main power lost - running on supercap backup",
    "main_voltage": 0.0,
    "supercap_voltage": 11.8,
    "estimated_runtime_sec": 28
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
  "series": "safety_edge_triggered",
  "val": null,
  "unit": null,
  "meta": {
    "event_type": "safety",
    "severity": "warning",
    "description": "Safety edge contact detected during operation",
    "input_channel": "SAFETY_IN_1",
    "asset_position": "75_percent_complete",
    "response_time_ms": 8
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
    "description": "Emergency shutdown sequence started",
    "supercap_voltage": 10.2,
    "data_flushed_mb": 2.4,
    "shutdown_reason": "power_outage"
  }
}
```

---

## Event Categories

### Connectivity Events

- `connectivity_lost` - Network connection lost
- `connectivity_restored` - Network connection restored
- `server_unreachable` - Cannot reach cloud server
- `sync_failed` - Data synchronization failed

### Power Events

- `power_outage` - Main power lost
- `power_restored` - Main power restored
- `low_voltage` - Voltage below threshold
- `backup_activated` - Backup power activated
- `supercap_low` - Supercapacitor voltage low
- `safe_shutdown_initiated` - Emergency shutdown started

### Safety Events

- `safety_edge_triggered` - Safety edge contact detected
- `emergency_stop_activated` - Emergency stop button pressed
- `light_curtain_breach` - Light curtain interrupted
- `asset_obstruction` - Movement obstructed
- `safety_circuit_fault` - Safety circuit malfunction

### System Events

- `device_boot` - Device startup
- `watchdog_reset` - Watchdog timer reset
- `sd_error` - SD card error
- `sensor_fault` - Sensor malfunction
- `ota_started` - OTA update started
- `ota_completed` - OTA update completed
- `memory_error` - Memory allocation error

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

- **Ambient**: temp_ambient (°C), RH (%), 1 Hz default; averaged to 1 min for cloud
- **Drive stator temperature**: temp_stator (°C), 1 Hz default; per‑cycle min/avg/max for load indication
- **Asset cycles**: increment on open→closed or start→stop transitions; event objects carry `cycle_id` and durations
- **Torque proxy**: motor current RMS at 100–500 Hz; compute per‑cycle stats (avg/max)
- **Vibration**: rolling RMS at 100–500 Hz; periodic 2–5 s high‑rate window (1–3 kHz) for spectral bands (e.g., bearing, imbalance)
- **Acoustics**: rolling RMS and event detection; periodic windows in audible/ultrasound bands for spectral features; flag high‑amplitude events (e.g., spring break)
- **Derived health KPIs**: over‑torque rate, vibration trend slope, mean time between faults
