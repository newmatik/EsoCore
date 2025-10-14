<!-- markdownlint-configure-file { "MD024": { "siblings_only": true } } -->

# Industrial Fieldbus Protocols

This page outlines the practical aspects of supporting Modbus, PROFIBUS and PROFINET on EsoCore hardware and firmware.
It focuses on physical interfaces, topology, addressing, timing, EMC, and certification considerations so designs are robust in industrial environments.

---

## Single Ethernet Port Architecture

**EsoCore v1 uses ONE Ethernet port and ONE IP address for all IP-based services.**
This unified architecture is the industry-standard approach for field devices and fully supported by specifications:

- **HTTPS Web UI** (port 443): On-device configuration and diagnostics
- **Modbus TCP** (port 502): PLC/SCADA integration
- **PROFINET Device** (RT): Real-time cyclic I/O via Layer-2 frames (EtherType 0x8892)

### How It Works

**Quality of Service (QoS)**: PROFINET RT frames use 802.1Q priority tagging (PCP=6) to ensure deterministic real-time performance.
Industrial switches prioritize RT traffic above best-effort HTTP or Modbus TCP, preventing conflicts.

**Industry Practice**: PROFINET Design Guidelines explicitly allow unrestricted TCP/IP traffic alongside PROFINET RT on the same interface.
This is how field devices are designed—not a limitation but an intentional design validated by PI (PROFIBUS & PROFINET International).

**Network Load**: Follow PI recommendations to keep total link utilization below 50%.
EsoCore's web UI and diagnostics are designed with rate limiting to respect real-time cycle requirements.

### Optional 2-Port Configuration (Future)

Some PROFINET devices expose two RJ45 jacks for **line topology** (daisy-chain wiring).
This uses an integrated Ethernet switch; both ports remain **one logical interface with one MAC/IP address**.
The 2-port option is for physical topology convenience, not protocol separation.

EsoCore v1 ships with one RJ45 port.
A 2-port variant with integrated switch can be provisioned via expansion header in future hardware revisions if line topology is required.

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

- Standard 10/100 Ethernet (shared with web UI and PROFINET)
- Uses the same RJ45 port, PHY, and IP address as other Ethernet services

### Topology & Addressing

- IP addressing via DHCP or static (same IP as web UI and PROFINET)
- TCP port 502

### Firmware Notes

- Lightweight TCP stack with Modbus application server
- Same function codes as RTU with PDU carried over TCP
- Coexists with PROFINET RT via QoS/priority handling
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

- Standard Ethernet (RJ45 with magnetics)
- **Single-port design**: EsoCore v1 uses one Ethernet port shared with web UI and Modbus TCP
- Star or line topology via switches; 2-port devices with integrated switch are available in future revisions for daisy-chain wiring

### Real‑Time Classes

- RT (Real Time): cyclic I/O over VLAN‑tagged frames (EtherType 0x8892, PCP=6)
- IRT (Isochronous Real Time): tighter jitter with special switch silicon (future option)

### Addressing & Discovery

- IP via DCP set name/IP, discovery with LLDP
- GSDML file describes device to engineering tools
- **One IP address**: All services (web UI, Modbus TCP, PROFINET) accessible via the same IP

### Firmware Notes

- Prioritized traffic, bounded cycle times (2–4 ms typical for RT)
- Alarms, diagnostics, records; SNMP/LLDP for topology
- QoS handling ensures RT frames take priority over best-effort IP traffic
- Rate-limited web UI and diagnostics to maintain <50% link utilization

### BOM & Connectors

- Existing Ethernet PHY and MagJack (shared with all Ethernet protocols)
- Optional 2‑port switch (future revision) for line topology; still one logical interface/IP

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
