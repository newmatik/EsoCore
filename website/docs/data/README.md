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
