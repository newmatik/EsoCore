# EsoCore Firmware

## Industrial IoT Edge Computing Platform

**EsoCore** is a comprehensive firmware platform for industrial IoT applications, featuring real-time edge intelligence, robust communication protocols, and enterprise-grade reliability.

## 🏗️ Architecture

```
EsoCore Edge Device ─── RS-485 ─── Sensor Modules
     │                                        │
     ├── WiFi/HTTP API                       ├── Vibration Analysis
     ├── TinyML Engine                       ├── Acoustic Monitoring
     ├── Safety I/O                          ├── Current Analysis
     ├── Power Management                    ├── Air Quality
     ├── Storage System                      ├── Oil Quality
     └── Configuration Management            └── More...
```

## 🎯 Key Features

- **Edge Intelligence**: TinyML with anomaly detection and predictive maintenance
- **Industrial Communication**: RS-485, Modbus RTU, WiFi, HTTP/HTTPS
- **Safety Compliance**: EN ISO 13849 safety I/O with emergency stops
- **Power Resilience**: PoE + supercapacitor backup
- **Enterprise Storage**: microSD with compression and power-safe writes
- **Remote Management**: OTA updates, configuration sync, health monitoring

## 🚀 Quick Start

### Prerequisites
- ARM GCC toolchain (`arm-none-eabi-gcc`)
- GNU Make
- STM32F4 (Edge) and STM32G0 (Sensors) target support

### Build Commands
```bash
# Build everything
make all

# Build specific components
make edge                    # Edge device
make vibration              # Vibration sensor
make acoustic               # Acoustic sensor
make current                # Current sensor
make air_quality           # Air quality sensor

# Flash devices
make flash_edge
make flash_vibration
```

## 📁 Project Structure

```
firmware/
├── common/                 # Shared components
│   ├── communication/     # WiFi, HTTP, RS-485, Modbus
│   ├── storage/           # Sensor interface, microSD
│   ├── safety/            # EN ISO 13849 safety I/O
│   ├── management/        # Power, configuration management
│   ├── intelligence/      # Event system, TinyML
│   └── ui/                # OLED display
├── stm32/
│   ├── edge/              # Edge device firmware
│   └── sensors/           # Sensor module firmware
├── build/                 # Generated build artifacts
├── release/               # Release packages
└── Makefile              # Build system
```

## 🛠️ Supported Sensors

| Sensor Type | Key Capabilities | Use Cases |
|-------------|------------------|-----------|
| **Vibration** | 3-axis FFT, bearing fault detection | Predictive maintenance |
| **Acoustic** | SPL measurement, ultrasound detection | Noise monitoring, leak detection |
| **Current** | Harmonic analysis, motor load detection | Power quality, motor monitoring |
| **Air Quality** | IAQ calculation, health risk assessment | Environmental monitoring |
| **Oil Quality** | Turbidity, viscosity, water content | Lubrication monitoring |
| **Pressure** | Multi-range sensing, hydraulic monitoring | Process control |
| **Temperature** | NTC/RTD/Thermocouple, high precision | Thermal management |
| **Proximity** | Inductive/capacitive/laser sensing | Position detection |

## 🔧 Configuration

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

## 📊 System Monitoring

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

## 🔌 API Reference

### HTTP Endpoints
- `GET /api/status` - System status
- `POST /api/heartbeat` - Heartbeat
- `GET /api/sensors` - Sensor list
- `POST /api/telemetry` - Telemetry data
- `GET /api/config` - Configuration
- `POST /api/firmware/check` - OTA updates

### RS-485 Protocol
- `DISCOVER` - Device discovery
- `HEARTBEAT` - Connection monitoring
- `DATA_REQUEST/RESPONSE` - Sensor data exchange
- `CONFIG_UPDATE` - Configuration updates

## 🧪 Testing

```bash
make test              # Unit tests
make integration_test  # Integration tests
make benchmark         # Performance tests
```

## 📄 License

Apache License 2.0

## 🆘 Support

- Issues: [GitHub Issues](https://github.com/your-org/esocore-firmware/issues)
- Wiki: [Documentation](https://github.com/your-org/esocore-firmware/wiki)

---

**EsoCore** - Industrial IoT with Edge Intelligence 🚀