# EsoCore – Monorepo (Docs, Hardware, Firmware, Software)

[![Sponsor](https://img.shields.io/badge/Sponsor-Newmatik-blue?style=for-the-badge&logo=github-sponsors)](https://www.newmatik.com)

This repository contains the public documentation website and the source for hardware, firmware, and supporting software for EsoCore.

## Website

The public website and docs live in `website/`. See `website/README.md` for
local development and deployment instructions.

## Structure

- `website/` – Public website (published folder)
  - `assets/` – Shared styles and scripts used by the site and docs app
  - `docs/` – Documentation content and shell
- `server/` – Django REST API backend
  - Django + Django REST Framework application
  - Core data models and API endpoints
- `portal/` – Nuxt.js frontend application
  - User dashboard and device management interface
  - Real-time monitoring and alerting
- `hardware/` – Hardware sources (see READMEs in subfolders)
  - `core/`
  - `sensors/`
    - `vibration sensor/`
    - `acoustic sensor/`
    - `current sensor/`
    - `air quality sensor/`
    - `light sensor/`
    - `oil quality sensor/`
    - `pressure sensor/`
    - `temperature spot sensor/`
    - `proximity position sensor/`
  - `cables/`
  - CAD files live under each device's `kicad/` folder; open the `.kicad_pro` projects with KiCad (8.x recommended).
- `firmware/` – Device and board-support firmware
  - `stm32/`, `common/`, etc.
- `tools/` – Utility scripts (e.g., sync, build, release)

## Backend (Django REST API)

The server backend is built with Django and Django REST Framework, providing:

- RESTful API endpoints for device telemetry ingestion
- Device management and provisioning
- User authentication and authorization
- Real-time event processing and alerting
- Data storage with SQLite (dev) / PostgreSQL (prod)

See `server/README.md` for setup and development instructions.

## Frontend (Nuxt.js Portal)

The user portal is built with Nuxt.js, providing:

- Device dashboard and monitoring
- Real-time telemetry visualization
- Alert management and notifications
- User management and role-based access
- Responsive web interface

See `portal/README.md` for setup and development instructions.

Notes on BOMs:

- Canonical BOM CSVs live in each device folder under `hardware/` (e.g., `hardware/edge/`, `hardware/sensors/*/`, `hardware/cables/`).
- The docs app reads CSVs from `website/docs/data/` for public rendering.
- After editing BOMs in `hardware/…/`, copy them to `website/docs/data/` to update the site.

Each folder under `hardware/` contains a `README.md` with more details.

## Documentation

- **[Hardware: Edge](website/docs/esocore-edge.md)** - Core device specifications and connectivity
- **[Hardware: Sensors](website/docs/esocore-sensors.md)** - Complete sensor module catalog and applications

## Open source

Website source: [github.com/newmatik/esocore](https://github.com/newmatik/esocore)

## Protocols

The hardware requirements and BOM include options for industrial fieldbus protocols:

- Modbus RTU (RS‑485, isolated) and Modbus TCP (Ethernet)
- PROFIBUS DP (DB‑9, switchable termination)
- PROFINET Device (over Ethernet)

## Legal

Copyright © 2025 Newmatik. All rights reserved.

Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
