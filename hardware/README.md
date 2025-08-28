# Hardware Structure

- `core/`
  - `bom.csv` – Canonical BOM for the Core
  - `pcb/pcb-specification.md` – PCB requirements and layout guidance
- `sensors/`
  - `vibration/`
    - `bom.csv`
    - `pcb/pcb-specification.md`
  - `acoustic/`
    - `bom.csv`
    - `pcb/pcb-specification.md`
  - `current/`
    - `bom.csv`
    - `pcb/pcb-specification.md`
  - `temperature/`
    - `bom.csv`
    - `pcb/pcb-specification.md`
- `altium/` – PCB project files
- `mechanical/` – STEP, drawings, enclosure references

Notes:
- Canonical BOMs live here; copy updates to `website/docs/data/` for publishing.
- Use Git LFS for binary design files (`altium/`, `mechanical/`).
