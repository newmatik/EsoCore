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
# Build everything
make all

# Build specific components
make edge                   # Edge device
make vibration              # Vibration sensor
make acoustic               # Acoustic sensor
make current                # Current sensor
make air_quality            # Air quality sensor

# Flash devices
make flash_edge
make flash_vibration
```

## Project Structure

```text
firmware/
├── common/                # Shared components
│   ├── communication/     # WiFi, HTTP, RS-485, Modbus
│   ├── storage/           # Sensor interface, microSD
│   ├── safety/            # EN ISO 13849 safety I/O (dual-channel inputs + relay outputs)
│   ├── management/        # Power, configuration management
│   ├── intelligence/      # Event system, TinyML
│   └── ui/                # OLED display
├── stm32/
│   ├── edge/              # Edge device firmware
│   └── sensors/           # Sensor module firmware
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

### HTTP Endpoints

- `GET /api/status` - System status
- `POST /api/heartbeat` - Heartbeat
- `GET /api/sensors` - Sensor list
- `POST /api/telemetry` - Telemetry data
- `GET /api/config` - Configuration
- `POST /api/firmware/check` - OTA updates

### RS-485 Sensor Bus Protocol

- `DISCOVER` - Device discovery
- `HEARTBEAT` - Connection monitoring
- `DATA_REQUEST/RESPONSE` - Sensor data exchange
- `CONFIG_UPDATE` - Configuration updates

## Testing

```bash
make test              # Unit tests
make integration_test  # Integration tests
make benchmark         # Performance tests
```

## License

Apache License 2.0

## Support

- Issues: [GitHub Issues](https://github.com/your-org/esocore-firmware/issues)
- Wiki: [Documentation](https://github.com/your-org/esocore-firmware/wiki)

---

**EsoCore** - Industrial IoT with Edge Intelligence
