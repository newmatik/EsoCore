# EdgeSentinel Temperature Sensor

## PCB Specification

- Board: 2-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 40 × 30 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ)

## Hardware

- MCU: STM32G031K8T6 (TSSOP20), 8 MHz crystal
- Sensor: 10 kΩ NTC (1%) with precision bias network
- ADC: ADS1115 16-bit for high-accuracy temperature measurement
- Bus: RS‑485 half‑duplex (SN65HVD72), bias/termination network
- Power: 12 V bus → 3.3 V buck (TPS62172)
- Connectors: 4‑pin screw terminal (12V, GND, D+, D‑)
- Protection: ESD array on bus and power lines
- Addressing: 4‑position DIP switch
- Indicators: Green status LED
- Programming: Tag‑Connect SWD footprint

## Layout Guidance

- Place precision resistors close to ADC; use Kelvin connections as appropriate
- Separate analog and digital grounds; single-point tie
- Filter and decouple the ADC reference and input nodes
- Provide strain relief and high-temp insulation for sensor lead exit

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
