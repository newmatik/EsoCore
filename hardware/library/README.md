# Hardware Library

This folder holds the shared Altium Designer libraries for EsoCore hardware.

## Structure

- altium — Altium integrated libraries (`.IntLib`), one per part.
- 3d — 3D models referenced by footprints (.step/.stp, .wrl as needed).

## Guidelines

- Prefer STEP (.step/.stp) for mechanical accuracy; include WRL only for legacy rendering.
- Keep filenames stable to preserve Altium Designer links.
- Use relative paths in project library tables pointing to this folder.
- When adding a new footprint, include or link its 3D model here.
- Do not place project-specific footprints here; keep those within the project's own `altium` folder if not intended for reuse.

## Repository conventions

- Library path: `hardware/library`
- Subfolders: `hardware/library/altium`, `hardware/library/3d`
- Track empty folders via `.gitkeep` files.

## Altium Designer setup (per project)

1. File → Open → From Version Control (or File → New → PCB Library) to manage libraries.
2. Add library path to `hardware/library/altium` (or to specific `.IntLib` files).
3. File → New → PCB Library → Design → Layer Stack Manager to configure 3D models path to `hardware/library/3d`.
