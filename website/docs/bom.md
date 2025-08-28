# Bill of Materials (Preliminary)

This is an early, approximate BOM at 1k quantity based on requirements. Prices are indicative only and subject to change.

> ⚠️ **Warning:** These figures reflect material (BOM) cost only. Assembly costs are not included.

The EdgeSentinel system consists of a **main edge device** (installed in control cabinet) and separate **sensor modules** (mounted directly on machinery). This modular approach allows optimal placement while keeping sensitive electronics protected.

**Industrial Connector Strategy**: All sensor modules use **4-pin screw terminal blocks** with PG9 cable glands for maximum field serviceability and cost effectiveness. Standard 4-conductor shielded cable can be cut to length and terminated in the field. The main edge device features dedicated **terminal block arrays** for different I/O types, enabling clean professional installation with clear functional separation.

---

## Main Edge Device

Control cabinet mounted unit with processing, connectivity, and I/O. Houses the STM32H743 MCU, fieldbus interfaces, PoE support, and safety systems.

<div id="bom-main">Loading BOM...</div>

---

## Vibration Sensor Module

Mounted directly on motor/gearbox housing for mechanical health monitoring. Includes 3-axis accelerometer with ≥1-3kHz sampling capability.

<div id="bom-vibration">Loading BOM...</div>

---

## Acoustic Sensor Module  

Positioned to capture machine operational sounds and high-amplitude events. Wide-band microphone with ultrasonic capability up to 80kHz.

<div id="bom-acoustic">Loading BOM...</div>

---

## Current Sensor Module

Non-invasive clamp-on installation on motor power cables. Provides isolated current measurement for torque/load monitoring via Hall effect sensor.

<div id="bom-current">Loading BOM...</div>

---

## Temperature Sensor Module

High-temperature NTC sensor for direct stator temperature monitoring. Precision conditioning for accurate thermal management.

<div id="bom-temperature">Loading BOM...</div>

---

## Cables & Interconnects

Shielded cables, connectors, and installation hardware for connecting sensor modules to main edge device.

<div id="bom-cables">Loading BOM...</div>

---

<!-- BOM tables are rendered by the docs app after this markdown loads. -->
