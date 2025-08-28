# EdgeSentinel: Universal Industrial Machine Monitoring System

EdgeSentinel is an advanced industrial IoT platform for comprehensive monitoring of factory machinery and equipment. By capturing critical operational data—from thermal conditions to mechanical health indicators—this system enables predictive maintenance, reduces downtime, and optimizes equipment performance across manufacturing facilities.

---

## Introduction

### The Problem

Unplanned equipment failures cost manufacturers millions in lost production, emergency repairs, and quality issues. Traditional reactive maintenance leads to premature replacements, unexpected downtime, and safety hazards. This is especially critical for high-volume production, precision manufacturing, and safety-critical operations where equipment reliability is paramount.

### The Solution

A robust edge-to-cloud telemetry platform that continuously monitors machine health through multiple sensor streams, providing early warning of issues and actionable insights for maintenance teams. The system is designed to work with virtually all industrial machinery types and integrates with existing factory systems, making it universally deployable across diverse manufacturing environments.

### Development Model

EdgeSentinel is developed and maintained by Newmatik through a collaborative partnership model with strategic industrial partners. The entire platform—including hardware designs, firmware, and cloud components—is fully open source under the Apache 2.0 License, ensuring maximum flexibility and preventing vendor lock-in. Partners can develop proprietary market-specific customizations while benefiting from shared platform development costs and continuous innovation. This unique approach combines open-source principles with commercial viability, creating a sustainable ecosystem where partners retain full independence and can fork the project if needed.

### Competitive Position

EdgeSentinel differentiates itself in the industrial IoT market through cross-domain capability (supporting both industrial doors and CNC machines in one platform), edge-first TinyML architecture for ultra-low latency anomaly detection, and comprehensive safety integration. While competitors like MachineMetrics, ASSA ABLOY Insight, and Fanuc FIELD focus on single verticals with proprietary platforms, EdgeSentinel's open-source foundation and unified approach provides unique value for facilities managing diverse industrial assets.

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

For these demanding applications, EdgeSentinel provides the complete operational visibility and zero-downtime reliability that traditional reactive maintenance cannot deliver.

---

## Project goals

- **Capture**:
  - Ambient temperature (°C) and relative humidity (%)
  - Equipment/motor temperature monitoring for thermal management
  - Operational cycles (machine starts/stops, production counts)
  - Multi-axis vibration (RMS and diagnostic windows) for bearing/mechanical health
  - Motor current and power consumption for load monitoring
  - Acoustics (audible + ultrasound monitoring) for tool wear, quality, and fault detection
  - Process parameters (pressure, flow, speed) via analog/digital interfaces
- **Local durability**: store on SD in a standardized, append-only, audit-friendly format with integrity checks.
- **Reliable sync**: upload to a cloud IoT platform via REST with strong auth, resumable uploads, and conflict-free idempotency.
- **Mixed connectivity**: Wi‑Fi or Ethernet (optional PoE), with automatic backfill after outages.
- **Fleet-grade ops**: secure provisioning, OTA firmware, device health, alerts, dashboards, and role-based access.

---

## System Architecture

### High-Level Components

| Component | Technology | Purpose |
|-----------|------------|---------|
| **Edge Device** | STM32 F4/F7/H7 | Real-time sensor data collection and local processing |
| **Cloud Platform** | Frappe Framework | Data management, analytics, and user interfaces |
| **Connectivity** | Ethernet/PoE or Wi-Fi | Reliable data transmission with offline resilience |
| **Storage** | Industrial microSD + Cloud DB | Local buffering and long-term data persistence |

### Technology Choices Rationale

- **STM32 MCU**: Proven ecosystem with robust peripherals, extensive RTOS support, and hardware crypto acceleration
- **Frappe Framework**: Rapid development platform with built-in RBAC (role-based access control), REST APIs, and dashboard capabilities
- **Industrial microSD**: Provides local resilience for 30+ days of offline operation
- **TLS over REST**: Simple, firewall-friendly protocol with strong security

### Modular RS‑485 Sensor Architecture

- **Distributed sensor bus (RS‑485)**: Sensor modules with local MCUs connect over an industrial RS‑485 multi‑drop bus, enabling plug‑and‑play expansion.
- **No edge redesign**: New sensor types can be added—customer‑specific, proprietary, or exclusive—without changing the edge device or platform architecture.
- **Scalable and robust**: Digital, noise‑immune wiring up to 100 m with auto‑discovery and hot‑plug support. See detailed specs in [Hardware Requirements](hardware-requirements.md).

---

## Hardware Overview

### Core Platform

**Microcontroller**: ARM Cortex-M4/M7 (STM32 recommended) with hardware crypto, sufficient for TLS and edge AI processing.

**Connectivity**: Industrial Ethernet with PoE option (preferred) or robust Wi-Fi module for reliable data transmission.

**Storage**: Industrial-grade microSD with wear leveling for 30+ days of local data buffering.

### Sensor Suite

**Environmental**: Ambient temperature/humidity, drive stator temperature monitoring for thermal management.

**Mechanical**: 3-axis vibration (1-3kHz), acoustic monitoring (audible + ultrasonic), machine cycle counting, motor current sensing for torque/load monitoring.

**Safety I/O**: EN ISO 13849 compliant inputs for safety edges, emergency stops, light curtains with <10ms response time.

### Power & Reliability

**Primary Power**: 12-24V DC with surge protection, optional PoE compatibility.

**Backup System**: Supercapacitor bank (10-30F) providing ≥30 seconds for safe shutdown during power outages.

**Environmental**: IP54-IP65 enclosure, -20°C to +60°C operation, industrial EMC compliance.

Detailed Specifications: See [Hardware Requirements](hardware-requirements.md) for complete component specifications, compliance standards, and reference designs.

---

## Firmware architecture

We standardize on Zephyr RTOS for the edge device firmware. The following task model maps well to STM32 and remains portable across supported Zephyr boards.

### 4.1 RTOS & services

- **Sensor Task**: sampling scheduler (configurable rates; e.g., temp/RH 1 Hz, stator temp 1 Hz, vibration bursts at 1–3 kHz for 1–5 s windows, acoustics windows in audible/ultrasound bands, current 100–500 Hz).
- **Safety I/O Task**: monitor safety edges, emergency stops, light curtains; handle safety interlocks with <10ms response time.
- **Feature Task**: on‑device statistics (RMS vibration, kurtosis, crest factor, bandpower; avg/max torque/current; cycle detection).
- **Edge Intelligence Task**: TinyML inference for vibration/acoustic anomaly detection, pattern recognition, predictive algorithms.
- **Power Monitor Task**: continuous voltage/current monitoring, brownout detection, supercap charge management.
- **Logwriter Task**: append to SD (atomic, CRC), rotate files by size/time.
- **Event Logger Task**: capture system events (connectivity, power, errors, safety) with timestamps and context.
- **Safe Shutdown Task**: triggered by power loss; emergency data flush, graceful peripheral shutdown, final event log.
- **Sync Task**: queue → batch → compress → REST upload with backoff.
- **Comms Task**: TLS sockets (mbedTLS/wolfSSL), NTP, DHCP, captive portal for provisioning.
- **OTA Task**: download, verify (Ed25519 or ECDSA), A/B swap, rollback.
- **Watchdog & Health**: feed WDT, record resets, SD health, sensor self‑tests, supercap health monitoring.

### 4.2 Data Storage & Event Logging

- **Local Storage**: JSON format with zstandard compression for human-readable, audit-friendly data storage on industrial SD cards. Power-safe writes ensure zero data loss during outages.
- **Event Logging**: Comprehensive system event capture including connectivity, power, safety, and security events with automated workflow triggers.
- **Data Integrity**: CRC32 checksums per record, SHA-256 per file, and atomic operations ensure 100% data reliability during power loss events.

Detailed Specifications: See [Data Format Specification](data-format-specification.md) for complete JSON schemas, event types, and file structure details.

### 4.3 Edge Intelligence & AI

- **AI-Powered Analytics**: On-device TinyML models for vibration and acoustic anomaly detection using lightweight neural networks (<16KB) optimized for STM32 processors.
- **Real-Time Processing**: <1 second anomaly detection with 90% reduction in data transmission through intelligent filtering and priority queuing.
- **Privacy & Reliability**: Sensitive data never leaves the device, ensuring GDPR/HIPAA compliance and continued operation during connectivity outages.
- **Continuous Learning**: OTA model updates with federated learning for fleet-wide intelligence improvement without compromising data privacy.

Detailed Specifications: See [Edge Intelligence](edge-intelligence.md) for complete TinyML model specifications, training pipelines, and performance benchmarks.

### 4.4 Cloud Synchronization

- **Reliable Upload**: Compressed JSON batches with idempotency keys, exponential backoff, and priority queuing ensure zero data loss during network outages.
- **Security**: TLS encryption with device-specific API keys, HMAC authentication, and OTA key rotation for enterprise-grade security.
- **Offline Resilience**: 30+ day local buffering with automatic resume from last sync point when connectivity returns.

Detailed Specifications: See [API Specification](api-specification.md) for complete REST API documentation, authentication methods, and sync protocols.

---

## Cloud Platform

### Technology Stack

**Frappe Framework**: Rapid development platform with built-in RBAC, REST APIs, workflow automation, and dashboard capabilities. Alternative platforms can be substituted based on customer requirements.

### Core Features

- **Device Management**: Centralized device registry with provisioning, configuration, and OTA firmware management across the entire fleet.
- **Data Analytics**: Time-series data storage with real-time dashboards showing device health, usage patterns, and predictive maintenance insights.
- **Event Automation**: Intelligent workflow engine that automatically creates maintenance tickets, sends alerts, and escalates issues based on configurable rules.
- **Access Control**: Role-based access ensuring customers see only their devices while providing administrative oversight capabilities.

Detailed Specifications: See [API Specification](api-specification.md) for complete platform architecture, database design, and integration details.

---

## Security & Device Management

- **Secure Provisioning**: Manufacturing-injected device credentials with QR-code claiming process for field deployment.
- **Enterprise Security**: TLS encryption, signed OTA updates, device authentication, and automatic rollback protection.
- **Fleet Management**: Centralized device configuration, firmware updates, and security key rotation.

---

## Testing & Validation

### Quality Assurance Framework

- **Comprehensive Testing**: Multi-layered validation including unit tests, hardware-in-loop testing, safety compliance verification, and end-to-end system validation.

### Performance Benchmarks

- 72-hour offline tolerance with full data recovery
- <10ms safety I/O response times
- 99.5%+ OTA success rate
- <0.01% data loss rate over 30-day soak tests

### Safety & Compliance

- EN ISO 13849 functional safety validation, EMC testing, and environmental qualification testing.

Detailed Procedures: See [Testing Procedures](testing-procedures.md) for complete test protocols, acceptance criteria, and validation procedures.

---

## Deployment & Operations

- **Environment Strategy**: Progressive deployment through dev → staging → production with separate databases and security keys.
- **Observability**: Comprehensive monitoring with Sentry error tracking, Prometheus metrics, and automated uptime checks.
- **Fleet Operations**: Staged OTA rollouts (5%/25%/100%) with canary devices, automated backups, and compliance-ready data retention policies.

---

## Technology Decisions

- **Data Format**: JSON with zstd compression for maintainability and debugging ease.
- **Connectivity**: Ethernet+PoE preferred for industrial environments; Wi-Fi for retrofits.
- **Protocols**: REST for all device communications; MQTT optional for real-time commands.
- **Database**: Frappe Framework with optional TimescaleDB for heavy analytics workloads.

---

## Technical Documentation

This page provides the business case and system overview. Detailed technical specifications are organized in separate documents:

### Core Specifications

- **[Hardware Requirements](hardware-requirements.md)** - Component specifications, safety standards, and compliance requirements
- **[Data Format Specification](data-format-specification.md)** - JSON schemas, event types, file structures, and data integrity protocols
- **[API Specification](api-specification.md)** - REST API documentation, authentication, cloud platform architecture
- **[Edge Intelligence](edge-intelligence.md)** - TinyML models, AI algorithms, training pipelines, and performance benchmarks

### Development & Operations

- **[Business Model & Partnerships](business-model-and-partnerships.md)** - Development model, funding structure, open source philosophy, and partnership framework
- **[Competitor Analysis](competitor-analysis.md)** - Market landscape, competitive positioning, and strategic differentiation in industrial doors and CNC machine monitoring
- **[Development Timeline & Budget](development-timeline-budget.md)** - Project phases, effort breakdown, resource requirements, and cost analysis
- **[Testing Procedures](testing-procedures.md)** - Comprehensive testing protocols, validation procedures, and acceptance criteria

### Quick Reference

- **Business Case**: See sections 1-2 above
- **Hardware Overview**: Section 3 (details in hardware docs)
- **Software Architecture**: Section 4 (details in API and edge intelligence docs)  
- **Security & Management**: Section 6 (details in API docs)
- **Quality Assurance**: Section 7 (details in testing docs)

---

## License

EdgeSentinel is licensed under the **Apache License 2.0**. This means you can:

- Use commercially and modify the code
- Distribute and sell products based on EdgeSentinel  
- Fork the project and continue independently
- Integrate into proprietary software/hardware

See the [License](license.md) page for complete terms and a detailed summary of what you can and cannot do.

---

*For questions about this specification or implementation details, please refer to the appropriate technical documentation linked above.*
