# EsoCore Hardware

## General Layout Requirements

- Prefer SMD components; avoid THT where feasible
- Provide Tag-Connect pads for MCU programming
- Include three fiducials on each SMD-assembled side
- Avoid single-sourcing; specify at least two MPNs where practical (exceptions: MCU or unique sensors)
- Evaluate cost-effective alternatives on LCSC, including reputable Chinese brands, not only established manufacturers

## Hardware sources

See READMEs in subfolders.

- `edge/`
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

Notes:

- BOMs live in the respective device folders.
- CAD/binaries belong under `kicad/` and `mechanical/`.

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
