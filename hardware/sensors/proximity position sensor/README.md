# EdgeSentinel Proximity Position Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 55 × 40 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: Inductive proximity sensor (2×) for metal detection
- Sensor: Capacitive proximity sensor for non-metal detection
- Sensor: Quadrature encoder 1000PPR for position/speed measurement
- Sensor: Magnetic reed switch (2×) for position sensing
- Sensor: VL53L0X laser distance sensor for precision positioning
- Bus: RS‑485 half‑duplex (SN65HVD72), bias/termination network
- Power: 12 V bus → 3.3 V buck (TPS62172) + LDO (MCP1700) for clean analog
- Connectors: 4‑pin screw terminal (12V, GND, D+, D‑), M12 proximity connectors, encoder connector
- Protection: ESD arrays on bus/power lines
- Addressing: 4‑position DIP switch
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

## Layout Guidance

- Avoid THT components if possible and use SMD components instead
- Ensure we have Tag-Connect pads on board for MCU flashing
- Ensure we have 3 fiducials on each SMD assembled side
- Separate analog and digital grounds; single-point tie
- Keep encoder differential pairs short and matched
- Provide proper shielding for proximity sensors
- Include mounting brackets for different sensor types
- Allow space for various connector types (M12, encoder)
- Route laser sensor away from EMI sources

## CAD Toolchain

- Tool: KiCad (8.x recommended)
- Projects: live under the `kicad/` subfolder of this device
- Main project: `proximity-position-sensor.kicad_pro`
- Notes: Open the `.kicad_pro` in KiCad; generate fabrication outputs via KiCad Plot and Drill tools.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
