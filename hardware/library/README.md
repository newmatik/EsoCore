# Hardware Library

This folder holds the shared KiCad libraries for EdgeSentinel hardware.

## Structure

- footprints — Common footprint libraries (.pretty directories with .kicad_mod files).
- 3d — 3D models referenced by footprints (.step/.stp, .wrl as needed).

## Guidelines

- Prefer STEP (.step/.stp) for mechanical accuracy; include WRL only for legacy rendering.
- Keep filenames stable to preserve KiCad links.
- Use relative paths in project library tables pointing to this folder.
- When adding a new footprint, include or link its 3D model here.
- Do not place project-specific footprints here; keep those within the project’s own `kicad` folder if not intended for reuse.

## Repository conventions

- Library path: `hardware/library`
- Subfolders: `hardware/library/footprints`, `hardware/library/3d`
- Track empty folders via `.gitkeep` files.

## KiCad setup (per project)

1. Preferences → Manage Footprint Libraries → Project Specific Libraries.
2. Add library path to `hardware/library/footprints` (or to specific `.pretty` directories).
3. Preferences → Configure Paths or Manage 3D Models → add `hardware/library/3d`.
