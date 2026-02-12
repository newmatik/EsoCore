# EsoCore: Universal Industrial Machine Monitoring System

EsoCore is an advanced industrial IoT platform for comprehensive monitoring of factory machinery and equipment. By capturing critical operational
data—from thermal conditions to mechanical health indicators—this system enables predictive maintenance, reduces downtime, and optimizes equipment
performance across manufacturing facilities.

EsoCore is an acronym: Edge Sensor Observation, Control, Operations, Reliability Engine. The name also recognizes our heritage—Newmatik began as ESO
Electronic Service Ottenbreit GmbH in 1990—carrying that tradition forward as we build everything new and automatic today.

---

## Introduction

### The Problem

Unplanned equipment failures cost manufacturers millions in lost production, emergency repairs, and quality issues. Traditional reactive maintenance
leads to premature replacements, unexpected downtime, and safety hazards. This is especially critical for high-volume production, precision
manufacturing, and safety-critical operations where equipment reliability is paramount.

### The Solution

A robust edge-to-cloud telemetry platform that continuously monitors machine health through multiple sensor streams, providing early warning of issues
and actionable insights for maintenance teams. The system is designed to work with virtually all industrial machinery types and integrates with
existing factory systems, making it universally deployable across diverse manufacturing environments.

### Development Model

EsoCore is developed and maintained by Newmatik through a collaborative partnership model with strategic industrial partners. The entire
platform—including hardware designs, firmware, and cloud components—is fully open source under the Apache 2.0 License,
ensuring maximum flexibility and preventing vendor lock-in. Partners can develop proprietary market-specific
customizations while benefiting from shared platform development costs and continuous innovation. This unique approach
combines open-source principles with commercial viability, creating a sustainable ecosystem where partners retain full
independence and can fork the project if needed.

### Competitive Position

EsoCore differentiates itself in the industrial IoT market through cross-domain capability (supporting both industrial doors and CNC machines in one
platform), edge-first TinyML architecture for ultra-low latency anomaly detection, and comprehensive safety integration. While competitors often focus
on single verticals with proprietary platforms, EsoCore's open-source foundation and unified approach provides unique value for facilities managing
diverse industrial assets.

### Key Benefits

- **Predictive Maintenance**: Detect issues before failure occurs using AI-powered edge intelligence
- **Edge Intelligence**: On-device anomaly detection with TinyML for faster response
- **Operational Insights**: Track usage patterns and energy efficiency with algorithmic analysis
- **Fleet Management**: Monitor multiple machines from a centralized dashboard with ML-driven analytics
- **Cost Reduction**: Optimize maintenance schedules and extend equipment life through intelligent predictions
- **Compliance**: Maintain audit trails for safety and regulatory requirements
- **Universal Compatibility**: Works with virtually all industrial machinery and manufacturing equipment
- **Complete Data Visibility**: Comprehensive monitoring for mission-critical production environments

---

## Target Applications

**Machining & Metal Working:**

- CNC milling machines, lathes, turning centers
- Stamping presses, punching machines, press brakes
- Grinding machines, surface finishing equipment
- Welding stations, cutting systems (plasma, laser, waterjet)

**Process Manufacturing:**

- Injection molding machines, extrusion equipment
- Packaging lines, filling and sealing machines
- Chemical reactors, mixing equipment
- Food processing machinery, bottling lines

**Material Handling & Automation:**

- Conveyor systems, robotic arms, pick-and-place units
- Automated guided vehicles (AGVs), shuttle systems
- Assembly line automation, testing equipment
- Warehouse automation (AS/RS, sorters)
- Sectional doors, overhead doors, door drives

**Support Infrastructure:**

- Industrial pumps, compressors, blowers
- Motors, gearboxes, belt drives, couplings
- HVAC systems, cooling towers, chillers
- Hydraulic and pneumatic systems

**Specialized Equipment:**

- Textile machinery (looms, knitting machines)
- Paper processing equipment (pulping, coating)
- Mining equipment (crushers, mills, separators)
- Energy equipment (turbines, generators)

For these demanding applications, EsoCore provides the complete operational visibility and zero-downtime reliability that traditional reactive
maintenance cannot deliver.

---

## Project goals

- **Capture**:
  - Comprehensive air quality monitoring (temperature, humidity, CO₂, VOCs, particulate matter)
  - Equipment environmental monitoring for thermal management and air quality
  - Light intensity monitoring (visible, UV, IR, color, spectral) for workplace safety and process control
  - Oil quality monitoring (viscosity, contamination, water content) for predictive lubrication maintenance
  - Pressure monitoring for hydraulic/pneumatic systems (leak detection, performance validation)
  - Targeted temperature monitoring (bearings, gearboxes, extreme environments)
  - Position sensing and cycle counting (automation, robotics, machine validation)
  - Operational cycles (machine starts/stops, production counts)
  - Multi-axis vibration (RMS and diagnostic windows) for bearing/mechanical health
  - Motor current and power consumption for load monitoring
  - Acoustics (audible + ultrasound monitoring) for tool wear, quality, and fault detection
  - Process parameters (pressure, flow, speed) via analog/digital interfaces
- **Local durability**: store on SD in a standardized, append-only, audit-friendly format with integrity checks.
- **Reliable sync**: upload to a cloud IoT platform via REST with strong auth, resumable uploads, and conflict-free idempotency.
- **Mixed connectivity**: Wi-Fi or dual Ethernet, with automatic backfill after outages.
- **Fleet-grade ops**: secure provisioning, OTA firmware, device health, alerts, dashboards, and role-based access.

---

## System Architecture

### High-Level Components

| Component          | Technology                                    | Purpose                                               |
| ------------------ | --------------------------------------------- | ----------------------------------------------------- |
| **Edge**           | STM32H747 (dual-core M7/M4)                   | Real-time sensor data collection and local processing |
| **Cloud Platform** | Django REST Framework Server + Nuxt.js Portal | Data management, analytics, and user interfaces       |
| **Connectivity**   | Dual Ethernet or Wi-Fi                        | Reliable data transmission with offline resilience    |
| **Storage**        | Industrial microSD + Cloud DB                 | Local buffering and long-term data persistence        |

### Technology Choices Rationale

- **STM32 MCU**: Proven ecosystem with robust peripherals, extensive RTOS support, and hardware crypto acceleration
- **Django REST Framework Server + Nuxt.js Portal**: Scalable backend with REST APIs, role-based access control, and modern responsive frontend with
  real-time dashboards
- **Industrial microSD**: Provides local resilience for 30+ days of offline operation
- **TLS over REST**: Simple, firewall-friendly protocol with strong security

### Data Acquisition Architecture

EsoCore supports three complementary data acquisition methods:

- **RS-485 Sensor Bus**: Distributed sensor modules with local MCUs connect over an industrial RS-485 multi-drop bus, enabling plug-and-play
  expansion. New sensor types can be added without changing the Edge or platform architecture. Digital, noise-immune wiring up to 100 m with
  auto-discovery and hot-plug support.
- **On-Board IEPE Analog Inputs (4 channels)**: Dedicated signal chain for IEPE/ICP vibration accelerometers wired directly to the Edge device.
  24-bit simultaneous ADC at 50 kSPS per channel with programmable gain and anti-aliasing filter.
- **On-Board Safety Digital Inputs (6 channels)**: Dual-channel optocoupled 24 V inputs (EN ISO 13849 Cat. 3) with cross-monitoring for safety
  devices (e-stops, light curtains, safety edges) and general-purpose signals (cycle counts, machine states).

See detailed specs in [Hardware: Edge](/docs/esocore-edge).

---

## Hardware Overview

### Edge Platform

- **Microcontroller**: STM32H747 dual core (Cortex-M7/M4) with hardware crypto, sufficient for TLS and edge AI processing.
- **Connectivity**: Dual industrial Ethernet with physical network separation (preferred) or robust Wi-Fi module for reliable data transmission.
- **Storage**: Industrial-grade microSD with wear leveling for 30+ days of local data buffering.
- **Primary Power**: Dedicated 24 V DC with aggressive surge/ESD protection, input filtering, and brownout-safe behavior.
- **Backup System**: Supercapacitor bank (20 F, with optional second unit for ≥30 s runtime) providing ≥30 seconds for
  safe shutdown during power outages.
- **Environmental**: IP54-IP65 enclosure, -20°C to +60°C operation, industrial EMC compliance.

Detailed Specifications: See [Hardware: Edge](/docs/esocore-edge) for complete component specifications, compliance standards, and reference designs.

### Sensor Suite

- **Environmental**: Comprehensive air quality monitoring (temperature, humidity, CO₂, VOCs, particulate matter) + light intensity monitoring
  (visible, UV, IR, color, spectral) + oil quality monitoring (viscosity, contamination, water content) for thermal management and environmental
  compliance.
- **Mechanical**: 3-axis vibration (1-3kHz), acoustic monitoring (audible + ultrasonic), multi-range pressure sensors (hydraulic/pneumatic),
  proximity/position sensors (automation, cycle counting), motor current sensing for torque/load monitoring.
- **Thermal**: Targeted temperature monitoring with NTC thermistors, RTD sensors, thermocouples, and IR sensors for bearings, gearboxes, and extreme
  environments.
Detailed Specifications: See [Hardware: Sensors](/docs/esocore-sensors) for complete component specifications, compliance standards, and reference
designs.

---

## Firmware

The firmware is written in C (C11) and targets two STM32 families: STM32H7 for the Edge device and STM32G0 for sensor
modules. It is built with ARM GCC and GNU Make.

Key firmware responsibilities include sensor sampling, safety I/O monitoring, local data storage (JSON with zstd
compression on microSD), on-device TinyML inference for anomaly detection, cloud synchronization over REST, and OTA
firmware updates.

See [Firmware](/docs/firmware-overview) for the complete architecture, code structure, build system, and toolchain setup
instructions.

---

## Cloud Platform

The cloud platform consists of two applications:

- **[Backend Server](/docs/backend-server)**: A Django REST Framework application (Python) that handles device
  registration, telemetry ingestion, event processing, alerting, and exposes REST APIs. Supports PostgreSQL in
  production with auto-generated OpenAPI documentation.
- **[Portal](/docs/portal)**: A Nuxt 4 web application (Vue.js / TypeScript) providing real-time dashboards, device
  management, fleet monitoring, and alerting interfaces. Built with Nuxt UI and Tailwind CSS.

The server authenticates devices via API keys and users via session-based auth. The portal communicates with the server
over REST APIs.

See also: [API Specification](/docs/api-specification) and
[Cloud Infrastructure](/docs/cloud-infrastructure) for deployment details.

---

## Security & Device Management

- **Secure Provisioning**: Manufacturing-injected device credentials with QR-code claiming process for field deployment.
- **Enterprise Security**: TLS encryption, signed OTA updates, device authentication, and automatic rollback protection.
- **Fleet Management**: Centralized device configuration, firmware updates, and security key rotation.

---

## Testing & Validation

### Quality Assurance Framework

- **Comprehensive Testing**: Multi-layered validation including unit tests, hardware-in-loop testing, safety compliance verification, and end-to-end
  system validation.

### Performance Benchmarks

- 72-hour offline tolerance with full data recovery
- 99.5%+ OTA success rate
- <0.01% data loss rate over 30-day soak tests

### Compliance

- EMC testing (EN 61000), functional safety (EN ISO 13849), electrical safety (EN 60204-1), and environmental qualification testing.

Detailed Procedures: See [Testing Procedures](/docs/testing-procedures) for complete test protocols, acceptance criteria, and validation procedures.

---

## Deployment & Operations

- **Environment Strategy**: Progressive deployment through dev → staging → production with separate databases and security keys.
- **Observability**: Comprehensive monitoring with Sentry error tracking, Prometheus metrics, and automated uptime checks.
- **Fleet Operations**: Staged OTA rollouts (5%/25%/100%) with canary devices, automated backups, and compliance-ready data retention policies.

---

## Technology Decisions

- **Data Format**: JSON with zstd compression for maintainability and debugging ease.
- **Connectivity**: Dual Ethernet preferred for industrial environments; Wi-Fi for retrofits.
- **Protocols**: REST for all device communications; MQTT optional for real-time commands.
- **Database**: Django with PostgreSQL and optional TimescaleDB for heavy analytics workloads.

---

## Technical Documentation

This page provides the business case and system overview. The documentation is organized into the following sections,
accessible from the sidebar:

### Hardware

- **[Edge Hardware](/docs/esocore-edge)** -- Edge device specifications, dual Ethernet, analog inputs, fieldbus
  protocols
- **[Sensors](/docs/esocore-sensors)** -- Complete sensor module catalog with individual datasheets

### Software

- **[Firmware](/docs/firmware-overview)** -- C11 firmware architecture, toolchain, build system, and setup guide
- **[Edge Intelligence](/docs/edge-intelligence)** -- TinyML models, anomaly detection, and training pipelines
- **[Data Format Specification](/docs/data-format-specification)** -- JSON schemas, event types, and storage format

### Cloud Platform

- **[Backend Server](/docs/backend-server)** -- Django REST API: architecture, endpoints, and local setup
- **[Portal](/docs/portal)** -- Nuxt 4 frontend: dashboards, device management, and local setup
- **[API Specification](/docs/api-specification)** -- Detailed REST API documentation and data models
- **[Cloud Infrastructure](/docs/cloud-infrastructure)** -- Deployment architecture and hosting options

### Development and Operations

- **[Development Environment](/docs/development-environment)** -- Toolchain setup across all disciplines
- **[Testing Procedures](/docs/testing-procedures)** -- Test protocols, validation procedures, and acceptance criteria

### Business

- **[Business Model & Partnerships](/docs/business-model-and-partnerships)** -- Partnership framework and open source
  philosophy
- **[Competitor Analysis](/docs/competitor-analysis)** -- Market landscape and competitive positioning

---

## License

EsoCore is licensed under the **Apache License 2.0**. This means you can:

- Use commercially and modify the code
- Distribute and sell products based on EsoCore
- Fork the project and continue independently
- Integrate into proprietary software/hardware

See the [License](/docs/license) page for complete terms and a detailed summary of what you can and cannot do.
