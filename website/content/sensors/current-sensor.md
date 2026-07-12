# Current Sensor Module

## Overview

Isolated Hall-effect current sensing with motor load analysis, harmonic content monitoring, and power
quality metrics. The module provides galvanically isolated measurement of AC and DC currents, making it
safe for retrofit installation on live circuits. On-board computation delivers RMS values, total harmonic
distortion, and individual harmonic magnitudes without requiring external analysis software.

## Features

- **+/-30 A range (others optional)** -- Default measurement range covers most industrial motor and drive
  applications, with alternative ranges available for higher or lower current circuits.
- **RMS current, THD, harmonics** -- Computes true RMS current, total harmonic distortion up to the 15th
  harmonic, and individual harmonic magnitudes for power quality assessment.
- **Load factor and torque estimation** -- Derives motor load percentage and estimated shaft torque from
  the current waveform, supporting efficiency monitoring without mechanical sensors.

## Key Specifications

| Parameter         | Value                     |
| ----------------- | ------------------------- |
| Default range     | +/-30 A AC/DC             |
| Accuracy          | +/-1% of reading          |
| Isolation voltage | 2.5 kV RMS                |
| Bandwidth         | DC - 1 kHz                |
| Harmonic analysis | Up to 15th harmonic       |
| Sampling rate     | 10 kSPS                   |
| Interface         | RS-485 (Modbus RTU)       |
| Supply voltage    | 5 V (from Edge backplane) |

## Applications

- **Motor load and efficiency** -- Track motor loading over time to identify underloaded or overloaded
  conditions, optimize energy consumption, and schedule maintenance based on actual duty cycles.
- **Pump and fan performance** -- Correlate current draw with flow or pressure measurements to detect
  degraded pump impellers, clogged filters, or slipping fan belts.
- **Electrical fault diagnostics** -- Detect phase imbalance, ground faults, and winding degradation
  through harmonic signature analysis and current waveform anomaly detection.

## Edge Integration

Streams RMS current, power metrics, and harmonic data to the Edge over RS-485 using Modbus RTU. Alarm
thresholds for overcurrent, undercurrent, and harmonic distortion are configurable through the Edge web
UI. The module supports auto-discovery on the sensor bus.

## Related

- [BOM: current sensor](/docs/bom/current-sensor)
