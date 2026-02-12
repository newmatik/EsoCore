# Acoustic Sensor Module

## Overview

Combines audible and ultrasonic sensing for leak detection, tool wear monitoring, and quality control.
The module pairs a digital MEMS microphone covering the audible range with an ultrasonic transducer for
high-frequency emission detection, enabling identification of phenomena invisible to standard audio
analysis. Integrated DSP performs real-time spectral analysis and pattern recognition directly on the
sensor board.

## Features

- **Digital MEMS microphone (audible)** -- Captures sound from 20 Hz to 20 kHz with a high signal-to-noise
  ratio, suitable for detecting mechanical noise anomalies in industrial equipment.
- **Ultrasonic transducer (ultrasound)** -- Extends sensing to 20-100 kHz for detecting compressed air
  leaks, electrical discharge, and high-frequency friction signatures.
- **Real-time DSP and pattern recognition** -- On-board processing computes FFT spectra, band energy
  levels, and learned acoustic fingerprints, reducing raw data volume before transmission.

## Key Specifications

| Parameter              | Value                         |
|------------------------|-------------------------------|
| Audible range          | 20 Hz - 20 kHz                |
| Ultrasonic range       | 20 kHz - 100 kHz              |
| MEMS SNR               | > 65 dB(A)                   |
| ADC resolution         | 24-bit                        |
| FFT window             | Up to 1024 bins               |
| Pattern memory         | 8 stored fingerprints         |
| Interface              | RS-485 (Modbus RTU)           |
| Supply voltage         | 5 V (from Edge backplane)     |

## Applications

- **Pneumatic leak detection** -- Identify compressed air and gas leaks by detecting the characteristic
  ultrasonic emissions produced by turbulent flow through small orifices.
- **Tool wear and lubrication assessment** -- Monitor changes in cutting tool acoustic emissions to
  predict tool end-of-life and detect insufficient lubrication in machining processes.
- **Gear mesh and bearing diagnostics** -- Capture audible noise signatures associated with gear tooth
  wear, bearing defects, and mechanical looseness for early fault detection.

## Edge Integration

Provides spectral data and diagnostic parameters to the Edge over RS-485 using Modbus RTU. Frequency
bands, sensitivity thresholds, and pattern-matching profiles are configurable through the Edge web UI.
The module is auto-discovered on the sensor bus during startup.

## Related

- [BOM: acoustic sensor](/docs/bom/acoustic-sensor)
