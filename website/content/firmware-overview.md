# Firmware

EsoCore firmware (v1.5.0) is written in C (C11) and targets two families of STM32 microcontrollers: the STM32H7
series for the Edge device and the STM32G0 series for sensor modules. The build system uses GNU Make with the ARM
GCC cross-compiler toolchain.

This page covers the firmware architecture, code structure, build system, and how to set up a development environment
for building and flashing firmware.

---

## Architecture

The firmware follows a bare-metal cooperative (superloop) architecture with a shared common library and
target-specific code for each device type. The HAL layer uses CMSIS device headers for register definitions with
thin, custom wrappers -- no dependency on ST's HAL library.

| Target | MCU | Core | Clock | Purpose |
| --- | --- | --- | --- | --- |
| **Edge** | STM32H747IIT6 | Cortex-M7 (+ M4) | 480 MHz | Central hub: dual Ethernet, analog acquisition, fieldbus, TinyML |
| **Sensors** | STM32G031xx | Cortex-M0+ | 64 MHz | Individual sensor modules connected via RS-485 bus |

The Edge device runs on the Cortex-M7 core of the dual-core STM32H747 at 480 MHz (PLL1 from 25 MHz HSE). It
manages:

- **Dual Ethernet** -- Port A (Machine/OT: PROFINET RT, Modbus TCP, static IP) and Port B (IT: HTTPS cloud sync,
  DHCP, OTA) via two KSZ8081RNACA PHYs
- **4-channel analog inputs** -- Dual-mode IEPE + DC with 24-bit simultaneous ADC (ADS1274) at 50 kSPS per channel,
  programmable gain (PGA280), and DMA double-buffered capture
- **6 dual-channel safety digital inputs** -- EN ISO 13849 Category 3 compliant with cross-monitoring
- **RS-485 sensor bus** -- Auto-discovery, hot-plug, 12V distributed power for sensor modules
- **Fieldbus interfaces** -- Modbus RTU, PROFIBUS DP, Interbus (each on dedicated isolated RS-485)
- **24V DC power management** -- Brownout detection, multi-rail monitoring, supercapacitor backup (20F, 3.8V)
- **Local storage** -- MicroSD with power-safe writes
- **Edge intelligence** -- TinyML inference for anomaly detection
- **Connectivity** -- ESP32-S3 WiFi/BT via UART AT commands

Each sensor module runs on a low-power STM32G0 MCU. Sensor firmware handles sampling, local signal conditioning, and
communication with the Edge over the RS-485 multi-drop bus.

---

## Code Structure

The firmware repository is organized under the `firmware/` directory:

```
firmware/
├── Makefile                    # Top-level build system
├── common/                     # Shared library code (used by both edge and sensors)
│   ├── analog/                 # Analog input subsystem and signal chain
│   ├── communication/          # Ethernet manager, sensor bus, Interbus, Modbus, HTTP, Wi-Fi
│   ├── storage/                # Sensor interface, SD card storage system
│   ├── safety/                 # Safety I/O handling (digital inputs + relay/SSR outputs)
│   ├── management/             # Power management (24V DC), configuration manager
│   ├── intelligence/           # Event system, TinyML inference engine
│   └── ui/                     # OLED display driver
├── stm32/
│   ├── edge/                   # Edge device application (main.c)
│   ├── sensors/                # Sensor module applications (main.c + per-sensor drivers)
│   ├── stm32h7/               # STM32H7 custom HAL, BSP, drivers, CMSIS, linker, startup
│   │   ├── bsp/               # Board support: pin mapping, system clock, board config
│   │   ├── cmsis/             # Minimal CMSIS device headers for STM32H747
│   │   ├── hal/               # Custom HAL: GPIO, SPI, UART, I2C, DMA, Timer, ETH, ADC
│   │   ├── drivers/           # Hardware drivers: KSZ8081, ADS1274, PGA280, TMUX1101, etc.
│   │   ├── linker/            # Linker script for Cortex-M7 (Flash + multi-region SRAM)
│   │   └── startup/           # Vector table and reset handler (assembly)
│   └── stm32g0/               # STM32G0 HAL, CMSIS, linker scripts, startup code
```

### Common Library

The `common/` directory contains platform-independent modules shared across firmware targets:

- **analog/**: 4-channel analog input manager (`analog_input`) with dual-mode IEPE/DC switching, programmable gain
  control, continuous DMA-driven acquisition from the ADS1274 ADC, and signal chain utilities for voltage conversion
  and noise floor calculation.
- **communication/**: Dual Ethernet manager (`ethernet_manager`) for isolated OT/IT networking, RS-485 sensor bus
  manager (`sensor_bus`) with auto-discovery and hot-plug, Interbus protocol layer (`interbus`), HTTP client for
  cloud sync, Wi-Fi management (ESP32-S3), Modbus RTU, and shared protocol layer.
- **storage/**: Sensor data interface abstraction and microSD storage system with wear leveling and power-safe writes.
- **safety/**: Safety I/O management for the 6 dual-channel optocoupled digital inputs (EN ISO 13849 Category 3)
  with cross-monitoring and periodic test pulses. Controls a safety relay (G7SA-2A2B) and solid-state relay
  (AQY212EHAZ) for outputs.
- **management/**: Power management for the 24V DC input system with multi-rail voltage monitoring via internal ADC,
  brownout state machine, supercapacitor charge estimation, power-up/down sequencing, and graceful shutdown.
  Runtime configuration management.
- **intelligence/**: Event system for structured event logging, and the TinyML inference engine for on-device anomaly
  detection.
- **ui/**: OLED display driver for local status display on the Edge device.

### STM32H7 HAL and Drivers

The custom HAL provides thin register-level wrappers using CMSIS device headers. No dependency on ST's HAL/LL
libraries -- every peripheral is configured via direct register writes for transparency and control.

**HAL modules** (`stm32h7/hal/`):

| Module | Description |
| --- | --- |
| `hal_gpio` | Pin configuration, atomic read/write/toggle, alternate function setup |
| `hal_spi` | Blocking and DMA transfers; double-buffer mode for continuous ADC capture |
| `hal_uart` | Blocking/interrupt TX/RX, RS-485 DE/RE control, RX callback |
| `hal_i2c` | Master read/write with register addressing, 100/400 kHz timing |
| `hal_eth` | Dual Ethernet MAC init, DMA descriptors, MDIO PHY management, frame TX/RX |
| `hal_dma` | Stream config with DMAMUX routing, circular and double-buffer modes |
| `hal_timer` | Periodic interrupt timers, microsecond delay via TIM2 |
| `hal_adc` | Internal ADC3 for power rail monitoring and die temperature readback |
| `hal_init` | Centralized GPIO initialization for all board pins from pin mapping |

**Hardware drivers** (`stm32h7/drivers/`):

| Driver | IC | Description |
| --- | --- | --- |
| `drv_ksz8081` | KSZ8081RNACA | 10/100 Ethernet PHY (x2) -- auto-negotiation, link status |
| `drv_ads1274` | ADS1274 | 24-bit 4-channel simultaneous ADC -- SPI + DMA continuous capture |
| `drv_pga280` | PGA280 | Programmable gain amplifier -- SPI register interface |
| `drv_tmux1101` | TMUX1101 | SPDT analog switch -- IEPE/DC mode selection per channel |
| `drv_iepe` | REF200AU gate | IEPE 4 mA current source enable/disable per channel |
| `drv_ssr` | AQY212EHAZ | Solid-state relay output control |

**BSP** (`stm32h7/bsp/`):

- `pin_mapping_v150.h` -- Complete GPIO pin assignment for all v1.5.0 peripherals
- `bsp_edge_v150.h` -- Board configuration: version, feature flags, power rail thresholds, defaults
- `system_clock.c` -- Full clock tree setup: 25 MHz HSE, PLL1 (480 MHz SYSCLK), PLL2 (50 MHz ETH), PLL3

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
`release/esocore_v1.5.0.tar.gz`.

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
