# Hardware Requirements

This document defines the hardware specifications, component requirements, and compliance standards for the EdgeSentinel edge monitoring device.

---

## PCB Assembly

### Core Processing

- **MCU/SoC**: ARM Cortex‑M4/M7 or equivalent with sufficient RAM/flash for TLS and a filesystem; hardware crypto preferred
- **Storage**: industrial‑grade microSD (SLC/MLC with wear leveling), via SPI or SDIO
- **Enclosure**: DIN‑rail or compact IP54–IP65 industrial housing; ambient sensor either airflow‑exposed or externally cabled

### Connectivity

- **Ethernet (recommended)** with optional PoE (802.3af), or
- **2.4 GHz Wi‑Fi** using an industrial module with a maintained TLS stack

### Power System

- **Input**: 12–24 V DC input compatible with door control cabinets, including reverse‑polarity and surge protection; PoE option if chosen
- **Backup**: Supercapacitor bank for safe shutdown during power loss (≥30 seconds runtime)

---

## Sensing & I/O Requirements

### Environmental Sensors

- **Ambient Temp/RH**: digital, factory‑calibrated I²C sensor
- **Drive stator temperature**: contact or embedded NTC/PT100/PT1000 or digital sensor on stator/end‑winding

### Mechanical Sensors

- **Door cycles**:
  - Count rising edges from limit switches/reed sensors, or
  - Quadrature encoder on drive shaft for high‑fidelity cycle/position (optional)
- **Vibration**: 3‑axis accelerometer mounted on motor/gearbox; support ≥1–3 kHz ODR for diagnostic windows
- **Acoustics**: MEMS microphone(s) with bandwidth into ultrasound range (≥40–80 kHz optional) to capture operational quality and catastrophic events

### Motor Monitoring

- **Torque**:
  - Start with motor current proxy via isolated Hall current sensor on motor phase/DC bus; calibrate to torque
  - Allow drop‑in direct torque/strain sensor later for higher accuracy

### Safety I/O

- **Safety-rated digital inputs** (4–6): optocoupled, 24V for safety edges, light curtains, emergency stops
- **Safety-rated relay outputs** for door control interlocking (optional)
- **Compliance**: EN ISO 13849 (Safety of machinery) for functional safety

### General I/O

- **Digital inputs**: 2–4 additional optocoupled inputs
- **Analog inputs**: 1–2 analog inputs (4–20 mA/0–10 V)
- **Relay output**: 1 relay output for service indicators (optional)

### Timing & Clock

- **RTC**: 32.768 kHz RTC crystal + supercap; NTP over network

---

## STM32 Reference Implementation

### Core Components

- **MCU**: STM32 family with enough RAM/flash for TLS + filesystem (e.g., STM32F4/F7/H7 depending on crypto/perf needs)
- **Storage**: microSD (industrial grade, SLC/MLC, wear leveling), SPI or SDIO
- **Enclosure**: DIN‑rail or compact IP54–IP65 industrial housing; ambient sensor should be airflow‑exposed or externally cabled

### Connectivity Options

- **Ethernet (recommended)** with optional PoE (802.3af), or
- **2.4 GHz Wi‑Fi** (industrial module with maintained TLS stack)

### Detailed Sensor Specifications

#### Environmental Monitoring

- **Ambient Temp/RH**: digital sensor (e.g., SHT3x/4x). I²C, factory‑calibrated
- **Drive stator temperature**: NTC/PT100/PT1000 or digital temperature sensor bonded to stator; ADC or I²C/SPI

#### Mechanical Monitoring

- **Door cycles**:
  - Count rising edges from limit switches/reed sensors, or
  - Read quadrature encoder on the drive shaft for high‑fidelity cycle + position (optional)
- **Vibration**: 3‑axis accelerometer (e.g., ADXL345/ADXL355/ICM‑42688). Mount on motor/gearbox housing; support ≥1–3 kHz ODR for diagnostic windows
- **Acoustics**: wide‑band MEMS microphone (optionally up to 80 kHz) for audible/ultrasound; capture high‑amplitude events (e.g., spring break)

#### Motor Monitoring

- **Torque (two options)**:
  - Proxy via motor current: isolated Hall current sensor on motor phase/DC bus; calibrate against known torque vs. current curve
  - Direct strain or inline torque sensor (higher BOM, better accuracy). Start with current proxy; allow future drop‑in of direct sensor

### Safety I/O (EN ISO 13849 compliant)

- **4–6 safety-rated digital inputs**: safety edges, light curtains, emergency stops, door sensors
- **Optocoupled inputs** with 24V compatibility and built-in diagnostics
- **Safety relay outputs** for door control interlocking and fault indication

### General I/O

- **2–4 additional digital inputs** (optocoupled)
- **1–2 analog inputs** (4–20 mA/0–10 V)
- **1 relay output** for service indicators

### Power & Backup System

- **Primary**: 12–24V DC input with surge protection and EMI filtering
- **Supercapacitor bank**: 10–30F (e.g., 6x 5F Maxwell supercaps) for 30+ second safe shutdown
- **Power monitoring**: voltage/current sensors with brownout/blackout detection

### Timing

- **Clocks/time**: 32.768 kHz RTC crystal + dedicated supercap backup; NTP sync via network

---

## Compliance & Robustness Standards

### EMC/ESD Requirements

- **EMC/ESD**: EN 61000‑6‑2/-6‑4, IEC 61000‑4‑2/-4‑4/-4‑5 where relevant

### Safety Standards

- **EN ISO 13849** (Safety of machinery) - functional safety compliance
- **IEC 61508** (Functional safety) for safety-related systems
- **EN 60204-1** (Electrical equipment of machines) for electrical safety
- **Safety**: creepage/clearance on mains‑adjacent parts, safety circuit isolation

### Environmental Specifications

- **Operating range**: −20 °C to +60 °C (target), 5–95% RH non‑condensing
- **Power backup reliability**: Supercap bank rated for 50,000+ cycles, −40 °C to +65 °C operation

---

## Power Management Specifications

### Power Input

- **Voltage range**: 12–24V DC nominal
- **Protection**: Reverse polarity protection, surge protection, EMI filtering
- **PoE option**: 802.3af compatibility where Ethernet is used

### Backup Power System

- **Technology**: Supercapacitor bank (10–30F total capacity)
- **Runtime**: ≥30 seconds for safe shutdown procedures
- **Monitoring**: Continuous voltage and charge state monitoring
- **Temperature range**: −40 °C to +65 °C operation
- **Cycle life**: 50,000+ charge/discharge cycles

### Power Monitoring

- **Input monitoring**: Voltage and current sensing
- **Brownout detection**: Early warning system for voltage sags
- **Blackout detection**: Immediate power loss detection
- **Backup activation**: Automatic switchover to supercap backup
