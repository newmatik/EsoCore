# EsoCore Rev1.3 Review

November 11th 2025

I have manually reviewed the project structure, I continue to dive into the schematics and the BOM. However I want to share the following findings as
they stand right now. Feel free to disagree, I would like you to take every point into consideration so you can make sure you make the right choice.

---

## Manual Review by Dominik

### PCB color in STEP file

The PCB in the STEP file appears to be purple, we want it to be black.

### Minor Issues in Schematic

- Project name in schematic is "EsoCore_EdgeBoard_V1-2.0.0.PrjPcb", I suggest "EsoCore_Edge.PrjPcb" without versioning.
- Version in schematic is "EsoCore_EdgeBoard_V1-2.0.0.PrjPcb" I suggest "V1.3.0.0" so just the version and the correct one.
- Sheet 13 has a different template, please use the same as on other pages.

### Project Structure

- Remove the version from the Symbol Library, no need for that.
- Remove the version from the Footprint Library, no need for that.
- Name folders consistently "Footprint library" --> "Footprint Library" (capital L).
- Use the correct naming of the project it is "EsoCore" not "esoCORE".

---

## Grok 4 AI Review

### Clock/Oscillator Configuration (Page 2: MCU, Page 7: Ethernet)

- **HSE Crystal (X2: ABM8G-25.000MHz-18-D2Y-T)**: Load caps C14/C15 = 10pF.
  - Datasheet (Abracon): CL = 18pF, ESR = 50Ω max, temp -40°C to +85°C.
  - STM32H747 datasheet (ST): Recommends total CL ~10-20pF (including stray ~4-6pF from pins/PCB). Formula: CL = (C1 \* C2) / (C1 + C2) + Cstray.
  - Calculation: With 10pF || 10pF = 5pF + 5pF stray = 10pF. **Mismatch**: Crystal needs 18pF for stable oscillation. **Major issue**: Oscillator may
    not start reliably, especially at temp extremes, leading to boot failures or clock instability.
  - **Fix**: Use 27-33pF caps (CL ≈ 13-16pF + stray ≈ 18-21pF). Reference STM32 AN2867 for tuning.

- **LSE Crystal (X3: ABS07-32.768KHZ-T)**: Caps C16/C36 = 12pF.
  - Datasheet (Abracon): CL = 12.5pF, ESR = 70kΩ max (-40°C to +85°C).
  - STM32: Typical 12.5pF ok. CL = 6pF + 5pF stray ≈ 11pF, close enough (slight underload, but LSE is tolerant).
  - **Minor issue**: May need tuning if RTC accuracy is critical.
  - **Fix**: Test; if needed, adjust to 18pF caps for CL ≈ 9pF + stray ≈ 14pF.
  - **ChatGPT Disagrement:** The suggestion to jump straight to 18 pF load caps might overshoot depending on your stray capacitance. Target CL = 12.5
    pF using the standard formula (Ceq = (C1·C2)/(C1+C2); CL = Ceq + Cstray). With typical 2–5 pF stray, 15–22 pF caps often land closer than 12 pF;
    finalize by measuring startup margin/accuracy on the bench. The core point (tune caps) stands.

- **Ethernet Crystal (X4: YXC X322525MOB4SI, 25MHz)**: Caps C47/C48 = 12pF.
  - Datasheet (YXC): CL = 12pF (from similar YSX321SL series; exact part assumes standard).
  - DP83825 datasheet (TI): Recommends 25MHz, CL 15-30pF, ESR ≤50Ω.
  - Calculation: CL ≈ 6pF + 5pF stray = 11pF. **Major issue**: Below min 15pF; may cause clock jitter or EMI issues in RMII Master mode.
  - **Fix**: Use 22-33pF caps (CL ≈ 11-16pF + stray ≈ 16-21pF).
- **Not Considered**: Drive level verification (crystals may overdrive/underdrive). No backup clock sources if crystals fail.

### Power Supplies and Decoupling

- **MCU Power (Page 2)**: Good decoupling (36x 100nF, bulk 4.7uF/10uF). Matches STM32 recommendation (100nF + 1-4.7uF per VDD pair).

- **Bucks (Page 11: LMR51430XDDCR)**:
  - 3.3V: Feedback R63=56kΩ / R64=12.4kΩ → Vout ≈3.31V (matches datasheet formula Vref=0.6V \* (1 + Rtop/Rbot)).
  - 5V: R66=100kΩ / R67=13.7kΩ → Vout ≈4.98V (good).
  - Inductors 6.8uH, output caps 22uF x2: Matches datasheet table for 500kHz operation (though table suggests 5.6uH for 3.3V; minor).
  - **Minor issue**: Input caps shared; add more filtering if noise is high.

- **Supercap (C100: Wurth 850617021004)**: 10F, 2.7V rated, ESR 35mΩ, 1000hrs @65°C.
  - Connected to +3V3_OUT (3.3V) via R109 680Ω (for charging limit).
  - **Critical issue**: Rated only 2.7V; at 3.3V, overvoltage risks leakage, reduced life, or explosion. Not considered: Voltage balancing or limit.
  - **Fix**: Use a 3.5V+ rated supercap (e.g., Eaton HV1030-2R7106-R, 10F 2.7V but series for higher V? No, replace with proper). Or add
    regulator/diode to limit to 2.7V.

- **PoE (Page 8: LTC4267CGN-3)**: Rclass=45.3Ω (Class 3, 13W, matches datasheet). Sense 250mΩ x2 parallel=125mΩ → Ipeak=100mV/0.125Ω=0.8A (ok for
  switcher peak in 5V/2A output).
  - Feedback R54=100kΩ / R61=33.2kΩ → ~4.93V (good for 5V).
  - Inductor 3.3uH (datasheet recommends 4.7uH; minor, may increase ripple).
  - Output cap 330uF + others, good.
  - **Major issue**: No input fuse on PoE lines (IEEE 802.3af recommends for safety).
  - **Fix**: Add PTC fuse or 1A fast-blow on VPORTP.

- **USB to 3.3V (Page 9: AMS1117-3.3)**: Good for device mode, but if USB host, current limit needed (AMS1117 up to 1A).

- **Not Considered**: Thermal analysis (e.g., bucks/PoE heat dissipation in enclosure). Load transients on bucks (add feedforward cap if needed).

### Ethernet (Page 7)

- DP83825 connections: RMII pins match datasheet (TX_EN pin1, TXD0/1 pins23/24, etc.).
- RBIAS 6.49kΩ 1%, matches (for VOD tuning).
- Reset pullup 10k, good.
- **Major issue**: Bob Smith termination incomplete. Standard requires 75Ω resistors from center taps (TCT/RCT) to chassis via 1-2nF/2kV cap. Here,
  only 1MΩ R35 + 4.7nF C107 to earth---insufficient for common-mode noise/EMI.
  - **Fix**: Add 75Ω x4 from TX/RX pairs to a common point, then 2nF cap to chassis GND.
  - **ChatGPT Disagreement:** The recommendation is correct if your magjack/magnetics don’t integrate common-mode terminations. Some magjacks include
    the 75 Ω ladder internally; others don’t. So I’d mark this as “verify whether implemented in magnetics vs. on PCB; add if missing”, not
    automatically a major flaw.
- LEDs with 270Ω, good.
- Not Considered: MAC address provisioning (if needed).

### WiFi/ESP32 (Page 3)

- Pins: EN pulled high 10k, good. UART TX0/RX0 for comms/flash.
- Boot control: Q3 UMH3N with AT_DTR/RST for auto-download mode, standard.
- Power: 22uF + 100nF x2, matches Espressif recommendation.
- **Minor issue**: IO0 (pin 27) not explicitly pulled for boot; relies on MCU SCL0 high.
- Not Considered: Antenna diversity if external (IO17/18 to TP, perhaps for u.FL).

### Fieldbus (Page 10)

- ADM2582: Pins match (TxD/RxD/DE/RE, A/B/Y/Z). Termination 120Ω switchable, good.
- SN65HVD1176 for Profibus, similar.
- Addressing DIP good.
- Isolation/power: VISO caps 100nF +22uF, good.
- Good overall.

### IO/Analog/Relay (Page 12)

- Optos PC817 on inputs, series 2.4k (current limit ~10mA at 24V), pullups 10k, good isolation.
- ADS1115: Addr to GND (0x48), pins match datasheet. Power 100nF, good.
- Relay SRD-05V: Driven by BC848 transistor, flyback diode, good.
- Safety G7SA-2A2B 24V: Coils driven (from page 13?), contacts to terminals.
- **Minor issue**: No input filtering on analog (e.g., RC low-pass for noise).
- Not Considered: Input voltage clamping (if >24V spikes).

### PoE/Protection/Switching (Page 8/13)

- Supply switch Q8/DMN15H310, DRV110 for hold/peak control, good.
- Bridges HD01 for mode A/B PoE.
- **Minor issue**: DRV110 sense protection (R108 1Ω), but add clamp if high transients.
- Not Considered: Overcurrent fuse on 24V input (page 14).

### MicroSD/UI/Tags (Pages 5/6/4)

- EMIF06 filter on SD, good.
- OLED: Assuming I2C mode (SCL/SDA), but parallel pins NC---confirm display datasheet.
- Tags standard.

### General Oversights

- **EMI/Compliance**: Partial (Y-caps, ferrites), but missing full Bob Smith, crystal grounding. Test for EN55022.
- **Thermal/Safety**: No heat sinks on bucks/PoE; G7SA rated for safety (EN50155?), good.
- **Firmware Considerations**: Pins multiplexed (e.g., STM32 RMII/SDIO ok), but ensure no conflicts.
- **BOM/Assembly**: One part (PA1134NLT) Mouser-only; ensure alternatives.
- **Testing Not Considered**: No provisions for current sense/test points on high-power lines.

### Conclusion

The design is ~85-90% correct---functional prototype likely works, but risks (e.g., oscillators, supercap) could cause failures. Prototype and test
(oscillation, EMI, load transients). Fix critical/major issues before production. If you share PCB layout, I can review signal integrity further.

---

## Findings from ChatGPT 5 AI Review

### ESP32-S3 module variant vs. antenna

BOM lists ESP32-S3-WROOM-1U (U.FL) which is the pigtail version but we want the PCB antenna variant -1, not 1U. This is also in the 3D rendering.

### 24 V input protection & compliance

I don’t see an input fuse/PTC or explicit reverse-polarity protection path on the 24 V terminal. Add a resettable fuse and a TVS matched to 24 V
industrial surges (e.g., SMBJ33A/36A range) on the DC input specifically.

### Power source OR-ing / priority (24 V vs PoE)

The schematic shows solid PoE and DC/DC blocks, but I don’t see a dedicated ideal-diode / eFuse / hot-swap controller for source switchover. If you’re
simply tying rails or relying on buck tolerances, specify a proper OR-ing/priority scheme (ideal diode controllers or FET-XORing) and inrush control.
Define which source wins and verify no back-feed into the RJ45 or DC jack.

- **Grok Disagreement** ChatGPT states there's no dedicated ideal-diode/eFuse/hot-swap for source switchover, potentially leading to back-feed risks.
  In my review, I praised "switching (DRV110/Q8 for prioritization)" based on Page 13's protection/supply switching block. However, upon
  double-checking the DRV110's purpose (it's a PWM driver primarily for solenoids/valves, per its datasheet), it may not be optimally suited for clean
  power OR-ing—it's more for peak/hold current control. If the schematic repurposes it for prioritization, it could work but isn't a standard
  ideal-diode solution (e.g., LTC4412). So, I disagree that there's no mechanism, but agree it might not be robust/ideal and needs verification to
  prevent back-feed.

### Hold-up / safe-shutdown

I can’t find any supercap/EDLC in the BOM, so there’s likely no hold-up time. If you want graceful shutdown (filesystem safety, telemetry flush),
budget a supercap bank with a load-time calculation and controlled charge path.

- **Grok Disagreement:** ChatGPT claims "I can’t find any supercap/EDLC in the BOM, so there’s likely no hold-up time." This is incorrect—the BOM
  explicitly includes C100 (Wurth 850617021004, a 10F supercapacitor) on row22, connected to +3V3_OUT via R109 (680Ω) for charging. My review not only
  acknowledges it but flags a critical issue with its 2.7V rating vs. 3.3V operation (risking overvoltage/failure). They seem to have missed this
  component entirely, perhaps due to its naming ("WCAP-STSC_10X25") not screaming "supercap." The suggestion for a supercap bank and load-time
  calculation is good, but the premise (no supercap) is wrong.

### Ethernet line interface passives

Double-check the 49.9 Ω terminations and AC-coupling caps around the PHY/magnetics. I didn’t see 49R9 in the BOM text; they may be present under
another value or bundled with the magnetics, but please confirm against the PHY ref design (TX/RX 49.9 Ω, 0.1 µF coupling, RBIAS resistor value, strap
pins for RMII & PHY address).

- **Grok Disagreement:** ChatGPT notes not seeing 49.9Ω terminations or AC-coupling caps, suggesting confirmation against the PHY ref design. The
  DP83825 datasheet confirms integrated MDI and MAC termination resistors for RMII mode, so external 49.9Ω may not be needed (BOM lacks them, which is
  fine). RBIAS is present (R23=6.49kΩ 1%, matching datasheet). However, I agree on verifying straps/address—my review focused more on the incomplete
  Bob Smith termination (which ChatGPT doesn't mention, but it's related to EMI).

### Relay & inductive load suppression

There are fast diodes in the BOM (ES1D, LL4148), but verify the SRD-05V coil has a proper flyback path and that contact snubbing (RC or MOV) is
present for inductive loads on the output.

### Opto input thresholds (24 V PLC-style)

With PC817 CTR spread and listed input resistors (2.2 k/10 k and others), simulate the worst-case LED current at 24 V (and 12 V if supported) across
temperature and tolerance to ensure clean logic thresholds and no false triggers. Add series TVS or RC if you expect long-line transients.

### Profibus physical layer details

Profibus typically needs a specific termination network (220 Ω/390 Ω/390 Ω to 5 V/0 V) and a DB-9 pinout. You have a DSUB-9 (J4); please confirm the
exact termination values and selectable termination as per IEC 61158-2/Profibus DP guidelines.

### Grounding & shielding strategy

Verify your shield/chassis bonding (RJ45 shield, DSUB shell, field-I/O commons) is to chassis/earth at a single point with the right RC/TVS bridging
to digital ground to control ESD/EMI. This isn’t obvious from the BOM and is critical for emissions/immunity.

### USB-C role + power domain

As implemented, this is UFP-only (device). If you ever plan for host/OTG, you’ll need a PD/DRP controller; otherwise, document it as device-only and
confirm VBUS-to-3V3 path is protected (inrush, OVP) and that VBUS never back-feeds other rails.

### Compliance targets

Before fab: decide target standards (EN 61000-6-2/-6-4, IEC 61000-4-x, surge on 24 V input, EFT on I/O, ESD at all external ports). Add any missing CM
chokes and TVS on RS-485 A/B, DC input, and relay contacts as needed to pass those tests.

### Manufacturability

Check that all fine-pitch parts (LQFP-176, QFN-24, USB-ESD arrays) have correct paste mask and fiducials. Confirm testpoints exist for: 3V3, 5V, PoE
secondary, NRST, BOOT0, SWDIO/SWCLK, RMII signals, RS-485 A/B.

### Other verifications

- PHY straps: RMII/MII mode pins, PHY address, autoneg/LED configs match your intended behavior.
- LTC4267-3 class & transformer: RCLASS value for the desired PoE class (likely Class 3) and the exact flyback secondary to your 5 V rail.
- Isolated RS-485 domain: Isolation barrier return path and bias/termination DIP logic (bias always-on vs. last-node only).
- ESP32 reset/boot pins: EN/IO0 handling so you can program reliably; ensure the STM32 can reset the ESP32 if you’re using it as a coprocessor.
- SDIO pull-ups: On CMD/DAT lines per the STM32/Toshiba SDIO guidance.
- Relay coil driver: Drive transistor sizing and coil surge current; confirm DRV (or discrete transistor) SOA.
