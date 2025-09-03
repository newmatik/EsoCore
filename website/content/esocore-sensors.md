# Hardware: Sensors

This page is a quick overview of the EsoCore sensor ecosystem. Each sensor module has its own dedicated page with full specifications and setup details.

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

- RS‑485 multi‑drop bus to the EsoCore Edge (12–24 V DC power)
- Auto‑discovery with runtime hot‑plug support
- DIP‑switch addressing and configurable thresholds in the Edge UI

For details, open the relevant sensor page above.
