# EdgeSentinel Vibration Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 40 × 30 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)

## Hardware

- MCU: STM32G071KBT6 (TSSOP20), 8 MHz crystal, enhanced DSP for FFT processing
- Sensor: ADXL355 (low-noise 3‑axis MEMS accelerometer, ≥1-3kHz ODR)
- Bus: RS‑485 half‑duplex (SN65HVD72), bias/termination network
- Power: 12 V bus → 3.3 V buck (TPS62172) + LDO (MCP1700) for clean analog
- Connectors: 4‑pin screw terminal (12V, GND, D+, D‑), PG9 cable gland
- Addressing: 4‑position DIP switch for module address
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## Layout Guidance

- Avoid THT components if possible and use SMD components instead
- Ensure we have Tag-Connect pads on board for MCU flashing
- Ensure we have 3 fiducials on each SMD assembled side
- Place ADXL355 near center; maintain symmetric mechanical coupling to mounting
- Keep analog supplies and sensor traces short; isolate from RS‑485 transients
- Place ESD array near the connector; route RS‑485 as differential pair with spacing
- Avoid vibration-sensitive tall components near sensor; use mounting studs with thread-lock

## CAD Toolchain

- Tool: KiCad (8.x recommended)
- Projects: live under the `kicad/` subfolder of this device
- Main project: `vibration-sensor.kicad_pro`
- Notes: Open the `.kicad_pro` in KiCad; generate fabrication outputs via KiCad Plot and Drill tools.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
