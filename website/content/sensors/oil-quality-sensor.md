# Oil Quality Sensor Module

## Overview

Monitors lubricant condition via turbidity, viscosity, water content, temperature, and dielectric constant
for predictive maintenance. The module provides continuous in-line analysis of oil health, replacing
periodic lab sampling with real-time data that enables condition-based oil change intervals. Early
detection of contamination or degradation helps prevent costly equipment damage from poor lubrication.

## Features

- **Turbidity and particle contamination** -- Optical turbidity measurement detects suspended particles,
  metal wear debris, and soot buildup that indicate component wear or external contamination.
- **Viscosity and temperature compensation** -- Inline viscosity measurement with automatic temperature
  compensation ensures accurate readings regardless of operating temperature variations.
- **Water content detection** -- Capacitive sensing detects dissolved and free water contamination that
  can reduce lubricant film strength and accelerate corrosion.

## Key Specifications

| Parameter              | Value                         |
|------------------------|-------------------------------|
| Viscosity range        | 1 - 1000 cSt                  |
| Viscosity accuracy     | +/-3% of reading               |
| Water detection        | 0 - 100% saturation            |
| Temperature range      | -20 to +120 C                  |
| Turbidity              | ISO 4406 cleanliness classes   |
| Dielectric constant    | 1.0 - 5.0                     |
| Interface              | RS-485 (Modbus RTU)            |
| Supply voltage         | 5 V (from Edge backplane)      |

## Applications

- **Gearbox and hydraulic oil monitoring** -- Continuously track oil condition in gearboxes, hydraulic
  power units, and lubrication systems to transition from time-based to condition-based oil changes.
- **Contamination and degradation trending** -- Log long-term trends in particle count, water content,
  and viscosity to identify gradual degradation patterns and correlate them with equipment duty cycles.
- **Bearing and pump lubrication assurance** -- Verify that lubricant quality remains within acceptable
  limits for critical bearings, pumps, and compressors where oil breakdown leads to rapid wear.

## Edge Integration

Provides oil health parameters, trend data, and alarm states to the Edge over RS-485 using Modbus RTU.
Alert thresholds for each measured parameter are individually configurable through the Edge web UI. The
module supports auto-discovery and registers its measurement capabilities on the sensor bus automatically.

## Related

- [BOM: oil quality sensor](/docs/bom/oil-quality-sensor)
