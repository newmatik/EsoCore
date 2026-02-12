# Proximity Position Sensor Module

## Overview

Multi-sensor positioning with inductive, capacitive, encoder, magnetic reed, and laser ranging for
industrial automation. The module combines multiple sensing technologies on a single board, enabling
detection of metallic and non-metallic targets, precise shaft rotation tracking, and accurate distance
measurement. This versatility supports a wide variety of position-sensing applications without requiring
separate sensor modules.

## Features

- **2x inductive (12 mm), capacitive input** -- Dual inductive proximity sensors detect ferrous and
  non-ferrous metals at distances up to 12 mm, while the capacitive input senses non-metallic materials
  such as plastics, liquids, and granulates.
- **1000 PPR quadrature encoder** -- High-resolution quadrature encoder input provides precise angular
  position and velocity measurement for rotating shafts, spindles, and feed drives.
- **Laser distance up to ~2 m** -- Time-of-flight laser ranging measures linear distances with
  millimeter-level resolution, suitable for level sensing, positioning, and dimensional verification.

## Key Specifications

| Parameter              | Value                         |
|------------------------|-------------------------------|
| Inductive range        | 0 - 12 mm (2 channels)        |
| Capacitive input       | 1 channel, configurable       |
| Encoder resolution     | 1000 PPR (4000 counts/rev)    |
| Encoder max speed      | 6000 RPM                      |
| Laser range            | 50 mm - 2000 mm               |
| Laser accuracy         | +/-3 mm                       |
| Reed/digital inputs    | 2 channels                    |
| Interface              | RS-485 (Modbus RTU)           |
| Supply voltage         | 5 V (from Edge backplane)     |

## Applications

- **Cycle counting and conveyor monitoring** -- Use inductive or reed inputs to count production cycles,
  detect item passage on conveyors, and measure conveyor belt speed for throughput tracking.
- **Robotic arm position validation** -- Verify end-effector and joint positions using encoder feedback
  and laser ranging to ensure accurate and repeatable robotic movements.
- **Door and gate position sensing** -- Monitor open/closed states and travel positions of industrial
  doors, safety gates, and sliding mechanisms using inductive or reed switch inputs.

## Edge Integration

Transmits position data, counter values, and discrete states to the Edge over RS-485 using Modbus RTU.
Thresholds, debounce settings, and IO mapping are configurable through the Edge web UI. The module is
auto-discovered on the sensor bus and registers its channel configuration automatically.

## Related

- [BOM: proximity position sensor](/docs/bom/proximity-position-sensor)
