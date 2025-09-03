# EsoCore Current Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 50 × 35 mm (DIN rail enclosure)
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)
- Black soldermash / white silkscreen

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: ACS723 ±30 A Hall current sensor (higher range, better linearity)
- Isolation: RS‑485 transceiver ADM2582E (integrated iso + DC/DC)
- Power: Isolated 12V→5V DC/DC (MEA1S1205SC) → 3.3 V LDO (MCP1700)
- Connectors: Two pluggable RS‑485 ports (IN and OUT): plug `2EDGK-5.0-04P` ↔ PCB mate `2EDGR-5.0-04P`; silk labels A/B/24V/GND; signal loop‑through; clamp-on current interface
- Protection: GDT surge suppressor on power; ESD arrays at bus/power
- Addressing: 8‑position SMD DIP; 1–7 = Modbus address bits, 8 = 120 Ω termination enable; silkscreen: "1‑7=ADDR, 8=TERM, ON/OFF"
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

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

- Respect creepage/clearance across isolation barrier; route isolated and non-isolated grounds appropriately
- Keep high-current paths wide and short; minimize loop area
- Place ESD and GDT near connectors; filter supply rails for ADC measurements
- Provide DIN rail mounting holes in accordance with enclosure spec

## CAD Toolchain

- Tool: Altium Designer
- Projects: live under the `altium/` subfolder of this device
- Main project: `current-sensor.PcbDoc`
- Notes: Open the `.PcbDoc` in Altium Designer; generate fabrication outputs via Altium Designer.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
