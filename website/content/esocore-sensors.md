# Hardware: Sensors

This page is a quick overview of the EsoCore sensor ecosystem. Each sensor module has its own dedicated page with full specifications and setup
details.

---

## Sensor Modules

- [Air Quality Sensor](sensors/air-quality-sensor) — Environmental monitoring: temperature, humidity, CO₂, VOCs, particulate matter, pressure.
- [Vibration Sensor](sensors/vibration-sensor) — 3‑axis vibration, FFT/RMS metrics for rotating equipment health.
- [Acoustic Sensor](sensors/acoustic-sensor) — Audible + ultrasonic monitoring for leaks, tool wear, and QA.
- [Current Sensor](sensors/current-sensor) — Isolated Hall current sensing, power quality, load analysis.
- [Light Sensor](sensors/light-sensor) — Visible/UV/IR/color and spectral analysis for safety & process control.
- [Oil Quality Sensor](sensors/oil-quality-sensor) — Turbidity, viscosity, water content, dielectric condition.
- [Pressure Sensor](sensors/pressure-sensor) — Hydraulic/pneumatic pressure; leak and filter monitoring.
- [Temperature Spot Sensor](sensors/temperature-spot-sensor) — NTC/RTD/thermocouple/IR temperature.
- [Proximity Position Sensor](sensors/proximity-position-sensor) — Inductive/capacitive/encoder/laser distance.

---

## How They Connect

Sensor modules connect to the EsoCore Edge via the **RS-485 multi-drop bus** (powered by 12 V DC from the Edge device). They support auto-discovery
with runtime hot-plug and DIP-switch addressing with configurable thresholds in the Edge UI.

In addition to the sensor bus, the Edge device also has **on-board inputs** that do not require external sensor modules:

- **4 dual-mode analog inputs** (IEPE or DC per channel) for vibration accelerometers or general analog sensors (24-bit, 50 kSPS per channel)
- **6 safety digital inputs** for safety devices and controller signals (dual-channel optocoupled, 24 V, EN ISO 13849 Cat. 3)

The sensor bus and on-board inputs can be used simultaneously, giving integrators maximum flexibility.

For details on each sensor module, open the relevant sensor page above. For on-board input specifications, see
[Analog Input Specification](/docs/edge-analog-inputs) and [Hardware: Edge](/docs/esocore-edge).
