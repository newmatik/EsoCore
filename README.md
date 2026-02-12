# EsoCore

[![Sponsor](https://img.shields.io/badge/Sponsor-Newmatik-blue?style=for-the-badge&logo=github-sponsors)](https://www.newmatik.com)

EsoCore (Edge Sensor Observation, Control, Operations, Reliability Engine) is an open-source industrial IoT platform
for monitoring factory machinery and equipment. It spans the full stack from PCB hardware and embedded firmware to a
Django REST API backend and a Nuxt.js user portal.

## Repository Structure

```text
esocore/
  server/       Django REST API backend
  portal/       Nuxt.js user portal (dashboard, devices, alerts)
  website/      Public documentation site (static Nuxt.js, GitHub Pages)
  firmware/     Embedded C firmware for Edge (STM32H7) and sensors (STM32G0)
  hardware/     PCB designs (Altium), BOMs, Gerber, STEP, schematics
  tools/        Utility scripts (BOM sync, STEP linter)
  assets/       Shared icons and images
```

## Tech Stack

| Layer | Technology |
|-------|------------|
| Backend | Python 3.11+, Django 5, Django REST Framework, Poetry |
| Portal | TypeScript, Nuxt 4 (Vue 3), Nuxt UI, Tailwind CSS 4, Pinia, Chart.js |
| Website | TypeScript, Nuxt 4 (Vue 3), static generation, GitHub Pages |
| Firmware | C (C11), STM32H7 / STM32G0, ARM GCC, GNU Make |
| Hardware | Altium Designer, 4-layer FR4 PCBs |

## Prerequisites

- **Python 3.11+** and [Poetry](https://python-poetry.org/) for the server
- **Node 22+** and [pnpm](https://pnpm.io/) for the portal and website
- **ARM GCC** (`arm-none-eabi-gcc`) and **GNU Make** for the firmware

## Getting Started

### Server (Django REST API)

```bash
cd server
poetry install
poetry run python manage.py migrate
poetry run python manage.py createsuperuser
poetry run python manage.py runserver
```

The API is served at `http://localhost:8000/api/` with Swagger docs at `http://localhost:8000/api/docs/`.

See `server/README.md` for full setup, authentication, and deployment details.

### Portal (Nuxt.js)

```bash
cd portal
pnpm install
pnpm run dev
```

The portal runs at `http://localhost:3000`.

See `portal/README.md` for configuration and branding details.

### Website (Documentation)

```bash
cd website
pnpm install
pnpm run dev
```

To generate the static site for production:

```bash
pnpm run generate
```

See `website/README.md` for deployment instructions.

### Firmware

```bash
cd firmware
make all          # Build Edge + all sensors
make edge         # Build Edge only
make vibration    # Build a single sensor target
make help         # List all available targets
```

Requires the ARM GCC toolchain. See `firmware/README.md` for architecture and protocol details.

## Hardware

PCB designs, schematics, and BOMs are organized under `hardware/`:

- `hardware/edge/` -- Edge device (STM32H747, dual Ethernet, RS-485, WiFi, IEPE inputs, safety I/O)
- `hardware/sensors/` -- RS-485 sensor modules
  - vibration, acoustic, current, air quality, light, oil quality, pressure, temperature spot, proximity position
- `hardware/cables/` -- Cable assemblies

CAD files are in each device's `altium/` folder (open the `.PcbDoc` projects with Altium Designer).
See `hardware/README.md` and individual sensor READMEs for specifications.

### BOM Management

Canonical BOM CSVs live in each device folder under `hardware/`. The website reads CSVs from
`website/public/bom/` for public rendering. After editing any BOM in `hardware/`, run:

```bash
python3 tools/sync_bom_data.py
```

## Protocols

The Edge device supports industrial fieldbus protocols:

- Modbus RTU (RS-485, isolated) and Modbus TCP (Ethernet)
- PROFIBUS DP (DB-9, switchable termination)
- PROFINET Device (over Ethernet)

## Documentation

- [Hardware: Edge](website/content/esocore-edge.md) -- Core device specifications and connectivity
- [Hardware: Sensors](website/content/esocore-sensors.md) -- Sensor module catalog and applications
- [Installation Guide](website/content/getting-started/installation-guide.md) -- Full setup walkthrough
- Full docs are published at [www.esocore.com](https://www.esocore.com)

## Open Source

Source code: [github.com/newmatik/esocore](https://github.com/newmatik/esocore)

## Legal

Copyright 2026 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/content/license.md` for details.
