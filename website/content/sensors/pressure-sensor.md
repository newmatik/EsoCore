# Pressure Sensor Module

## Overview

Multi-range pressure sensing for hydraulic and pneumatic systems with diagnostics for leak detection and
process control. The module accommodates multiple pressure transducer inputs across different ranges,
enabling simultaneous monitoring of high-pressure hydraulic circuits and low-pressure pneumatic lines.
On-board processing computes rate-of-change and statistical metrics for early anomaly detection.

## Features

- **Ranges: 0-1, 0-10, 0-100 bar** -- Three selectable full-scale ranges cover applications from
  low-pressure pneumatics and HVAC systems to high-pressure hydraulic presses and injection molding.
- **Differential, gauge, and absolute options** -- Supports all three measurement modes, allowing
  monitoring of pressure drops across filters, gauge pressure in tanks, or absolute pressure in vacuum
  systems.
- **Fast response for transients** -- High sampling rate captures rapid pressure spikes, water hammer
  events, and valve transition dynamics that slower sensors would miss.

## Key Specifications

| Parameter           | Value                     |
| ------------------- | ------------------------- |
| Pressure ranges     | 0-1 / 0-10 / 0-100 bar    |
| Accuracy            | +/-0.25% FS               |
| Overpress rating    | 2x full scale             |
| Response time       | < 1 ms                    |
| Sampling rate       | Up to 1 kSPS              |
| Media compatibility | Stainless steel diaphragm |
| Interface           | RS-485 (Modbus RTU)       |
| Supply voltage      | 5 V (from Edge backplane) |

## Applications

- **Leak rate and filter clogging** -- Detect gradual pressure loss indicative of system leaks or
  increasing differential pressure across filters that signals clogging and the need for replacement.
- **Pump and valve performance** -- Monitor discharge pressure profiles and valve response times to
  identify pump wear, cavitation onset, or sticking valves before they cause downtime.
- **Process pressure validation** -- Continuously verify that operating pressures remain within specified
  tolerances during manufacturing processes such as molding, extrusion, or autoclave curing.

## Edge Integration

Streams real-time pressure data, rate-of-change metrics, and alarm states to the Edge over RS-485 using
Modbus RTU. Sampling rates, digital filters, and alarm thresholds are configurable through the Edge web
UI. The module is auto-discovered on the sensor bus during startup.

## Related

- [BOM: pressure sensor](/docs/bom/pressure-sensor)
