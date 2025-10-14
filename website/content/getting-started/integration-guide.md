# EsoCore Integration Guide

Comprehensive guide for integrating EsoCore with existing industrial systems including CMMS, SCADA, MES, ERP, and industrial protocols. Learn how to connect
EsoCore monitoring data to your existing workflows, automate maintenance processes, and maximize the value of equipment health insights.

---

## Integration Overview

EsoCore supports multiple integration pathways to fit into your existing industrial infrastructure:

**Industrial Protocols:**
- Modbus RTU/TCP
- PROFINET Device
- PROFIBUS DP
- OPC UA (planned)

**IT Systems:**
- REST API for custom integrations
- Webhook notifications for events
- Database access for analytics
- File exports (CSV, JSON, Parquet)

**Common Integrations:**
- CMMS (Computerized Maintenance Management Systems)
- SCADA (Supervisory Control and Data Acquisition)
- MES (Manufacturing Execution Systems)
- ERP (Enterprise Resource Planning)
- BI Tools (Business Intelligence and Analytics)

---

## REST API Integration

### API Overview

EsoCore provides a comprehensive REST API for programmatic access to all platform features.

**Base URL:** `https://[EDGE-DEVICE-IP]/api/v1/`

**Authentication:**
```http
X-API-Key: your-api-key-here
```

**Response Format:** JSON

### Authentication

**Generate API Key:**
1. Login to web UI
2. Navigate to Settings → API Keys
3. Click "Generate New Key"
4. Enter description: "Integration with [SYSTEM]"
5. Set permissions (read-only or read-write)
6. Copy key (shown once only)
7. Store securely

**Using API Key:**
```python
import requests

headers = {
    "X-API-Key": "your-api-key",
    "Content-Type": "application/json"
}

response = requests.get(
    "https://edge-device/api/v1/telemetry/",
    headers=headers
)
```

### Key API Endpoints

**Device Information:**
```http
GET /api/v1/device/info           # Device details
GET /api/v1/device/health          # System health status
GET /api/v1/sensors/               # List all sensors
GET /api/v1/sensors/{id}/          # Specific sensor details
```

**Telemetry Data:**
```http
GET /api/v1/telemetry/             # Latest readings
GET /api/v1/telemetry/history/     # Historical data
GET /api/v1/telemetry/sensor/{id}/ # Sensor-specific data
```

Query parameters:
- `start_time`: ISO 8601 timestamp
- `end_time`: ISO 8601 timestamp
- `sensor_id`: Filter by sensor
- `metric`: Filter by metric type (temperature, vibration_rms, etc.)
- `limit`: Maximum results (default 1000)

**Alerts:**
```http
GET /api/v1/alerts/                # Active alerts
GET /api/v1/alerts/history/        # Alert history
POST /api/v1/alerts/{id}/ack       # Acknowledge alert
GET /api/v1/alerts/rules/          # Alert rules
```

**Configuration:**
```http
GET /api/v1/config/                # Current configuration
PUT /api/v1/config/                # Update configuration
POST /api/v1/config/restart        # Restart services
```

### Example Integrations

**Python Integration:**
```python
import requests
from datetime import datetime, timedelta

class EsoCoreClient:
    def __init__(self, base_url, api_key):
        self.base_url = base_url
        self.headers = {
            "X-API-Key": api_key,
            "Content-Type": "application/json"
        }
    
    def get_current_readings(self):
        """Get latest sensor readings"""
        response = requests.get(
            f"{self.base_url}/telemetry/",
            headers=self.headers
        )
        return response.json()
    
    def get_sensor_history(self, sensor_id, hours=24):
        """Get sensor history"""
        end_time = datetime.now()
        start_time = end_time - timedelta(hours=hours)
        
        response = requests.get(
            f"{self.base_url}/telemetry/history/",
            headers=self.headers,
            params={
                "sensor_id": sensor_id,
                "start_time": start_time.isoformat(),
                "end_time": end_time.isoformat()
            }
        )
        return response.json()
    
    def get_active_alerts(self):
        """Get all active alerts"""
        response = requests.get(
            f"{self.base_url}/alerts/",
            headers=self.headers,
            params={"status": "active"}
        )
        return response.json()

# Usage
client = EsoCoreClient(
    base_url="https://192.168.1.100/api/v1",
    api_key="your-api-key"
)

# Get current readings
readings = client.get_current_readings()
for reading in readings:
    print(f"{reading['sensor_name']}: {reading['value']} {reading['unit']}")

# Check for alerts
alerts = client.get_active_alerts()
if alerts:
    print(f"WARNING: {len(alerts)} active alerts!")
```

**JavaScript/Node.js Integration:**
```javascript
const axios = require('axios');

class EsoCoreClient {
    constructor(baseUrl, apiKey) {
        this.baseUrl = baseUrl;
        this.headers = {
            'X-API-Key': apiKey,
            'Content-Type': 'application/json'
        };
    }
    
    async getCurrentReadings() {
        const response = await axios.get(
            `${this.baseUrl}/telemetry/`,
            { headers: this.headers }
        );
        return response.data;
    }
    
    async getActiveAlerts() {
        const response = await axios.get(
            `${this.baseUrl}/alerts/`,
            { 
                headers: this.headers,
                params: { status: 'active' }
            }
        );
        return response.data;
    }
}

// Usage
const client = new EsoCoreClient(
    'https://192.168.1.100/api/v1',
    'your-api-key'
);

client.getCurrentReadings()
    .then(readings => console.log(readings))
    .catch(error => console.error(error));
```

---

## CMMS Integration

### Overview

Integrate EsoCore with Computerized Maintenance Management Systems to automatically create work orders based on equipment condition.

**Supported CMMS Platforms:**
- SAP PM
- IBM Maximo
- Infor EAM
- eMaint
- Fiix
- Generic via REST API

### Integration Methods

**Method 1: Webhook Notifications**

Configure EsoCore to send alerts directly to CMMS:

1. **In EsoCore:**
   - Settings → Integrations → Webhooks
   - Add new webhook
   - Webhook URL: `https://cmms.company.com/api/work-orders`
   - Authentication: Bearer token or API key
   - Event types: Select alert severities to trigger
   - Payload template: Map EsoCore data to CMMS fields

2. **Payload Example:**
```json
{
    "equipment_id": "{{device_id}}",
    "alert_type": "{{alert_severity}}",
    "description": "{{alert_message}}",
    "priority": "{{priority_mapping}}",
    "detected_at": "{{timestamp}}",
    "sensor_data": {
        "sensor": "{{sensor_name}}",
        "value": "{{current_value}}",
        "threshold": "{{threshold_value}}"
    }
}
```

3. **Test integration** with sample alert

**Method 2: Scheduled Polling**

CMMS polls EsoCore API periodically:

```python
# CMMS-side script (runs every 5 minutes)
import requests

ESOCORE_API = "https://edge-device/api/v1"
ESOCORE_KEY = "api-key"
CMMS_API = "https://cmms/api/work-orders"
CMMS_KEY = "cmms-api-key"

# Get new alerts from EsoCore
alerts = requests.get(
    f"{ESOCORE_API}/alerts/",
    headers={"X-API-Key": ESOCORE_KEY},
    params={"status": "new", "acknowledged": False}
).json()

# Create work orders in CMMS
for alert in alerts:
    work_order = {
        "equipment_id": alert["device_id"],
        "title": f"Predictive Alert: {alert['alert_type']}",
        "description": alert["message"],
        "priority": map_priority(alert["severity"]),
        "due_date": calculate_due_date(alert["severity"])
    }
    
    # Create work order
    response = requests.post(
        CMMS_API,
        headers={"Authorization": f"Bearer {CMMS_KEY}"},
        json=work_order
    )
    
    if response.ok:
        # Acknowledge alert in EsoCore
        requests.post(
            f"{ESOCORE_API}/alerts/{alert['id']}/ack",
            headers={"X-API-Key": ESOCORE_KEY},
            json={"work_order_id": response.json()["id"]}
        )
```

**Method 3: Email-to-Ticket**

If CMMS supports email-to-ticket:

1. **In EsoCore:**
   - Settings → Notifications → Email
   - Add CMMS email address: `workorders@cmms.company.com`
   - Configure email template with required CMMS fields in subject/body

2. **Email Template:**
```
Subject: [EQUIPMENT:{{device_name}}][PRIORITY:{{severity}}] {{alert_type}}

Equipment: {{device_name}}
Location: {{location}}
Alert: {{alert_type}}
Severity: {{severity}}
Detected: {{timestamp}}

Description:
{{alert_message}}

Sensor Data:
{{sensor_readings}}

Recommended Action:
{{recommended_action}}
```

### SAP PM Integration Example

```python
# SAP PM work order creation
from pyrfc import Connection

def create_sap_notification(alert):
    # Connect to SAP
    conn = Connection(
        ashost='sap.company.com',
        sysnr='00',
        client='100',
        user='INTERFACE_USER',
        passwd='password'
    )
    
    # Call BAPI to create notification
    result = conn.call('BAPI_ALM_NOTIF_CREATE', {
        'NOTIFHEADER': {
            'NOTIF_TYPE': 'M2',  # Maintenance notification
            'EQUIPMENT': alert['equipment_id'],
            'PRIORITY': map_to_sap_priority(alert['severity'])
        },
        'NOTIFTASK': [{
            'TASK_TEXT': alert['message'],
            'TASK_CODE': 'PRED',  # Predictive maintenance
            'TASK_CODEGRP': 'PM'
        }]
    })
    
    if result['RETURN']['TYPE'] == 'S':
        notification_id = result['NOTIFHEADER']['NOTIF_NO']
        return notification_id
    else:
        raise Exception(f"SAP Error: {result['RETURN']['MESSAGE']}")
```

---

## SCADA Integration

### Modbus TCP Integration

Expose EsoCore data to SCADA via Modbus TCP:

**Configuration:**
1. EsoCore Web UI → Settings → Fieldbus → Modbus TCP
2. Enable Modbus TCP Server
3. Port: 502 (default) or custom
4. Configure register mapping:
   - Starting register: 40001
   - Data type: Float (2 registers) or Integer (1 register)
   - Map sensors to register ranges

**Register Map Example:**
| Register | Parameter | Type | Unit |
|----------|-----------|------|------|
| 40001-40002 | Motor 1 Vibration RMS | Float | mm/s |
| 40003-40004 | Motor 1 Temperature | Float | °C |
| 40005-40006 | Motor 1 Current Phase A | Float | A |
| 40007 | Motor 1 Status | Int | 0=OK, 1=Warning, 2=Alert |
| 40008 | Motor 1 Alert Code | Int | See docs |
| ... | ... | ... | ... |

**SCADA Configuration:**

*Allen-Bradley / Rockwell:*
```
Controller: Modbus/TCP
IP Address: 192.168.1.100
Port: 502
Unit ID: 1
Register Map: Import CSV
Scan Rate: 1000ms
```

*Siemens WinCC:*
```
Driver: ModbusTCP.chn
IP: 192.168.1.100
Port: 502
Register Configuration: Import from XML
Update Cycle: 1s
```

*Ignition SCADA:*
```python
# Add device
Name: EsoCore_Edge1
Driver: Modbus TCP
Hostname: 192.168.1.100
Port: 502

# Configure tags
Motor1_Vibration: Float, 40001, 2 registers
Motor1_Temperature: Float, 40003, 2 registers
Motor1_Status: Int16, 40007, 1 register
```

### OPC UA Integration (Planned)

Future release will include OPC UA server:
- Standard OPC UA information model
- TLS-encrypted communication
- User authentication
- Browse-able address space
- Subscriptions for real-time updates

---

## MES Integration

### Manufacturing Execution System Integration

Connect equipment health data to production systems:

**Use Cases:**
- Display equipment health on production dashboards
- Prevent production scheduling on degraded equipment
- Correlate quality issues with equipment condition
- Track equipment availability for OEE calculation

**Integration Example:**

```python
# MES Integration Script
import requests
from datetime import datetime

class MESIntegration:
    def __init__(self, esocore_url, esocore_key, mes_url, mes_key):
        self.esocore_url = esocore_url
        self.esocore_headers = {"X-API-Key": esocore_key}
        self.mes_url = mes_url
        self.mes_headers = {"Authorization": f"Bearer {mes_key}"}
    
    def update_equipment_status(self):
        """Update MES with equipment health status"""
        # Get equipment health from EsoCore
        devices = requests.get(
            f"{self.esocore_url}/devices/",
            headers=self.esocore_headers
        ).json()
        
        for device in devices:
            # Get current health status
            health = self.get_device_health(device['id'])
            
            # Update MES equipment status
            requests.put(
                f"{self.mes_url}/equipment/{device['equipment_id']}/status",
                headers=self.mes_headers,
                json={
                    "health_score": health['score'],
                    "status": health['status'],
                    "alerts": health['active_alerts'],
                    "last_updated": datetime.now().isoformat()
                }
            )
    
    def check_production_readiness(self, equipment_id):
        """Check if equipment is ready for production"""
        device = self.get_device_by_equipment_id(equipment_id)
        
        # Get active alerts
        alerts = requests.get(
            f"{self.esocore_url}/alerts/",
            headers=self.esocore_headers,
            params={"device_id": device['id'], "status": "active"}
        ).json()
        
        # Check for critical alerts
        critical_alerts = [a for a in alerts if a['severity'] == 'critical']
        
        return {
            "ready": len(critical_alerts) == 0,
            "alerts": critical_alerts,
            "recommendation": "Schedule maintenance" if critical_alerts else "OK"
        }
```

---

## ERP Integration

### Enterprise Resource Planning Integration

Connect equipment data to business systems:

**Use Cases:**
- Automatic spare parts ordering based on predictions
- Maintenance cost tracking and budgeting
- Asset lifecycle management
- Warranty and vendor management

**Integration Points:**

*Procurement:*
```python
# Trigger spare parts order based on alert
def trigger_parts_order(alert):
    # Map alert to required spare parts
    parts = get_required_parts(alert['device_type'], alert['alert_type'])
    
    # Create purchase requisition in ERP
    for part in parts:
        create_purchase_req(
            part_number=part['number'],
            quantity=1,
            priority="Normal" if alert['severity'] != 'critical' else "Urgent",
            requester="Predictive Maintenance System",
            notes=f"Based on alert: {alert['message']}"
        )
```

*Finance:*
```python
# Track actual maintenance costs vs predicted
def track_maintenance_roi():
    # Get prevented failures
    prevented = get_prevented_failures()
    
    # Calculate avoided costs
    avoided_costs = sum([
        failure['estimated_downtime_hours'] * facility_hourly_rate +
        failure['estimated_repair_cost']
        for failure in prevented
    ])
    
    # Get actual monitoring costs
    monitoring_costs = get_monitoring_system_costs()
    
    # Calculate ROI
    roi = (avoided_costs - monitoring_costs) / monitoring_costs * 100
    
    # Post to ERP financial module
    post_cost_savings(
        category="Predictive Maintenance",
        avoided_costs=avoided_costs,
        actual_costs=monitoring_costs,
        roi=roi
    )
```

---

## Business Intelligence Integration

### Data Export for Analytics

**CSV Export:**
```python
# Export historical data for analysis
def export_to_csv(device_id, start_date, end_date):
    data = requests.get(
        f"{API_URL}/telemetry/history/",
        headers=headers,
        params={
            "device_id": device_id,
            "start_time": start_date,
            "end_time": end_date,
            "limit": 100000
        }
    ).json()
    
    # Convert to pandas DataFrame
    import pandas as pd
    df = pd.DataFrame(data)
    df.to_csv(f"esocore_export_{device_id}.csv", index=False)
```

**Database Integration:**
```python
# Sync to data warehouse
import psycopg2

def sync_to_warehouse():
    # Connect to data warehouse
    conn = psycopg2.connect(
        host="warehouse.company.com",
        database="analytics",
        user="esocore_sync",
        password="password"
    )
    
    # Get latest data from EsoCore
    latest_data = get_latest_telemetry()
    
    # Insert into warehouse
    cursor = conn.cursor()
    for reading in latest_data:
        cursor.execute("""
            INSERT INTO equipment_telemetry 
            (timestamp, device_id, sensor_id, metric, value, unit)
            VALUES (%s, %s, %s, %s, %s, %s)
            ON CONFLICT (timestamp, device_id, sensor_id, metric) 
            DO UPDATE SET value = EXCLUDED.value
        """, (
            reading['timestamp'],
            reading['device_id'],
            reading['sensor_id'],
            reading['metric'],
            reading['value'],
            reading['unit']
        ))
    
    conn.commit()
    cursor.close()
    conn.close()
```

**Power BI Integration:**
```python
# Power BI REST API integration
def publish_to_powerbi(dataset_id, table_name, data):
    import requests
    
    # Get Power BI access token (OAuth2)
    token = get_powerbi_token()
    
    # Push data to Power BI
    response = requests.post(
        f"https://api.powerbi.com/v1.0/myorg/datasets/{dataset_id}/tables/{table_name}/rows",
        headers={
            "Authorization": f"Bearer {token}",
            "Content-Type": "application/json"
        },
        json={"rows": data}
    )
    
    return response.ok
```

---

## Best Practices

### Security

**API Key Management:**
- Generate separate keys for each integration
- Use read-only keys when possible
- Rotate keys regularly (quarterly recommended)
- Store keys securely (vault, environment variables)
- Never commit keys to source control

**Network Security:**
- Use TLS/HTTPS for all API connections
- Implement firewall rules limiting access
- Use VPN for remote integrations
- Monitor API usage for anomalies

### Performance

**Optimize API Usage:**
- Cache data when appropriate
- Use bulk endpoints vs many individual calls
- Implement exponential backoff for retries
- Set reasonable polling intervals (1-5 minutes typical)
- Use webhooks instead of polling when possible

**Data Management:**
- Only query data you need (use filters)
- Limit historical queries to necessary time ranges
- Implement pagination for large datasets
- Archive old data regularly

### Reliability

**Error Handling:**
```python
import time
from requests.adapters import HTTPAdapter
from requests.packages.urllib3.util.retry import Retry

def get_session_with_retries():
    session = requests.Session()
    retry = Retry(
        total=3,
        backoff_factor=1,
        status_forcelist=[429, 500, 502, 503, 504]
    )
    adapter = HTTPAdapter(max_retries=retry)
    session.mount('https://', adapter)
    return session

# Usage
session = get_session_with_retries()
response = session.get(url, headers=headers)
```

**Monitoring:**
- Log all integration errors
- Track API response times
- Monitor data freshness
- Alert on integration failures
- Maintain fallback procedures

---

## Support and Resources

### Documentation

- [REST API Reference](/docs/api-specification)
- [Hardware Specifications](/docs/esocore-edge)
- [Installation Guide](/docs/getting-started/installation-guide)
- [Quick Start Guide](/docs/getting-started/quick-start)

### Examples and Code

- GitHub repository: Sample integrations and code examples
- Community contributions: Share your integrations
- Professional services: Custom integration development available

### Support

- Community forum: GitHub Discussions
- Commercial support: support@newmatik.com
- Integration consulting: consulting@newmatik.com

---

Successful integration amplifies the value of equipment monitoring by connecting insights to action. Choose the integration methods that best fit your existing
infrastructure and workflows.

[API Documentation](/docs/api-specification) | [Installation Guide](/docs/getting-started/installation-guide) | [Quick Start](/docs/getting-started/quick-start)

