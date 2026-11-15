# EsoCore Firmware

## Industrial IoT Edge Computing Platform

**EsoCore** is a comprehensive firmware platform for industrial IoT applications, featuring real-time edge intelligence, robust communication
protocols, and enterprise-grade reliability.

## Architecture

```text
                          Data Acquisition
                   ┌──────────┬────────────────────┐
                   │          │                    │
              RS-485 Bus   4x Analog          6x Safety DI
              (Sensor       (Dual-mode          (Dual-channel
               Modules)      IEPE/DC)            optocoupled)
                   │          │                    │
                   └──────┬───┴────────────────────┘
                          │
                   EsoCore Edge Device (STM32H747)
                          │
          ┌───────────────┼───────────────┐
          │               │               │
     Dual Ethernet    WiFi/HTTP      Fieldbus
     Port A  Port B    API           RS-485
     (OT)   (IT)                     Modbus RTU
                                     PROFIBUS
     Internal Services:              Interbus
     ├── TinyML Engine
     ├── Safety I/O (6 dual-channel inputs + relay + SSR)
     ├── Power Management (24V DC + supercap)
     ├── Storage System (microSD)
     └── Configuration Management
```

### Data Acquisition Paths

EsoCore supports three complementary data acquisition methods:

- **RS-485 Sensor Bus**: Distributed sensor modules with local MCUs connect over an industrial RS-485 multi-drop bus. Each module performs its own
  conditioning and preprocessing. Supports auto-discovery and hot-plug. Best for: diverse sensor types, long cable runs (up to 100 m), scalable
  multi-sensor deployments.
- **Dual-Mode Analog Inputs (4 channels)**: On-board signal chain with per-channel mode selection via TMUX1101 analog switch. In IEPE mode:
  constant-current source enabled, AC-coupled for vibration accelerometers. In DC mode: current source off, direct bypass for 0-10 V, 4-20 mA,
  or other analog sensors. Both modes share 24-bit simultaneous ADC (ADS1274) at 50 kSPS, PGA (PGA280), and 15 kHz anti-aliasing filter. Best
  for: high-fidelity vibration monitoring or general-purpose analog sensing directly wired to the Edge device.
- **Safety Digital Inputs (6 channels)**: Dual-channel optocoupled 24V inputs with cross-monitoring (EN ISO 13849 Cat. 3). Each channel uses
  two independent optocouplers; firmware verifies agreement between both channels. Response time <10 ms. Best for: safety devices (e-stops,
  light curtains, safety edges, door sensors) as well as general machine state monitoring, production counting, and PLC integration.

## Key Features

- **Edge Intelligence**: TinyML with anomaly detection and predictive maintenance
- **Industrial Communication**: Dual Ethernet, RS-485, Modbus RTU/TCP, PROFINET, PROFIBUS, Interbus, WiFi, HTTP/HTTPS
- **Data Acquisition**: RS-485 sensor bus + 4 dual-mode analog inputs (IEPE/DC, 24-bit) + 6 safety digital inputs (dual-channel)
- **Safety I/O**: 6 dual-channel inputs (EN ISO 13849 Cat. 3) + safety relay (G7SA-2A2B-DC24) + SSR (AQY212EHAZ)
- **Power Resilience**: 24V DC with aggressive surge/ESD protection + supercapacitor backup
- **Enterprise Storage**: microSD with compression and power-safe writes
- **Remote Management**: OTA updates, configuration sync, health monitoring

## Quick Start

### Prerequisites

- ARM GCC toolchain (`arm-none-eabi-gcc`)
- GNU Make
- STM32H7 (Edge) and STM32G0 (Sensors) target support

### Build Commands

```bash
# Build everything (edge + all sensor modules)
make all

# Build specific components
make edge-objects           # Edge device: compile every source file, no linking
make sensor-objects         # Sensor modules: compile every source file, no linking
make edge                   # Edge device
make vibration              # Vibration sensor
make acoustic               # Acoustic sensor
make current                # Current sensor
make air_quality            # Air quality sensor
make oil_quality            # Oil quality sensor
make pressure               # Pressure sensor
make temperature            # Temperature sensor
make proximity              # Proximity sensor
make light                  # Light sensor

# Flash devices
make flash_edge
make flash_vibration
make flash_acoustic
make flash_current
make flash_air_quality

# List all available targets
make help
```

> **Build status:** `make edge-objects` compiles every source file of the Edge firmware and `make sensor-objects`
> every source file of the sensor modules (`stm32/sensors/`) with the ARM toolchain, and `make test` runs the host
> unit tests described below. No target links yet: `make edge` stops because the STM32H7
> startup file is not in the repository, and the sensor targets need the STM32G0 HAL, CMSIS, startup and linker files
> (`stm32/stm32g0/`), which are missing too. Issue [#128](https://github.com/newmatik/EsoCore/issues/128) tracks this.

## Project Structure

```text
firmware/
├── common/                # Shared components
│   ├── analog/            # Analog inputs and signal chain
│   ├── communication/     # WiFi, HTTP, RS-485, Modbus
│   ├── sensors/           # Sensor interface
│   ├── storage/           # microSD
│   ├── safety/            # EN ISO 13849 safety I/O (dual-channel inputs + relay outputs)
│   ├── management/        # Power, configuration management
│   ├── intelligence/      # Event system, TinyML
│   └── ui/                # OLED display
├── stm32/
│   ├── edge/              # Edge device firmware
│   └── sensors/           # Sensor module firmware
├── tests/host/            # Unit tests that run on the development machine (`make test`)
├── build/                 # Generated build artifacts
├── release/               # Release packages
└── Makefile               # Build system
```

## RS-485 Sensor Bus Modules

External sensor modules connect via the RS-485 multi-drop bus. Each module has its own MCU and performs local conditioning:

| Sensor Type     | Key Capabilities                          | Use Cases                        |
| --------------- | ----------------------------------------- | -------------------------------- |
| **Vibration**   | 3-axis FFT, bearing fault detection       | Predictive maintenance           |
| **Acoustic**    | SPL measurement, ultrasound detection     | Noise monitoring, leak detection |
| **Current**     | Harmonic analysis, motor load detection   | Power quality, motor monitoring  |
| **Air Quality** | IAQ calculation, health risk assessment   | Environmental monitoring         |
| **Oil Quality** | Turbidity, viscosity, water content       | Lubrication monitoring           |
| **Pressure**    | Multi-range sensing, hydraulic monitoring | Process control                  |
| **Temperature** | NTC/RTD/Thermocouple, high precision      | Thermal management               |
| **Proximity**   | Inductive/capacitive/laser sensing        | Position detection               |
| **Light**       | Ambient/UV/IR sensing, lux measurement    | Light level monitoring           |

## On-Board Inputs

In addition to the sensor bus, the Edge device has direct on-board inputs:

| Input Type           | Channels | Specification                                      | Use Cases                                             |
| -------------------- | -------- | -------------------------------------------------- | ----------------------------------------------------- |
| **Analog (IEPE/DC)** | 4        | 24-bit ADC, 50 kSPS, PGA, 15 kHz AAF, TMUX1101     | IEPE accelerometers, 0-10V, 4-20mA sensors            |
| **Safety Digital**   | 6        | Dual-channel optocoupled, 24V, EN ISO 13849 Cat. 3 | E-stops, safety edges, light curtains, machine states |

## Configuration

### Edge Device

```c
#define WIFI_SSID           "EsoCore_Network"
#define SERVER_URL          "https://api.esocore.com"
#define HEARTBEAT_INTERVAL_MS    30000
#define OTA_CHECK_INTERVAL_MS    3600000
```

### Sensor Modules

```c
#define SENSOR_DEVICE_ADDRESS   0x02
#define SENSOR_DEVICE_TYPE      ESOCORE_DEVICE_TYPE_VIBRATION
#define DATA_TRANSMIT_INTERVAL_MS 2000
```

## System Monitoring

### Health Monitoring

```c
esocore_config_status_t status;
esocore_config_get_status(&status);
uint8_t health = status.system_health; // 0-100
```

### Event Logging

```c
esocore_event_log(ESOCORE_EVENT_SENSOR_DATA_READY,
                 ESOCORE_EVENT_SEVERITY_INFO,
                 (uint8_t *)"Data processed", 0);
```

### Performance Metrics

- CPU usage monitoring
- Memory utilization tracking
- Network latency measurement
- Sensor data throughput
- Storage compression ratios
- Power consumption monitoring

## API Reference

### Server Endpoints (IoT API)

The Edge device communicates with the EsoCore server at `/api/iot/v1/`:

- `POST /api/iot/v1/auth/handshake` - Device authentication via API key
- `POST /api/iot/v1/telemetry/batch` - Submit telemetry data batches
- `GET  /api/iot/v1/config` - Fetch device configuration
- `GET  /api/iot/v1/ota/check` - Check for firmware updates
- `POST /api/iot/v1/ota/report` - Report OTA update result

### RS-485 Sensor Bus Protocol

The Edge (`common/communication/sensor_bus.c`) and the sensor modules (`common/communication/protocol.c`) use one frame
layout, defined in `protocol.h` and built and parsed only by `common/communication/protocol_frame.c`. Multi-byte fields
are little endian.

| Offset | Size | Field                                                       |
| ------ | ---- | ----------------------------------------------------------- |
| 0      | 1    | Start byte `0xAA`                                           |
| 1      | 1    | Source address (`0x00` is the Edge)                         |
| 2      | 1    | Destination address (`0xFF` broadcast)                      |
| 3      | 1    | Message type (`esocore_message_type_t`)                     |
| 4      | 1    | Sequence number                                             |
| 5      | 1    | Flags (`ESOCORE_FLAG_*`)                                    |
| 6      | 2    | Payload length n, 0 to 256                                  |
| 8      | n    | Payload                                                     |
| 8 + n  | 2    | CRC-16/MODBUS (as Modbus RTU) over bytes 0 to 7 + n         |

A receiver drops bytes until it sees a start byte. It skips a start byte whose length is above 256 or whose frame has a
wrong CRC (the sensor modules answer a wrong CRC with `NACK`) and searches on from the next byte, so a damaged length
cannot swallow the following frame. A partial frame that gets no byte for 50 ms (`ESOCORE_PROTOCOL_FRAME_GAP_TIMEOUT_MS`)
is abandoned the same way. There is no end delimiter and no byte stuffing.

Both ends measure time with a millisecond clock the application passes in: `sensor_bus_config_t.get_time_ms` (required)
and `esocore_protocol_set_time_source()`. The Edge passes `system_clock_get_tick` (SysTick) to both. The Edge sends a
discovery request on the first `sensor_bus_poll()` after `sensor_bus_start()` and then every `discovery_interval_ms`,
and reports a module offline when no heartbeat, data or discovery response came from it for three heartbeat intervals
(`ESOCORE_PROTOCOL_HEARTBEAT_INTERVAL_MS`, 15 s, which the modules also send at: 45 s). A partial frame is abandoned
when no byte is waiting and the last one arrived (time stamped in the receive interrupt) more than 50 ms ago, so a late
poll does not drop a frame whose rest is already buffered. The sensor modules have no
tick in the repository yet (the STM32G0 support files are missing), so they set no clock and
`esocore_protocol_receive_message()` reads once and returns instead of waiting for its timeout.

| Type   | Message             | Payload                                                                                        |
| ------ | ------------------- | ---------------------------------------------------------------------------------------------- |
| `0x01` | `DISCOVER`          | none; Edge to broadcast                                                                        |
| `0x02` | `DISCOVER_RESPONSE` | device type, firmware major and minor, hardware version, serial (2), capabilities, status, uptime in s (4) |
| `0x03` | `HEARTBEAT`         | device type, status, 2 reserved bytes                                                          |
| `0x11` | `DATA_RESPONSE`     | sensor data; longer data in consecutive frames flagged `FRAGMENTED`, the last also `LAST_FRAGMENT` |
| `0x13` | `DATA_ACK`          | sequence number of the acknowledged frame, `0x01`                                              |
| `0x31` | `NACK`              | sequence number of the rejected frame, `0x00`                                                  |
| `0x27` | `COMMAND`           | `esocore_command_t` byte, then its data; Edge to one module                                    |

The other message types of `esocore_message_type_t` (configuration, control, firmware update) use the same frame. The
Edge's data callback gets each frame with its sequence number and flags; putting fragments back together is up to the
application. The sensor data payload is the module's reading in the fixed layout of the next section, cut into fragments
when it is longer than 256 bytes.

### Sensor Data Payload

A module never sends its data struct as it is in memory (padding, byte order and the size of `bool` and enums depend on
the compiler). `common/sensors/sensor_codec.c` and one codec file per module (`stm32/sensors/<module>_codec.c`) write
and read this layout; `<module>_sensor_encode_data()` and `<module>_sensor_decode_data()` are the entry points.

| Offset | Size | Field                                                                    |
| ------ | ---- | ------------------------------------------------------------------------ |
| 0      | 1    | Device type (`esocore_device_type_t`), checked by the decoder            |
| 1      | 1    | Layout version, `ESOCORE_SENSOR_PAYLOAD_VERSION` (1), checked            |
| 2      |      | The fields of the module's data struct, in the order its header declares |

Fields follow each other without padding. Integers are little endian (two's complement when signed): `uint8_t`,
`int8_t`, `bool` (0 or 1; any other byte is refused) and enums take 1 byte, 16-bit integers 2 and 32-bit integers 4.
`float` is IEEE 754 binary32, little endian. `char[n]` takes n bytes: the text, then zeros; the decoder always
terminates it. Arrays are their elements in index order, nested structs their fields. The decoder accepts only the exact
length. Any change of a layout raises the layout version.

`base_data` (`esocore_sensor_data_t`, 276 bytes) is: timestamp (4), sensor type (1), sensor id (1), data points (2),
data type (1), units (8), quality flags (1), data size (2), data (256).

Vibration (`vibration_sensor_data_t`, 1501 bytes):

| Offset | Size | Section                                                                   |
| ------ | ---- | ------------------------------------------------------------------------- |
| 2      | 276  | `base_data`                                                               |
| 278    | 18   | `raw_data`: x, y, z raw (`int32_t` each), timestamp, temperature (`int16_t`) |
| 296    | 52   | `processed_data`: 12 `float` (acceleration, velocity, displacement per axis, RMS, peak, crest factor), timestamp |
| 348    | 1046 | `fft_data`: 256 frequency bins, dominant frequency (`uint16_t`), 5 `float` |
| 1394   | 24   | `bearing_analysis`                                                        |
| 1418   | 18   | `gear_analysis`                                                           |
| 1436   | 1    | `overall_condition`                                                       |
| 1437   | 64   | `condition_description`                                                   |

Current (`current_sensor_data_t`, 2689 bytes):

| Offset | Size | Section                                                                   |
| ------ | ---- | ------------------------------------------------------------------------- |
| 2      | 276  | `base_data`                                                               |
| 278    | 2057 | `raw_data`: 1024 samples (`int16_t`), sample count, timestamp, temperature (`int8_t`) |
| 2335   | 32   | `processed_data`: 7 `float`, timestamp                                    |
| 2367   | 205  | `harmonic_data`: magnitudes, phases, percentages (16 `float` each), significant harmonics, 3 `float` |
| 2572   | 32   | `power_data`: 8 `float`                                                   |
| 2604   | 20   | `motor_data`                                                              |
| 2624   | 1    | `current_condition`                                                       |
| 2625   | 64   | `condition_description`                                                   |

Acoustic (`acoustic_sensor_data_t`, 6602 bytes):

| Offset | Size | Section                                                                   |
| ------ | ---- | ------------------------------------------------------------------------- |
| 2      | 276  | `base_data`                                                               |
| 278    | 4105 | `raw_data`: 2048 samples (`int16_t`), sample count, timestamp, temperature (`int8_t`) |
| 4383   | 32   | `processed_data`: 6 `float`, measurement duration, timestamp              |
| 4415   | 2074 | `fft_data`: 512 frequency bins, dominant frequency (`uint16_t`), 6 `float` |
| 6489   | 36   | `frequency_bands`: 9 octave bands (`float`), 31.5 Hz to 8 kHz             |
| 6525   | 12   | `ultrasound_data`                                                         |
| 6537   | 1    | `acoustic_condition`                                                      |
| 6538   | 64   | `condition_description`                                                   |

Air quality (`air_quality_sensor_data_t`, 985 bytes):

| Offset | Size | Section                                                                   |
| ------ | ---- | ------------------------------------------------------------------------- |
| 2      | 276  | `base_data`                                                               |
| 278    | 192  | `gas_sensors`: 8 entries of 24 bytes                                      |
| 470    | 15   | `pm_data`                                                                 |
| 485    | 20   | `env_data`: 5 `float`                                                     |
| 505    | 23   | `iaq_data`                                                                |
| 528    | 392  | `health_data`: 4 risk bytes, risk description (128), recommendations (256), timestamp |
| 920    | 1    | `air_quality_score`                                                       |
| 921    | 64   | `quality_description`                                                     |

Light (`light_sensor_data_t`, 519 bytes; no `base_data`):

| Offset | Size | Section                                                                   |
| ------ | ---- | ------------------------------------------------------------------------- |
| 2      | 16   | `visible_data`                                                            |
| 18     | 12   | `uv_data`                                                                 |
| 30     | 16   | `color_data`                                                              |
| 46     | 41   | `spectral_data`: 10 channels (`float`), flicker frequency                 |
| 87     | 8    | `ir_data`                                                                 |
| 95     | 152  | `workplace_compliance`: three pairs of flag and status text (48), overall flag, timestamp |
| 247    | 120  | `environmental_analysis`                                                  |
| 367    | 1    | `light_quality_score`                                                     |
| 368    | 64   | `condition_description`                                                   |
| 432    | 13   | TSL2591: illuminance, channel 0 and 1, infrared ratio, gain               |
| 445    | 13   | VEML6075: UVA, UVB, UV index, gain                                        |
| 458    | 12   | MLX90614: object and ambient temperature, emissivity                      |
| 470    | 16   | TCS34725: red, green, blue, clear (`uint16_t`), color temperature, lux    |
| 486    | 28   | AS7341: 10 channels (`uint16_t`), dominant wavelength, peak intensity     |
| 514    | 5    | `sensor_status`, `timestamp`                                              |

`tests/host/test_sensor_codec.c` checks these offsets and sizes.

### Modbus RTU

`common/communication/modbus_rtu.c` implements the protocol layer of Modbus RTU; it has no UART driver yet.

In slave mode it serves these function codes from its data map (coils and holding registers optionally through callbacks):

| Code | Function                 | Limit per request |
| ---- | ------------------------ | ----------------- |
| 0x01 | Read Coils               | 2000 coils        |
| 0x02 | Read Discrete Inputs     | 2000 inputs       |
| 0x03 | Read Holding Registers   | 125 registers     |
| 0x04 | Read Input Registers     | 125 registers     |
| 0x05 | Write Single Coil        |                   |
| 0x06 | Write Single Register    |                   |
| 0x0F | Write Multiple Coils     | 1968 coils        |
| 0x10 | Write Multiple Registers | 123 registers     |

Errors are answered with exception responses: 01 for any other function code, 03 for a bad quantity, value or request length, 02 for an address
outside the data map, 04 when a callback fails. Broadcast requests (address 0) are executed and not answered. The application sets discrete inputs
and input registers with `modbus_set_discrete_input()` and `modbus_set_input_register()`.

RTU framing follows the timing of the serial line specification. The UART driver passes every received character to `modbus_rtu_receive_byte()`
with the microsecond time at which it was completely received, and calls `modbus_rtu_poll()` when the T3.5 timer expires (or periodically):

- A silence of more than T1.5 (1.5 character times) inside a frame discards the whole frame.
- A silence of at least T3.5 (3.5 character times) ends the frame; it is then checked (length, CRC, address) and answered. `modbus_rtu_poll()`
  closes the frame one character time after T3.5, when it is certain that no further character had begun. A frame that is complete when the next
  one begins is still executed, but no longer answered: its response would collide with the request that has already started.
- After start-up or a change of the communication parameters the receiver accepts frames only once it has seen T3.5 of silence, so that the tail
  of a frame it joined midway is never taken for a frame.
- Up to 19200 Bd both times follow from the character format (9600 Bd 8E1: 1719 µs and 4011 µs); above 19200 Bd they are fixed at 750 µs and
  1750 µs. `modbus_rtu_get_frame_timing()` returns them.

This layer sees a character only when it is complete. Before it transmits a response, the UART driver must check that the line is idle, because a
master that gave up waiting may just have begun its next request.

Master mode builds the requests for the same eight function codes, but `modbus_send_request()` is still a stub without a UART, so no master
request completes yet.

## Host Unit Tests

```bash
make test
```

Builds the tests in `tests/host/` with the host C compiler (`HOST_CC`, default `cc`), with the address and undefined-behaviour sanitizers, and
runs them. No ARM toolchain or target hardware is needed. They cover the Modbus RTU slave: every function code with the request and response
examples of the Modbus specification, the exception responses, broadcast, and the T1.5/T3.5 receiver; and the sensor bus frame codec
(`test_protocol.c`): the byte layout, frames encoded by one side and decoded by the other, partial, corrupt and back-to-back frames, and the
discovery payload. `test_protocol_timing.c` checks that `esocore_protocol_receive_message()` returns after its timeout, also across the
32-bit clock wrap, and without a clock after one read. `test_sensor_bus.c` runs the Edge's `sensor_bus.c` against a fake UART, GPIO and clock:
`last_seen_ms`, the heartbeat timeout (also across the clock wrap), the discovery schedule and the partial-frame gap timeout.
`test_sensor_codec.c` checks the sensor reading payloads: the field encodings, every module's round trip (encode, decode, encode again gives
the same bytes), the documented sizes and section offsets, and refused payloads (wrong length, device type, layout version or bool byte). The
Firmware CI workflow runs the same command on every pull request.

## Hardware Abstraction Layer (HAL) Status

The firmware uses a layered architecture where higher-level modules call into HAL functions that
interface with hardware peripherals. HAL functions are implemented as stubs (returning `true` or
`false` without real hardware interaction) until the corresponding hardware driver is integrated.
This is a standard pattern for embedded projects under active development.

Stub functions are marked with `/* TODO: ... */` comments. Current module completion status:

| Module                                    | Implemented | Stubs | Complete |
| ----------------------------------------- | ----------- | ----- | -------- |
| `common/communication/ethernet_manager.c` | 9           | 0     | 100%     |
| `common/safety/safety_io.c`               | 25          | 3     | 89%      |
| `common/ui/oled_display.c`                | 42          | 8     | 84%      |
| `common/storage/storage_system.c`         | 18          | 12    | 60%      |
| `common/intelligence/tinyml_engine.c`     | 20          | 15    | 57%      |
| `common/sensors/sensor_interface.c`       | 20          | 15    | 57%      |
| `common/communication/wifi_manager.c`     | 12          | 13    | 48%      |

To find all HAL stubs, search for `TODO` comments in the `common/` directory.

## License

Apache License 2.0

## Support

- Issues: [GitHub Issues](https://github.com/newmatik/esocore/issues)
- Docs: [esocore.com](https://www.esocore.com)

---

**EsoCore** - Industrial IoT with Edge Intelligence
