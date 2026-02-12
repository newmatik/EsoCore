# Vibration Sensor Module

## Overview

Provides 3-axis vibration analysis with FFT processing and RMS metrics for rotating equipment health
monitoring and predictive maintenance. The module captures high-resolution acceleration data across three
orthogonal axes, enabling detection of faults such as imbalance, misalignment, looseness, and bearing wear.
On-board digital signal processing reduces data bandwidth while preserving the diagnostic information
needed for condition-based maintenance strategies.

## Features

- **High-precision 3-axis accelerometer** -- Industrial-grade MEMS sensor with low noise density for
  accurate vibration capture across a wide frequency range.
- **FFT spectra, RMS, crest factor, kurtosis** -- Multiple statistical and spectral metrics computed
  on-device, providing a comprehensive vibration signature for fault classification.
- **1-3 kHz ODR windows for diagnostics** -- Selectable output data rate windows allow targeted analysis
  of specific frequency bands relevant to bearings, gears, or structural resonances.
- **On-device DSP to reduce bandwidth** -- Local processing extracts key features before transmission,
  minimizing RS-485 bus traffic and Edge CPU load.

## Key Specifications

| Parameter              | Value                       |
|------------------------|-----------------------------|
| Axes                   | 3 (X, Y, Z)                |
| Measurement range      | +/-16 g (selectable)        |
| Frequency range        | 1 Hz - 3 kHz               |
| FFT resolution         | Up to 2048 bins             |
| Noise density          | < 80 ug/sqrt(Hz)           |
| Output data rate       | 1 kHz / 3 kHz selectable    |
| Interface              | RS-485 (Modbus RTU)         |
| Supply voltage         | 5 V (from Edge backplane)   |

## Applications

- **Motor bearing monitoring** -- Detect early-stage bearing defects such as inner race, outer race,
  and rolling element faults through envelope analysis and characteristic frequency tracking.
- **Gear mesh and imbalance detection** -- Identify gear tooth wear, misalignment, and rotor imbalance
  by monitoring gear mesh frequencies and 1x/2x running speed harmonics.
- **Pump cavitation and structural resonance** -- Recognize cavitation signatures in pumps and detect
  structural resonance conditions that may lead to fatigue failures.

## Edge Integration

Streams processed metrics and raw spectra to the Edge over RS-485 using Modbus RTU. Sampling windows,
FFT resolution, and alarm thresholds are fully configurable through the Edge web UI. The module supports
auto-discovery on the sensor bus, enabling plug-and-play deployment without manual address configuration.

## Related

- [BOM: vibration sensor](/docs/bom/vibration-sensor)
