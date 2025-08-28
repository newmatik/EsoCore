# Hardware: Core

This document defines the hardware specifications for the EdgeSentinel Core device - the central processing and connectivity hub of the EdgeSentinel monitoring platform.

---

## PCB Assembly

### Core Processing

- **MCU/SoC**: ARM Cortex‑M4/M7 or equivalent with sufficient RAM/flash for TLS and a filesystem; hardware crypto preferred
- **Storage**: industrial‑grade microSD (SLC/MLC with wear leveling), via SPI or SDIO
- **Enclosure**: DIN‑rail or compact IP54–IP65 industrial housing

### Connectivity

- **Ethernet (recommended)** with Power over Ethernet (PoE) support:
  - IEEE 802.3af compliant Powered Device (PD) controller
  - Integrated isolation transformer with PoE capability
  - 12.95W maximum power draw within 802.3af limits
  - Automatic detection and classification as Class 3 device
- **2.4 GHz Wi‑Fi** using a certified industrial module from a reputable vendor (e.g., u‑blox NINA series) with maintained TLS stack and regulatory approvals

### Industrial Fieldbus Support

- **Modbus RTU (RS‑485 half‑duplex)**: isolated transceiver, A/B/GND terminal block, on‑board termination (120 Ω) enable via DIP switch
- **Modbus TCP**: supported over the existing Ethernet interface
- **PROFIBUS DP**: isolated RS‑485 PHY compatible with PROFIBUS levels, DB‑9 male connector with shield, termination enable via DIP
- **PROFINET Device (single‑port)**: supported over Ethernet with RJ45 MagJack and PHY; optional 2‑port switch can be provisioned via expansion header in future revisions

### Power System

- **Primary Input Options**:
  - **12–24 V DC**: Terminal block input compatible with door control cabinets, including reverse‑polarity and surge protection
  - **Power over Ethernet (PoE)**: IEEE 802.3af compliant (up to 12.95W) with automatic detection and switching
- **Power Management**: Automatic source selection between DC input and PoE with priority given to DC input when both available
- **Backup**: High-capacity supercapacitor bank (3x 10F) for safe shutdown during power loss (≥30 seconds runtime)

---

## Sensor Bus Architecture

EdgeSentinel uses a **distributed sensor network** where each sensor module contains its own MCU for local processing and standardized digital communication. This architecture provides:

- **Scalability**: Connect multiple sensors of the same type (e.g., 3 air quality, 2 vibration sensors)
- **Flexibility**: Add/remove sensors based on specific machine requirements
- **Noise Immunity**: Digital signals over industrial RS-485 bus
- **Local Intelligence**: Each sensor performs conditioning, filtering, and preprocessing
- **Simplified Wiring**: Single 4-wire bus supports all sensor modules

### Integration Strategy

- **Modular by design**: New sensor types (including partner‑specific or proprietary modules) can be introduced without any changes to the Core or firmware architecture. The RS‑485 protocol layer abstracts capabilities, so the platform scales by adding sensor modules rather than redesigning hardware.
- **Hot‑plug and discovery**: Auto‑enumeration assigns addresses and registers sensor capabilities at runtime, enabling field upgrades and customer‑specific configurations.

### Sensor Bus Protocol Selection

- **Physical Layer**: RS-485 differential signaling over shielded twisted pair (Belden 3105A or equivalent)
- **Cable**: 4-conductor (Power+, Power-, Data+, Data-) with drain wire for shield grounding
- **Distance**: Up to 100 meters from Core to sensors with automatic repeaters
- **Speed**: 115.2 kbps for real-time data with deterministic latency (<10ms)
- **Topology**: Multi-drop bus with daisy-chain or star configurations
- **Addressing**: 8-bit addresses (supports 254 sensor modules per bus) configured via DIP switches
- **Power Distribution**: +12V @ 2A total bus power with individual module draw limits
- **Termination**: 120Ω resistors at bus endpoints with bias resistors for idle state
- **Error Detection**: CRC16 checksums and automatic retry mechanism
- **Discovery Protocol**: Automatic enumeration with sensor type identification and capability reporting

### Digital Sensor Bus Interface

- **RS-485 Transceiver**: SN65HVD72 half-duplex transceiver with fail-safe biasing
- **Bus Power**: TPS54331 12V @ 2A switching supply with current limiting and short-circuit protection
- **Termination**: 120Ω termination resistors at bus endpoints with 560Ω bias resistors
- **Main Device Connectors**:
  - Primary sensor bus port (4-pin terminal block) for backbone connection
  - Secondary sensor bus port for daisy-chain or redundant connections
  - Dedicated terminal blocks for Safety I/O (8-pin input, 6-pin output)
  - General I/O terminal block (12-pin) for analog/digital signals
  - Fieldbus terminal block (6-pin) for Modbus RTU/PROFIBUS
- **Sensor Module Connectors**: 4-pin screw terminal blocks (5.08mm pitch) with PG9 cable glands for field serviceability
- **Distribution Strategy**: IP54/IP67 junction boxes with screw terminal strips for sensor bus fan-out
- **Cable System**: Standard 4-conductor shielded RS-485 cable (Belden 3105A) cut to length with field-installed terminals
- **Discovery Protocol**: Broadcast enumeration with collision detection and automatic address assignment
- **Hot-Plug Support**: Sensors can be added/removed during operation with automatic re-discovery
- **Isolation**: Bus interface isolated from main MCU for noise immunity and safety
- **Status Monitoring**: Bus voltage and current monitoring with fault detection and reporting

---

## Safety I/O

- **Safety-rated digital inputs** (4–6): optocoupled, 24V for safety edges, light curtains, emergency stops
- **Safety-rated relay outputs** for door control interlocking (optional)
- **Compliance**: EN ISO 13849 (Safety of machinery) for functional safety

## General I/O

- **Digital inputs**: 2–4 additional optocoupled inputs for discrete signals
- **Analog inputs**: 1–2 analog inputs (4–20 mA/0–10 V) for legacy process signals
- **Relay output**: 1 relay output for service indicators (optional)
- **Addressing & Termination**: 8‑position DIP switch for node addressing/config and bus termination control (Modbus/PROFIBUS)

## Timing & Clock

- **RTC**: 32.768 kHz RTC crystal + supercap; NTP over network

---

## User Interface (on-device)

- **OLED Status Display**: 0.96" 128×64 monochrome OLED (I²C, 3.3 V) mounted on board for debugging, provisioning info, and basic menu UI
- **Menu Buttons**: 3 × SMD tactile buttons placed below the display (Up/Down/Enter) for simple navigation; two additional buttons reserved for Reset/Boot already specified
- **Electrical**: I²C pulled‑up to 3.3 V; debounce in firmware; ESD protection on button lines via existing arrays
- **Mechanical**: Keepout and bezel height to ensure readability in enclosure; optional standoffs for display support

---

## STM32 Reference Implementation

### Core Components

- **MCU**: STM32 family with enough RAM/flash for TLS + filesystem (e.g., STM32F4/F7/H7 depending on crypto/perf needs)
- **Storage**: microSD (industrial grade, SLC/MLC, wear leveling), SPI or SDIO
- **Enclosure**: DIN‑rail or compact IP54–IP65 industrial housing

### Connectivity Options

- **Ethernet (recommended)** with Power over Ethernet (PoE) support:
  - 10/100 Mbps Ethernet PHY
  - IEEE 802.3af compliant PoE Powered Device (PD) controller
  - RJ45 MagJack connector with integrated isolation transformer
  - Automatic power source detection and management
- **2.4 GHz Wi‑Fi** via certified industrial module (u‑blox NINA‑W1/W10x or similar) with proven security maintenance
- **Fieldbus**: RS‑485 isolated transceiver (Modbus RTU), PROFIBUS‑compatible RS‑485 PHY with DB‑9; Modbus TCP/PROFINET over Ethernet

### Safety I/O (EN ISO 13849 compliant)

- **4–6 safety-rated digital inputs**: safety edges, light curtains, emergency stops, door sensors
- **Optocoupled inputs** with 24V compatibility and built-in diagnostics
- **Safety relay outputs** for door control interlocking and fault indication

### General I/O (Reference Implementation)

- **2–4 additional digital inputs** (optocoupled)
- **1–2 analog inputs** (4–20 mA/0–10 V)
- **1 relay output** for service indicators

### Power & Backup System

- **Primary**: 12–24V DC input with surge protection and EMI filtering
- **Supercapacitor bank**: 30F total capacity (e.g., 3x 10F Maxwell supercaps) for 30+ second safe shutdown
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

- **DC Input**: 12–24V DC nominal via terminal block
  - Reverse polarity protection, surge protection, EMI filtering
  - 3A maximum current draw at 12V
- **Power over Ethernet (PoE)**: IEEE 802.3af compliant
  - Automatic detection and classification as Class 3 device (6.49–12.95W)
  - Maximum power consumption: 12.95W at 37–57V input
  - Isolation transformer with PoE signature resistor (25.5kΩ)
  - Integrated switching controller with current limiting

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
