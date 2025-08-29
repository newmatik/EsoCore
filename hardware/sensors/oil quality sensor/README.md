# EsoCore Oil Quality Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 60 × 45 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: Turbidity sensor for contamination detection
- Sensor: DS18B20 oil temperature sensor
- Sensor: Piezoelectric viscosity sensor
- Sensor: Capacitive water content sensor
- Sensor: Dielectric constant sensor
- Bus: RS‑485 isolated transceiver (ADM2582E)
- Power: Isolated 12V→5V DC/DC (MEA1S1205SC) → 3.3 V LDO (MCP1700)
- Connectors: 4‑pin screw terminal (12V, GND, D+, D‑), oil sampling port G1/8 BSP
- Protection: GDT surge suppressor, ESD arrays, oil filter mesh
- Addressing: 4‑position DIP switch
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## Applications

- Gearbox oil condition monitoring
- Hydraulic oil quality assessment
- Lubrication system health monitoring
- Predictive maintenance for oil-lubricated equipment
- Contamination detection in industrial lubricants
- Oil degradation trend analysis

## General Layout Requirements

- Prefer SMD components; avoid THT where feasible
- Provide Tag-Connect pads for MCU programming
- Include three fiducials on each SMD-assembled side
- Avoid single-sourcing; specify at least two MPNs where practical (exceptions: MCU or unique sensors)
- Evaluate cost-effective alternatives on LCSC, including reputable Chinese brands, not only established manufacturers

## Layout Guidance

- Respect creepage/clearance across isolation barrier
- Keep high-current paths wide and short
- Place ESD and GDT near connectors
- Include oil sampling port with proper sealing
- Provide oil filter mesh protection for sensors
- Separate oil and electronics compartments
- Allow space for oil-resistant gaskets and seals

## CAD Toolchain

- Tool: KiCad (8.x recommended)
- Projects: live under the `kicad/` subfolder of this device
- Main project: `oil-quality-sensor.kicad_pro`
- Notes: Open the `.kicad_pro` in KiCad; generate fabrication outputs via KiCad Plot and Drill tools.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
