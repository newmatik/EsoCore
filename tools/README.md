# Tools

Utility scripts (e.g., sync, build, release)

## step_linter.py

**STEP File Linter and Normalizer**

A comprehensive tool for cleaning up and standardizing STEP files in the hardware library.

### Features

- ✅ Fix FILE_NAME attributes to match actual filenames
- ✅ Remove duplicate ISO-10303-21 lines
- ✅ Normalize formatting and spacing for consistency
- ✅ Standardize header format across all files
- ✅ Support for both individual files and directories

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

```
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
✅ All STEP files processed successfully
```

## Copyright and License

Copyright © 2025 Newmatik. All rights reserved.
Licensed under the Apache License, Version 2.0. See `website/docs/license.md` for details.
