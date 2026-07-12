# Dual Ethernet Ports (Physical Separation)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│ Dual Ethernet Ports (2× RJ-45)                              │
│ Physically isolated networks                                │
└─────────────────────────────────────────────────────────────┘
           │                             │
           ↓                             ↓
    Port A (Machine)                Port B (IT)
    MAC Address: ...A0               MAC Address: ...B0
           │                             │
    ┌──────┴─────────┐         ┌─────────┴─────────┐
    │ PROFINET RT    │         │ HTTPS (Cloud)     │
    │ Modbus TCP     │         │ REST API          │
    │ SINUMERIK HMI  │         │ NTP               │
    │ PLC Comms      │         │ OTA Updates       │
    │ 192.168.10.x   │         │ 10.x.x.x (DHCP)   │
    └────────────────┘         └───────────────────┘
           │                             │
           ↓                             ↓
    Machine VLAN Switch          Corporate Network
    (No internet access)         (Firewall to internet)
```

## Technical Implementation

```
STM32H747XI Microcontroller
│
├─── Ethernet MAC 0 (Machine Network)
│    │
│    ├─── RMII Interface (Reduced MII)
│    │    - 7 signals: TXD[1:0], RXD[1:0], TX_EN, RX_DV, REF_CLK
│    │    - 50 MHz clock from PHY (crystal oscillator)
│    │
│    ├─── PHY IC: KSZ8081RNACA (Microchip)
│    │    - Industrial temp: -40°C to +85°C
│    │    - Auto-MDIX (crossover cable not needed)
│    │    - Link/Activity LED drivers (2× outputs)
│    │    - ESD protection: ±8 kV contact, ±15 kV air
│    │    - Package: 24-QFN (5×5 mm)
│    │
│    ├─── Magnetics: Würth 7499111121 (RJ-45 integrated)
│    │    - 1:1 transformer (galvanic isolation)
│    │    - Common-mode choke (EMI suppression)
│    │    - Built-in LEDs (green=link, yellow=activity)
│    │    - PoE capable footprint (not populated for V1)
│    │
│    └─── Protection
│         - TVS diodes on each pair: PESD5V0L1BA (±30V clamp)
│         - Series termination: 33Ω resistors
│         - Chassis ground via 1 nF Y-capacitor
│
└─── Ethernet MAC 1 (IT Network)
     - Identical hardware to MAC 0
     - Separate MAC address (burned in OTP)
     - Independent PHY, magnetics, connector
```

## Why KSZ8081RNA?

- Industrial Temperature: -40°C to +85°C (customer plant environment)
- Low Power: 140 mW typical (battery backup compatible)
- Auto-MDIX: Works with straight or crossover cables
- LED Drivers: Built-in link/activity indication
- MII Management: MDIO/MDC for PHY configuration
- Proven: Used in millions of industrial devices
- Cost: ~$2 in volume (affordable)
- Availability: Multiple suppliers (Microchip, OnSemi alternatives)
- Compliance: FCC, CE, EN 55032 Class B EMC

## Advantages

- True Network Isolation
  - Physical separation = no VLAN hopping attacks
  - Machine network cannot reach internet (security)
  - IT network cannot disrupt real-time traffic (reliability)

- Independent Configuration
  - Machine network: Static IP, no DHCP dependency
  - IT network: DHCP, dynamic, can change without affecting machine
  - Failure isolation: Cloud down? Machine keeps running

- Clear Security Boundary
  - Airgap between OT (Operational Technology) and IT
  - Compliance: Some industries require physical separation
  - Defense in depth: Compromised cloud doesn't touch PLC

- Easier Troubleshooting
  - "Port A has link" vs "Port B has link"
  - Technician can see two separate LEDs
  - Wireshark on one port doesn't show other port's traffic

- Future-Proofing
  - Can add switch functionality (bridge/route between ports)
  - Can support redundant PROFINET (MRP - Media Redundancy Protocol)
  - Higher bandwidth ceiling (2× 100 Mbps vs 1× 100 Mbps)

- Customer-Specific Context
  - Large OEM plants often have physically separate networks
  - Machine network might be on different physical infrastructure
  - Simplifies integration: "Plug Port A here, Port B there"

## PCB Layout Guidelines

Critical Layout Rules for Ethernet:

1. Differential Pair Routing (TXD, RXD)

```
   ┌─────────────────────────────────────┐
   │ - Impedance: 50Ω ± 10% differential │
   │ - Trace width: ~8 mil (0.2 mm)      │
   │ - Spacing: ~8 mil (keep tight)      │
   │ - Length match: ±5 mil              │
   │ - Via count: Minimize (<4 per pair) │
   │ - Keep away: 3× trace width from    │
   │   other signals                     │
   └─────────────────────────────────────┘
```

2. Clock Signal (REF_CLK - 50 MHz)
   - Route as controlled impedance (50Ω)
   - Keep away from differential pairs
   - Series termination: 22-33Ω at source
   - Guard with GND traces on both sides

3. Ground Plane Strategy
   - Solid GND plane under entire Ethernet section
   - No splits or gaps under differential pairs
   - Chassis GND separate from circuit GND
   - Stitch with vias every 5 mm around perimeter

4. Component Placement
   PHY → Magnetics → RJ-45 (straight line)
   - Total path length: <5 cm preferred
   - No 90° bends in differential pairs (use arcs)
   - Keep decoupling caps close to PHY VDD pins

5. EMC Considerations
   - Ferrite bead on PHY power: 600Ω @ 100 MHz
   - Common-mode choke in RJ-45 magnetics
   - TVS diodes at connector (fast response)
   - Chassis connection via Y-cap (1 nF, 2 kV)
