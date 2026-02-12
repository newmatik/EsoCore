# Light Sensor Module

## Overview

The Light Sensor Module provides multi-spectrum light analysis for industrial safety and process control,
combining visible light, UV, IR, color and spectral sensing. It enables compliance monitoring, quality
inspection, and environmental assessment across a broad range of wavelengths from ultraviolet through
near-infrared.

## Features

- Visible light (lux, wide dynamic range)
- UV-A/UV-B monitoring
- IR temperature via non-contact sensor (optional)
- RGB color with IR blocking filter
- 10-channel spectral sensing (violet to NIR)

## Key Specifications

| Parameter              | Value                         |
|------------------------|-------------------------------|
| Lux range              | 0.01 - 120,000 lux           |
| UV-A range             | 280 - 400 nm                  |
| Spectral channels      | 10 (415 - 940 nm)            |
| Color output           | RGB + clear + IR              |
| IR temperature range   | -40 to +380 C (optional)     |
| Dynamic range          | 1:1,000,000                   |
| Interface              | RS-485 (Modbus RTU)           |
| Supply voltage         | 5 V (from Edge backplane)     |

## Applications

- Workplace lighting compliance and emergency lighting validation
- UV exposure safety monitoring
- Process control for light-sensitive manufacturing
- Color quality and batch uniformity checks
- Spectral analysis for material characterization

## Edge Integration

Connects to the Edge via RS-485. Auto-discovery registers capabilities; thresholds and sampling are
configurable in the web UI.

## Related

- [BOM: Light Sensor](/docs/bom/light-sensor)
