# EsoCore Edge

## PCB Specification

- Board: 4-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 100 × 160 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ) outer/inner
- Black soldermash / white silkscreen

## Hardware

- MCU: STM32H747IIT6 (LQFP176) dual-core M7/M4, external HSE 25 MHz, 32.768 kHz RTC
- Storage: microSD socket (push–push), industrial microSD card
- Power: 12–24 V DC input → 5 V buck (3 A) → 3.3 V buck/LDO rails
- Backup: Six 2.7 V 5 F supercaps for ≥30 s safe shutdown
- Protection: TVS on input and interfaces; ESD arrays on USB/GPIO/sensor bus
- Interfaces:
  - USB-C device/service with common-mode choke
  - Ethernet 10/100 (DP83825I) with MagJack; PoE PD (LTC4267) option
  - RS‑485 sensor bus (SN65HVD72), termination selectable
  - Fieldbus: Isolated Modbus RTU, PROFIBUS-compatible RS‑485 (DB‑9)
- I/O connectors:
  - Pluggable terminal blocks (Degson): plugs `2EDGK-5.0-xxP` mating with PCB connectors `2EDGR-5.0-xxP`; all PCB mates are 90° angled at PCB edge for enclosure access
  - 2× 4-pin sensor bus: plug `2EDGK-5.0-04P` ↔ PCB mate `2EDGR-5.0-04P`
  - Safety I/O: 8-pin input `2EDGK-5.0-08P` ↔ `2EDGR-5.0-08P`, 6-pin output `2EDGK-5.0-06P` ↔ `2EDGR-5.0-06P`
  - General I/O: 2× 6-pin `2EDGK-5.0-06P` ↔ `2EDGR-5.0-06P` (DI, AI, relay)
  - Fieldbus: 6-pin `2EDGK-5.0-06P` ↔ `2EDGR-5.0-06P`; PROFIBUS: DB‑9
  - Power input: 2-pin `2EDGK-5.0-02P` ↔ `2EDGR-5.0-02P`
  - SWD 2×5 1.27 mm header (or Tag‑Connect footprint)
- Safety: Dual-channel safety relays with monitoring IC (PLd target)
- UI: 0.96" 128×64 OLED (I²C), Reset/Boot/Menu buttons, status LEDs
- PoE: IEEE 802.3af PD controller with isolation transformer; signature resistor 25.5 kΩ

## General Layout Requirements

- Black solder mask with white silk screen
- Silk screen for important components with information value only (such as connectors)
- Prefer SMD components; avoid THT where feasible
- Single-sided assembly: place all SMD and THT on one side only (no backside assembly)
- Prefer increasing PCB size over double-sided assembly to reduce cost and simplify enclosure
- Provide Tag-Connect pads for MCU programming
- Include three fiducials on the assembled side
- Avoid single-sourcing; specify at least two MPNs where practical (exceptions: MCU or unique sensors)
- Evaluate cost-effective alternatives on LCSC, including reputable Chinese brands, not only established manufacturers

## Layout Guidance

- Separate power/PoE, digital, and analog zones; star ground tie at ADC/MCU ref
- Keep RS‑485, Ethernet, and USB return paths tight; add stitching vias near chokes
- Place TVS/ESD close to connectors; keep supercap high-current paths short
- Respect creepage/clearance around isolated fieldbus and PoE sections

## CAD Toolchain

- Tool: Altium Designer
- Project: `hardware/edge/altium/core.PcbDoc`
- Notes: Open the `.PcbDoc` in Altium Designer; edit schematics/layout in `.SchDoc`/`.PcbDoc`. Generate fabrication outputs via Altium Designer.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
