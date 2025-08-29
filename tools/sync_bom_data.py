#!/usr/bin/env python3
"""
Sync BOM CSV files from the hardware directory into the website/docs/data directory.

Mapping rules:
 - hardware/edge/bom.csv            -> website/docs/data/bom-main.csv
 - hardware/cables/bom.csv          -> website/docs/data/bom-cables.csv
 - hardware/sensors/<name>/bom.csv  -> website/docs/data/bom-<slug(name)>.csv

Where slug(name) lowercases the sensor folder name and replaces non-alphanumerics
with hyphens, collapsing repeats (e.g., "vibration sensor" -> "vibration-sensor").

Run manually as needed. Use --dry-run to preview changes and --verbose for details.
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
from pathlib import Path
from typing import List, Tuple


def slugify(name: str) -> str:
    """Convert a folder name to a URL/file-system friendly slug."""
    slug = re.sub(r"[^a-zA-Z0-9]+", "-", name.strip().lower())
    slug = re.sub(r"-+", "-", slug)
    return slug.strip("-")


def copy_file(src: Path, dest: Path, *, dry_run: bool, verbose: bool) -> Tuple[bool, str]:
    """Copy a single file with directories created as needed.

    Returns (changed, message).
    """
    if not src.exists():
        return False, f"SKIP: Source does not exist: {src}"

    action = "COPY" if not dest.exists() else "OVERWRITE"
    if verbose:
        print(f"{action}: {src} -> {dest}")

    if dry_run:
        return False, f"DRY-RUN {action}: {src} -> {dest}"

    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(src, dest)
    return True, f"{action}: {src.name} -> {dest.relative_to(dest.parent.parent.parent)}"


def sync_main_and_cables(hardware_dir: Path, data_dir: Path, *, dry_run: bool, verbose: bool) -> List[str]:
    messages: List[str] = []
    mapping = [
        (hardware_dir / "edge" / "bom.csv", data_dir / "bom-main.csv"),
        (hardware_dir / "cables" / "bom.csv", data_dir / "bom-cables.csv"),
    ]
    for src, dest in mapping:
        changed, msg = copy_file(src, dest, dry_run=dry_run, verbose=verbose)
        messages.append(msg)
    return messages


def sync_sensors(hardware_dir: Path, data_dir: Path, *, dry_run: bool, verbose: bool) -> List[str]:
    messages: List[str] = []
    sensors_dir = hardware_dir / "sensors"
    if not sensors_dir.exists():
        return [f"SKIP: Sensors directory not found: {sensors_dir}"]

    for sensor_dir in sorted(p for p in sensors_dir.iterdir() if p.is_dir()):
        src = sensor_dir / "bom.csv"
        sensor_slug = slugify(sensor_dir.name)
        dest = data_dir / f"bom-{sensor_slug}.csv"
        changed, msg = copy_file(src, dest, dry_run=dry_run, verbose=verbose)
        messages.append(msg)
    return messages


def parse_args(argv: List[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Sync hardware BOM CSVs to website/docs/data")
    parser.add_argument(
        "--hardware-dir",
        type=Path,
        help="Path to hardware directory (default: <repo>/hardware)",
    )
    parser.add_argument(
        "--data-dir",
        type=Path,
        help="Path to website/docs/data directory (default: <repo>/website/docs/data)",
    )
    parser.add_argument(
        "--only",
        choices=["all", "main", "cables", "sensors"],
        default="all",
        help="Restrict sync scope (default: all)",
    )
    parser.add_argument("--dry-run", action="store_true", help="Do not write files; show actions")
    parser.add_argument("--verbose", action="store_true", help="Print detailed actions")
    return parser.parse_args(argv)


def find_repo_root() -> Path:
    # Assume this script is located at <repo>/tools/sync_bom_data.py
    return Path(__file__).resolve().parent.parent


def main(argv: List[str]) -> int:
    args = parse_args(argv)
    repo_root = find_repo_root()

    hardware_dir = args.hardware_dir or (repo_root / "hardware")
    data_dir = args.data_dir or (repo_root / "website" / "docs" / "data")

    if args.verbose:
        print(f"Repo root: {repo_root}")
        print(f"Hardware dir: {hardware_dir}")
        print(f"Data dir: {data_dir}")

    messages: List[str] = []

    if args.only in ("all", "main"):
        messages.extend(sync_main_and_cables(hardware_dir, data_dir, dry_run=args.dry_run, verbose=args.verbose))

    if args.only in ("all", "sensors"):
        messages.extend(sync_sensors(hardware_dir, data_dir, dry_run=args.dry_run, verbose=args.verbose))

    if args.only == "cables" and args.only != "all":
        # If explicitly only cables, run just that mapping
        messages.extend(
            [
                copy_file(
                    hardware_dir / "cables" / "bom.csv",
                    data_dir / "bom-cables.csv",
                    dry_run=args.dry_run,
                    verbose=args.verbose,
                )[1]
            ]
        )

    # Print a concise summary
    if not messages:
        print("No actions performed.")
    else:
        for msg in messages:
            print(msg)

    if args.dry_run:
        print("\nDry-run complete. No files were modified.")

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))


