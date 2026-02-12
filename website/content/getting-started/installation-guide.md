# EsoCore Installation Guide

Complete installation guide for deploying EsoCore industrial IoT monitoring across your facility. This comprehensive guide covers hardware
installation, network configuration, sensor deployment, cloud platform setup, and system integration for production deployments.

---

## Prerequisites

### Hardware Requirements

**Per Monitored Machine:**

- 1× EsoCore Edge device (STM32H747-based)
- 1-5× Sensor modules (based on monitoring requirements)
- 24V DC power supply (2A minimum, dedicated external supply recommended)
- Industrial microSD card (8GB+, SLC or MLC with wear leveling)
- Ethernet cable (Cat5e or better) OR WiFi access
- RS-485 cable (4-conductor shielded, Belden 3105A or equivalent)

**Tools Required:**

- Screwdrivers (Phillips and flathead)
- Wire strippers and crimpers
- Multimeter for voltage verification
- Drill and mounting hardware
- Cable ties and labels
- Laptop or tablet for configuration

### Network Requirements

**Connectivity Options:**

- Dual Ethernet with DHCP on IT port and static IP on machine port (recommended)
- Static IP assignment on both ports
- WiFi (2.4 GHz, WPA2 minimum)

**Bandwidth:**

- Minimal: <1 KB/min per device normal operation
- Peak: 10-50 KB during anomaly events
- Cloud sync: 50-100 MB/month per device

**Firewall Requirements:**

- Outbound HTTPS (port 443) for cloud sync
- Optional: Inbound HTTPS (port 443) for local web UI access
- Optional: Modbus TCP (port 502), PROFINET for integration

### Skills Required

**Basic Installation:**

- Industrial equipment familiarity
- Basic electrical knowledge (12-24V DC safety)
- Network configuration basics
- Reading installation diagrams

**Advanced Installation:**

- Industrial protocol knowledge (Modbus, PROFINET)
- System integration experience
- Network troubleshooting capabilities

---

## Installation Planning

### Step 1: Equipment Assessment

**Identify Monitoring Priorities:**

1. List all equipment to be monitored
2. Classify by criticality (High/Medium/Low)
3. Document historical failure modes
4. Determine sensor requirements per machine
5. Calculate total hardware requirements

**Site Survey:**

- Verify 24V DC power availability
- Check network connectivity at each location
- Identify sensor mounting locations
- Plan cable routing paths
- Document environmental conditions

**Create Installation Plan:**

- Deployment schedule (phased approach recommended)
- Resource allocation (personnel, tools, time)
- Downtime windows for installation
- Testing and validation timeline
- Training schedule

### Step 2: Hardware Procurement

**Edge Devices:**

- Quantity needed: [_____]
- Configuration: Standard or custom
- Accessories: Mounting hardware, power supplies
- Lead time: 2-4 weeks typical

**Sensor Modules:**

- Vibration sensors: [_____]
- Temperature sensors: [_____]
- Current sensors: [_____]
- Pressure sensors: [_____]
- Acoustic sensors: [_____]
- Other sensors: [_____]

**Cables and Accessories:**

- RS-485 cable length needed: [_____] meters
- Ethernet cables: [_____]
- Power cables: [_____]
- Junction boxes: [_____]
- Cable management: Ties, conduit, labels

**Spare Parts:**

- Recommend 10% spare sensors
- 2-3 spare Edge devices
- Spare cables and connectors

---

## Edge Device Installation

### Physical Installation

**Location Selection:**

- Within 100 meters of sensors (RS-485 range limit)
- Protected from physical damage
- Temperature: -20°C to +60°C
- Protected from moisture and contaminants
- Good WiFi signal if not using Ethernet
- Accessible for maintenance and viewing display

**Mounting Options:**

_DIN Rail Mount:_

1. Measure required rail space (10cm width typical)
2. Position device on 35mm DIN rail
3. Press down until clips engage
4. Verify secure mounting

_Wall Mount:_

1. Mark mounting hole locations
2. Drill holes appropriate for wall material
3. Install anchors if needed
4. Secure device with provided screws
5. Ensure level and secure mounting

_Panel Mount:_

1. Cut opening in panel per template
2. Insert device from front
3. Secure with provided brackets
4. Seal edges if needed for environment

### Power Connection

**24V DC Power Connection:**

1. Verify power supply voltage: 24V DC nominal
2. Confirm current capacity: 2A minimum
3. Identify polarity (typically red = +, black = -)
4. **Double-check polarity before connecting!**
5. Connect wires to terminal block:
   - V+ to positive terminal
   - GND to negative terminal
6. Tighten terminal screws securely
7. Apply power
8. Verify power LED illuminates
9. Check OLED display shows boot sequence

**Power Verification:**

- Display shows firmware version and IP address
- Status LEDs indicate normal operation
- No error messages on display
- Device responds to network ping

### Network Configuration

**DHCP Configuration (Default):**

1. Connect Ethernet cable or configure WiFi
2. Device obtains IP automatically
3. Note IP address from OLED display
4. Verify connectivity: `ping [IP_ADDRESS]`
5. Access web UI: `https://[IP_ADDRESS]`

**Static IP Configuration:**

1. Initially connect via DHCP to access web UI
2. Navigate to Settings → Network
3. Change from DHCP to Static
4. Enter static IP configuration:
   - IP Address: [_____]
   - Subnet Mask: [_____]
   - Gateway: [_____]
   - DNS Server: [_____]
5. Apply settings
6. Device reboots with new IP
7. Verify connectivity at new IP address

**WiFi Configuration:**

1. Power on device without Ethernet connected
2. Device enters AP mode: "EsoCore-[SERIAL]"
3. Connect laptop/phone to this network
4. Browse to: http://192.168.4.1
5. Enter WiFi credentials:
   - SSID: [_____]
   - Password: [_____]
   - Security: WPA2 (recommended)
6. Click "Connect"
7. Device reboots and connects to WiFi
8. Note IP address from display
9. Verify connectivity

**Network Troubleshooting:**

- No IP address: Check DHCP server, cable connection
- Cannot access web UI: Check firewall, use HTTPS not HTTP
- Intermittent connectivity: Check WiFi signal strength, interference
- Slow response: Check network congestion, bandwidth

### USB Connection (Maintenance and Development)

**Purpose:**

The USB connection on the EsoCore Edge device is used exclusively for:

- **Firmware flashing:** Updating device firmware
- **Serial communication:** Terminal access for diagnostics and direct device communication

**Important Notes:**

- USB is NOT used for normal operation or data collection
- USB is NOT used for network connectivity or data transfer
- Use network interfaces (Ethernet/WiFi) for operational communication
- USB access requires physical connection to the device

**Serial Terminal Access:**

1. Connect USB cable between Edge device and computer
2. Install appropriate USB-to-serial drivers if needed
3. Use terminal application (PuTTY, screen, minicom):
   - Baud rate: 115200
   - Data bits: 8
   - Parity: None
   - Stop bits: 1
4. Access diagnostic commands and system logs

**Firmware Updates:**

1. Download firmware file (.bin or .hex)
2. Connect via USB
3. Enter bootloader mode (follow device-specific procedure)
4. Flash firmware using appropriate tool
5. Device reboots with new firmware

---

## Sensor Installation

### Vibration Sensor Installation

**Location Selection:**

- Close to rotating components (bearings, gears)
- Rigid mounting surface (not flexible sheet metal)
- Accessible for future inspection
- Away from excessive heat or contamination

**Mounting Methods:**

_Stud Mount (Preferred - Best Accuracy):_

1. Drill and tap mounting hole (M6 or M8 typical)
2. Clean mounting surface thoroughly
3. Apply thin layer of coupling compound
4. Thread sensor onto stud
5. Tighten to specified torque
6. Verify sensor is firmly mounted

_Magnetic Mount (Quick Installation):_

1. Clean mounting surface (remove paint, oil, rust)
2. Ensure surface is flat and smooth
3. Place magnet base on surface
4. Verify strong magnetic attachment
5. Apply safety cable as backup

_Adhesive Mount (Temporary or Testing):_

1. Clean surface with isopropyl alcohol
2. Apply industrial adhesive or epoxy
3. Press sensor firmly to surface
4. Allow curing time per adhesive specs
5. Verify bond before leaving

**Wiring:**

1. Route 4-conductor shielded cable to sensor
2. Strip wire ends carefully
3. Connect to terminal block:
   - Power+ (red)
   - Power- (black)
   - Data+ (green/A)
   - Data- (white/B)
4. Secure strain relief
5. Connect shield to ground at one end only

**DIP Switch Addressing:**

1. Each sensor needs unique address (1-254)
2. Set address using binary DIP switches
3. Example addresses:
   - 00000001 = Address 1
   - 00000010 = Address 2
   - 00000011 = Address 3
   - etc.
4. Document address for each sensor
5. Label sensor with address number

### Temperature Sensor Installation

**Mounting Locations:**

- Bearing housings (direct contact)
- Motor bodies
- Hydraulic reservoirs
- Gearbox housings
- Process fluids (with thermowells)

**Installation:**

_Surface Mount:_

1. Clean mounting surface
2. Apply thermal compound for good contact
3. Secure sensor with clamp or adhesive
4. Insulate from ambient if needed
5. Verify good thermal contact

_Thermowell Installation (Process Fluids):_

1. Install thermowell in process per specifications
2. Fill thermowell with thermal compound
3. Insert temperature sensor
4. Secure compression fitting
5. Allow thermal stabilization (15-30 minutes)

**Wiring:** Same 4-conductor bus connection as vibration sensors Set unique DIP switch address

### Current Sensor Installation

**CT Clamp Installation:**

1. Identify motor power conductors
2. Select appropriate clamp size for conductor
3. Open clamp jaws
4. Place around **single** conductor (not bundled cable)
5. Close clamp until it clicks
6. Arrow on clamp indicates current flow direction
7. Secure clamp to prevent movement
8. For 3-phase motors: Install clamp on each phase

**Important Safety Notes:**

- **Never** work on live circuits without proper training
- Follow lockout/tagout procedures
- Verify circuits are de-energized before installation
- Use appropriate PPE
- Follow electrical codes and regulations

**Wiring:** Same 4-conductor bus connection Set unique DIP switch address

### Additional Sensor Types

**Pressure Sensors:**

- Install in pressure taps with appropriate fittings
- Use thread sealant or tape
- Verify pressure range matches application
- Consider pulsation dampeners if needed

**Acoustic Sensors:**

- Mount near sound source (bearings, valves, leaks)
- Avoid mounting on vibrating surfaces
- Use foam isolation pad if needed
- Protect from moisture and contamination

**Proximity/Position Sensors:**

- Mount with specified air gap
- Align with target surface/object
- Verify detection distance
- Secure mounting to prevent movement

### RS-485 Bus Wiring

**Cable Routing:**

1. Use 4-conductor shielded cable (Belden 3105A or equivalent)
2. Keep away from power cables (minimum 12" separation)
3. Avoid parallel runs with AC power
4. Use conduit or cable tray for protection
5. Support cable every 3-6 feet
6. Avoid sharp bends (minimum 4× cable diameter radius)

**Daisy Chain Topology:**

```text
Edge Device → Sensor 1 → Sensor 2 → Sensor 3 → ... → Last Sensor
```

1. Connect Edge device to first sensor (4 wires)
2. Connect first sensor to second sensor (4 wires)
3. Continue chain to last sensor
4. Maximum total length: 100 meters
5. Shield grounded at one end only (usually at Edge device)

**Termination:**

- Install 120Ω termination resistor at last sensor
- Some sensors have DIP switch for built-in termination
- Required for proper signal integrity
- Edge device typically has built-in termination on its end

**Bus Power:**

- Edge device supplies +12V to sensor bus
- Total current limit: 2A
- Typical sensor consumption: 50-150mA each
- Calculate total: [Number of sensors] × [150mA] = [_____] mA
- Must be less than 2000mA

---

## System Configuration

### Initial Setup

**Access Web Interface:**

1. Note IP address from Edge device display
2. Open web browser on computer (same network)
3. Navigate to: `https://[IP_ADDRESS]`
4. Accept self-signed certificate warning (first time only)
5. Default credentials:
   - Username: `admin`
   - Password: `esocore`

**First-Time Setup Wizard:**

1. Change admin password (required)
2. Set device name and location
3. Configure timezone and NTP server
4. Set network preferences (if not using defaults)
5. Review settings and apply

### Sensor Discovery

**Automatic Discovery:**

1. Navigate to: Sensors → Discovery
2. Click "Scan RS-485 Bus"
3. System discovers all connected sensors
4. Review discovered sensors list
5. Verify expected sensors found

**Assign Descriptive Names:** For each sensor:

1. Click sensor to edit
2. Change name from default to descriptive:
   - "Motor 1 Drive End Bearing"
   - "Pump Discharge Pressure"
   - "Spindle Temperature"
3. Assign location/machine association
4. Set measurement units if applicable
5. Save changes

### Sensor Configuration

**Sampling Rates:**

Configure appropriate rates for each sensor type:

_Vibration Sensors:_

- Analysis windows: 1-3 kHz for 1-5 seconds
- Window frequency: Every 5-15 minutes
- Continuous RMS: 1-10 Hz
- Alert evaluation: Every 30-60 seconds

_Temperature Sensors:_

- Sampling rate: 0.1-1 Hz (every 1-10 seconds)
- Alert evaluation: Every 60 seconds
- Thermal time constant consideration

_Current Sensors:_

- Sampling rate: 100-500 Hz
- RMS calculation: 1-10 Hz
- Power analysis: Every 1-5 seconds

_Pressure Sensors:_

- Sampling rate: 1-10 Hz
- Alert evaluation: Every 10-60 seconds
- Filtering for pulsations if needed

**Measurement Ranges:**

- Select appropriate range for application
- Avoid saturation (clipping)
- Optimize for signal-to-noise ratio
- Can be adjusted based on baseline data

### Baseline Collection

**Understanding Baselines:**

- System needs 2-4 weeks to learn normal operation
- Automatic process (no interaction required)
- Alerts disabled during baseline period
- Must include all operating modes

**Monitoring Progress:**

1. Navigate to Dashboard → Baseline Status
2. View percentage complete for each sensor
3. Green checkmarks indicate sufficient data
4. Typical completion: 2-4 weeks

**Accelerating Baseline:**

- Operate equipment through all modes
- Include startups and shutdowns
- Cover different load conditions
- Run through full operational range
- 24/7 operation speeds learning

### Alert Configuration

**After Baseline Completes:**

1. Navigate to Settings → Alerts
2. Review suggested thresholds:
   - Warning: 20% deviation from baseline
   - Attention: 40% deviation
   - Critical: 80% deviation
3. Adjust sensitivity:
   - More sensitive = earlier warnings, more false positives
   - Less sensitive = fewer alerts, potentially late warnings
4. Start with suggested values
5. Tune based on experience

**Notification Methods:**

_Email:_

1. Settings → Notifications → Email
2. Enter SMTP server details:
   - Server: smtp.example.com
   - Port: 587 (TLS) or 465 (SSL)
   - Username: [_____]
   - Password: [_____]
3. Add recipient email addresses
4. Select severity levels to notify
5. Test email delivery

_Local Outputs:_

1. Wire relay outputs to alarm devices
2. Configure trigger conditions
3. Test before relying on

_CMMS Integration:_

1. Settings → Integrations → CMMS
2. Enter CMMS API endpoint
3. Configure authentication
4. Enable auto work order creation
5. Map alert severities to work order priorities

---

## Cloud Platform Setup (Optional)

### Self-Hosted Deployment

**Server Requirements:**

- Linux server (Ubuntu 20.04+ recommended)
- 4 CPU cores minimum
- 8 GB RAM minimum
- 100 GB storage
- Docker and Docker Compose installed

**Installation:**

```bash
# Clone EsoCore repository
git clone https://github.com/newmatik/EsoCore.git
cd EsoCore/server

# Install Python dependencies with Poetry
pip install poetry
poetry install

# Run database migrations
poetry run python manage.py migrate

# Create admin user
poetry run python manage.py createsuperuser

# Start the server
poetry run python manage.py runserver 0.0.0.0:8000
```

> **Note:** Docker deployment files are planned but not yet included.
> For production, configure a WSGI/ASGI server (e.g. Gunicorn) behind a reverse proxy (e.g. Nginx).

**Access Platform:**

- Admin interface: https://[SERVER_IP]/admin
- API endpoint: https://[SERVER_IP]/api/
- Configure DNS and SSL certificates for production

### Managed Cloud Service

**Sign Up:**

1. Visit: https://cloud.esocore.com (if using managed service)
2. Create account
3. Select plan based on device count
4. Enter payment information

**Device Provisioning:**

1. Navigate to Devices → Add Device
2. Enter device serial number
3. Generate API key
4. Note API key (shown once)
5. Device status shows "Pending"

**Edge Device Configuration:**

1. On Edge device web UI: Settings → Cloud
2. Enter cloud platform URL
3. Enter device API key
4. Configure sync interval (5-15 minutes recommended)
5. Click "Test Connection"
6. Enable "Auto Sync"
7. Device appears online in cloud platform

---

## System Integration

### Modbus Integration

**Modbus RTU (RS-485):**

1. Connect Modbus devices to dedicated RS-485 port
2. Configure in web UI:
   - Settings → Fieldbus → Modbus RTU
   - Baud rate: 9600, 19200, or 115200
   - Device addresses to poll
   - Register map configuration
3. Enable Modbus polling
4. Verify data appears in dashboard

**Modbus TCP:**

1. Configure in web UI:
   - Settings → Fieldbus → Modbus TCP
   - Enable Modbus TCP server
   - Port: 502 (default)
2. Configure Modbus master to connect to Edge device IP
3. Provide register map documentation
4. Test read/write operations

### PROFINET Integration

1. Settings → Fieldbus → PROFINET
2. Configure device name (GSD file provided)
3. Set I/O data mapping
4. Import into TIA Portal or PLC configuration tool
5. Configure PLC to read EsoCore data
6. Test communication

### REST API Integration

**Authentication:**

- Token authentication for users (via `Authorization: Token <token>`)
- API key authentication for devices (via `X-Auth-Key` header)

**Common Endpoints:**

```http
GET /api/devices/devices/         # List devices
GET /api/devices/sites/           # List sites
GET /api/telemetry/points/        # Get telemetry data
GET /api/events/events/           # Get system events
GET /api/dashboard/summary/       # Dashboard summary
```

**Example Integration:**

```python
import requests

API_URL = "https://your-server/api"
TOKEN = "your-auth-token"
headers = {"Authorization": f"Token {TOKEN}"}

# Get current telemetry data
response = requests.get(f"{API_URL}/telemetry/points/", headers=headers)
data = response.json()
```

---

## Testing and Validation

### Installation Verification Checklist

**Hardware:**

- [ ] All Edge devices powered and online
- [ ] All sensors discovered and responding
- [ ] Network connectivity stable
- [ ] Display shows correct status
- [ ] No error LEDs or messages

**Configuration:**

- [ ] Admin password changed from default
- [ ] Device names and locations set
- [ ] All sensors named descriptively
- [ ] Sampling rates configured
- [ ] Time zone and NTP configured
- [ ] Network settings correct (static IP if applicable)

**Data Collection:**

- [ ] Live data visible on dashboard
- [ ] Sensor readings appear reasonable
- [ ] Data logging to SD card verified
- [ ] Cloud sync functioning (if enabled)
- [ ] No data gaps or errors

**Alerts:**

- [ ] Test email notifications working
- [ ] CMMS integration tested (if configured)
- [ ] Local alarm outputs tested (if used)
- [ ] Alert thresholds set appropriately

### Functional Testing

**Test Scenarios:**

1. **Normal Operation:** Verify data collection during normal equipment operation
2. **Network Loss:** Disconnect network, verify local operation and data buffering
3. **Power Loss:** Simulate power loss, verify safe shutdown and data integrity
4. **Sensor Fault:** Disconnect sensor, verify fault detection and alerting
5. **Alert Testing:** Create alert condition, verify notification delivery

**Performance Verification:**

- Response time: <100ms for edge processing
- Data accuracy: Compare to reference instruments
- Alert latency: <1 second from detection to notification
- System uptime: 99.9% target

---

## Documentation

### Installation Records

Create documentation package including:

**Equipment List:**

- Edge device serial numbers and locations
- Sensor serial numbers and locations
- IP addresses (static or DHCP)
- API keys and credentials (securely stored)

**Wiring Diagrams:**

- RS-485 bus topology
- Power connections
- Network connections
- Integration connections

**Configuration:**

- Network settings
- Sensor configurations
- Alert thresholds
- Integration settings

**Contact Information:**

- Technical support
- Vendor contacts
- Internal IT contacts
- Maintenance team

### User Training

**Operator Training:**

- Accessing web interface
- Viewing dashboard and data
- Responding to alerts
- Basic troubleshooting

**Maintenance Training:**

- System overview
- Alert interpretation
- Planned maintenance based on alerts
- Advanced configuration

**Administrator Training:**

- User management
- System configuration
- Integration management
- Troubleshooting

---

## Maintenance

### Routine Maintenance

**Monthly:**

- Review system health dashboard
- Verify all sensors responding
- Check SD card health
- Review alert logs

**Quarterly:**

- Clean sensors and connections
- Inspect cable conditions
- Verify sensor mounting secure
- Update firmware if available

**Annually:**

- Sensor calibration verification
- Battery backup replacement (if applicable)
- SD card replacement (preventive)
- System configuration backup

### Troubleshooting

**Common Issues:**

See [Quick Start Guide](/docs/getting-started/quick-start) for detailed troubleshooting steps.

---

## Support Resources

- **Documentation:** [Complete technical docs](/docs)
- **Community:** GitHub Discussions
- **Commercial Support:** support@newmatik.com
- **Emergency:** Include escalation procedures

---

Comprehensive installation complete! Your industrial IoT monitoring system is now operational and protecting your critical equipment.

[Quick Start Guide](/docs/getting-started/quick-start) | [Integration Guide](/docs/getting-started/integration-guide) |
[API Documentation](/docs/api-specification)
