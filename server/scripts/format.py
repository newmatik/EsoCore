#!/usr/bin/env python3
"""
Format script for the Django project.

This script runs all formatting tools:
- Ruff (with auto-fix)
- Black (code formatter)
- isort (import sorter)
"""

import subprocess
import sys
from pathlib import Path


def run_command(cmd: list[str], description: str) -> bool:
    """Run a command and return True if successful."""
    print(f"🔧 {description}")
    try:
        result = subprocess.run(cmd, check=True, capture_output=False, text=True)
        print(f"✅ {description} completed")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ {description} failed:")
        print(e.stdout)
        print(e.stderr)
        return False


def main():
    """Main formatting function."""
    print("🚀 Starting formatting process...\n")

    # Get the project root directory
    project_root = Path(__file__).parent.parent

    # Change to project root
    import os

    os.chdir(project_root)

    success = True

    # Run Ruff with auto-fix
    if not run_command(["ruff", "check", ".", "--fix"], "Ruff auto-fix"):
        success = False

    # Run Black formatter
    if not run_command(["black", "."], "Black formatting"):
        success = False

    # Run isort
    if not run_command(["isort", "."], "Import sorting"):
        success = False

    if success:
        print("\n🎉 All formatting completed successfully!")
        return 0
    print("\n💥 Some formatting failed.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
