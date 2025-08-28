# EdgeSentinel – Monorepo (Docs, Hardware, Firmware, Software)

This repository contains the public documentation website and the source for hardware, firmware, and supporting software for EdgeSentinel.

## Website

The public website and docs live in `website/`. See `website/README.md` for
local development and deployment instructions.

## Structure

- `website/` – Public website (published folder)
  - `assets/` – Shared styles and scripts used by the site and docs app
  - `docs/` – Documentation content and shell
- `hardware/` – Hardware sources
  - `bom/` – Canonical Bill of Materials CSVs (author/edit here)
  - `altium/` – PCB projects (add here)
  - `mechanical/` – STEP/drawings (add here)
- `firmware/` – Device and board-support firmware
  - `stm32/`, `common/`, etc.
- `software/` – Host-side tools/services
- `tools/` – Utility scripts (e.g., sync, build, release)

Notes on BOMs:
- The canonical BOM CSVs live in `hardware/bom/`.
- The docs app reads CSVs from `website/docs/data/` for public rendering.
- After editing CSVs in `hardware/bom/`, copy them to `website/docs/data/` to update the site. We can add a small sync script in `tools/` if desired.

## Open source

Website source: [github.com/newmatik/edgesentinel](https://github.com/newmatik/edgesentinel)

## Protocols

The hardware requirements and BOM include options for industrial fieldbus protocols:

- Modbus RTU (RS‑485, isolated) and Modbus TCP (Ethernet)
- PROFIBUS DP (DB‑9, switchable termination)
- PROFINET Device (over Ethernet)
