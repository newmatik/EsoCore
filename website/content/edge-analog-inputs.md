# Analog Input Specification (Dual-Mode Design)

## Core Design Philosophy

The 4 analog input channels support two operating modes per channel, selectable via firmware GPIO:

- **IEPE Mode**: For IEPE/ICP accelerometers (2-wire constant-current powered). This is the industry standard for CNC machine vibration monitoring
  and covers 95%+ of industrial accelerometers. The constant-current source is enabled and the signal is AC-coupled to strip the DC bias.
- **DC Mode**: For general-purpose analog sensors such as 4-20 mA transmitters (with shunt resistor), 0-10 V voltage outputs, thermocouples (with
  external conditioning), or any DC/low-frequency sensor. The constant-current source is disabled and the signal bypasses the AC coupling capacitor,
  preserving the full DC content.

Both modes share the same high-performance signal chain (PGA, AAF, 24-bit ADC) and can be configured independently per channel.

## Per-Channel Circuit Architecture (4 channels)

```
┌──────────────────────────────────────────────────────────────────────────┐
│ Dual-Mode Analog Input Channel (x4)                                      │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│                          ┌─── IEPE path: [Current Source] → [AC Cap] ─┐  │
│ [Sensor] → [Protection] →│                                            ├→ [Buffer] → [PGA] → [AAF] → [ADC]
│                          └─── DC path:   [Direct bypass] ─────────────┘  │
│                                     ^                                    │
│                               [TMUX1101]                                 │
│                            (GPIO-controlled)                             │
└──────────────────────────────────────────────────────────────────────────┘
```

Mode selection per channel (2 GPIOs):
- **IEPE Mode**: current source ON (Si2301CDS), analog switch routes through AC coupling cap
- **DC Mode**: current source OFF, analog switch routes through direct bypass to buffer

## Detailed Specifications

### 1. Input Protection & Interface

```
Component: Input terminal (10-pin pluggable screw terminal, Degson 2EDGR-5.0-10P)
│
├─── ESD Protection
│    - Part: PESD5V0L1BA (Nexperia), TVS diode (+/-30V clamp, fast response)
│    - Protects against cable static discharge
│
├─── Series Resistor
│    - Part: ERJ-3EKF1000V (Panasonic), 100 ohm 0.25W
│    - Purpose: Limits fault current if sensor shorts or cable is damaged
│
└─── Parallel Capacitor
     - Value: 100 nF ceramic (X7R), 50V
     - Purpose: HF noise bypass to ground
```

### 2. IEPE Constant-Current Source

Design: Precision current source (per channel)

```
┌─────────────────────────────────────┐
│ Current Source Specification        │
├─────────────────────────────────────┤
│ Output Current:  4 mA ± 0.2 mA      │
│ Compliance:      2-20 mA capable    │
│ Supply Voltage:  24V (from PSU)     │
│ Output Voltage:  0-24V              │
│ Enable Control:  GPIO (per channel) │
│ Short Circuit:   Auto shutdown      │
│ Temperature:     ±50 ppm/°C         │
└─────────────────────────────────────┘
```

Implementation: Precision Current Source IC
- IC: REF200AU (TI) -- dual 100 uA sources, scaled with external resistor
- Configuration: 40x multiplier to produce 4 mA output
- Enable: P-channel MOSFET switch (Si2301CDS) per channel, controlled via GPIO
- Rationale: simple, robust, proven design with excellent temperature stability

### 3. Signal Path and Mode Switching

The signal path after the current source stage splits into two routes selected by an analog switch.
A TMUX1101 (TI) SPDT analog switch per channel selects between the IEPE path and the DC bypass path.

```
Mode Switch (TMUX1101, one per channel):
│
│    IEPE Path (switch position A):
│    ├─── DC Blocking Capacitor
│    │    - Part: ECW-F2106JL (Panasonic), 10 uF polypropylene film, 50V
│    │    - Purpose: Block 8-12V DC bias from IEPE sensor, pass AC vibration signal
│    │    - High-pass corner: fc = 1/(2pi x 10uF x 100kohm) = 0.16 Hz
│    │    - Ensures full bandwidth from 0.5 Hz to 10 kHz
│    │
│    └─── Input Resistor to Ground
│         - Part: ERA-3AEB1003V (Panasonic), 100kohm 1% metal film
│         - Establishes AC ground reference; creates high-pass filter with coupling cap
│
│    DC Path (switch position B):
│    └─── Direct connection from protection stage to buffer
│         - No DC blocking: full DC content preserved for voltage/current sensors
│         - Input impedance set by 100kohm ground resistor (shared)
│
└─── Buffer Op-Amp (common to both paths)
     - IC: OPA4188AIDR (TI, quad, low noise: 3.3 nV/rtHz)
     - Configuration: Unity-gain follower
     - Purpose: High impedance input, drives PGA and AAF stages
```

TMUX1101 Analog Switch Specification:
- Part: TMUX1101DCR (TI), 4 units (one per channel)
- Type: SPDT (single-pole double-throw)
- On-resistance: 1 ohm typical
- Bandwidth: 300 MHz (does not limit 10 kHz signals)
- Charge injection: 0.2 pC (negligible for 24-bit ADC)
- THD: -120 dB (well below ADC noise floor)
- Supply: 3.3 V from analog LDO
- Control: 1 GPIO per channel from STM32H747
- Package: SOT-23-6

DC Mode Use Cases:
- **4-20 mA transmitters**: Add a 250 ohm shunt resistor at the connector to convert to 1-5 V (within PGA input range)
- **0-10 V voltage sensors**: Direct connection, PGA at gain 1x
- **Thermocouples**: With external cold-junction compensation module
- **Potentiometric sensors**: Direct voltage divider output
- **Any DC or low-frequency analog signal** within the +/-10 V PGA input range


### 4. Programmable Gain Amplifier (PGA)

```
┌────────────────────────────────────────────────┐
│ Why PGA? Different sensors, different gains:   │
│ - Low-cost sensor: 100 mV/g                    │
│ - Precision sensor: Could be 10-500 mV/g       │
│ PGA adapts to sensor sensitivity               │
└────────────────────────────────────────────────┘
```

PGA Specification:
- IC: PGA280AIDR (TI), 4 units (one per channel)
- Gain settings: 1, 2, 5, 10, 20 (binary control via SPI/GPIO)
- Control: SPI or GPIO from STM32H747
- Bandwidth: >1 MHz (does not limit 10 kHz signals)
- Noise: <10 nV/rtHz
- Input range: +/-10 V (handles worst case)

Gain Selection Strategy:

```
┌────────────────┬──────────────┬────────────────┐
│ Sensor Type    │ Sensitivity  │ PGA Gain       │
├────────────────┼──────────────┼────────────────┤
│ Low-cost       │ 100 mV/g     │ 10× (±5V in)   │
│ Precision      │ 500 mV/g     │ 2× (±5V in)    │
│ High-sens      │ 1000 mV/g    │ 1× (±10V in)   │
│ Low-sens       │ 10 mV/g      │ 20× (±2.5V in) │
└────────────────┴──────────────┴────────────────┘
```

Firmware auto-calibration:
1. Apply known test signal (or idle baseline)
2. Measure ADC output at each gain
3. Select optimal gain for 70% ADC full-scale
4. Store in EEPROM per sensor

### 5. Anti-Aliasing Filter (AAF)

Purpose: Prevent aliasing from >25 kHz noise

Design: 4-pole Butterworth active filter
- Topology: Sallen-Key (two cascaded 2-pole sections per channel)
- Cutoff: 15 kHz (-3 dB)
- Rolloff: 80 dB/decade (24 dB/octave)
- Passband ripple: <0.1 dB
- Group delay: <50 us at 10 kHz

Why 15 kHz cutoff?
- Sensor bandwidth: 0.5 Hz -- 10 kHz
- Nyquist: need >20 kHz sampling
- AAF at 15 kHz: passes full 10 kHz signal
- Attenuates >25 kHz noise before ADC

Implementation:
- Op-amp: OPA4188AIDR (TI, quad, shared across buffer and AAF stages; 2 units total for 4 channels)
- Passive components: 1% metal film resistors (ERA-3AEB), C0G/NP0 capacitors (GRM15)
- Stages: two 2-pole Sallen-Key sections cascaded per channel

### 6. Analog-to-Digital Converter (ADC)

```
┌─────────────────────────────────────────────────┐
│ ADC Requirements for Vibration Monitoring       │
├─────────────────────────────────────────────────┤
│ Resolution:      24-bit (143 dB dynamic range)  │
│ Sample Rate:     50 kSPS per channel minimum    │
│ Channels:        4 simultaneous                 │
│ Input Range:     ±10V (after PGA)               │
│ Noise Floor:     <10 µVrms                      │
│ Latency:         <100 µs (for real-time FFT)    │
└─────────────────────────────────────────────────┘
```

Selected IC: ADS1274IPAPR (Texas Instruments)
- Architecture: 4-channel simultaneous delta-sigma
- Resolution: 24-bit
- Sample Rate: up to 128 kSPS per channel (operated at 50 kSPS)
- Input: differential, +/-10 V with PGA
- Interface: SPI (up to 50 MHz clock)
- Package: HTQFP-32
- Power: 330 mW (low for 24-bit)
- Features:
  - Simultaneous sampling (critical for phase information)
  - Built-in digital filter
  - Low group delay variation
  - Industrial temp range (-40 C to +105 C)

Connection to STM32H747:
- SPI3 or SPI4 (up to 50 MHz)
- DMA for zero-copy transfers
- Double buffering: Fill one buffer while processing other
- Interrupt on data ready (DRDY pin)

## Complete Signal Chain Summary

### IEPE Mode (vibration accelerometers)

```
┌───────────────────────────────────────────────────────────────────┐
│ IEPE Sensor → 24V/4mA → DC: 8-12V + AC: +/-5V peak                │
│              Current              (vibration signal)              │
│              Source (ON)                                          │
│                     ↓                                             │
│              TMUX1101 → IEPE path (position A)                    │
│                     ↓                                             │
│              10uF AC Coupling → AC signal only: +/-5V peak        │
│                     ↓                                             │
│              OPA4188 Buffer → PGA (1x-20x) → 15 kHz AAF           │
│                     ↓                                             │
│              24-bit ADC @ 50 kSPS → Digital samples               │
│                     ↓                                             │
│              STM32H747 (DMA) → FFT, RMS, kurtosis, ML             │
└───────────────────────────────────────────────────────────────────┘
```

### DC Mode (general analog sensors)

```
┌───────────────────────────────────────────────────────────────────┐
│ Analog Sensor → 0-10V / 1-5V (via shunt) / any DC signal          │
│              Current                                              │
│              Source (OFF)                                         │
│                     ↓                                             │
│              TMUX1101 → DC path (position B)                      │
│                     ↓                                             │
│              Direct to buffer (DC content preserved)              │
│                     ↓                                             │
│              OPA4188 Buffer → PGA (1x-20x) → 15 kHz AAF           │
│                     ↓                                             │
│              24-bit ADC @ 50 kSPS → Digital samples               │
│                     ↓                                             │
│              STM32H747 (DMA) → averaging, filtering, logging      │
└───────────────────────────────────────────────────────────────────┘
```

Noise Budget Analysis (IEPE Mode):

```
┌─────────────────────┬──────────────┬────────────┐
│ Source              │ Noise (µVrms)│ Equiv (µg) │
├─────────────────────┼──────────────┼────────────┤
│ Sensor (66212ANZ1)  │ 350          │ 3.5        │
│ Current source      │ 50           │ 0.5        │
│ Buffer op-amp       │ 15           │ 0.15       │
│ PGA (gain=10)       │ 100          │ 1.0        │
│ AAF                 │ 30           │ 0.3        │
│ ADC                 │ 10           │ 0.1        │
├─────────────────────┼──────────────┼────────────┤
│ Total (RSS)         │ ~380 µVrms   │ ~3.8 µg    │
└─────────────────────┴──────────────┴────────────┘
```

At 100 mV/g sensitivity: 380 uV / 100 mV = 0.0038g = 3.8 ug.
This is sensor-limited, not electronics-limited.

In DC mode, the current source noise term is eliminated, reducing total electronics noise. The TMUX1101 analog switch contributes negligible noise
(on-resistance thermal noise: ~0.13 nV/rtHz at 1 ohm) and distortion (-120 dB THD), well below the ADC noise floor.

## PCB Layout Critical Requirements

1. Analog Ground Plane
   - Separate from digital ground
   - Star ground connection at ADC
   - No digital switching under analog section

2. Power Supply Sequencing
   - Analog 3.3V from dedicated low-noise LDO
   - Isolated from digital 3.3V rail
   - Filter: LC (10 µH + 10 µF ceramic + 10 µF tantalum)

3. Component Placement
   - Current source → TMUX1101 → AC coupling / bypass → buffer → PGA → AAF → ADC (linear path)
   - TMUX1101 switches placed adjacent to AC coupling caps to minimize trace stubs
   - Keep trace lengths <2 cm where possible
   - Minimize vias in signal path

4. Shielding
   - Guard ring around analog section (grounded)
   - Top and bottom ground planes (stitched with vias)
   - Keep away from Ethernet PHY, switching PSU

5. Input Connector
   - 10-pin pluggable screw terminal (Degson 2EDGR-5.0-10P, 5.00 mm pitch)
   - Pinout: 4 channels x (signal + shield) plus supply and ground pins
   - Chassis ground separate from circuit ground
   - Shield connected to chassis, not circuit GND