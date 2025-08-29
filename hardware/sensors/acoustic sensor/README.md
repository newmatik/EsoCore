# EsoCore Acoustic Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 40 × 30 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: Digital MEMS microphone ICS-43434 (wide-band 50Hz-20kHz ±3dB)
- Sensor: Ultrasound transducer MA40S4R (40±1kHz frequency response)
- Bus: RS‑485 half‑duplex (SN65HVD72), bias/termination network
- Power: 12 V bus → 3.3 V buck (TPS62172) + LDO (MCP1700) for clean analog
- Connectors: 4‑pin screw terminal (12V, GND, D+, D‑), PG9 cable gland
- Addressing: 4‑position DIP for module address
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## General Layout Requirements

- Prefer SMD components; avoid THT where feasible
- Provide Tag-Connect pads for MCU programming
- Include three fiducials on each SMD-assembled side
- Avoid single-sourcing; specify at least two MPNs where practical (exceptions: MCU or unique sensors)
- Evaluate cost-effective alternatives on LCSC, including reputable Chinese brands, not only established manufacturers

## Layout Guidance

- Keep microphone port unobstructed; consider acoustic opening and gasket
- Separate analog and digital grounds; single-point tie
- Place ESD near connector; route RS‑485 differential pair
- Provide mechanical support for enclosure mounting; consider magnetic base interface
- Allow space for both audible and ultrasonic frequency sensors

## CAD Toolchain

- Tool: KiCad (8.x recommended)
- Projects: live under the `kicad/` subfolder of this device
- Main project: `acoustic-sensor.kicad_pro`
- Notes: Open the `.kicad_pro` in KiCad; generate fabrication outputs via KiCad Plot and Drill tools.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
