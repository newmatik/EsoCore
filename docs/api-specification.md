# API Specification

This document defines the REST APIs, authentication mechanisms, and cloud platform specifications for the EdgeSentinel IoT system using Frappe Framework.

---

## Cloud Platform Architecture

Any capable backend can fulfill these roles. The following concrete model assumes Frappe Framework for speed and maintainability.

### Doctypes / Core Data Model (Frappe Reference)

- **Device**: id, serial, model, keys, firmware, last_seen, site/customer, tags, status
- **Door**: door_id, site/customer, type (industrial/residential), drive model
- **Telemetry Packet (ingest ledger)**: device_id, upload_id, checksum, status
- **Timeseries (partitioned)**:
  - TelemetryPoint: ts, device_id, metric, value, unit, meta (JSON)
  - TelemetryWindow: ts, device_id, metric, stats (rms/max/std/bandpower)
- **System Events**:
  - SystemEvent: ts, device_id, event_type, severity, description, duration, resolved
  - EventLog: comprehensive audit trail with full context and metadata
- **Door Events**: cycle_open, cycle_close, alerts (over‑torque, vibration)
- **Event Workflows**: automated response rules triggered by system events
- **Notification Queue**: pending alerts with delivery status and retries
- **Config Profile**: sampling rates, window sizes, thresholds per device type
- **Firmware Bundle**: version, hash, channel (stable/beta), rollout policy
- **Alert Rule & Notification**: thresholds, conditions, recipients, channels
- **Customer Portal Models**: Site, UserDeviceRole, Dashboards

---

## REST API (Device-Facing)

### Authentication Endpoint

```text
POST /api/iot/v1/auth/handshake
```

Returns nonce & server time (optional)

### Telemetry Upload

```text
POST /api/iot/v1/telemetry/batch
```

**Headers:**

- `X-Device-Id`: Unique device identifier
- `X-Auth-Key`: Device authentication key
- `Idempotency-Key`: UUIDv4 for duplicate detection
- `Content-SHA256`: Request body hash

**Body:** gzip/zstd JSON batch

**Response:**

```json
{
  "accepted": 1250,
  "duplicates": 3,
  "rejected": 0,
  "next_hint": "backoff_ms: 5000"
}
```

### Configuration Management

```text
GET /api/iot/v1/config
```

Returns device configuration (sampling rates, thresholds, NTP servers, endpoints)

### OTA Management

```text
GET /api/iot/v1/ota/check
```

Returns firmware version and signed download URL

```text
POST /api/iot/v1/ota/report
```

Device reports OTA update status

### Authentication Methods

- **Frappe API Key/Secret** per Device, or
- **OAuth2 Client Credentials** for central key rotation

---

## Sync Logic & Protocol

### Authentication

- **Device API Key/Secret** (provisioned once), HMAC on body, and TLS
- **Key rotation** via OTA updates

### Data Batching

- **Batch size**: ~1–5 MB or 5–10k records per POST
- **Compression**: gzip or zstd compression

### Idempotency & Reliability

- **Idempotency-Key** header (UUIDv4) + server‑side deduplication
- **Exponential backoff** with jitter
- **Priority queuing**: newest data first, then backfill

### Clock Synchronization

- **NTP** at boot and every 12 hours
- **Firmware tolerance**: ±2 min skew
- **Server re-stamping**: available on request

---

## Event Processing & Workflow Automation

### Event Ingestion Pipeline

```text
System Event → Event Parser → Severity Classification → Workflow Trigger → Notification Queue → Delivery
```

### Workflow Examples

| Event Type | Trigger Condition | Automated Actions |
|------------|------------------|-------------------|
| `connectivity_lost` | Duration > 5 minutes | Email to maintenance team, escalate if > 30 min |
| `power_outage` | Severity = critical | Immediate SMS + email, create maintenance ticket |
| `safe_shutdown_initiated` | Any occurrence | Critical alert + incident creation, prepare site visit |
| `supercap_low` | Voltage < 80% rated | Maintenance alert, check backup power system |
| `safety_edge_triggered` | Frequency > threshold | Safety audit alert, check door alignment |
| `emergency_stop_activated` | Any activation | Immediate safety team notification + incident log |
| `light_curtain_breach` | Unexpected breach | Security alert if outside operating hours |
| `server_unreachable` | Retry count > 5 | Notify IT team, check server status |
| `sensor_fault` | Any sensor failure | Schedule maintenance, disable affected alerts |
| `tamper_detected` | Security event | Instant security team alert, log incident |
| `sd_error` | Storage failure | Emergency notification, prepare replacement |

### Frappe Workflow Integration

- **Document Workflows**: Auto-create maintenance tickets from critical events
- **Email Templates**: Customizable notifications with event context
- **Notification Hooks**: Slack/Teams/webhook integrations  
- **Escalation Rules**: Time-based escalation chains (L1 → L2 → L3 support)
- **Conditional Logic**: Event severity, time of day, on-call schedules
- **Batch Processing**: Group related events to prevent alert fatigue

### Event Correlation

- **Pattern Detection**: Identify recurring issues across device fleet
- **Root Cause Analysis**: Link cascading events (power → connectivity → sync failure)
- **Anomaly Detection**: Machine learning on event patterns for predictive alerting

---

## Admin & UX Features

### Dashboards

**Per device & per site:**

- Live status (online/offline), last sync, SD health, firmware version
- Trends: ambient/stator temperature, RH, torque/current, vibration RMS/bandpower, acoustics level/features
- Door cycle counters & daily/weekly histograms

### Alerting System

- **Email/Slack/Webhooks** for thresholds (e.g., over‑torque, rising vibration, elevated acoustics/ultrasound events)
- **Device offline** notifications
- **Sync backlog** alerts

### Data Management

- **Data Explorer**: filter by site/door/device/metric/time, CSV/Parquet export
- **Device Provisioning UI**: create Device, print claim QR (Device ID + bootstrap token), see first‑seen time & firmware channel

### Access Control

- **RBAC**: Customer can see only their sites/devices

## Storage & Performance

### Database Design

- **Partitioning**: tables by month for telemetry; indexes on `(device_id, ts)` and `(metric, ts)`
- **External TSDB (optional)**: TimescaleDB/ClickHouse via Frappe integration for heavy analytics
- **Cold storage**: optional S3 for raw vibration windows

---

## Device Provisioning & Security

### Manufacturing Process

- **Flash firmware**: inject unique `device_id`, API key/secret, and initial config
- **QR code generation**: for technician claiming process

### Device Claiming

- **Technician process**: scan QR → portal links device to customer/site → server enables the key
- **Security**: device remains inactive until properly claimed

### Security Features

- **Mutual TLS (optional tier)**: pin server CA; rotate keys annually; OTA key roll supported
- **Secure boot & OTA**: signed images, A/B partition, automatic rollback on watchdog

---

## Sync & Reliability Details

### Offline Buffer Management

- **Minimum capacity**: 30 days at nominal rates (size calculated during design)
- **Resume capability**: maintain upload cursor per file + per record offset
- **Recovery**: on reconnect, resume at last acknowledged position

### Duplicate Protection

- **Server storage**: `(device_id, batch_id, record_hash)` for 30 days
- **Idempotency**: guarantee exactly-once processing

### Backpressure Control

- **Server response**: `next_hint: backoff_ms` for rate limiting
- **Dynamic configuration**: server can reduce device sampling via `/config` endpoint

---

## Error Handling

### HTTP Status Codes

- **200**: Successful data ingestion
- **400**: Invalid request format
- **401**: Authentication failure
- **409**: Duplicate data (idempotency key conflict)
- **429**: Rate limiting active
- **500**: Server error
- **503**: Service unavailable (maintenance mode)

### Retry Logic

- **Exponential backoff**: starting at 1s, max 300s
- **Jitter**: ±25% to prevent thundering herd
- **Circuit breaker**: fail fast after consecutive failures
- **Fallback**: local storage with sync on recovery
