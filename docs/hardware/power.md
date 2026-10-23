# CoWaS power and electrical audit

Written in HW1 (2026-09-25). §1 shows how power flows **today**, and §3 shows the **proposed pilot topology** (ROADMAP §3.8). §4 is the HW1 measurement checklist. **Every `___` is unknown until it is measured or read on the machine.** After the measurements, HW1 fills in this file and `pinout.md`, and lists the fixes and parts in ROADMAP §11. HW5 adds the battery, the boost and the energy figures (§5).

Related findings: F20 (pins the shield brings out), F25 (DAC range, pump driver), F26 (flowmeter level on D13), F32 (Due supply, cold boots), F36 (SPI levels and cables), F34 (A4988 holding current), F50 (no pressure sensor).

---

## 1. Today (as built, to be confirmed)

What the code and its comments say, with the gaps left open. Settings.h says the A4988 VMOT is taken from the VNH5019 shield's 24 V rail (not a dedicated supply).

```
 SUPPLY  type: ___ (bench PSU / adapter / battery)   V = ___ V   max ___ A
    │    main fuse: ___      main switch: ___      E-stop: none installed
    ▼
 ═══ 24 V rail ═══════════════════════════════════════════════════════════════════
    │                  │                          │                      │
    │ relay on D42     │ VIN terminal             │ from the shield's    │ relay/driver boards
    │ (coil ___ V,     │                          │ 24 V (Settings.h)    │ D44 → V1, D46 → Vman,
    │  diode? ___)     ▼                          ▼                      │ D40 → V23 relay only (F51)
    ▼              Pololu dual VNH5019        A4988 VMOT                ▼
 PUMP DRIVER       motor shield               bulk cap? ___         SOLENOID VALVES
 model ___         M1 → spool (disabled)      Vref ___ V            V1, Vman: ___ V, ___ W,
 input ___ V       M2 → manifold DC motor     Rsense ___ Ω          flyback? ___
 speed in ← DAC1   logic VDD from ___         │
 (~0.55–2.75 V,    ARDVIN=VOUT jumper: ___ ─┐ ▼
  buffer? ___)                               │ NEMA17 42SHD034-20B
    │                                        │ rated ___ A/phase
    ▼                                        │
 24 V DIAPHRAGM PUMP  ___ A at 90 %          │   (if fitted: shield VIN → Due VIN!)
                                             │
 MICRO PUMP (shield)  supply rail ___ V ← transistor on D48 (type ___, diode? ___)
                                             │
 ARDUINO DUE ◄───────────────────────────────┘
    power in: Vin pin / jack / 5V pin: ___   at ___ V   from ___
    USB: Programming port ← laptop / Pi (power + data?) ___     Native port: unused
    3.3 V pin feeds: ABP connector (none installed, F50), AMT22? ___, flowmeter? ___
    5 V pin feeds: ___ (AMT22? flowmeter? button pull-ups? shield logic?)

 RASPBERRY PI (legacy)  supply ___ (own USB adapter?)   USB → Due Programming port

 GROUND: 24 V− ↔ Due GND continuity ___   star point? ___   laptop on mains while on USB? ___
```

### 1.1 Loads today

| Load | Rail | Switched by (Due pin) | Rated V / I (label) | Measured I (§4) | Fuse | Flyback | Notes |
|---|---|---|---|---|---|---|---|
| Diaphragm pump + driver | 24 V | relay D42 + speed DAC1 | ___ | ___ | ___ | relay coil: ___ | F25: DAC "0 %" = 0.55 V; only the relay stops the pump |
| VNH5019 shield (M2 manifold motor) | 24 V | D3/D6/D7 | shield 5.5–24 V | ___ | ___ | n/a (H-bridge) | M1 (spool) disabled |
| A4988 + NEMA17 (O-ring lock) | 24 V (from the shield) | D9/D10/D11 | motor ___ A/phase | ___ | ___ | n/a | coils enabled while locked today (F34) |
| Valve V1 (air inlet) | ___ | relay D44 | ___ | ___ | ___ | ___ | |
| Valve Vman (deployment line) | ___ | relay D46 | ___ | ___ | ___ | ___ | |
| V23 relay (valve removed, F51) | ___ | D40 | ___ | ___ | ___ | ___ | still clicks |
| Micro pump (DNA/RNA Shield) | ___ | transistor D48 | ___ | ___ | ___ | ___ | |
| Arduino Due | ___ | — | Vin 7–12 V (6–16 V limits) | ___ | ___ | — | F32 |
| AMT22 encoder | ___ (5 V or 3.3 V?) | — | ___ | — | — | — | F36: MISO level |
| Flowmeter MW-FS-2.0 (D13) | ___ | — | 2.5–24 V (output swings to its supply) | — | — | — | F26 |
| Raspberry Pi (legacy) | own supply? ___ | — | 5 V ≥ 3 A | ___ | ___ | — | |

---

## 2. Rules this must meet (ROADMAP §11.3, §3.8)

- The Due is 3.3 V only; no pin is 5 V tolerant (SAM3X absolute max = VDDIO + 0.3 V ≈ **3.6 V**).
- The DAC gives ~0.55–2.75 V and is fragile: buffer it; the relay stops the pump, never "0 %".
- The Due never sees raw 24 V, nor a charged LiFePO4 above 16 V.
- One ground star point; motor currents stay out of sensor grounds.
- A flyback diode on every coil (valves, relays, micro pump motor).
- SPI and I²C cables < 30 cm.
- Every 24 V load must accept the rail's real maximum (a regulated 24 V; a raw 24 V LiFePO4 reaches 29.2 V).

---

## 3. Proposed pilot topology (ROADMAP §3.8)

```
 12 V LiFePO4 ≥ 50 Ah with BMS   (10.0–14.6 V)                          [HW5]
    │
    ├─ main fuse ___ A ── main switch ── shunt energy meter (Wh per cycle)
    │
 ═══ 12 V bus ══════════════════════════════════════════════════════════════════
    │                               │                          │
    ▼                               ▼                          ▼
 BOOST 12 → 24 V, regulated      BUCK 5 V ≥ 3 A, low noise    DUE SUPPLY (decided after §4 M7–M10)
 ≥ 1.5 × measured peak ___ A     fuse ___ A                    fuse 1 A; default: regulated 9 V buck
    │                               │                          → Due Vin (or the jack). Not cut by the
    ▼                               ▼                          E-stop, so the Due logs the E-stop.
 E-STOP, latching, NC contact    Pi CM4 + TOFU (Pilot B)       Pilot A: laptop on USB instead of the Pi
 DC rating ≥ 2 × rail current      │
 2nd NC contact → D37 (P1)         └─ USB → Due Native port (host link, FW7)
    │
 ═══ 24 V actuator rail (dead when the E-stop is pressed) ══════════════════════
    ├─ fuse ___ → relay D42 → pump driver → diaphragm pump    (relief valve 2.5–3 bar: HW2)
    ├─ fuse ___ → VNH5019 shield VIN   (ARDVIN=VOUT jumper removed)
    ├─ fuse ___ → A4988 VMOT, ≥ 47 µF across VMOT at the driver
    ├─ fuse ___ → valves V1, Vman (diode across each coil)
    └─ fuse ___ → micro pump (or its own rail, per M-results)

 GROUND: one star at the battery negative / shunt. Pump and motor returns go straight to the
 star; the Due, sensor and SPI grounds join it at one point only.
```

- **Why a regulated 24 V:** a 24 V LiFePO4 pack reaches 29.2 V. The shield is rated 5.5–24 V and many 24 V solenoids only ±10 % (26.4 V).
- **The Due supply:** a 12 V battery (≤ 14.6 V) is inside the Due's 6–16 V limit but above the 7–12 V recommended, and it sags when the pump starts. A small regulated buck (9 V → Vin) keeps the Due independent of the pump and of the Pi. Decided in step 3 of HW1, once M7–M10 are known.
- **The E-stop cuts the actuators only** (pump, motors, stepper, valves). The Due and the host stay up and see the E-stop on D37 (FW6).
- **Sizing** (from §4): fuses ~1.5 × each branch's measured current; E-stop DC rating ≥ 2 × the rail total; boost ≥ 1.5 × the peak (pump start + stepper + valves at once).

---

## 4. HW1 measurement checklist

**Safety while measuring.** No pressure sensor is installed (F50): the pump cap can't trip. Pump **only with an open outlet into a bucket**, a person at the machine, runs under 1 min, and the supply switch within reach. Unlock the O-ring before any rotation (F5): `linear_actuator` → `e` → `f` as needed → `x`. Power off before touching wiring; measure live only with probes.

**Tools:** a multimeter (DC V, 10 A range or a DC clamp), an oscilloscope for M15 (or skip it and note "no scope"), a measuring cylinder and a timer. Console: `pio device monitor` (the test menu starts at boot).

**Expected DAC voltages** (`analogWrite(DAC1, map(p, 0, 100, 0, 255))`, 8-bit, range 1/6–5/6 of 3.3 V): 1 % → 0.57 V, 50 % → 1.65 V, 90 % → 2.53 V, 100 % → 2.75 V. After a stop the code writes 0 → 0.55 V.

Fill in the **Result** column (or send me the values with the M-numbers).

| # | What | How | Acceptable | Result |
|---|---|---|---|---|
| **Supply and 24 V rail** | | | | |
| M1 | Present supply: type, rated V and A | Read the label | Known; regulated; rating ≥ 1.5 × the peak of M34 | |
| M2 | 24 V rail voltage, idle and with the pump at 90 % | DMM on the rail at the shield VIN, idle then `pump` 90 | 22.8–25.2 V; sag at pump start < 1 V | |
| **Pump driver (F25)** | | | | |
| M3 | Pump driver: model, supply range, speed-input type and range | Label, photo, datasheet (is the driver inside the pump?) | Input range known (e.g. 0–5 V, 0–10 V, PWM); supply range includes 24 V | |
| M4 | Is DAC1 buffered? | Power off. Trace DAC1 → driver input: op-amp follower, series resistor, or a direct wire? | Buffer (op-amp follower + series R) present; a direct wire = fix (F25) | |
| M5 | DAC1 at 1 / 50 / 90 / 100 % | DMM DAC1 → GND, `pump` then 1, 50, 90, 100 (a key stops between runs); measure at the Due pin **and** at the driver input | Due pin within ±0.1 V of 0.57 / 1.65 / 2.53 / 2.75 V; driver input equal to the Due pin within 0.02 V (else the driver loads the DAC) | |
| M6 | DAC1 after a stop, relay off | DMM after `pump` has been stopped | ~0.55 V; the pump is silent (relay open) | |
| M7 | Pump speed at "0 %" (1 % = 0.57 V) with the relay on | `pump` 1, open outlet into a cylinder, 60 s | Record mL/min. Any flow > 0 confirms the relay is the only stop (already the rule) | |
| M8 | Does 100 % reach the driver's full speed? | Compare 2.75 V with the driver's full-scale input (M3); `pump` 100, 60 s into the cylinder | 2.75 V ≥ 90 % of the full-scale input. Below that, "100 %" is not full speed (F25: buffer with gain) | |
| **Due power (F32)** | | | | |
| M9 | What feeds the Due: Vin pin, jack or 5V pin, from where | Power off; follow the wires; **check the VNH5019 shield's ARDVIN=VOUT jumper** | Exactly one source on Vin or the jack, regulated; ARDVIN jumper **removed** if the shield supply is > 12 V | |
| M10 | Voltage at the Due Vin pin (USB unplugged) | DMM Vin → GND | 7–12 V (limits 6–16). **24 V here: switch off at once** | |
| M11 | Due 5V and 3.3V pins, in three cases: USB only, Vin only, both | DMM on the power header | 5V: 4.75–5.25 V; 3.3V: 3.20–3.40 V in every case | |
| M12 | USB power path and grounds | Which port is plugged in (Programming / Native), from what (laptop / Pi); does the Due run with the 24 V off? DMM continuity 24 V− ↔ Due GND ↔ laptop USB shield | Record. One common ground, joined at one point | |
| M13 | Cold boots on today's supply | Supply off 10 s, on; the menu answers (`clocklist`) | 5 / 5 boot without pressing reset | |
| M14 | Pi supply | Label; `vcgencmd get_throttled` on the Pi after 5 min | Own 5 V ≥ 3 A supply; `throttled=0x0` | |
| **Flowmeter (F26)** | | | | |
| M15 | Flowmeter supply voltage | DMM red wire → black wire at the sensor | Record; 3.3 V preferred (MW-FS-2.0: 2.5–24 V, output = supply) | |
| M16 | D13 high and low levels | Blow gently or turn the rotor by hand until the output sits high, then low; DMM D13 → GND | High ≤ 3.3 V (abs max 3.6 V); low ≤ 0.8 V. Above 3.6 V: unplug the sensor until HW3 | |
| M17 | Is anything wired to D12 (`flow_sensor_big`)? | Look / continuity | Record (ROADMAP §11.2 wants D12 for `Q_out` in P2) | |
| **SPI (F36)** | | | | |
| M18 | AMT22 supply voltage | DMM at the encoder connector | Record; must match the datasheet variant | |
| M19 | AMT22 MISO high level | Scope on MISO (SPI header), trigger on D38 falling; send `enc_rot` a few times | High ≤ 3.3 V (abs max 3.6 V); clean edges, no ringing past 3.6 V | |
| M20 | ABP supply at its connector (none installed, F50) | DMM on the P1 connector's supply pin vs GND (ABP pin 2 vs pin 1) | 3.3 V ± 0.15 V (ABP 3.3 V parts: 3.0–3.6 V). **5 V: rewire before HW4** | |
| M21 | SPI cable length to the AMT22 and to the ABP position | Tape measure; shielded / twisted? | < 30 cm, or twisted with GND | |
| **VNH5019 shield (!3)** | | | | |
| M22 | EN/DIAG pull-up level at D5 (M2) and D30 (M1) | Motors idle, no fault; DMM D5 → GND, D30 → GND | 3.0–3.4 V. ~5 V = the shield logic VDD comes from 5 V: fix before FW5 reads it | |
| M23 | Shield logic VDD source and CS level | Which pin feeds VDD (5V / 3.3V / IOREF)? DMM A11 while the manifold rotates (`slot5` after unlock) | VDD = 3.3 V, or EN/DIAG safe per M22; A11 ≤ 3.3 V | |
| **Other inputs (!2, !4)** | | | | |
| M24 | Button and switch levels D24–D29 | DMM each pin → GND, released and pressed | ≤ 3.4 V in both states; pull resistors go to 3.3 V, not 5 V | |
| M25 | A4988 ENABLE (D11) at power-up | Watch the motor during a cold boot and an upload | No twitch; else add the 10 kΩ pull-up to 3.3 V (Settings.h TODO) | |
| **Flyback diodes** | | | | |
| M26 | Pump relay coil (D42) | Power off; look for a diode across the coil (relay boards usually have one), cathode on + | Present, right polarity | |
| M27 | V1 relay coil and V1 valve coil | Same; the valve coil switched by the relay contact also needs its own diode | Both present | |
| M28 | Vman relay coil and Vman valve coil | Same | Both present | |
| M29 | V23 relay coil (D40) | Same | Present | |
| M30 | Micro pump motor (D48 transistor) | Same, diode across the motor | Present | |
| M31 | Rated voltage of every 24 V load (valves, relay coils, pump) | Labels / datasheets | Accepts a regulated 24 V ± 5 % (record the max) | |
| **24 V rail current** (DMM 10 A in series with the rail, a DC clamp, or the bench supply readout) | | | | |
| M32 | Idle: Due booted, everything off, A4988 disabled | Read after boot | Record; expected < 0.15 A | |
| M33 | Pump at 50 % and 90 %, steady | `pump` 50, then 90, 30 s each, open outlet | 90 %: ≤ 0.5 A (the `POWER_PUMP` comment); record both | |
| M34 | Pump start peak | Peak-hold (clamp) or the supply's max reading at `pump` 90 | Record; sizes the fuse, the E-stop and the boost | |
| M35 | Stepper enabled, holding | `linear_actuator` → `e`, wait 10 s | Record | |
| M36 | Stepper moving | From unlocked: `b` (one turn toward the lock, 200 < 210 steps, no stall), then `f` back | Record | |
| M37 | Valves on | `valves` (each valve on 5 s, twice; read during each 5 s) | Record each | |
| M38 | Manifold rotating | Unlock first; `slot5`, then `slot0` | Record the peak | |
| M39 | Micro pump on | `micro_pump` (10 s) | Record, and which rail it draws from | |
| **A4988 current** | | | | |
| M40 | Motor rated current | 42SHD034-20B label / datasheet | Record A/phase | |
| M41 | A4988 board type and sense resistors | Read the marking (R050 / R068 / R100 = 0.05 / 0.068 / 0.1 Ω) | Record | |
| M42 | Vref | Logic powered; DMM from the trim-pot wiper to GND. I_max = Vref / (8 × Rsense); full step gives 0.7 × I_max per coil | I_max ≤ the rated current; ≤ ~1.0 A without a heatsink. **Vref was raised for torque (user, 2026-09; value not recorded, LOG F172).** The stall point, so `LINEAR_ACT_LOCKED_STEPS`, depends on it. Any change → redo `calibrate_actuator` and record it | |
| M43 | A4988 temperature after 2 min enabled | `linear_actuator` → `e`, 2 min; finger / IR thermometer | < 70 °C | |
| **Pins the shield brings out (F20)** | | | | |
| M44 | Reachable pins | For each pin in pinout.md, which connector or pad of the PCB shield brings it out; at least D36, D37, D39, D41, D43, D45, D47, A0, A5, A6, SDA/SCL 20/21 (ROADMAP §11.2) | Every planned P0/P1 pin is reachable, or a free substitute is named | |

---

## 5. Energy (HW5)

ROADMAP §3.8 estimates, to replace with measurements from M32–M39 and a shunt meter over one full cycle.

| Load | Estimate | Measured power | Measured energy |
|---|---|---|---|
| Pump | ≤ 12 W, ~5 Wh/sample | ___ W | ___ Wh/sample |
| Stepper while locked | 0 W once the coils are off (FW4) | ___ | ___ |
| Valves, micro pump | a few W, < 1 Wh/sample | ___ | ___ |
| Due + shield logic | ~1–1.5 W (24–36 Wh/day) | ___ | ___ Wh/day |
| Pi CM4 + TOFU (+ LTE) | ~3–5 W (+1–2 W) | ___ | ___ Wh/day |
| **One full cycle** (`sample_process`) | — | — | ___ Wh |
| **Idle, whole machine** | — | ___ W | ___ Wh/day |
