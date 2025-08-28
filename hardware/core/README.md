# EdgeSentinel Core

## PCB Specification

- Board: 4-layer FR4 1.55 mm +/-10%
- Dimensions: Approx. 100 × 160 mm
- Surface finish: ENIG
- Copper: 35 µm (1 OZ) outer/inner

## Hardware

- MCU: STM32H757ZIT6 (LQFP144), external HSE 25 MHz, 32.768 kHz RTC
- Storage: microSD socket (push–push), industrial microSD card
- Power: 12–24 V DC input → 5 V buck (3 A) → 3.3 V buck/LDO rails
- Backup: Six 2.7 V 5 F supercaps for ≥30 s safe shutdown
- Protection: TVS on input and interfaces; ESD arrays on USB/GPIO/sensor bus
- Interfaces:
  - USB-C device/service with common-mode choke
  - Ethernet 10/100 (DP83825I) with MagJack; PoE PD (LTC4267) option
  - RS‑485 sensor bus (SN65HVD72), termination selectable
  - Fieldbus: Isolated Modbus RTU, PROFIBUS-compatible RS‑485 (DB‑9)
- I/O connectors:
  - 2× 4-pin sensor bus (primary/secondary)
  - Safety I/O: 8-pin input, 6-pin output
  - General I/O: 12-pin (DI, AI, relay)
  - Fieldbus: 6-pin terminal, DB‑9 for PROFIBUS
  - SWD 2×5 1.27 mm header (or Tag‑Connect footprint)
- Safety: Dual-channel safety relays with monitoring IC (PLd target)
- UI: 0.96" 128×64 OLED (I²C), Reset/Boot/Menu buttons, status LEDs
- PoE: IEEE 802.3af PD controller with isolation transformer; signature resistor 25.5 kΩ

## Layout Guidance

- Avoid THT components if possible and use SMD components instead
- Ensure we have Tag-Connect pads on board for MCU flashing
- Ensure we have 3 fiducials on each SMD assembled side
- Separate power/PoE, digital, and analog zones; star ground tie at ADC/MCU ref
- Keep RS‑485, Ethernet, and USB return paths tight; add stitching vias near chokes
- Place TVS/ESD close to connectors; keep supercap high-current paths short
- Respect creepage/clearance around isolated fieldbus and PoE sections

## CAD Toolchain

- Tool: KiCad (8.x recommended)
- Project: `hardware/core/kicad/core.kicad_pro`
- Notes: Open the `.kicad_pro` in KiCad; edit schematics/layout in `.kicad_sch`/`.kicad_pcb`. Generate fabrication outputs via KiCad Plot and Drill tools.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
