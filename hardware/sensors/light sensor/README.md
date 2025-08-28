# EdgeSentinel Light Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 45 × 35 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: TSL2591 visible light sensor (high dynamic range 0.002-32k lux)
- Sensor: VEML6075 UVA/UVB light sensor for UV exposure monitoring
- Sensor: MLX90614 infrared temperature sensor for non-contact measurement
- Sensor: TCS34725 RGB color light sensor with IR blocking filter
- Sensor: AS7341 10-channel spectral sensor (380-1000nm)
- Bus: RS‑485 half‑duplex (SN65HVD72), bias/termination network
- Power: 12 V bus → 3.3 V buck (TPS62172) + LDO (MCP1700) for clean analog
- Connectors: 4‑pin screw terminal (12V, GND, D+, D‑), PG9 cable gland
- Protection: ESD arrays on bus/power lines
- Addressing: 4‑position DIP switch
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## Applications

- Workplace lighting compliance monitoring
- UV exposure safety monitoring for workers
- Process control for light-sensitive manufacturing
- Emergency lighting system validation
- Color quality control in production processes
- Spectral analysis for material characterization
- Non-contact temperature measurement
- Environmental light level monitoring

## General Layout Requirements

- Prefer SMD components; avoid THT where feasible
- Provide Tag-Connect pads for MCU programming
- Include three fiducials on each SMD-assembled side
- Avoid single-sourcing; specify at least two MPNs where practical (exceptions: MCU or unique sensors)
- Evaluate cost-effective alternatives on LCSC, including reputable Chinese brands, not only established manufacturers

## Layout Guidance

- Position light sensors to avoid mutual interference
- Use optical diffusers for uniform light measurement
- Keep sensor apertures unobstructed
- Provide proper shielding from electromagnetic interference
- Separate analog and digital grounds; single-point tie
- Include light baffles to prevent cross-talk between sensors
- Allow space for optical components and mounting hardware

## CAD Toolchain

- Tool: KiCad (8.x recommended)
- Projects: live under the `kicad/` subfolder of this device
- Main project: `light-sensor.kicad_pro`
- Notes: Open the `.kicad_pro` in KiCad; generate fabrication outputs via KiCad Plot and Drill tools.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
