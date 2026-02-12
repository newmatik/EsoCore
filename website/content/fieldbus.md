<!-- markdownlint-configure-file { "MD024": { "siblings_only": true } } -->

# Industrial Fieldbus Protocols

This page outlines the practical aspects of supporting Modbus, PROFIBUS, PROFINET, and Interbus on EsoCore hardware and firmware. It focuses on
physical interfaces, topology, addressing, timing, EMC, and certification considerations so designs are robust in industrial environments.

---

## Dual Ethernet Port Architecture

**EsoCore uses two physically separated Ethernet ports** for true network isolation between operational technology (OT) and IT infrastructure:

- **Port A (Machine Network)**: PROFINET RT, Modbus TCP, on-device web UI (HTTPS), PLC communications
  - Static IP (e.g., 192.168.10.x), connected to machine VLAN switch
  - No internet access; isolated from corporate network
- **Port B (IT Network)**: HTTPS cloud sync, REST API, NTP time sync, OTA firmware updates
  - DHCP (e.g., 10.x.x.x), connected to corporate network with firewall to internet

Each port has an independent PHY (KSZ8081RNACA), independent magnetics (Wurth 7499111121), and a separate MAC address. This provides:

- **True isolation**: No VLAN hopping attacks; physical separation between OT and IT
- **Independent failure domains**: Cloud outage does not affect machine communication; machine network issues do not disrupt cloud sync
- **Clear security boundary**: Airgap between OT and IT; compliance with industries requiring physical network separation
- **Easier troubleshooting**: Separate LEDs per port; Wireshark on one port does not show other port's traffic

See [Dual Ethernet Ports](/docs/ethernet) for full architecture details, PHY selection rationale, and PCB layout guidelines.

### How It Works

**Port A carries all real-time and machine-facing traffic.** PROFINET RT frames use 802.1Q priority tagging (PCP=6) for deterministic performance.
Modbus TCP (port 502) and the on-device web UI (HTTPS, port 443) coexist on Port A with QoS ensuring RT priority.

**Port B handles all IT-facing traffic.** Cloud sync, OTA updates, NTP, and REST API access run on Port B, completely isolated from machine
communications. This means cloud-side issues (DNS failures, certificate problems, high latency) never impact real-time machine control.

**Network Load**: Follow PI recommendations to keep Port A link utilization below 50%. EsoCore's web UI and diagnostics on Port A are designed with
rate limiting to respect real-time cycle requirements.

---

## Modbus RTU (RS‑485)

### Physical Layer

- RS‑485 half‑duplex, differential pair A/B plus reference GND
- Isolated transceiver recommended for ESD/EMC and ground potential differences
- 3‑pin terminal block (A/B/G) with shield termination near chassis
- Switchable 120 Ω termination + optional biasing (pull‑up/pull‑down)

### Topology & Cabling

- Linear bus (daisy chain), avoid stubs
- Cable: twisted pair, 120 Ω characteristic impedance
- Max nodes: 32 per segment without repeaters (more with modern transceivers)

### Addressing & Framing

- Node addressing: 1–247 (DIP switch or software configurable)
- 8‑E‑1 or 8‑N‑2 common, CRC‑16 integrity

### Timing & Throughput

- Typical 9600–115200 bps (up to 1 Mbps in some systems)
- Silent interval between frames (3.5 character times) must be enforced

### Firmware Notes

- UART with DMA + precise inter‑frame timing
- CRC‑16 table or hardware accelerator

### BOM & Connectors

- Isolated RS‑485 transceiver
- 3‑pin 5.08 mm terminal block
- DIP switch for address and termination enable

---

## Modbus TCP (Ethernet)

### Physical Layer

- Standard 10/100 Ethernet on Port A (Machine Network)
- Shares Port A with web UI and PROFINET; isolated from Port B (IT Network)

### Topology & Addressing

- IP addressing via static IP on Port A (same IP as web UI and PROFINET on machine network)
- TCP port 502

### Firmware Notes

- Lightweight TCP stack with Modbus application server
- Same function codes as RTU with PDU carried over TCP
- Coexists with PROFINET RT on Port A via QoS/priority handling
- Multiple simultaneous client connections supported

---

## PROFIBUS DP

### Physical Layer

- RS‑485 with PROFIBUS electrical levels
- DB‑9 male with shield and integrated chassis connection
- Termination selectable via switch at both ends of the segment

### Topology & Performance

- Linear bus with repeaters for large networks
- 9.6 kbps to 12 Mbps, timing tighter than generic RS‑485

### Addressing & Configuration

- Node address 0–126, usually via rotary/DIP switch or software tool
- GSD file describes device to the PLC/engineering tool

### Firmware Notes

- Real‑time deterministic stack required for high baud rates
- State machine: parameterization → configuration → data exchange

### BOM & Connectors

- PROFIBUS‑compliant RS‑485 transceiver
- DB‑9 male with shield
- DIP switch for address/termination

---

## PROFINET (Device)

### Physical Layer & Topology

- Standard Ethernet (RJ45 with magnetics) on Port A (Machine Network)
- **Dual-port design**: Port A carries PROFINET RT alongside web UI and Modbus TCP; Port B is isolated for IT traffic
- Star or line topology via switches; dual-port design supports future MRP (Media Redundancy Protocol) capability

### Real‑Time Classes

- RT (Real Time): cyclic I/O over VLAN‑tagged frames (EtherType 0x8892, PCP=6)
- IRT (Isochronous Real Time): tighter jitter with special switch silicon (future option)

### Addressing & Discovery

- IP via DCP set name/IP, discovery with LLDP
- GSDML file describes device to engineering tools
- **Port A IP address**: Web UI, Modbus TCP, and PROFINET all accessible via Port A's IP on the machine network

### Firmware Notes

- Prioritized traffic, bounded cycle times (2–4 ms typical for RT)
- Alarms, diagnostics, records; SNMP/LLDP for topology
- QoS handling ensures RT frames take priority over best-effort IP traffic on Port A
- Rate-limited web UI and diagnostics to maintain <50% link utilization on Port A

### BOM & Connectors

- Port A Ethernet PHY (KSZ8081RNACA) and MagJack (Wurth 7499111121) for all machine-facing protocols
- Port B Ethernet PHY and MagJack are independent and dedicated to IT traffic
- Dual-port design supports future MRP (Media Redundancy Protocol) for line topology redundancy

---

## Interbus

### Background

Interbus is an obsolete fieldbus protocol still found on some older Bosch Rexroth controller machines. While Interbus modules are increasingly hard
to source, customer demand for legacy equipment support requires EsoCore to provide connectivity.

### Physical Layer

- RS-485 differential signaling at the physical layer
- Dedicated isolated RS-485 transceiver (ADM2582EBRWZ) separate from Modbus RTU and PROFIBUS interfaces
- 4-pin terminal block connector (A/B/GND/Shield) with switchable 120 ohm termination
- Cable: shielded twisted pair, 120 ohm characteristic impedance

### Protocol Characteristics

- **Master/slave architecture** with summation-frame approach
- **Deterministic cyclic behavior**: strict cycle timing with defined scan rates
- **Frame construction**: summation frame collects data from all slaves in a single bus cycle
- **Not a generic RS-485 protocol**: despite using RS-485 at the physical layer, Interbus has its own protocol timing, frame structure, and
  diagnostics that differ fundamentally from Modbus RTU

### Firmware Considerations

Adding native Interbus support is feasible in principle but non-trivial due to:

- **Strict cycle timing**: master scheduling must meet deterministic deadlines
- **Frame construction/parsing**: summation-frame protocol requires precise assembly and disassembly of multi-slave data
- **Diagnostics**: Interbus has its own diagnostic channel and fault reporting mechanisms
- **Scarcity of tooling**: modern, maintained embedded stacks and test equipment for Interbus are rare
- **Validation**: limited availability of Interbus slave devices for testing and certification

### BOM & Connectors

- Isolated RS-485 transceiver (ADM2582EBRWZ) dedicated to Interbus
- 4-pin pluggable terminal block (Degson 2EDGR-5.0-04P / 2EDGK-5.0-04P)
- 120 ohm termination resistor (switchable)

---

## EMC, Safety and Certification

- Cable shield bonding at the connector to chassis ground
- ESD arrays on data lines; common‑mode chokes for USB/Ethernet where needed
- Creepage/clearance and isolation for RS‑485 and power domains
- Follow conformance test plans: Modbus interoperability tests, PROFIBUS (PIT), PROFINET (PNO) certification

---

## References

- See the Hardware Requirements for mechanical and connector details
- See the BOM for specific transceivers, connectors, and switches
