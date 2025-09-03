# EsoCore Temperature Spot Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 50 × 35 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)
- Black soldermash / white silkscreen

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: High-temp NTC thermistor (4×) for bearings and hot spots
- Sensor: PT100 RTD (2×) for precision temperature measurement
- Sensor: Type K thermocouple amplifier (2×) for extreme temperatures
- Sensor: MLX90614 IR temperature sensor for non-contact measurement
- ADC: ADS1115 16-bit for high-precision temperature measurements
- Bus: RS‑485 half‑duplex (SN65HVD72), bias/termination network
- Power: 12 V bus → 3.3 V buck (TPS62172) + LDO (MCP1700) for clean analog
- Connectors: Two pluggable RS‑485 ports (IN and OUT): plug `2EDGK-5.0-04P` ↔ PCB mate `2EDGR-5.0-04P`; silk labels A/B/24V/GND; signal loop‑through; thermocouple connectors
- Protection: ESD arrays on bus/power lines
- Addressing: 8‑position SMD DIP; 1–7 = Modbus address bits, 8 = 120 Ω termination enable; silkscreen: "1‑7=ADDR, 8=TERM, ON/OFF"
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## Applications

- Motor bearing temperature monitoring
- Gearbox temperature hotspots
- Welding torch temperature control
- Oven and furnace temperature monitoring
- Hydraulic system component temperatures
- Non-contact temperature measurement for moving parts

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

- Place precision resistors close to ADC; use Kelvin connections
- Separate analog and digital grounds; single-point tie
- Keep thermocouple wires short and twisted
- Provide high-temperature insulation for extreme environments
- Include mineral-insulated thermocouple connectors
- Allow space for multiple sensor types and connectors

## CAD Toolchain

- Tool: Altium Designer
- Projects: live under the `altium/` subfolder of this device
- Main project: `temperature-spot-sensor.PcbDoc`
- Notes: Open the `.PcbDoc` in Altium Designer; generate fabrication outputs via Altium Designer.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
