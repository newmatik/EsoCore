# Tools

Utility scripts. Both are standard-library Python 3; run the unit tests from the repository root with `python3 -m unittest discover -s tools -v` (the
Tools CI workflow runs the same command).

## sync_bom_data.py

Copies the BOM CSVs from `hardware/` to `website/public/bom/`, which the website renders. Run it from the repository root after editing any `bom.csv`.
It looks only in the three places listed below. Unless `--dry-run` is given, every run copies each BOM it finds there and overwrites its file in
`website/public/bom/`, whether the source changed or not, so edit BOMs only under `hardware/`:

```bash
python3 tools/sync_bom_data.py            # copy the BOMs listed below, overwriting the website copies
python3 tools/sync_bom_data.py --dry-run  # show what would be copied
python3 tools/sync_bom_data.py --verbose  # print every action
```

- `hardware/edge/bom.csv` → `website/public/bom/bom-main.csv`
- `hardware/cables/bom.csv` → `website/public/bom/bom-cables.csv`
- `hardware/sensors/<name>/bom.csv` → `website/public/bom/bom-<slug>.csv` (for example `bom-vibration-sensor.csv`)

## step_linter.py

### STEP File Linter and Normalizer

A comprehensive tool for cleaning up and standardizing STEP files in the hardware library.

### Features

- Fix FILE_NAME attributes to match actual filenames
- Remove duplicate ISO-10303-21 lines
- Normalize formatting and spacing for consistency (quoted strings are left unchanged)
- Set FILE_NAME to the actual filename (schema, timestamp and authors are kept)
- Support for both individual files and directories

### Usage

```bash
# Process all STEP files in the default 3D library
./step_linter.py

# Process a specific directory
./step_linter.py /path/to/step/files

# Process a single file
./step_linter.py /path/to/file.step

# Dry run to see what would be changed
./step_linter.py --dry-run

# Verbose output
./step_linter.py --verbose
```

### Example Output

```text
STEP Linter starting in normal mode
Target path: ../hardware/library/3d
Found 39 STEP files in ../hardware/library/3d
Changed ESP32-WROOM-32.step: fixed header with correct FILE_NAME, normalized formatting
Changed STM32H743IIT6.step: normalized formatting
...

PROCESSING COMPLETE
Files processed: 39
Files changed: 15
Errors: 0
All STEP files processed successfully
```

## Copyright and License

Copyright © 2026 Newmatik. All rights reserved. Licensed under the Apache License, Version 2.0. See `website/content/license.md` for details.
