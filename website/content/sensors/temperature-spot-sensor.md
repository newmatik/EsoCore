# Temperature Spot Sensor Module

## Overview

High-precision temperature measurement using NTCs, RTDs, thermocouples, and IR for industrial monitoring.
The module supports up to eight contact-based channels and one non-contact infrared channel, covering a
wide temperature range from cryogenic to high-heat applications. Each channel is individually configurable
for sensor type, sampling rate, and alarm thresholds.

## Features

- **4x NTC, 2x PT100, 2x Type-K** -- Eight contact measurement channels spanning diverse temperature
  ranges, from precision low-temperature NTC monitoring to high-temperature thermocouple applications.
- **16-bit ADC conversion** -- High-resolution analog-to-digital conversion ensures accurate readings
  with minimal quantization error across the full measurement range.
- **IR non-contact measurement** -- Infrared pyrometer channel enables temperature monitoring of moving
  parts, electrically live components, or surfaces in hazardous locations.

## Key Specifications

| Parameter              | Value                         |
|------------------------|-------------------------------|
| NTC channels           | 4 (range: -40 to +125 C)     |
| PT100 channels         | 2 (range: -200 to +850 C)    |
| Type-K channels        | 2 (range: -200 to +1250 C)   |
| IR channel             | 1 (range: -40 to +380 C)     |
| ADC resolution         | 16-bit                        |
| Accuracy (PT100)       | +/-0.1 C                      |
| Accuracy (Type-K)      | +/-1.5 C                      |
| Interface              | RS-485 (Modbus RTU)           |
| Supply voltage         | 5 V (from Edge backplane)     |

## Applications

- **Motor bearings and gearboxes** -- Continuously monitor bearing and gearbox housing temperatures to
  detect lubrication breakdown, overloading, or incipient bearing failure before damage occurs.
- **Ovens, furnaces, and welding** -- Track process temperatures in thermal treatment, curing, and
  welding operations where precise temperature control directly affects product quality.
- **Moving parts and hazardous areas (IR)** -- Measure surface temperatures on rotating shafts, conveyor
  belts, and electrically live busbars without physical contact using the infrared channel.

## Edge Integration

Publishes all temperature channels over RS-485 using Modbus RTU with computed gradients, rate-of-change
metrics, and configurable alarm thresholds. Each channel can be individually named and assigned alert
levels through the Edge web UI. Auto-discovery registers the module and its channel configuration
automatically.

## Related

- [BOM: temperature spot sensor](/docs/bom/temperature-spot-sensor)
