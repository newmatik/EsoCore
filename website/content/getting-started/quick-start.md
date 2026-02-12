# EsoCore Quick Start Guide

Get up and running with EsoCore industrial IoT monitoring in under 30 minutes. This guide walks you through your first equipment monitoring
deployment, from hardware setup to viewing live sensor data and configuring alerts.

---

## Prerequisites

Before starting, ensure you have:

**Hardware:**

- 1 EsoCore Edge device (STM32H747-based)
- 1 or more sensor modules (vibration, temperature, or current sensor recommended for first deployment)
- 24V DC power supply (dedicated external supply recommended for maximum robustness)
- Ethernet cables (2x recommended: one for machine network, one for IT network) or WiFi credentials
- microSD card (industrial grade, 8GB+ recommended)

**Network:**

- Ethernet connection (preferred) or WiFi access
- DHCP or static IP configuration
- Outbound HTTPS access for cloud sync (optional)
- Local network access for configuration web UI

**Skills:**

- Basic understanding of industrial equipment
- Ability to mount sensors on equipment
- Basic networking knowledge (helpful but not required)

**Time Required:**

- Hardware installation: 15-30 minutes
- Initial configuration: 10-15 minutes
- Baseline data collection: 2-4 weeks (automatic, no interaction needed)
- Total hands-on time: 25-45 minutes

---

## Step 1: Hardware Setup (15-30 minutes)

### Edge Device Installation

**Location Selection:**

- Within 100 meters of monitored equipment (for RS-485 sensor bus)
- Protected from physical damage and extreme temperatures
- Good WiFi signal if not using Ethernet
- Access to 24V DC power supply

**Mounting:**

1. Mount Edge device on DIN rail or wall mount
2. Ensure adequate ventilation around device
3. Position so status display and buttons are accessible
4. Keep away from high EMI sources (VFDs, arc welders, etc.)

**Power Connection:**

_24V DC Power:_

1. Connect 24V DC power supply to terminal block
2. Observe polarity marking on device
3. Reverse polarity protection prevents damage but won't power on if backwards
4. Power LED should illuminate when correctly connected

**Network Connection:**

_Option A: Dual Ethernet (Recommended)_

1. Connect Ethernet cable to Port A (Machine Network) for PLC/SCADA integration
2. Connect Ethernet cable to Port B (IT Network) for cloud sync and remote access
3. Port A defaults to static IP; Port B obtains IP via DHCP
4. Note displayed IP addresses from OLED screen
5. Static IP configuration available via web UI for both ports

_Option B: WiFi_

1. Power on device (initially in WiFi setup mode)
2. Connect phone/laptop to "EsoCore-XXXX" WiFi network
3. Browse to 192.168.4.1
4. Enter your WiFi SSID and password
5. Device connects and displays IP on OLED screen

### Sensor Installation

**Vibration Sensor (Example):**

1. Clean mounting surface on equipment
2. Apply thin layer of coupling compound (for best accuracy)
3. Mount sensor with stud or magnet base
4. Location recommendations:
   - Motors: Housing near drive end bearing
   - Pumps: Pump body near bearing
   - Gearboxes: Housing near gear mesh
   - General: Closest to rotating components

**Temperature Sensor (Example):**

1. Identify critical temperature measurement point
2. Mount sensor in contact with surface
3. Secure with thermal adhesive or mechanical clamp
4. Ensure good thermal contact for accurate readings
5. Locations: Bearing housings, motor bodies, hydraulic tanks

**Current Sensor (Example):**

1. Identify motor power leads
2. Clamp CT around single phase conductor
3. Do not clamp around bundled cable (measures net current = 0)
4. Arrow on CT indicates current flow direction
5. Secure CT to prevent movement

**Sensor Bus Wiring:**

1. Run 4-conductor shielded cable from Edge to first sensor
2. Connect to sensor terminal block:
   - Power+ (red)
   - Power- (black)
   - Data+ (green)
   - Data- (white)
3. For multiple sensors, daisy-chain from first to next
4. Maximum bus length: 100 meters total
5. Install 120Ω termination resistor at final sensor

**DIP Switch Addressing:**

1. Each sensor needs unique address (1-254)
2. Set via DIP switches on sensor module
3. Binary encoding: 00000001 = address 1, 00000010 = address 2, etc.
4. Document addresses for your reference

---

## Step 2: Initial Configuration (10-15 minutes)

### Access Web Interface

1. Note IP address from Edge device OLED display
2. Open web browser on computer connected to same network
3. Navigate to `https://[IP_ADDRESS]`
4. Accept self-signed certificate (first-time setup)
5. Default credentials: admin / esocore (change immediately)

### Device Configuration

**Network Settings:**

1. Navigate to Settings → Network
2. Configure static IP if required (optional, DHCP works for most)
3. Set DNS servers if needed
4. Configure NTP server for time synchronization (critical for data timestamps)

**Change Default Password:**

1. Navigate to Settings → Security
2. Change admin password to secure value
3. Document password securely

**Sensor Discovery:**

1. Navigate to Sensors → Discovery
2. Click "Scan Bus" button
3. System auto-discovers all connected sensors
4. Verify expected sensors appear in list
5. Assign descriptive names (e.g., "Motor 1 Bearing DE", "Pump Inlet Pressure")

**Sensor Configuration:**

1. Click on each sensor to configure
2. Set sampling rates:
   - Vibration: 1-3 kHz for analysis windows
   - Temperature: 1 Hz continuous
   - Current: 100-500 Hz
   - Pressure: 1-10 Hz
3. Configure measurement ranges if sensors have multiple options
4. Enable desired measurements (some sensors have multiple channels)

### Cloud Sync Configuration (Optional)

If using cloud platform for historical analytics:

1. Navigate to Settings → Cloud
2. Enter cloud platform URL
3. Enter device API key (from cloud provisioning)
4. Configure sync interval (recommend 5-15 minutes)
5. Click "Test Connection" to verify
6. Enable "Auto Sync" when ready

**Note:** Cloud sync is optional. Edge device operates fully independently with local storage and alerting.

---

## Step 3: Baseline Data Collection (Automatic)

### Understanding Baselines

EsoCore needs 2-4 weeks of normal operation data to establish baseline patterns for each machine. This is automatic—no interaction required, just let
it run.

**What's Happening:**

- Sensors continuously collect data
- Edge device processes and stores on local SD card
- Statistical baselines calculated for:
  - Vibration RMS and frequency spectra
  - Temperature normal operating ranges
  - Current signatures and load patterns
  - Pressure normal ranges and stability
- Seasonal and operational patterns identified
- Hour-of-day and day-of-week patterns recognized

**During Baseline Period:**

- Alerts are disabled (learning mode)
- Status dashboard shows live data
- Data is stored locally on SD card (30+ days capacity)
- Optional cloud sync uploads data for historical viewing
- OLED display shows "Baseline Mode" status

**After Baseline Period:**

- System automatically enables anomaly detection
- Alert thresholds configured based on learned baselines
- Monitoring transitions from passive to active mode
- You'll receive email notification when complete

### Monitoring Baseline Progress

1. Navigate to Dashboard
2. View "Baseline Status" widget
3. Shows percentage complete for each sensor
4. Green checkmarks appear when baseline is sufficient
5. Typically completes in 2-4 weeks depending on equipment operating patterns

**Accelerating Baseline Collection:**

- Ensure equipment operates in all typical modes
- Include startup and shutdown cycles
- Cover different load conditions if applicable
- Run through full operational range

---

## Step 4: View Live Data

While baseline collects, you can view live sensor data:

### Dashboard Overview

1. Navigate to Dashboard (home screen)
2. See tile for each monitored machine
3. Current values displayed for all sensors
4. Color coding indicates status:
   - Gray: Baseline mode (learning)
   - Green: Normal operation
   - Yellow: Minor deviation (watch)
   - Orange: Significant deviation (attention needed)
   - Red: Critical deviation (maintenance required)

### Sensor Detail View

1. Click on any sensor tile
2. View real-time waveforms (vibration, current)
3. See FFT spectra for frequency analysis
4. Historical trend charts (temperature, pressure)
5. Statistical metrics (RMS, peak, average, etc.)

### Time-Series Charts

1. Navigate to Analytics → Trends
2. Select sensor(s) to view
3. Choose time range (last hour, day, week, month)
4. Overlay multiple sensors for correlation
5. Zoom and pan to examine details
6. Export data as CSV if needed

### FFT Waterfall Display

For vibration sensors:

1. Navigate to Analytics → FFT Analysis
2. View waterfall display showing frequency content over time
3. Identify specific frequencies of interest
4. Compare to bearing and gear frequencies
5. Look for developing patterns

---

## Step 5: Configure Alerts (After Baseline)

Once baseline collection completes (2-4 weeks), configure alerting:

### Alert Thresholds

1. Navigate to Settings → Alerts
2. System suggests thresholds based on baseline:
   - Warning: 20% deviation from baseline
   - Attention: 40% deviation
   - Critical: 80% deviation
3. Adjust sensitivity based on your preferences:
   - More sensitive = earlier warnings, more false positives
   - Less sensitive = fewer alerts, potentially late warnings
4. Start with suggested values and tune based on experience

### Alert Methods

**Email Alerts:**

1. Navigate to Settings → Notifications → Email
2. Enter SMTP server details
3. Configure recipient list
4. Set alert severity levels to send (recommend critical + attention)
5. Test email delivery

**SMS Alerts (via SMTP-to-SMS gateway):**

1. Use carrier's email-to-SMS gateway
2. Configure as email recipient
3. Example: 5551234567@txt.att.net

**Local Alarm Output:**

1. Wire relay output to alarm light or horn
2. Configure trigger conditions
3. Test before relying on

**CMMS Integration:**

1. Navigate to Settings → Integrations → CMMS
2. Enter CMMS API endpoint
3. Configure authentication
4. Enable auto work order creation
5. Test integration

### Alert Rules

Create custom alert logic:

1. Navigate to Settings → Alert Rules
2. Click "New Rule"
3. Define conditions:
   - Sensor: Select from list
   - Metric: Choose parameter (RMS, temperature, etc.)
   - Condition: Threshold and duration
   - Example: "Vibration RMS > 8 m/s² for >5 minutes"
4. Set severity level
5. Assign notification recipients
6. Enable rule

---

## Step 6: First Maintenance Action

### When Alert Triggers

**Investigation Workflow:**

1. Review alert details:
   - Which sensor triggered?
   - What parameter exceeded threshold?
   - By how much?
   - How long has it been elevated?

2. Check correlated sensors:
   - Has temperature also increased?
   - Has current draw changed?
   - Are there acoustic changes?
   - Multiple symptoms = higher confidence

3. View historical trend:
   - Is this sudden or gradual?
   - When did deviation start?
   - What is rate of change?
   - Gradual = predictable failure, sudden = investigate cause

4. Plan maintenance action:
   - Schedule during next available maintenance window
   - Order replacement parts if needed
   - Prepare tools and resources
   - Document planned action

**Maintenance Execution:**

1. Perform identified maintenance (bearing replacement, etc.)
2. Note findings during maintenance
3. Verify issue was as predicted
4. Document in CMMS
5. Update EsoCore with maintenance record

**Post-Maintenance:**

1. Equipment returns to normal operation
2. Sensor values return to baseline
3. System recalculates baseline to include new component
4. Alert clears automatically
5. Document prevented failure value (avoided downtime and catastrophic damage)

---

## Step 7: Expand Monitoring

After success with first machine:

**Add More Sensors to Same Machine:**

1. Install additional sensor modules
2. Connect to RS-485 bus
3. System auto-discovers
4. Configure and start baseline

**Add Additional Machines:**

1. Install Edge device on next machine
2. Follow same setup process
3. Leverage learnings from first deployment
4. Standardize sensor packages by equipment type

**Fleet Analytics:**

1. With multiple machines monitored, compare performance
2. Identify outliers (best and worst performers)
3. Benchmark similar equipment
4. Share best practices across fleet

---

## Common Issues and Solutions

### Issue: Edge Device Won't Power On

**Troubleshooting:**

- Verify 24V DC polarity is correct
- Check power supply output voltage (should be 24V DC nominal)
- Check for blown fuse (if equipped)
- Power LED should be solid when powered

**Solution:** Verify power source, check polarity, test with known good supply

### Issue: Cannot Connect to Web UI

**Troubleshooting:**

- Verify device and computer on same network
- Check IP address on OLED display
- Try pinging device IP
- Disable firewall temporarily to test
- Clear browser cache

**Solution:** Use IP from OLED, ensure network connectivity, try different browser

### Issue: Sensors Not Discovered

**Troubleshooting:**

- Verify sensor power LED is illuminated
- Check RS-485 wiring (Data+, Data-, Power+, Power-)
- Verify shield is grounded at one end only
- Check DIP switch addresses are unique
- Try one sensor at a time to isolate issue
- Verify 120Ω termination resistor at bus end

**Solution:** Check wiring systematically, verify power at each sensor, test one at a time

### Issue: False Alerts After Baseline

**Troubleshooting:**

- Review alert threshold settings
- Check if equipment operation changed
- Verify sensors are securely mounted
- Look for new sources of vibration or noise
- Review baseline data quality

**Solution:** Increase alert thresholds slightly, extend baseline period, improve sensor mounting

### Issue: High Vibration Readings

**Note:** This might be real, not an issue!

**Investigation:**

- Compare to equipment specifications
- Check if readings correlate with equipment condition
- Verify sensor mounting is secure and proper
- Review installation location (on rigid structure, not flexible)
- Look for external vibration sources

**Solution:** If sensor installation is correct, this is likely detecting real issue—schedule inspection

---

## Next Steps

### Learn More

- **Full Documentation:** [www.esocore.com/docs](/docs)
- **Hardware Specifications:** [Edge device details](/docs/esocore-edge)
- **Sensor Guide:** [Sensor specifications](/docs/esocore-sensors)
- **API Documentation:** [REST API reference](/docs/api-specification)

### Expand Capabilities

- **Predictive Maintenance:** [Implementation guide](/docs/solutions/predictive-maintenance)
- **CNC Monitoring:** [Application-specific guide](/docs/industries/cnc-machine-monitoring)
- **Door Monitoring:** [Industrial door guide](/docs/industries/industrial-door-monitoring)
- **Integration Guide:** [Connect to existing systems](/docs/getting-started/integration-guide)

### Get Support

- **Community Forum:** GitHub Discussions
- **Issue Tracking:** GitHub Issues
- **Commercial Support:** support@newmatik.com
- **Professional Services:** consulting@newmatik.com

### Best Practices

- Start with 1-2 machines to learn system
- Document sensor locations and addresses
- Take photos of installations for reference
- Keep spare sensors on hand
- Review dashboard weekly during first month
- Tune alert thresholds based on experience
- Document prevented failures for ROI tracking

---

## Success Checklist

After completing this guide, you should have:

- [ ] Edge device installed and powered
- [ ] Sensors mounted on equipment
- [ ] RS-485 bus wired correctly
- [ ] Network connectivity established
- [ ] Web UI accessible
- [ ] Default password changed
- [ ] Sensors discovered and named
- [ ] Sampling rates configured
- [ ] Baseline collection running
- [ ] Dashboard showing live data
- [ ] Alert thresholds configured (or will configure after baseline)
- [ ] Notification methods configured
- [ ] Documentation created for installation

---

Congratulations! You've completed your first EsoCore deployment. Over the next 2-4 weeks, the system will learn your equipment's normal behavior and
begin detecting developing issues. You're now on the path to predictive maintenance and reduced downtime.

**Questions?** Join our community discussions or contact support@newmatik.com

[View Full Documentation](/docs) | [Installation Guide](/docs/getting-started/installation-guide) | [Get Support](https://github.com/newmatik/EsoCore/issues)
