# Development Environment

This page documents the end-to-end development tooling used across hardware, firmware, mechanical, edge software, and cloud for EdgeSentinel, with rationale and minimum setup.

---

## PCB Design and Schematics

- **Primary toolchain**: Altium Designer (latest stable)
- **Why**: Enterprise-grade schematic/layout, strong library and variant management, impedance control, output automation (fabrication/assembly). Common with EMS partners.
- **Artifacts**:
  - Source: .PrjPcb, .SchDoc, .PcbDoc
  - Manufacturing: Gerbers, drill files, IPC-356 netlist, ODB++ (preferred), Pick-and-Place, BOM (CSV/XLSX)
- **Collaboration**: Altium 365 or exported ODB++/Gerbers stored in GitHub Releases for traceability.

## Enclosures and Mechanical

- **Primary toolchain**: Autodesk Fusion 360
- **Why**: Fast iterative design, CAM readiness, parametric modeling, good STEP interoperability.
- **Artifacts**:
  - Exchange: .step (AP214/AP242), .stl for 3D printing, drawings as .dxf/.pdf
  - 3D print profiles: PrusaSlicer/Cura project files when applicable
- **Conventions**: All mechanical interfaces (mounting holes, clearances, keep-outs) referenced to PCB origin and exported as STEP for ECAD-MCAD sync.

## Sensor Firmware (MCUs on Sensor Modules)

- **Reference MCU family**: STMicroelectronics STM32 (e.g., STM32G0/G4/F4/H7 as required)
- **IDE/tooling**:
  - Standard: STM32CubeIDE (required for official MCU projects)
  - Alternatives (optional): VS Code + Cortex-Debug + CMake with arm-none-eabi-gcc; CLion with embedded plugin
- **Utilities**:
  - STM32CubeMX for clock/peripheral init code generation
  - ST-LINK CLI (ST-LINK_CLI or STM32_Programmer_CLI) for flashing/debug
  - Serial console: screen/minicom
  - Live plotting: STM32CubeMonitor (modern replacement for legacy STMStudio)
- **Why**: STM32 ecosystem offers robust HAL, long-term availability, and wide community support. The GCC toolchain is open, stable, and reproducible.
- **Build**:
  - Compiler: gcc-arm-none-eabi (10+)
  - CMake presets for Debug/Release, -ffunction-sections -fdata-sections -Wl,--gc-sections
  - Unit tests on host where feasible (pure C modules)
- **Debug/flash**:
  - ST-LINK V2/V3 over SWD; OpenOCD or STM32CubeProgrammer
- **Artifacts**: .elf, .bin, .hex, optional .dfu

## Edge Device Software (Gateway)

- **OS target**: Zephyr RTOS on STM32 (Cortex-M4/M7)
- **Languages**: C/C++ for firmware and services; Python/Node.js optional for host-side tooling
- **Why**: Zephyr provides deterministic real-time scheduling, mature drivers, networking (when needed), secure OTA patterns, and an MCU-focused footprint with long-term support. Tooling is standardized around CMake and west for reproducible builds and CI.
- **Tooling**:
  - Build system: Zephyr (CMake + west); Ninja as default generator
  - Cross-compile: Zephyr SDK or arm-none-eabi-gcc toolchain; optional Docker for reproducibility
  - Artifacts: firmware images (.elf/.bin/.hex/.dfu) with versioned metadata
- **Debugging**: gdb via OpenOCD/J-Link, RTT/SWO logging, semihosting; on-target tracing where applicable

### Edge Device Hardware & OS prerequisites

- STM32-based main board (Cortex-M4/M7) with Ethernet (PoE optional), microSD, RS-485, RTC, and secure boot/OTA pathway.
- Tooling on dev host: Zephyr SDK, west, CMake/Ninja, OpenOCD or J-Link tools; Python runtime for scripting.

## Cloud / Frappe Framework

- **Stack**: Frappe Framework (Python) + Node.js asset pipeline
- **Editors**: VS Code or Cursor
- **Why**: Frappe provides rapid CRUD, RBAC, background jobs, and REST APIs with a strong admin UI, accelerating time-to-market.
- **Local dev requirements**:
  - Python 3.10–3.11, Node.js 18 LTS, Redis, MariaDB/PostgreSQL per Frappe version
  - bench CLI for managing sites/apps
- **Coding standards**:
  - Python: ruff + black; JS/TS: eslint + prettier
  - Type-annotate public APIs; avoid any in TS
- **Artifacts**: App packages, migration scripts, REST API specs (kept in docs/), CI pipelines

## Editors and Productivity

- **VS Code / Cursor**: Shared workspace settings and recommended extensions (C/C++, Python, Docker, Markdown, YAML, GitLens, Cortex-Debug).
- **GitHub**: All source is versioned on GitHub; issues and PRs are the canonical workflow. Use feature branches and conventional commits where possible.

## Data and Testing

- **Data formats**: See the [Data Format Specification](data-format-specification.md) page for CSV/JSON schemas.
- **Tests**:
  - Firmware: on-target tests with assertions; host-side unit tests for libraries
  - Cloud: pytest for Python apps; Playwright for UI if applicable
  - Hardware: bring-up checklists and boundary tests (see [Testing & Validation](testing-procedures.md))

## Minimum Setup Per Role

- **Hardware engineer**:
  - Altium Designer + PCB libraries
  - Fusion 360 for mechanical fit checks
  - GitHub access and LFS for large binaries if required
- **Firmware engineer**:
  - STM32CubeIDE (standard), arm-none-eabi-gcc, ST-LINK, CubeMX, STM32CubeProgrammer, STM32CubeMonitor
  - Serial console tools; OpenOCD (optional). VS Code toolchain optional.
- **Edge/platform engineer**:
  - Dev host (Linux/macOS/Windows) with Zephyr SDK and cross-compile toolchains
  - west, CMake/Ninja, OpenOCD or J-Link, Python 3.11; optional Docker
- **Cloud engineer**:
  - Python 3.11, Node.js 18, Frappe bench, Redis, MariaDB/Postgres; VS Code/Cursor

## Rationale Summary

- Altium Designer: industry-standard for complex mixed-signal boards and manufacturing outputs.
- Fusion 360: rapid iteration and broad vendor compatibility via STEP.
- STM32 + GCC/CMake: stable, cost-effective, open toolchain with excellent peripherals and community.
- Zephyr RTOS on MCU: deterministic real-time behavior, small footprint, maintained drivers/networking/filesystem, and established OTA strategies.
- Frappe (Python/Node.js): fast delivery of admin and API features with strong RBAC.
- GitHub: unified collaboration, reviews, issues, CI, and releases.

---

## Repositories and Version Control

- **Hosting**: GitHub organization repositories. Use branches per feature; PRs with mandatory reviews.
- **Binaries**: Release artifacts (firmware .bin, ECAD ODB++, STEP/STL) attached to GitHub Releases for traceability.
- **Versioning**: Firmware and software follow SemVer; hardware uses revision codes (A, B, C...).
