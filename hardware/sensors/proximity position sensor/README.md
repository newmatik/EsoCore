# EsoCore Proximity Position Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 55 × 40 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)
- Black soldermash / white silkscreen

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: Inductive proximity sensor (2×) for metal detection
- Sensor: Capacitive proximity sensor for non-metal detection
- Sensor: Quadrature encoder 1000PPR for position/speed measurement
- Sensor: Magnetic reed switch (2×) for position sensing
- Sensor: VL53L0X laser distance sensor for precision positioning
- Bus: RS‑485 half‑duplex (SN65HVD72), bias/termination network
- Power: 12 V bus → 3.3 V buck (TPS62172) + LDO (MCP1700) for clean analog
- Connectors: Two pluggable RS‑485 ports (IN and OUT): plug `2EDGK-5.0-04P` ↔ PCB mate `2EDGR-5.0-04P`; silk labels A/B/24V/GND; signal loop‑through; M12 proximity connectors, encoder connector
- Protection: ESD arrays on bus/power lines
- Addressing: 8‑position SMD DIP; 1–7 = Modbus address bits, 8 = 120 Ω termination enable; silkscreen: "1‑7=ADDR, 8=TERM, ON/OFF"
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## Applications

- Machine cycle counting and positioning
- Conveyor belt position monitoring
- Robotic arm position validation
- Door and gate position sensing
- Assembly line part detection
- Speed and RPM monitoring
- Precision distance measurement

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

- Separate analog and digital grounds; single-point tie
- Keep encoder differential pairs short and matched
- Provide proper shielding for proximity sensors
- Include mounting brackets for different sensor types
- Allow space for various connector types (M12, encoder)
- Route laser sensor away from EMI sources

## CAD Toolchain

- Tool: Altium Designer
- Projects: live under the `altium/` subfolder of this device
- Main project: `proximity-position-sensor.PcbDoc`
- Notes: Open the `.PcbDoc` in Altium Designer; generate fabrication outputs via Altium Designer.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
