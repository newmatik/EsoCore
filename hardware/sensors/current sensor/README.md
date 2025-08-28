# EdgeSentry Current Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 50 × 35 mm (DIN rail enclosure)
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: ACS724 ±20 A Hall current sensor (board-mounted)
- Isolation: RS‑485 transceiver ADM2582E (integrated iso + DC/DC)
- Power: Isolated 12V→5V DC/DC (MEA1S1205SC) → 3.3 V LDO (MCP1700)
- Connectors: 4‑pin screw terminal (12V, GND, D+, D‑), clamp-on current interface
- Protection: GDT surge suppressor on power; ESD arrays at bus/power
- Addressing: 4‑position DIP switch
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## Layout Guidance

- Respect creepage/clearance across isolation barrier; route isolated and non-isolated grounds appropriately
- Keep high-current paths wide and short; minimize loop area
- Place ESD and GDT near connectors; filter supply rails for ADC measurements
- Provide DIN rail mounting holes in accordance with enclosure spec

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
