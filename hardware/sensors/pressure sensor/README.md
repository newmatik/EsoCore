# EsoCore Pressure Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 50 × 40 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: MPL3115A2 (0-10bar differential pressure)
- Sensor: BMP388 (0-100bar industrial pressure)
- Sensor: BMP280 (0-1bar low-pressure monitoring)
- Bus: RS‑485 half‑duplex (SN65HVD72), bias/termination network
- Power: 12 V bus → 3.3 V buck (TPS62172) + LDO (MCP1700) for clean analog
- Connectors: 4‑pin screw terminal (12V, GND, D+, D‑), pressure fittings G1/4 BSP
- Protection: ESD arrays on bus/power, pressure relief valve
- Addressing: 4‑position DIP switch
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## Applications

- Hydraulic system pressure monitoring
- Pneumatic system pressure monitoring
- Pump performance monitoring
- Filter clogging detection
- Leak detection in pressurized systems
- Process pressure control validation

## General Layout Requirements

- Prefer SMD components; avoid THT where feasible
- Provide Tag-Connect pads for MCU programming
- Include three fiducials on each SMD-assembled side
- Avoid single-sourcing; specify at least two MPNs where practical (exceptions: MCU or unique sensors)
- Evaluate cost-effective alternatives on LCSC, including reputable Chinese brands, not only established manufacturers

## Layout Guidance

- Separate analog and digital grounds; single-point tie
- Place pressure sensors away from heat sources
- Include pressure relief valve for safety
- Route RS‑485 differential pair with proper spacing
- Provide proper sealing for pressure ports
- Allow space for pressure fittings and relief valve

## CAD Toolchain

- Tool: KiCad (8.x recommended)
- Projects: live under the `kicad/` subfolder of this device
- Main project: `pressure-sensor.kicad_pro`
- Notes: Open the `.kicad_pro` in KiCad; generate fabrication outputs via KiCad Plot and Drill tools.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
