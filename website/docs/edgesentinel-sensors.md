# Hardware: Sensors

This document details all available EdgeSentinel sensor modules, their capabilities, and applications for industrial preventive maintenance and monitoring.

---

## Air Quality Sensor Module

### Features

- **Temperature & Humidity**: Precise environmental monitoring
- **CO₂ Monitoring**: Indoor air quality assessment
- **VOC Detection**: Volatile organic compound sensing
- **Particulate Matter**: PM2.5 and PM10 monitoring
- **IAQ Index**: Integrated Air Quality scoring
- **Pressure Sensing**: Barometric pressure measurement

### Applications

- HVAC system optimization
- Clean room monitoring
- Workplace air quality compliance
- Environmental control systems
- Predictive maintenance based on air quality trends

---

## Vibration Sensor Module

### Features

- **3-Axis Accelerometer**: High-precision ADXL355 sensor
- **FFT Analysis**: Frequency domain vibration analysis
- **RMS Calculation**: Root mean square vibration levels
- **Bearing Health Monitoring**: Early fault detection
- **Enhanced DSP**: STM32G071 MCU for advanced processing
- **Wide Frequency Range**: 1-3kHz ODR for comprehensive monitoring

### Applications

- Motor bearing condition monitoring
- Gearbox vibration analysis
- Pump cavitation detection
- Fan imbalance monitoring
- Structural integrity assessment
- Predictive maintenance scheduling

---

## Acoustic Sensor Module

### Features

- **Digital MEMS Microphone**: ICS-43434 for audible frequencies
- **Ultrasound Transducer**: MA40S4R for ultrasonic monitoring
- **Wide Frequency Range**: 50Hz to 40kHz coverage
- **Real-time Analysis**: On-device signal processing
- **Noise Pattern Recognition**: Tool wear and fault detection
- **Dual-frequency Monitoring**: Audible + ultrasonic analysis

### Applications

- Tool wear detection in machining
- Bearing lubrication monitoring
- Leak detection in pneumatic systems
- Gear mesh analysis
- Quality control in manufacturing
- Preventive maintenance alerts

---

## Current Sensor Module

### Features

- **Isolated Hall Sensor**: ACS723 ±30A range
- **High Accuracy**: Better linearity than previous models
- **Motor Load Analysis**: Torque and power calculations
- **Harmonic Content Analysis**: Power quality monitoring
- **Safety Isolation**: Enhanced electrical safety
- **Wide Current Range**: Suitable for various motor sizes

### Applications

- Motor load monitoring
- Energy consumption tracking
- Pump performance analysis
- Conveyor system monitoring
- HVAC fan current monitoring
- Predictive maintenance for electrical systems

---

## Light Sensor Module

### Features

- **Visible Light Sensor**: TSL2591 high dynamic range (0.002-32k lux)
- **UV Radiation Monitor**: VEML6075 UVA/UVB sensing
- **Infrared Sensor**: MLX90614 non-contact temperature
- **Color Sensor**: TCS34725 RGB with IR blocking filter
- **Spectral Analysis**: AS7341 10-channel spectral sensor
- **Multi-spectrum Monitoring**: Comprehensive light analysis

### Applications

- Workplace lighting compliance (OSHA/EU standards)
- UV exposure safety monitoring for workers
- Emergency lighting system validation
- Process control for light-sensitive manufacturing
- Color quality control in production
- Spectral analysis for material characterization
- Environmental light pollution monitoring

---

## Oil Quality Sensor Module

### Features

- **Turbidity Measurement**: Contamination detection in oil
- **Viscosity Sensing**: Piezoelectric viscosity monitoring
- **Water Content Detection**: Capacitive water sensing
- **Temperature Monitoring**: Oil temperature assessment
- **Dielectric Constant Analysis**: Oil condition assessment
- **Multi-parameter Analysis**: Comprehensive oil health monitoring

### Applications

- Gearbox oil condition monitoring
- Hydraulic oil quality assessment
- Lubrication system health monitoring
- Oil degradation trend analysis
- Preventive maintenance scheduling
- Contamination detection in industrial lubricants

---

## Pressure Sensor Module

### Features

- **Multi-range Sensing**: 0-1bar, 0-10bar, 0-100bar options
- **Hydraulic Monitoring**: High-pressure system analysis
- **Pneumatic Sensing**: Compressed air system monitoring
- **Leak Detection**: Pressure drop analysis
- **Filter Monitoring**: Clogging and efficiency assessment
- **Process Control**: Pressure validation for manufacturing

### Applications

- Hydraulic system pressure monitoring
- Pneumatic system leak detection
- Pump performance analysis
- Filter clogging prevention
- Process pressure control validation
- Safety system pressure monitoring

---

## Temperature Spot Sensor Module

### Features

- **NTC Thermistors**: 4× high-temperature NTC sensors
- **RTD Sensors**: 2× PT100 precision temperature measurement
- **Thermocouple Support**: 2× Type K thermocouple amplifiers
- **IR Temperature Sensor**: MLX90614 non-contact measurement
- **16-bit ADC**: High-precision temperature conversion
- **Extreme Environment Support**: Handles wide temperature ranges

### Applications

- Motor bearing temperature monitoring
- Gearbox hotspot detection
- Welding process temperature control
- Oven and furnace monitoring
- Hydraulic system temperature tracking
- Non-contact temperature measurement for moving parts

---

## Proximity Position Sensor Module

### Features

- **Inductive Sensors**: 2× 12mm sensing distance for metal detection
- **Capacitive Sensor**: Non-metal detection capability
- **Quadrature Encoder**: 1000PPR high-resolution positioning
- **Magnetic Sensors**: 2× reed switches for position sensing
- **Laser Distance Sensor**: VL53L0X precision ranging (up to 2m)
- **Industrial Connectors**: M12 standard connectors

### Applications

- Machine cycle counting
- Conveyor position monitoring
- Robotic arm position validation
- Door and gate position sensing
- Assembly line part detection
- Speed and RPM monitoring
- Precision distance measurement for automation

---

## Sensor Module Architecture

All EdgeSentinel sensor modules share a common architecture:

### Hardware Foundation

- **STM32G031 MCU**: 32MHz Cortex-M0+ for local processing
- **RS-485 Communication**: Industrial-grade bus interface
- **12V Power Supply**: Standard industrial voltage
- **IP65/IP67 Enclosures**: Harsh environment protection
- **DIN Rail Mounting**: Standard industrial mounting

### Software Capabilities

- **Local Processing**: On-device filtering and analysis
- **Real-time Monitoring**: Continuous data acquisition
- **Threshold Detection**: Configurable alarm triggers
- **Data Buffering**: Local storage during connectivity issues
- **Auto-calibration**: Self-maintaining accuracy
- **Hot-swap Support**: Runtime module replacement

### Integration Features

- **Plug-and-Play**: Automatic module detection
- **Address Configuration**: DIP switch addressing
- **LED Status Indicators**: Visual operational feedback
- **Cable Glands**: Strain relief and environmental sealing
- **Termination Options**: Bus termination configuration

---

## Applications by Industry

### Manufacturing & Machining

- **CNC Machines**: Vibration, acoustic, current, temperature monitoring
- **Stamping Presses**: Pressure, vibration, cycle counting
- **Grinding Equipment**: Acoustic monitoring, vibration analysis
- **Welding Stations**: Temperature, light, current monitoring

### Process Industry

- **Injection Molding**: Temperature, pressure, cycle monitoring
- **Packaging Lines**: Position, current, vibration monitoring
- **Chemical Reactors**: Temperature, pressure, air quality
- **Food Processing**: Temperature, humidity, air quality

### Material Handling

- **Conveyor Systems**: Vibration, position, current monitoring
- **Robotics**: Position, temperature, current monitoring
- **AGVs**: Position, current, proximity monitoring
- **Automated Storage**: Vibration, position, air quality

### Infrastructure

- **HVAC Systems**: Air quality, temperature, current monitoring
- **Pumps & Compressors**: Vibration, current, temperature monitoring
- **Motors & Gearboxes**: Vibration, current, oil quality monitoring
- **Hydraulic Systems**: Pressure, temperature, oil quality monitoring

---

## Sensor Selection Guide

### Choose by Application

- **Predictive Maintenance**: Vibration + Current + Temperature
- **Air Quality Compliance**: Air Quality + Light + Temperature
- **Process Control**: Pressure + Temperature + Position
- **Energy Monitoring**: Current + Voltage + Temperature
- **Safety Systems**: Light + Position + Pressure

### Choose by Equipment Type

- **Rotating Equipment**: Vibration + Current + Oil Quality
- **Hydraulic Systems**: Pressure + Temperature + Oil Quality
- **Pneumatic Systems**: Pressure + Acoustic + Position
- **Electrical Systems**: Current + Voltage + Temperature
- **Environmental Monitoring**: Air Quality + Light + Temperature

### Choose by Industry

- **Discrete Manufacturing**: Vibration + Acoustic + Position
- **Process Industry**: Pressure + Temperature + Air Quality
- **Material Handling**: Position + Current + Vibration
- **Infrastructure**: Current + Temperature + Oil Quality

---

## Sensor Compatibility Matrix

| Sensor Type | RS-485 Bus | PoE Compatible | Hazardous Areas | High Temperature |
|-------------|------------|----------------|-----------------|-----------------|
| Air Quality | ✅ | ✅ | ⚠️ (consult spec) | ✅ |
| Vibration | ✅ | ✅ | ✅ | ✅ |
| Acoustic | ✅ | ✅ | ✅ | ✅ |
| Current | ✅ | ⚠️ (isolated) | ⚠️ (consult spec) | ✅ |
| Light | ✅ | ✅ | ✅ | ✅ |
| Oil Quality | ✅ | ⚠️ (isolated) | ⚠️ (consult spec) | ✅ |
| Pressure | ✅ | ✅ | ⚠️ (consult spec) | ✅ |
| Temperature | ✅ | ✅ | ⚠️ (consult spec) | ✅ |
| Proximity | ✅ | ✅ | ✅ | ✅ |

**Legend:**

- ✅ Compatible
- ⚠️ Special considerations required
- Contact manufacturer for hazardous area certifications

---

## Technical Specifications Summary

| Sensor Module | Dimensions | Power | Communication | Enclosure | Weight |
|---------------|------------|-------|----------------|-----------|--------|
| Air Quality | 60×40mm | 12V, 0.5A | RS-485 | IP54 | 150g |
| Vibration | 40×30mm | 12V, 0.3A | RS-485 | IP67 | 120g |
| Acoustic | 40×30mm | 12V, 0.4A | RS-485 | IP54 | 130g |
| Current | 50×35mm | 12V, 0.6A | RS-485 | IP67 | 200g |
| Light | 45×35mm | 12V, 0.4A | RS-485 | IP65 | 140g |
| Oil Quality | 60×45mm | 12V, 0.8A | RS-485 | IP67 | 250g |
| Pressure | 50×40mm | 12V, 0.5A | RS-485 | IP67 | 180g |
| Temperature | 50×35mm | 12V, 0.4A | RS-485 | IP67 | 160g |
| Proximity | 55×40mm | 12V, 0.5A | RS-485 | IP67 | 170g |

---

## Installation & Configuration

### Mounting Options

- **DIN Rail**: Standard industrial mounting
- **Panel Mount**: Direct panel attachment
- **Magnetic Base**: Quick attachment/removal
- **Bracket Mount**: Adjustable positioning

### Configuration

- **DIP Switches**: Module address configuration
- **Software Setup**: Web interface configuration
- **Calibration**: Automatic or manual calibration options
- **Thresholds**: Configurable alarm thresholds

### Wiring

- **Power**: 12-24V DC with reverse polarity protection
- **Communication**: RS-485 bus with termination options
- **Sensors**: Industry-standard connectors (M12, terminal blocks)
- **Cable Types**: Shielded, twisted-pair industrial cables

---

## Maintenance & Support

### Sensor Calibration

- **Automatic**: Self-calibrating sensors
- **Manual**: Field calibration procedures
- **Scheduled**: Regular calibration intervals
- **Remote**: Over-the-air calibration updates

### Diagnostics

- **Self-test**: Built-in sensor diagnostics
- **Status LEDs**: Visual operational feedback
- **Data Validation**: Automatic data integrity checks
- **Fault Detection**: Sensor health monitoring

### Support

- **Documentation**: Comprehensive installation guides
- **Technical Support**: Manufacturer support channels
- **Firmware Updates**: Over-the-air updates
- **Training**: Installation and maintenance training
