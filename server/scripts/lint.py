#!/usr/bin/env python3
"""
Lint script for the Django project.

This script runs all linting tools:
- Ruff (fast Python linter)
- Black (code formatter, check mode)
- isort (import sorter, check mode)
"""

import subprocess
import sys
from pathlib import Path


def run_command(cmd: list[str], description: str) -> bool:
    """Run a command and return True if successful."""
    print(f"🔍 {description}")
    try:
        subprocess.run(cmd, check=True, capture_output=True, text=True)
        print(f"✅ {description} passed")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ {description} failed:")
        print(e.stdout)
        print(e.stderr)
        return False


def main():
    """Main linting function."""
    print("🚀 Starting linting process...\n")

    # Get the project root directory
    project_root = Path(__file__).parent.parent

    # Change to project root
    import os

    os.chdir(project_root)

    success = True

    # Run Ruff linter
    if not run_command(["ruff", "check", "."], "Ruff linting"):
        success = False

    # Run Black in check mode
    if not run_command(["black", "--check", "."], "Black formatting check"):
        success = False

    # Run isort in check mode
    if not run_command(["isort", "--check-only", "."], "Import sorting check"):
        success = False

    if success:
        print("\n🎉 All linting checks passed!")
        return 0
    print(
        "\n💥 Some linting checks failed. Run 'poetry run format' to auto-fix issues."
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
