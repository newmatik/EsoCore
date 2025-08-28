# EdgeSentinel Temperature Spot Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 50 × 35 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: High-temp NTC thermistor (4×) for bearings and hot spots
- Sensor: PT100 RTD (2×) for precision temperature measurement
- Sensor: Type K thermocouple amplifier (2×) for extreme temperatures
- Sensor: MLX90614 IR temperature sensor for non-contact measurement
- ADC: ADS1115 16-bit for high-precision temperature measurements
- Bus: RS‑485 half‑duplex (SN65HVD72), bias/termination network
- Power: 12 V bus → 3.3 V buck (TPS62172) + LDO (MCP1700) for clean analog
- Connectors: 4‑pin screw terminal (12V, GND, D+, D‑), thermocouple connectors
- Protection: ESD arrays on bus/power lines
- Addressing: 4‑position DIP switch
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## Applications

- Motor bearing temperature monitoring
- Gearbox temperature hotspots
- Welding torch temperature control
- Oven and furnace temperature monitoring
- Hydraulic system component temperatures
- Non-contact temperature measurement for moving parts

## Layout Guidance

- Avoid THT components if possible and use SMD components instead
- Ensure we have Tag-Connect pads on board for MCU flashing
- Ensure we have 3 fiducials on each SMD assembled side
- Place precision resistors close to ADC; use Kelvin connections
- Separate analog and digital grounds; single-point tie
- Keep thermocouple wires short and twisted
- Provide high-temperature insulation for extreme environments
- Include mineral-insulated thermocouple connectors
- Allow space for multiple sensor types and connectors

## CAD Toolchain

- Tool: KiCad (8.x recommended)
- Projects: live under the `kicad/` subfolder of this device
- Main project: `temperature-spot-sensor.kicad_pro`
- Notes: Open the `.kicad_pro` in KiCad; generate fabrication outputs via KiCad Plot and Drill tools.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
