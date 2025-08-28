# Development Timeline & Budget

- **Total Duration**: 32-38 weeks (8-9 months)  
- **Total Effort**: 5,620 hours  
- **Total Budget**: €505,800 (base) + €75,900-€101,200 (contingency) = €581,700-€607,000

---

## Project Phases & Timeline

| Phase | Duration | Parallel Work | Key Deliverables |
|-------|----------|---------------|------------------|
| **Phase 0: Project Setup** | 2 weeks | Documentation, Planning | Requirements finalization, team setup |
| **Phase 1: Hardware Development** | 12 weeks | Hardware design & prototyping | EVT hardware, initial testing |
| **Phase 2: Firmware Foundation** | 8 weeks | Basic firmware & drivers | Core functionality, sensor integration |
| **Phase 3: Cloud Platform MVP** | 10 weeks | Frappe development | Basic cloud platform, APIs |
| **Phase 4: Edge Intelligence** | 6 weeks | TinyML integration | AI models, anomaly detection |
| **Phase 5: System Integration** | 8 weeks | End-to-end testing | Complete system validation |
| **Phase 6: Compliance & Hardening** | 6 weeks | Certification, security | Production-ready system |

---

## Detailed Effort Breakdown

### Hardware Development (12 weeks)

| Task | Hours | Cost (€90/h) | Owner |
|------|-------|-------------|--------|
| **Component Research & Selection** | 120 | €10,800 | Hardware Engineer |
| **Schematic Design** | 200 | €18,000 | Hardware Engineer |
| - MCU, connectivity, power management | 80 | €7,200 | |
| - Safety I/O circuits (EN ISO 13849) | 60 | €5,400 | |
| - Sensor interfaces & signal conditioning | 60 | €5,400 | |
| **PCB Layout & Design** | 180 | €16,200 | Hardware Engineer |
| - 6-layer industrial PCB design | 120 | €10,800 | |
| - EMC optimization & routing | 60 | €5,400 | |
| **Supercapacitor Power System** | 100 | €9,000 | Hardware Engineer |
| **Enclosure & Mechanical Design** | 80 | €7,200 | Mechanical Engineer |
| **Prototype Assembly & Bring-up** | 120 | €10,800 | Hardware Engineer |
| **Hardware Testing & Validation** | 150 | €13,500 | Hardware Engineer |
| **EMC Pre-compliance Testing** | 60 | €5,400 | Hardware Engineer |
| **Production Documentation** | 40 | €3,600 | Hardware Engineer |
| **Subtotal Hardware** | **1,050** | **€94,500** | |

### Firmware Development (14 weeks total, overlapping phases)

| Task | Hours | Cost (€90/h) | Owner |
|------|-------|-------------|--------|
| **RTOS Setup & Core Framework** | 120 | €10,800 | Firmware Engineer |
| **Hardware Drivers** | 300 | €27,000 | Firmware Engineer |
| - Sensor drivers (temp, vibration, acoustic) | 120 | €10,800 | |
| - Safety I/O with <10ms response | 80 | €7,200 | |
| - Network stack (Ethernet/WiFi, TLS) | 100 | €9,000 | |
| **Power Management & Safe Shutdown** | 100 | €9,000 | Firmware Engineer |
| **Data Storage & File System** | 120 | €10,800 | Firmware Engineer |
| - JSON format implementation | 40 | €3,600 | |
| - Power-safe SD card operations | 80 | €7,200 | |
| **Event Logging System** | 80 | €7,200 | Firmware Engineer |
| **Edge Intelligence Integration** | 200 | €18,000 | AI/Firmware Engineer |
| - TinyML model deployment | 120 | €10,800 | |
| - Real-time inference optimization | 80 | €7,200 | |
| **Cloud Synchronization** | 120 | €10,800 | Firmware Engineer |
| **OTA Update System** | 100 | €9,000 | Firmware Engineer |
| **Comprehensive Testing** | 200 | €18,000 | Firmware Engineer |
| **Safety System Validation** | 80 | €7,200 | Firmware Engineer |
| **Performance Optimization** | 60 | €5,400 | Firmware Engineer |
| **Subtotal Firmware** | **1,480** | **€133,200** | |

### Cloud Platform Development (10 weeks)

| Task | Hours | Cost (€90/h) | Owner |
|------|-------|-------------|--------|
| **Frappe Framework Setup** | 40 | €3,600 | Backend Engineer |
| **Database Design & Implementation** | 120 | €10,800 | Backend Engineer |
| - Device management schema | 40 | €3,600 | |
| - Time-series data optimization | 80 | €7,200 | |
| **REST API Development** | 180 | €16,200 | Backend Engineer |
| - Device authentication & management | 60 | €5,400 | |
| - Telemetry ingestion with idempotency | 80 | €7,200 | |
| - Configuration & OTA endpoints | 40 | €3,600 | |
| **Event Workflow Automation** | 150 | €13,500 | Backend Engineer |
| - Event processing pipeline | 80 | €7,200 | |
| - Notification & escalation rules | 70 | €6,300 | |
| **Dashboard Development** | 200 | €18,000 | Frontend Engineer |
| - Real-time device status | 80 | €7,200 | |
| - Data visualization & trends | 120 | €10,800 | |
| **User Management & RBAC** | 100 | €9,000 | Backend Engineer |
| **API Documentation** | 60 | €5,400 | Backend Engineer |
| **Cloud Testing & Deployment** | 120 | €10,800 | DevOps Engineer |
| **Subtotal Cloud Platform** | **970** | **€87,300** | |

### Testing & Quality Assurance (8 weeks)

| Task | Hours | Cost (€90/h) | Owner |
|------|-------|-------------|--------|
| **Test Framework Development** | 80 | €7,200 | QA Engineer |
| **Unit Testing (Firmware)** | 120 | €10,800 | Firmware Engineer |
| **Hardware-in-Loop Testing** | 150 | €13,500 | Test Engineer |
| **Safety I/O Validation** | 100 | €9,000 | Safety Engineer |
| **Power Management Testing** | 80 | €7,200 | Test Engineer |
| **End-to-End System Testing** | 120 | €10,800 | QA Engineer |
| **Performance & Stress Testing** | 100 | €9,000 | Test Engineer |
| **Security & Penetration Testing** | 80 | €7,200 | Security Engineer |
| **Field Testing (5-10 pilot machines)** | 120 | €10,800 | Test Engineer |
| **Test Documentation** | 60 | €5,400 | QA Engineer |
| **Subtotal Testing** | **1,010** | **€90,900** | |

### Compliance & Certification (6 weeks)

| Task | Hours | Cost (€90/h) | Owner |
|------|-------|-------------|--------|
| **Safety Standards Documentation** | 80 | €7,200 | Safety Engineer |
| **EMC Testing & Certification** | 100 | €9,000 | Compliance Engineer |
| **Functional Safety Validation** | 120 | €10,800 | Safety Engineer |
| **Regulatory Documentation** | 60 | €5,400 | Compliance Engineer |
| **Certification Process Management** | 40 | €3,600 | Project Manager |
| **Subtotal Compliance** | **400** | **€36,000** | |

### Project Management & Documentation (Throughout project)

| Task | Hours | Cost (€90/h) | Owner |
|------|-------|-------------|--------|
| **Project Management** | 200 | €18,000 | Project Manager |
| **Technical Documentation** | 150 | €13,500 | Technical Writer |
| **Architecture & Design Reviews** | 100 | €9,000 | Lead Engineer |
| **Risk Management** | 80 | €7,200 | Project Manager |
| **Stakeholder Communication** | 100 | €9,000 | Project Manager |
| **Process & Quality Management** | 80 | €7,200 | Project Manager |
| **Subtotal Management** | **710** | **€63,900** | |

---

## Resource Requirements

### Team Composition

- **1x Project Manager** (throughout)
- **1x Hardware Engineer** (phases 0-2, 5-6)
- **1x Firmware Engineer** (phases 1-6)
- **1x Backend Engineer** (phases 3-6)
- **1x Frontend Engineer** (phase 3-4)
- **1x AI/ML Engineer** (phase 4)
- **1x QA Engineer** (phases 5-6)
- **1x Safety/Compliance Engineer** (phases 5-6)

### Critical Path Dependencies

1. **Hardware prototypes** must be ready before firmware integration testing
2. **Basic firmware** needed before cloud platform integration
3. **Safety certification** requires completed hardware and firmware
4. **Field testing** depends on end-to-end system completion

---

## Budget Summary

| Category | Hours | Cost (€90/h) | % of Total |
|----------|-------|-------------|------------|
| **Hardware Development** | 1,050 | €94,500 | 22% |
| **Firmware Development** | 1,480 | €133,200 | 31% |
| **Cloud Platform** | 970 | €87,300 | 20% |
| **Testing & QA** | 1,010 | €90,900 | 21% |
| **Compliance** | 400 | €36,000 | 8% |
| **Project Management** | 710 | €63,900 | 15% |
| **TOTAL** | **5,620** | **€505,800** | **100%** |

---

## Risk Contingency

- **Recommended contingency**: 15-20% for technical risks
- **Contingency budget**: €75,900 - €101,200
- **Total project budget**: €581,700 - €607,000
