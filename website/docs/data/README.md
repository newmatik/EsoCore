# BOM data (generated from hardware)

This folder contains CSV exports of the Bill of Materials used by the website.

- Source of truth: `hardware/`
- Destination: this folder `website/docs/data`
- Files here are overwritten during sync. Do not edit these CSVs manually.

Run the sync script to copy the latest BOMs from `hardware/`:

```bash
# From the repo root
python3 tools/sync_bom_data.py --verbose
```

Options:

- `--dry-run`: preview actions without writing files
- `--only {all,main,cables,sensors}`: restrict scope
- `--hardware-dir PATH` / `--data-dir PATH`: override defaults

Mapping rules used by the script:

- `hardware/core/bom.csv` -> `website/docs/data/bom-main.csv`
- `hardware/cables/bom.csv` -> `website/docs/data/bom-cables.csv`
- `hardware/sensors/<name>/bom.csv` -> `website/docs/data/bom-<slug(name)>.csv`
  - `slug(name)`: lowercased; non-alphanumerics replaced with `-`; repeated `-` collapsed

Any manual changes here will be lost the next time the sync script runs.
