#!/usr/bin/env python3
"""
STEP File Linter and Normalizer
===============================

A comprehensive tool for cleaning up and standardizing STEP files.

Features:
- Fix FILE_NAME attributes to match actual filenames
- Remove duplicate ISO-10303-21 lines
- Normalize formatting and spacing for consistency
- Standardize header format
- Support for configurable target directories

Usage:
    python step_linter.py [path/to/step/files] [--dry-run] [--verbose]

Arguments:
    path/to/step/files    Directory containing STEP files (default: ../hardware/library/3d)
    --dry-run            Show what would be changed without making changes
    --verbose           Enable verbose output
    --help              Show this help message

Author: EsoCore Development Team
"""

import argparse
import glob
import os
import re
import sys
from pathlib import Path


class StepLinter:
    """Main STEP file linter class"""

    def __init__(self, target_path=None, dry_run=False, verbose=False):
        self.target_path = Path(target_path) if target_path else self._find_default_target()
        self.dry_run = dry_run
        self.verbose = verbose
        self.stats = {
            'files_processed': 0,
            'files_changed': 0,
            'errors': 0
        }

        if not self.target_path.exists():
            raise ValueError(f"Target path does not exist: {self.target_path}")

    def _find_default_target(self):
        """Find the default STEP files directory"""
        # Try relative to script location
        script_dir = Path(__file__).parent
        candidate = script_dir.parent / "hardware" / "library" / "3d"
        if candidate.exists():
            return candidate

        # Try absolute path from project root
        candidate = Path(__file__).parent.parent / "hardware" / "library" / "3d"
        if candidate.exists():
            return candidate

        # Fallback to current directory
        return Path.cwd()

    def log(self, message, level="info"):
        """Log a message if verbose mode is enabled"""
        if self.verbose or level == "error":
            print(f"[{level.upper()}] {message}")

    def find_step_files(self):
        """Find all STEP files in target path"""
        if self.target_path.is_file():
            if self.target_path.suffix.lower() == '.step':
                self.log(f"Processing single file: {self.target_path}")
                return [str(self.target_path)]
            else:
                raise ValueError(f"File is not a STEP file: {self.target_path}")
        else:
            # It's a directory
            pattern = str(self.target_path / "*.step")
            step_files = glob.glob(pattern)
            self.log(f"Found {len(step_files)} STEP files in {self.target_path}")
            return sorted(step_files)

    def fix_duplicate_iso_lines(self, content):
        """Remove duplicate ISO-10303-21 lines"""
        lines = content.split('\n')
        if len(lines) >= 2 and lines[0].strip() == "ISO-10303-21;" and lines[1].strip() == "ISO-10303-21;":
            lines = lines[1:]
            return '\n'.join(lines), True
        return content, False

    def normalize_formatting(self, content):
        """Normalize STEP file formatting to use consistent spacing"""
        lines = content.split('\n')
        normalized_lines = []

        for line in lines:
            original_line = line

            # Skip empty lines and comments
            if not line.strip() or line.strip().startswith('//'):
                normalized_lines.append(line)
                continue

            # Pattern 1: Add space after entity ID and before =
            # #123=SOMETHING → #123 = SOMETHING
            line = re.sub(r'(#\d+)=', r'\1 = ', line)

            # Pattern 2: Add space after = and before function name
            # #123 = SOMETHING( → #123 = SOMETHING(
            line = re.sub(r'(#\d+ = )([A-Z_]+)\(', r'\1\2(', line)

            # Pattern 3: Handle commas with spaces (carefully)
            # Add space after comma when followed by #
            line = re.sub(r',(\s*)(#)', r', \2', line)

            # Add space after comma when followed by letters
            line = re.sub(r',(\s*)([A-Za-z][A-Za-z0-9_]*)', r', \2', line)

            # Add space after comma when followed by $
            line = re.sub(r',(\s*)(\$)', r', \2', line)

            # Add space after comma when followed by .TRUE. or .FALSE.
            line = re.sub(r',(\s*)(\.TRUE\.|\.FALSE\.|\.T\.|\.F\.)', r', \2', line)

            # Pattern 4: Add space before closing parenthesis when followed by semicolon
            line = re.sub(r'(\S)\);', r'\1) ;', line)

            # Clean up any double spaces
            line = re.sub(r'  +', ' ', line)

            normalized_lines.append(line)

        return '\n'.join(normalized_lines)

    def fix_header(self, content, filename):
        """Fix the header section with correct FILE_NAME and formatting"""
        # Standard header format (without ISO line - that's handled separately)
        standard_header = f"""HEADER;
FILE_DESCRIPTION(('3D model data'),'2;1');
FILE_NAME('{filename}','2024-01-01T00:00:00',(''),(''),'KiCad Library','Unspecified','');
FILE_SCHEMA(('AUTOMOTIVE_DESIGN'));
ENDSEC;"""

        # Find and replace header section
        header_match = re.search(r'HEADER;(.*?)ENDSEC;', content, re.DOTALL | re.IGNORECASE)
        if header_match:
            new_content = re.sub(r'HEADER;(.*?)ENDSEC;', standard_header, content, flags=re.DOTALL | re.IGNORECASE)
            return new_content, True

        return content, False

    def process_file(self, file_path):
        """Process a single STEP file"""
        filename = os.path.basename(file_path)

        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                original_content = f.read()

            content = original_content
            changes_made = []

            # 1. Fix duplicate ISO lines
            content, duplicate_fixed = self.fix_duplicate_iso_lines(content)
            if duplicate_fixed:
                changes_made.append("removed duplicate ISO-10303-21 lines")

            # 2. Fix header
            content, header_fixed = self.fix_header(content, filename)
            if header_fixed:
                changes_made.append("fixed header with correct FILE_NAME")

            # 3. Normalize formatting
            original_formatted = content
            content = self.normalize_formatting(content)
            if content != original_formatted:
                changes_made.append("normalized formatting")

            # Write changes if any were made
            if content != original_content and changes_made:
                if self.dry_run:
                    self.log(f"Would change {filename}: {', '.join(changes_made)}")
                else:
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.write(content)
                    self.log(f"Changed {filename}: {', '.join(changes_made)}")
                self.stats['files_changed'] += 1
            else:
                self.log(f"No changes needed for {filename}", "debug")

            self.stats['files_processed'] += 1

        except Exception as e:
            self.log(f"Error processing {filename}: {e}", "error")
            self.stats['errors'] += 1

    def run(self):
        """Main execution method"""
        self.log(f"STEP Linter starting in {'dry-run' if self.dry_run else 'normal'} mode")
        self.log(f"Target path: {self.target_path}")

        step_files = self.find_step_files()
        if not step_files:
            self.log("No STEP files found!", "error")
            return

        for step_file in step_files:
            self.process_file(step_file)

        # Print summary
        print(f"\n{'DRY RUN RESULTS' if self.dry_run else 'PROCESSING COMPLETE'}")
        print(f"Files processed: {self.stats['files_processed']}")
        print(f"Files changed: {self.stats['files_changed']}")
        print(f"Errors: {self.stats['errors']}")

        if self.stats['errors'] > 0:
            print(f"⚠️  {self.stats['errors']} files had errors")
        elif self.dry_run:
            print("ℹ️  Use without --dry-run to apply changes")
        else:
            print("✅ All STEP files processed successfully")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="STEP File Linter and Normalizer",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )

    parser.add_argument(
        'target_path',
        nargs='?',
        help='Path to STEP file or directory containing STEP files (default: auto-detect)'
    )

    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Show what would be changed without making changes'
    )

    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Enable verbose output'
    )

    args = parser.parse_args()

    try:
        linter = StepLinter(
            target_path=args.target_path,
            dry_run=args.dry_run,
            verbose=args.verbose
        )
        linter.run()

    except KeyboardInterrupt:
        print("\nOperation cancelled by user")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
