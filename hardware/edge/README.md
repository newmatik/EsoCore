# EsoCore Edge

## PCB Specification

- Board: 4-layer FR4 1.55 mm +/-10%
- Dimensions: 86 × 171 mm (H × W) – sized for custom DIN Rail housing
- Surface finish: ENIG
- Copper: 35 µm (1 OZ) outer/inner
- Black soldermask / white silkscreen

## Hardware

- MCU: STM32H747IIT6 (LQFP176) dual-core M7/M4, 25 MHz active oscillator (shared with Ethernet PHY), 32.768 kHz RTC
- MCU support: 1.2 V LDO (TLV70212DBVR) for internal regulator bypass; ferrite bead (BLM18PG121SN1D) on VDD rail
- Storage: microSD socket (push-push) with EMIF06-MSD02N16 EMI filter on SDIO lines, industrial microSD card
- Power: Dedicated 24 V DC input -> 5 V buck (TPS54331, 3 A) -> 3.3 V digital LDO + 3.3 V low-noise analog LDO (TPS7A4533)
- Backup: 3.8 V 20 F supercap (SLA3R8L2060813) with 680 ohm charge current limiting and Schottky diode (CDBA240LL-HF)
  charge path control for safe shutdown; evaluate adding second unit if 30 s runtime target requires more capacity
- Protection: TVS diodes + varistors on 24 V input; input LC filter (47 uH + 470 uF); ESD arrays on USB/GPIO/sensor bus
- Interfaces:
  - USB-C device/service with common-mode choke (DLW21SN900SQ2L) and ESD protection (USBULC6-2M6); firmware flashing
    and terminal communication only; not used for normal operation or data collection
  - Dual Ethernet 10/100 (2x KSZ8081RNACA) with integrated magnetics (2x Wurth 7499111121) and ferrite beads
    (FCM1608KF-601T03) per PHY; Port A (Machine), Port B (IT)
  - RS-485 sensor bus (SN65HVD72), termination selectable
  - Fieldbus: Isolated Modbus RTU, PROFIBUS-compatible RS-485 (DB-9), dedicated Interbus RS-485
- I/O connectors:
  - Pluggable terminal blocks (Degson): plugs `2EDGK-5.0-xxP` mating with PCB connectors `2EDGR-5.0-xxP`; all PCB mates are 90° angled at PCB edge for
    enclosure access
  - 2x 4-pin sensor bus: plug `2EDGK-5.0-04P` / PCB mate `2EDGR-5.0-04P`
  - Safety digital inputs: 14-pin `2EDGK-5.0-14P` / `2EDGR-5.0-14P` (6 dual-channel safety inputs, 12 optocouplers)
  - Safety relay output: 6-pin `2EDGK-5.0-06P` / `2EDGR-5.0-06P`
  - Analog inputs: 10-pin `2EDGK-5.0-10P` / `2EDGR-5.0-10P` (4 dual-mode IEPE/DC channels)
  - Fieldbus: 6-pin `2EDGK-5.0-06P` / `2EDGR-5.0-06P`; PROFIBUS: DB-9
  - Interbus: 4-pin `2EDGK-5.0-04P` / `2EDGR-5.0-04P`
  - Power input: 2-pin `2EDGK-5.0-02P` / `2EDGR-5.0-02P`
- Programming/debug:
  - STM32 SWD: Tag-Connect TC2050-IDC-NL (10-pin needle footprint)
  - ESP32 UART: Tag-Connect TC2030-IDC-NL (6-pin needle footprint)
- WiFi/BT: ESP32-S3-WROOM-1-N16R8 with USB VBUS LDO (AMS1117-3.3) and auto-programming circuit (UMH3N)
- Safety I/O: 6 dual-channel safety inputs (12x PC817X2NIP1B optocouplers, EN ISO 13849 Cat. 3) + safety relay
  (1x G7SA-2A2B-DC24, 2NO+2NC) with DRV110APWR coil driver for PLd interlocking
- UI: 1.5" 128x64 OLED (I2C) with BSS138LT1G level shifter, Reset/Boot/Menu buttons, status LEDs
- Analog: 4-ch IEPE signal chain (ADS1274 ADC, PGA280, OPA4188, REF200AU current sources)

## Mechanical Constraints

- **PCB dimensions**: 86 mm (H) × 171 mm (W) to fit in custom DIN Rail housing
- **Housing**: 90 mm height × 175 mm width (10 × 17.5 mm for DIN rail mounting)
- **Clearance**: 2 mm on all sides between PCB edge and housing walls
- **Dimension logic**:
  - Height: 90 mm housing − 2 mm (top) − 2 mm (bottom) = **86 mm PCB**
  - Width: Housing must be multiple of 17.5 mm; 10 × 17.5 = 175 mm housing − 2 mm (left) − 2 mm (right) = **171 mm PCB**
- **Note**: Height is fixed at 86 mm; width can be adjusted in 17.5 mm increments (with corresponding 4 mm reduction for PCB)
- **Edge access**:
  - **Top edge**: All pluggable terminal blocks (green Degson connectors)
  - **Bottom edge**: USB-C, 2x Ethernet RJ45, DB-9 (PROFIBUS)
  - **Left and right edges**: Keep clear (no connectors or tall components)

## General Layout Requirements

- Black solder mask with white silk screen
- Silk screen for important components with information value only (such as connectors)
- Prefer SMD components; avoid THT where feasible
- Single-sided assembly: place all SMD and THT on one side only (no backside assembly)
- Prefer increasing PCB width (if needed) over double-sided assembly to reduce cost and simplify enclosure
- Provide Tag-Connect pads: TC2050 footprint for STM32 SWD and TC2030 footprint for ESP32 UART
- Include three fiducials on the assembled side
- Avoid single-sourcing; specify at least two MPNs where practical (exceptions: MCU or unique sensors)
- Evaluate cost-effective alternatives on LCSC, including reputable Chinese brands, not only established manufacturers

## Layout Guidance

### Component Placement

- **Display and UI**: Place 1.5" OLED display and buttons (Reset/Boot/Menu) on front face for easy access
- **Buttons**: Position Reset/Boot/Menu buttons directly below the display with sufficient spacing for easy operation; ensure no obstructions above
  button areas
- **Connectors**:
  - Group all pluggable terminal blocks (Degson green connectors) along the top edge
  - Place USB-C, Ethernet RJ45, and DB-9 along the bottom edge
  - Keep left and right edges completely clear
- **Tall components**: Keep away from left and right edges to maintain housing clearance

### Electrical Layout

- Separate power, digital, and analog zones; star ground tie at ADC/MCU ref
- Keep RS‑485, Ethernet, and USB return paths tight; add stitching vias near chokes
- Place TVS/ESD close to connectors; keep supercap high-current paths short
- Respect creepage/clearance around isolated fieldbus and Interbus sections

## CAD Toolchain

- Tool: Altium Designer
- Project: `hardware/edge/v1.4.0/Altium/PCB/EsoCore_Edge.PcbDoc`
- Schematics: `hardware/edge/v1.4.0/Altium/Schematics/*.SchDoc` (14 sheets)
- Footprint library: `hardware/edge/v1.4.0/Altium/Footprint library/EsoCore.PcbLib`
- Notes: Open the `.PcbDoc` in Altium Designer; edit schematics/layout in `.SchDoc`/`.PcbDoc`. Generate fabrication
  outputs via Altium Designer.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved. Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
