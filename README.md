# EdgeSentinel – Monorepo (Docs, Hardware, Firmware, Software)

This repository contains the public documentation website and the source for hardware, firmware, and supporting software for EdgeSentinel.

## Website

The public website and docs live in `website/`. See `website/README.md` for
local development and deployment instructions.

## Structure

- `website/` – Public website (published folder)
  - `assets/` – Shared styles and scripts used by the site and docs app
  - `docs/` – Documentation content and shell
- `hardware/` – Hardware sources (see READMEs in subfolders)
  - `core/`
  - `sensors/`
    - `vibration sensor/`
    - `acoustic sensor/`
    - `current sensor/`
    - `temperature sensor/`
  - `cables/`
  - CAD files live under each device's `kicad/` folder; open the `.kicad_pro` projects with KiCad (8.x recommended).
- `firmware/` – Device and board-support firmware
  - `stm32/`, `common/`, etc.
- `software/` – Host-side tools/services
- `tools/` – Utility scripts (e.g., sync, build, release)

Notes on BOMs:

- Canonical BOM CSVs live in each device folder under `hardware/` (e.g., `hardware/core/`, `hardware/sensors/*/`, `hardware/cables/`).
- The docs app reads CSVs from `website/docs/data/` for public rendering.
- After editing BOMs in `hardware/…/`, copy them to `website/docs/data/` to update the site.

Each folder under `hardware/` contains a `README.md` with more details.

## Open source

Website source: [github.com/newmatik/edgesentinel](https://github.com/newmatik/edgesentinel)

## Protocols

The hardware requirements and BOM include options for industrial fieldbus protocols:

- Modbus RTU (RS‑485, isolated) and Modbus TCP (Ethernet)
- PROFIBUS DP (DB‑9, switchable termination)
- PROFINET Device (over Ethernet)

## Legal

Copyright © 2025 Newmatik. All rights reserved.

Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
