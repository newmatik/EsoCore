# EdgeSentry Acoustic Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 40 × 30 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: Wide-band MEMS microphone (SPU0410LR5H)
- Analog front-end: MAX9814 microphone preamp with AGC
- Bus: RS‑485 half‑duplex (SN65HVD72), bias/termination network
- Power: 12 V bus → 3.3 V buck (TPS62172) → LDO (MCP1700) for low-noise analog
- Connectors: 4‑pin screw terminal (12V, GND, D+, D‑), PG9 cable gland
- Addressing: 4‑position DIP for module address
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## Layout Guidance

- Keep microphone port unobstructed; consider acoustic opening and gasket
- Separate analog ground for preamp; single-point tie to digital ground
- Place ESD near connector; route RS‑485 differential pair; keep AGC lines short
- Provide mechanical support for enclosure mounting; consider magnetic base interface

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
