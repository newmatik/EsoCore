# Hardware: Edge

This document defines the hardware specifications for the EsoCore Edge device — the central processing and connectivity hub of the EsoCore monitoring
platform.

---

## Edge Hardware Requirements

### Edge Processing

- **MCU/SoC**: STM32H747IIT6 dual-core Cortex-M7/M4 with sufficient RAM/flash for TLS and a filesystem; hardware crypto
  preferred; 1.2 V LDO (TLV70212DBVR) for internal regulator bypass; ferrite bead (BLM18PG121SN1D) on VDD rail
- **Clock**: 25 MHz active oscillator (shared between STM32 and Ethernet PHYs); 32.768 kHz RTC crystal
- **Storage**: industrial-grade microSD (SLC/MLC with wear leveling) via SDIO, with EMIF06-MSD02N16 EMI filter on all
  SDIO lines for EMC compliance and signal integrity
- **Enclosure**: DIN-rail or compact IP54-IP65 industrial housing

### Connectivity

- **Dual Ethernet** with physical network separation:
  - 2x 10/100 Mbps Ethernet ports (2x RJ-45) with independent PHY ICs (KSZ8081RNACA)
  - Port A (Machine Network): PROFINET RT, Modbus TCP, SINUMERIK HMI, PLC communications (static IP, e.g., 192.168.10.x)
  - Port B (IT Network): HTTPS cloud sync, REST API, NTP, OTA updates (DHCP, e.g., 10.x.x.x)
  - True physical isolation between OT and IT networks (no VLAN hopping, independent failure domains)
  - Industrial temperature range PHYs (-40C to +85C), auto-MDIX, integrated magnetics with galvanic isolation
  - See [Dual Ethernet Ports](/docs/ethernet) for full architecture and PCB layout guidelines
- **2.4 GHz Wi-Fi** using ESP32-S3-WROOM-1-N16R8 module (WiFi 802.11b/g/n + Bluetooth 5.0, 16 MB flash, 8 MB PSRAM) with
  maintained TLS stack and regulatory approvals; USB VBUS LDO (AMS1117-3.3) for programming without main power; dedicated
  Tag-Connect TC2030 debug port; auto-programming circuit (UMH3N) for DTR/RTS UART flashing

### Industrial Fieldbus Support

EsoCore uses **dual Ethernet ports with physical separation**: Port A (Machine Network) carries all OT protocols (PROFINET RT, Modbus TCP, on-device
web UI), while Port B (IT Network) handles cloud connectivity, OTA updates, and REST API access. This dual-port architecture provides true network
isolation between operational technology and IT infrastructure.

- **Modbus RTU (RS-485 half-duplex)**: isolated transceiver, A/B/GND terminal block, on-board termination (120 ohm) enable via DIP switch
- **Modbus TCP**: supported over Port A machine Ethernet interface (TCP port 502)
- **PROFIBUS DP**: isolated RS-485 PHY compatible with PROFIBUS levels, DB-9 male connector with shield, termination enable via DIP
- **PROFINET Device**: supported over Port A machine Ethernet interface (RT via Layer-2 frames, EtherType 0x8892); dual-port design supports
  future MRP (Media Redundancy Protocol) capability
- **Interbus**: dedicated isolated RS-485 transceiver for legacy Bosch Rexroth controller machines; master/slave summation-frame protocol with
  deterministic cyclic timing; see [Industrial Fieldbus Protocols](/docs/fieldbus) for implementation details

### Power System

- **Primary Input**: Dedicated **24 V DC** external power supply via terminal block
  - Designed for maximum robustness against grid disturbances and supply issues
  - Aggressive surge/ESD protection: TVS diodes (SMAJ24A), varistors, input LC filter
  - Reverse-polarity protection, resettable polyfuse overcurrent protection
  - Brownout-safe behavior with early warning detection and graceful degradation
  - Robust 24 V to 5 V buck conversion (TPS54331, 3A) with dedicated low-noise analog 3.3 V LDO (TPS7A4533)
  - Input bulk capacitor: 470 uF for industrial transient ride-through
- **Backup**: Supercapacitor (3.8 V rated, 20 F) with Schottky diode charge path and 680 ohm inrush limiting for safe
  shutdown during power loss (>=30 seconds runtime target)

---

## Data Acquisition Architecture

EsoCore supports three complementary data acquisition methods: an RS-485 sensor bus for distributed sensor modules, on-board dual-mode analog
inputs for vibration accelerometers or general-purpose analog sensors, and on-board safety digital inputs for controller/encoder signals and
safety devices. This multi-path approach gives integrators maximum flexibility to match the monitoring strategy to each machine.

### RS-485 Sensor Bus

The RS-485 sensor bus is a **distributed sensor network** where each sensor module contains its own MCU for local processing and standardized
digital communication. This architecture provides:

- **Scalability**: Connect multiple sensors of the same type (e.g., 3 air quality, 2 vibration sensors)
- **Flexibility**: Add/remove sensors based on specific machine requirements
- **Noise Immunity**: Digital signals over industrial RS-485 bus
- **Local Intelligence**: Each sensor performs conditioning, filtering, and preprocessing
- **Simplified Wiring**: Single 4-wire bus supports all sensor modules

### Integration Strategy

- **Modular by design**: New sensor types (including partner‑specific or proprietary modules) can be introduced without any changes to the Edge or
  firmware architecture. The RS‑485 protocol layer abstracts capabilities, so the platform scales by adding sensor modules rather than redesigning
  hardware.
- **Hot‑plug and discovery**: Auto‑enumeration assigns addresses and registers sensor capabilities at runtime, enabling field upgrades and
  customer‑specific configurations.

### Sensor Bus Protocol Selection

- **Physical Layer**: RS-485 differential signaling over shielded twisted pair (Belden 3105A or equivalent)
- **Cable**: 4-conductor (Power+, Power-, Data+, Data-) with drain wire for shield grounding
- **Distance**: Up to 100 meters from Edge to sensors with automatic repeaters
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
  - Dual-mode analog input terminal block (10-pin) for 4 IEPE or DC sensor channels
  - Safety digital input terminal block (14-pin) for 6 dual-channel safety inputs
  - Fieldbus terminal block (6-pin) for Modbus RTU/PROFIBUS
  - Interbus terminal block (4-pin)
- **Sensor Module Connectors**: 4-pin screw terminal blocks (5.08mm pitch) with PG9 cable glands for field serviceability
- **Distribution Strategy**: IP54/IP67 junction boxes with screw terminal strips for sensor bus fan-out
- **Cable System**: Standard 4-conductor shielded RS-485 cable (Belden 3105A) cut to length with field-installed terminals
- **Discovery Protocol**: Broadcast enumeration with collision detection and automatic address assignment
- **Hot-Plug Support**: Sensors can be added/removed during operation with automatic re-discovery
- **Isolation**: Bus interface isolated from main MCU for noise immunity and safety
- **Status Monitoring**: Bus voltage and current monitoring with fault detection and reporting

### On-Board Dual-Mode Analog Inputs (4 Channels)

The Edge device includes 4 on-board analog input channels, each supporting two operating modes selectable per channel via firmware GPIO:

- **IEPE Mode**: For IEPE/ICP vibration accelerometers. The 4 mA constant-current source is enabled and the signal is AC-coupled (10 uF film
  capacitor) to strip the sensor DC bias. Provides sensor-limited noise performance (~380 uVrms) across a 0.5 Hz to 10 kHz bandwidth.
- **DC Mode**: For general-purpose analog sensors (0-10 V, 4-20 mA with shunt resistor, thermocouples with external conditioning). The current
  source is disabled and a TMUX1101 analog switch bypasses the AC coupling capacitor, preserving the full DC content.

Both modes share the same high-performance signal chain: input protection (TVS + series resistor), programmable gain amplifier (PGA280, gains
1/2/5/10/20), 4-pole Sallen-Key anti-aliasing filter (15 kHz), and a 4-channel simultaneous 24-bit delta-sigma ADC (ADS1274) at 50 kSPS per
channel.

See [Analog Input Specification](/docs/edge-analog-inputs) for the complete circuit architecture, mode switching details, noise budget, and PCB
layout requirements.

### On-Board Safety Digital Inputs (6 Channels)

The Edge device includes 6 dual-channel safety digital input channels (24 V, optocoupled) compliant with EN ISO 13849 Category 3 architecture.
Each channel uses two independent optocouplers (12 total) with firmware cross-monitoring: both channels must agree for a valid input state; any
discrepancy triggers a safety fault and forces the system to a safe state. Response time is <10 ms.

These inputs are suitable for safety edges, emergency stops, light curtains, door sensors, as well as general-purpose signals such as encoder
pulses, cycle counts, and machine states from PLCs or controllers. Periodic test pulses verify optocoupler health and detect stuck-at faults.

### Relay Output

1 solid-state relay output (AQY212EHAZ, 60V/500mA) for service indicators or interlock signaling, plus 1 dual-channel safety relay
(G7SA-2A2B-DC24, 2NO+2NC) with DRV110APWR coil driver for interlocking applications.

### Addressing and Termination

8-position DIP switch for node addressing/config and bus termination control (Modbus/PROFIBUS).

---

## Timing & Clock

- **HSE clock**: 25 MHz active oscillator shared between STM32 and Ethernet PHY(s)
- **RTC**: 32.768 kHz RTC crystal + CR2032 coin cell backup; NTP over network

---

## User Interface (on-device)

- **OLED Status Display**: 1.5" 128×64 monochrome OLED (I²C, 3.3 V, Winstar WEO015664A) mounted on board for debugging, provisioning info, and
  basic menu UI
- **Menu Buttons**: 4 × SMD tactile buttons placed below the display (Up/Down/Enter/Back) for menu navigation; two additional buttons reserved for
  Reset/Boot already specified
- **Electrical**: I2C with BSS138LT1G MOSFET level shifter between MCU 3.3 V and OLED VDD domain; debounce in firmware;
  ESD protection on button lines via existing arrays
- **Mechanical**: Keepout and bezel height to ensure readability in enclosure; optional standoffs for display support

---

## STM32 Reference Implementation

### Edge Components

- **MCU**: STM32H747IIT6 (dual-core M7/M4) with enough RAM/flash for TLS + filesystem; 1.2 V LDO (TLV70212DBVR) for
  internal regulator bypass; ferrite bead (BLM18PG121SN1D) on VDD; 25 MHz active oscillator shared with Ethernet PHYs
- **Storage**: microSD (industrial grade, SLC/MLC, wear leveling) via SDIO with EMIF06-MSD02N16 EMI filter
- **Enclosure**: DIN-rail or compact IP54-IP65 industrial housing

### Connectivity Options

- **Dual Ethernet** with physical network separation:
  - 2x 10/100 Mbps Ethernet PHY (KSZ8081RNACA, industrial -40C to +85C)
  - 2x RJ-45 MagJack with integrated magnetics (Wurth 7499111121) and galvanic isolation
  - Port A (Machine): PROFINET RT, Modbus TCP, on-device web UI (static IP, OT network)
  - Port B (IT): HTTPS cloud sync, REST API, NTP, OTA updates (DHCP, corporate network)
  - Independent MACs, independent PHYs, true physical network isolation
- **2.4 GHz Wi-Fi** via ESP32-S3-WROOM-1-N16R8 (WiFi 802.11b/g/n + Bluetooth 5.0) with proven security maintenance;
  USB VBUS LDO (AMS1117-3.3) for standalone programming; auto-programming circuit (UMH3N); dedicated TC2030 debug port
- **Fieldbus**: RS-485 isolated transceiver (Modbus RTU), PROFIBUS-compatible RS-485 PHY with DB-9, dedicated Interbus
  RS-485 transceiver; Modbus TCP/PROFINET use Port A Ethernet

### I/O Summary (Reference Implementation)

- **4 dual-mode analog inputs**: IEPE or DC per channel; 24-bit simultaneous ADC (ADS1274), 50 kSPS, PGA, 15 kHz AAF, TMUX1101 mode switch
- **6 safety digital inputs**: dual-channel optocoupled, 24 V, EN ISO 13849 Cat. 3, cross-monitored, <10 ms response
- **Safety relay output**: 1x G7SA-2A2B-DC24 dual-channel relay (2NO+2NC) with DRV110APWR driver for PLd interlocking
- **1 SSR output**: AQY212EHAZ solid-state relay (60V/500mA) for service indicator or interlock

### Power & Backup System

- **Primary**: Dedicated 24 V DC input with aggressive surge/ESD protection, input LC filtering, and brownout-safe behavior
- **Conversion**: 24 V to 5 V buck (TPS54331, 3A) plus dedicated low-noise analog 3.3 V LDO (TPS7A4533), digital 3.3 V
  LDO, and 1.2 V LDO (TLV70212DBVR) for STM32 internal regulator bypass
- **Supercapacitor**: 3.8 V rated, 20 F with Schottky diode charge path (CDBA240LL-HF) and 680 ohm inrush limiting for
  30+ second safe shutdown
- **Power monitoring**: voltage/current sensors with brownout/blackout detection and early warning

### Timing

- **Clocks/time**: 25 MHz active oscillator (STM32 + Ethernet PHYs); 32.768 kHz RTC crystal + CR2032 backup; NTP sync
  via network

---

## Compliance & Robustness Standards

### EMC/ESD Requirements

- **EMC/ESD**: EN 61000‑6‑2/-6‑4, IEC 61000‑4‑2/-4‑4/-4‑5 where relevant

### Safety Standards

- **EN ISO 13849** (Safety of machinery) - functional safety compliance for dual-channel safety digital inputs
- **EN 60204-1** (Electrical equipment of machines) for electrical safety
- **Safety**: creepage/clearance on mains‑adjacent parts, safety circuit isolation

### Environmental Specifications

- **Operating range**: −20 °C to +60 °C (target), 5–95% RH non‑condensing
- **Power backup reliability**: Supercap bank rated for 50,000+ cycles, −40 °C to +65 °C operation

---

## Power Management Specifications

### Power Input

- **DC Input**: Dedicated 24 V DC nominal via terminal block
  - Designed for maximum robustness against grid disturbances and supply issues
  - Reverse polarity protection (P-channel MOSFET or ideal diode)
  - Aggressive surge protection: TVS diodes (SMAJ24A) plus varistors (MOV) for transient clamping
  - Input LC filter (47 uH inductor + 470 uF capacitor) for EMI suppression
  - Resettable polyfuse for overcurrent protection
  - 3A maximum current draw at 24 V
  - Brownout detection with configurable thresholds and early warning
  - Controlled power path for industrial environments with unstable mains

### Backup Power System

- **Technology**: Supercapacitor (3.8 V rated, 20 F) with Schottky diode charge path and 680 ohm inrush limiting
- **Runtime**: >=30 seconds for safe shutdown procedures (evaluate adding second supercap if needed)
- **Monitoring**: Continuous voltage and charge state monitoring
- **Temperature range**: -40 C to +65 C operation
- **Cycle life**: 50,000+ charge/discharge cycles

### Power Conversion

- **24 V to 5 V**: TPS54331 buck converter (3A, non-synchronous), powers IEPE current sources and digital logic
- **5 V to 3.3 V (digital)**: TLV1117LV33 LDO for digital MCU and peripheral rails
- **5 V to 3.3 V (analog)**: TPS7A4533 low-noise LDO (dedicated, isolated from digital rail) for ADC and analog signal
  chain
- **3.3 V to 1.2 V**: TLV70212DBVR LDO for STM32H747 internal regulator bypass (VCAP/DSI pins)
- **Power sequencing**: Analog rails powered before ADC to prevent latch-up

### Power Monitoring

- **Input monitoring**: Voltage and current sensing on 24 V input
- **Brownout detection**: Configurable threshold early warning system for voltage sags
- **Blackout detection**: Immediate power loss detection with timestamped event logging
- **Backup activation**: Automatic switchover to supercap backup with controlled shutdown sequence
