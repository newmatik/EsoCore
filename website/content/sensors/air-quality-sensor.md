# Air Quality Sensor Module

## Overview

Monitors temperature, humidity, CO2, VOCs, particulate matter (PM2.5/PM10), and barometric pressure for
HVAC optimization and workplace safety. The module combines multiple sensing elements into a single board
to provide a comprehensive indoor air quality assessment. Computed IAQ indices and trend data support
automated ventilation control and regulatory compliance reporting.

## Features

- **Temperature and humidity** -- Precision digital sensor measures ambient temperature and relative
  humidity for thermal comfort monitoring and dew point calculation.
- **CO2 (NDIR) and VOCs** -- Non-dispersive infrared CO2 sensor and metal-oxide VOC sensor detect
  elevated occupancy, poor ventilation, and the presence of volatile organic compounds.
- **PM2.5/PM10 particles** -- Laser scattering particle counter measures fine and coarse particulate
  concentrations relevant to respiratory health and clean room standards.
- **IAQ index and pressure** -- Computes a composite indoor air quality index and measures barometric
  pressure for altitude compensation and differential pressure monitoring.

## Key Specifications

| Parameter              | Value                         |
|------------------------|-------------------------------|
| Temperature range      | -10 to +60 C                  |
| Temperature accuracy   | +/-0.3 C                      |
| Humidity range         | 0 - 100% RH                   |
| CO2 range (NDIR)       | 400 - 5000 ppm                |
| CO2 accuracy           | +/-50 ppm + 3% of reading     |
| PM2.5 range            | 0 - 500 ug/m3                 |
| VOC output             | TVOC index (1 - 500)          |
| Interface              | RS-485 (Modbus RTU)           |
| Supply voltage         | 5 V (from Edge backplane)     |

## Applications

- **Demand-controlled ventilation** -- Use real-time CO2 and VOC levels to modulate HVAC airflow rates,
  reducing energy consumption while maintaining healthy indoor air quality.
- **Clean room compliance** -- Continuously monitor particulate counts against ISO 14644 cleanliness
  classes and generate alerts when thresholds are exceeded.
- **Filter maintenance via particulate trends** -- Track PM2.5/PM10 trends downstream of air handling
  units to detect filter degradation and schedule replacements before air quality deteriorates.

## Edge Integration

Communicates with the Edge over RS-485 using Modbus RTU. All measured parameters and the computed IAQ
index are available as individual registers. Sampling intervals, alert thresholds, and ventilation
control triggers are configurable through the Edge web UI. Auto-discovery registers the module and its
sensor capabilities on startup.

## Related

- [BOM: air quality sensor](/docs/bom/air-quality-sensor)
