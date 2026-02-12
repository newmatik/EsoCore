# Firmware

EsoCore firmware is written in C (C11) and targets two families of STM32 microcontrollers: the STM32H7 series for the
Edge device and the STM32G0 series for sensor modules. The build system uses GNU Make with the ARM GCC cross-compiler
toolchain.

This page covers the firmware architecture, code structure, build system, and how to set up a development environment
for building and flashing firmware.

---

## Architecture

The firmware follows a bare-metal architecture with a shared common library and target-specific code for each device
type.

| Target | MCU | Core | Purpose |
| --- | --- | --- | --- |
| **Edge** | STM32H747IIT6 | Cortex-M7 (+ M4) | Central hub: sensor aggregation, storage, networking, TinyML |
| **Sensors** | STM32G031xx | Cortex-M0+ | Individual sensor modules connected via RS-485 bus |

The Edge device runs on the Cortex-M7 core of the dual-core STM32H747 and handles all high-level functions: sensor
data collection over RS-485, local storage to microSD, cloud synchronization over Ethernet or Wi-Fi, on-device TinyML
inference, safety I/O monitoring, and OTA firmware updates.

Each sensor module runs on a low-power STM32G0 MCU. Sensor firmware handles sampling, local signal conditioning, and
communication with the Edge over the RS-485 multi-drop bus using Modbus RTU.

---

## Code Structure

The firmware repository is organized under the `firmware/` directory:

```
firmware/
├── Makefile                    # Top-level build system
├── common/                     # Shared library code (used by both edge and sensors)
│   ├── communication/          # HTTP client, Wi-Fi manager, Modbus RTU, protocol
│   ├── storage/                # Sensor interface, SD card storage system
│   ├── safety/                 # Safety I/O handling
│   ├── management/             # Power management, configuration manager
│   ├── intelligence/           # Event system, TinyML inference engine
│   └── ui/                     # OLED display driver
├── stm32/
│   ├── edge/                   # Edge device application (main.c)
│   ├── sensors/                # Sensor module applications (main.c + per-sensor drivers)
│   ├── stm32h7/               # STM32H7 HAL, CMSIS, linker scripts, startup code
│   └── stm32g0/               # STM32G0 HAL, CMSIS, linker scripts, startup code
```

### Common Library

The `common/` directory contains platform-independent modules shared across all firmware targets:

- **communication/**: HTTP client for cloud sync, Wi-Fi management (ESP32-S3 co-processor), Modbus RTU for the RS-485
  sensor bus, and a shared protocol layer.
- **storage/**: Sensor data interface abstraction and microSD storage system with wear leveling and power-safe writes.
- **safety/**: Safety I/O management for the 6 dual-channel optocoupled safety inputs (EN ISO 13849 Category 3).
- **management/**: Power management (brownout detection, supercapacitor monitoring, safe shutdown) and runtime
  configuration.
- **intelligence/**: Event system for structured event logging, and the TinyML inference engine for on-device anomaly
  detection.
- **ui/**: OLED display driver for local status display on the Edge device.

### Sensor Drivers

Individual sensor types each have a dedicated source file under `stm32/sensors/`:

- `vibration_sensor.c` -- ADXL355 3-axis accelerometer with FFT processing
- `acoustic_sensor.c` -- Audible and ultrasonic microphone sampling
- `current_sensor.c` -- Motor current monitoring via hall-effect sensor
- `air_quality_sensor.c` -- Temperature, humidity, CO2, VOC, particulate matter
- `oil_quality_sensor.c` -- Viscosity, contamination, water content
- `pressure_sensor.c` -- Hydraulic/pneumatic pressure monitoring
- `temperature_spot_sensor.c` -- NTC, RTD, thermocouple, and IR sensor support
- `proximity_position_sensor.c` -- Inductive/capacitive proximity and position encoding
- `light_sensor.c` -- Visible, UV, IR, and spectral light measurement

---

## Toolchain

### Required Tools

| Tool | Version | Purpose |
| --- | --- | --- |
| `arm-none-eabi-gcc` | 10+ | Cross-compiler for ARM Cortex-M targets |
| `arm-none-eabi-objcopy` | (bundled) | Binary format conversion (ELF to HEX/BIN) |
| `arm-none-eabi-size` | (bundled) | Memory usage reporting |
| GNU Make | 4.0+ | Build system |
| OpenOCD or STM32CubeProgrammer | latest | Flashing and debugging |
| ST-LINK V2/V3 | -- | Debug probe hardware (SWD interface) |

### Installing the Toolchain

**macOS (Homebrew):**

```bash
brew install --cask gcc-arm-embedded
brew install make
```

**Ubuntu/Debian:**

```bash
sudo apt-get install gcc-arm-none-eabi make
```

**Windows:**

Download the [GNU Arm Embedded Toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain) installer
from ARM and add it to your PATH. Install Make via MSYS2 or use the version bundled with STM32CubeIDE.

Verify your installation:

```bash
arm-none-eabi-gcc --version
make --version
```

---

## Build System

The firmware uses a GNU Make build system defined in `firmware/Makefile`. All builds are invoked from the `firmware/`
directory.

### Build Targets

**Edge device:**

```bash
make edge                # Build Edge firmware (STM32H7, Cortex-M7)
```

**Individual sensor modules:**

```bash
make vibration           # Build vibration sensor firmware
make acoustic            # Build acoustic sensor firmware
make current             # Build current sensor firmware
make air_quality         # Build air quality sensor firmware
make oil_quality         # Build oil quality sensor firmware
make pressure            # Build pressure sensor firmware
make temperature         # Build temperature sensor firmware
make proximity           # Build proximity sensor firmware
```

**All targets:**

```bash
make all                 # Build edge + all sensor modules
make sensors             # Build all sensor modules only
```

**Other commands:**

```bash
make clean               # Remove all build artifacts
make info                # Print build system configuration
make help                # Show all available targets and options
```

### Build Options

| Option | Default | Description |
| --- | --- | --- |
| `DEBUG=1` | `0` | Enable debug build (`-O0`, debug symbols, `DEBUG` macro defined) |
| `OPTIMIZATION` | `-O2` | Compiler optimization level |

Example debug build:

```bash
make edge DEBUG=1
```

### Build Output

Build artifacts are placed in the `build/` directory:

```
build/
├── edge/
│   ├── esocore_edge.elf       # Linked ELF binary (for debugging)
│   ├── esocore_edge.hex       # Intel HEX (for flashing)
│   └── esocore_edge.bin       # Raw binary
└── sensors/
    ├── vibration/
    │   ├── esocore_vibration.elf
    │   ├── esocore_vibration.hex
    │   └── esocore_vibration.bin
    └── ...
```

### Compiler Configuration

The build uses the following key compiler flags:

- **Standard**: C11 (`-std=c11`)
- **Warnings**: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion`
- **Optimization**: `-ffunction-sections -fdata-sections` with `-Wl,--gc-sections` for dead code elimination
- **Specs**: `nano.specs` and `nosys.specs` for minimal C library (newlib-nano)

MCU-specific flags are applied per target:

- **Edge (Cortex-M7)**: `-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard`
- **Sensors (Cortex-M0+)**: `-mcpu=cortex-m0plus -mthumb`

---

## Flashing and Debugging

### Flashing with OpenOCD

Connect an ST-LINK probe to the target board via the SWD header, then:

```bash
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg \
  -c "program build/edge/esocore_edge.hex verify reset exit"
```

For sensor modules (STM32G0):

```bash
openocd -f interface/stlink.cfg -f target/stm32g0x.cfg \
  -c "program build/sensors/vibration/esocore_vibration.hex verify reset exit"
```

### Flashing with STM32CubeProgrammer

```bash
STM32_Programmer_CLI -c port=SWD -d build/edge/esocore_edge.hex -v -rst
```

### Debugging with GDB

Start OpenOCD as a GDB server, then connect:

```bash
# Terminal 1: start OpenOCD
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg

# Terminal 2: connect GDB
arm-none-eabi-gdb build/edge/esocore_edge.elf
(gdb) target remote :3333
(gdb) monitor reset halt
(gdb) load
(gdb) continue
```

VS Code users can use the Cortex-Debug extension for integrated debugging with breakpoints and variable inspection.

---

## Release Packaging

To create a release archive containing all firmware binaries:

```bash
make release
```

This builds all targets and packages the `.bin` and `.hex` files into
`release/esocore_v1.0.0.tar.gz`.

---

## Future Plans

The documentation for the Edge device references Zephyr RTOS as the long-term target OS for the edge firmware. The
current implementation is bare-metal C with direct HAL access. A migration to Zephyr would bring:

- Deterministic real-time scheduling with preemptive multitasking
- Mature networking stack (TCP/IP, TLS, MQTT)
- Built-in filesystem support (LittleFS, FAT)
- Standardized OTA update mechanisms
- `west` and CMake-based build system for reproducible builds

The sensor module firmware will remain bare-metal due to the constrained resources of the Cortex-M0+ MCUs.

---

## Related Documentation

- [Edge Hardware](/docs/esocore-edge) -- Edge device hardware specifications
- [Sensors](/docs/esocore-sensors) -- Sensor module hardware catalog
- [Edge Intelligence & TinyML](/docs/edge-intelligence) -- On-device AI and anomaly detection
- [Data Format Specification](/docs/data-format-specification) -- Local storage format and JSON schemas
- [Development Environment](/docs/development-environment) -- Full toolchain setup across all disciplines
