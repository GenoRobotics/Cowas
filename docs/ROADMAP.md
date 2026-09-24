# CoWaS roadmap: from the bench to Lake Geneva

**Goal:** first eDNA data from Lake Geneva in **about 8 weeks**, in two stages:

1. **Pilot A (supervised)**: 1–2 days from a pier, pontoon or boat, on battery. Someone from the team stays on site. The inlet is at a fixed depth. The run includes blanks and manual comparison samples, and the filters go to the lab.
2. **Pilot B (unattended)**: 1–2 weeks with 14 samples, on battery, with remote monitoring and alerts.

Everything that is not needed for these two pilots comes **after the first data**. That work is in [ROADMAP_LATER.md](ROADMAP_LATER.md), with its prompts ready. The previous version of this roadmap is kept in [archive/ROADMAP_v1.md](archive/ROADMAP_v1.md); §12 maps its step IDs to the new ones.

---

## 0. How to use this document

- **One step = one fresh Claude Code session = one branch/PR.** Copy the step's prompt from §8 into a new session on a new branch, and fill in the `<...>` placeholders. Every prompt says which sections of this file to read, so the prompts stay short.
- **Branches (decided 2026-09-24):** all roadmap work lives on the long-lived branch **`cowas-v2`**, which starts at FW0 (tag `baseline-v0`). Each step gets its own branch from `cowas-v2` (e.g. `fw1-protocol-spec`, `ho1-host-skeleton`) and is merged back into `cowas-v2` through a PR. **`main` keeps the pre-roadmap code** until Pilot A has passed; then `cowas-v2` is merged into `main` once, and tagged `v2.0`.
- **Tracks** can run in parallel, one owner each:

  | Track | ID | Owner (fill in) | Works on |
  |---|---|---|---|
  | Firmware | `FW` | `Quentin` | Arduino Due code |
  | Host | `HO` | `Quentin` | Python on the laptop/Pi: scheduler, sample records, remote access |
  | Hardware & field | `HW` | `Quentin` | Wiring, power, fluidics, enclosure, site |
  | Lab / eDNA | `LA` | `Quentin` | Protocols, contamination, shield, the lab interface |
  | Validation | `VA` | whole team | Acceptance campaigns and pilots |

- **Priorities:**
  - **P0**: needed for Pilot A. Nothing else starts until P0 is on track.
  - **P1**: needed for Pilot B (unattended).
  - **P2**: after the first data (see ROADMAP_LATER.md).
- **Before a step:** the tree is clean and the step's dependencies (§7) are merged.
- **After a step:** the unit tests pass. For firmware, you flash and run the step's **hardware checklist**, and its `TEST|` lines are logged (§5). Then tick the step in §7 and merge.
- A hardware task (`HW`) is done by a person. When Claude can help (a calculation, a document, a checklist, a script), the task has a prompt too.

---

## 1. Milestones and critical path

### 1.1 Milestones

| # | Milestone | Target | Meaning | Gate |
|---|---|---|---|---|
| **M1** | Bench-safe | week 3 | The pressure sensor reads correctly. The safety fixes are in. The mechanical test suite passes. A full production cycle runs on the bench, supervised. | FW0–FW4 done; HW1–HW3 done |
| **M2** | Sample quality known | week 5 | We know the real filtered volume, the DNA shield volume per filter, the residual water, the carry-over between samples, and the cross-talk between slots. SOPs are written. | LA1–LA4 done |
| **M3** | Host-driven bench run | week 6 | A mission of 14 samples runs from the host, alone on a bucket of water, and produces a sample CSV. STOP, E-stop and power cuts recover correctly. | FW5–FW8, HO1–HO3, VA1 passed |
| **M4** | **Pilot A** | **week 8** | Supervised 1–2 days on the lake, battery, fixed depth. Filters and CSV handed to the lab. | VA2 checklist |
| **M5** | **Pilot B** | week 12–16 | Unattended 1–2 weeks, 14 samples, remote monitoring and alerts. | All P1 steps, VA3 soak test |
| M6 | v2 | after | Depth (spool), filter clogging analytics (2 P + 2 Q), phone app, environmental sensors, controls | ROADMAP_LATER.md |

### 1.2 Critical path

```
Week        1         2         3         4         5         6         7         8
FW   FW0─►FW1(spec)─►FW2 tests─►FW3 P sensor─►FW4 fixes─►FW5 supervisor─►FW6 stop/recover─►FW7 protocol─►FW8 sample record
HO             (from FW1) HO1 skeleton+sim ─► HO2 slots+records ─► HO3 mission runner ───────────┐
HW   HW0 order─►HW1 electrical audit─►HW2 safety hw─►HW3 flow─►HW5 power─►HW6 field fluidics+box   │
LA   LA1 lab alignment─►LA2 SOPs ─────────► LA3 shield/residual (needs FW3+FW4) ─► LA4 carry-over│
VA                                                                      VA1 bench acceptance ◄───┘─► VA2 PILOT A
```

- **The longest chain is the firmware**: FW3 (pressure) → FW4 → FW5 → FW6 → FW7. It fixes the date.
- **FW3 needs an ABP physically on the line (HW4).** The FW0 bench check (2026-09-24) found **no pressure sensor installed at all**: the analog A2 sensor was removed, and nothing answers on the SPI CS D8 (F50). FW3 can write and unit-test the driver fix (F27) before any sensor arrives. Its hardware checklist needs one: the 150 psi part if the team still has it, otherwise the 060 from HW0 #1. **Order #1 in week 0: it is now on the critical path.**
- **The host team works against a simulator** from week 2, so it doesn't wait for FW7.
- **The laptop is the host for Pilot A.** The host code is cross-platform, so the Pi (HO4) is P1. If HO4 is ready in time, use the Pi in Pilot A, and keep the laptop as the fallback.

### 1.3 Weekly plan (small team)

| Week | Firmware | Host | Hardware & field | Lab / eDNA |
|---|---|---|---|---|
| 0–1 | FW0 baseline, FW1 protocol spec | read the spec | **HW0 order everything**, HW1 electrical audit | LA1 meeting with the lab |
| 2 | FW2 mechanical test suite | HO1 skeleton + simulator | HW2 relief valve, E-stop, fuses; HW3 flowmeter | LA2 SOPs (loading, cleaning) |
| 3 | FW3 pressure sensor | HO1 → HO2 | HW4 install P sensor; HW7 lock endurance | review the FW2 test logs |
| 4 | FW4 fixes (volume, purge, timeout) | HO2 slots + records | HW5 power: battery, rails, budget | LA3 shield + residual water |
| 5 | FW5 supervisor + watchdog; FW6 stop/recover | HO3 mission runner | HW6 intake, outlet, splash-proof box | LA4 carry-over + cross-talk |
| 6 | FW7 protocol; FW8 sample record | HO3 against the real Due | site visit, mounting plan (HW8) | pilot sampling plan (VA2) |
| 7 | **VA1 bench acceptance** (whole team), bug fixes | | | |
| 8 | **VA2 Pilot A** | | | |
| 9–16 | FW9 hardening | HO4 Pi, HO5 remote + alerts | HW7 home switch, HW8 enclosure, mooring | lab results → decide on P2 |

If a week slips, **cut scope, not safety.** You can cut: replicates, FW8's clog end condition (the volume and timeout still apply), the Pi (use the laptop). You cannot cut: FW3, FW4, FW5, FW6, the relief valve, the E-stop, LA3, LA4.

---

## 2. What changed from the previous roadmap, and why

1. **Organized around the pilots, not the target architecture.** The previous plan reached "field-ready" after about 30 steps (Phase 0 → 5), which is 6+ months. Here, the P0 path is 9 firmware steps, 3 host steps and 4 lab steps. The rest is postponed unchanged (ROADMAP_LATER.md).
2. **A lab / eDNA track was added.** The previous plan was mostly machine engineering. The biggest risks to *usable data* are biological:
   - carry-over between samples through the shared intake line: the current 100 mL purge is smaller than a 5 m intake hose (F30);
   - residual water diluting the DNA shield (F28);
   - the shield draining out of the filters over the days until collection (F29);
   - the absence of blanks.
   These are cheap to measure and fix now, and expensive to discover after a deployment.
3. **Pressure sensor moved onto the critical path.** It was a "floating" step. Without a trustworthy pressure reading, no production cycle can run, so it is now FW3, first by fixing the installed sensor.
4. **The protocol spec comes early (FW1)**, so the host team can work in parallel from week 2.
5. **Merged and slimmed.** Supervisor + watchdog (FW5); STOP + safe state + recovery + the modes (FW6). The mechanical restructure (split `Tests.cpp`, command table), runtime parameters, and the full phone app are P2. Calibration values stay in `Settings.h` for now.
6. **Power and field hardware are first-class.** You chose battery only, so the always-on load (Due + Pi + modem) matters more than the pump (§3.8). Also new: the intake strainer, the quagga mussels, the outlet placement.
7. **New electrical findings** (F23–F36), e.g. the Due DAC can't output 0 V (F25), the pressure conversion constants are wrong for both sensors (F27), and the flow calibration factor is truncated to an integer (F31).
8. **A test strategy that covers every level** (§5). Every step lists its native unit tests or pytest tests, its on-device `TEST|` checks, and its acceptance criteria.

---

## 3. Design decisions

The decisions from v1 still hold unless noted. This section is condensed; archive/ROADMAP_v1.md §1 has the long form.

### 3.1 Architecture: who owns what

```
 Operator (laptop in Pilot A; phone + Tailscale in Pilot B)
        │  CLI over SSH (Pilot A/B), web app later (P2)
        ▼
 HOST  (laptop for Pilot A → Raspberry Pi CM4 + TOFU for Pilot B)    Python, cross-platform
        schedule · slot inventory · sample records (CSV) · device logs · alerts (P1)
        │
        │  Native USB port (SerialUSB): protocol v1, JSON lines + forwarded logs
        │  (Programming port stays free for a laptop console)
        ▼
 CONTROLLER  (Arduino Due)
        every sensor and actuator · safety interlocks · procedures (purge, sample, shield, empty)
        service console for tests and calibrations
```

| Concern | Due | Host | Why |
|---|---|---|---|
| Actuators, sensors, PID, stepper | ✅ | | Real-time; must not depend on the link |
| Safety: pressure cap, E-stop, watchdog, timeouts, rotation interlock | ✅ | monitors, alerts | Must work if the host crashes or the link drops |
| Procedures (valve, pump and manifold sequences) | ✅ | | The host says *what*, the Due knows *how* |
| Which slot, when, blank or sample, replicates | | ✅ | Needs a clock and persistence |
| Slot inventory, sample records | measures, reports | ✅ stores, exports | The Due forgets everything on reset (no RTC, no EEPROM) |
| Calibration values | `Settings.h` (P0) | stores them from P2 | Runtime parameters are P2 |

Rule: **the Due never starts a sample on its own, and the host never toggles a valve directly.**

### 3.2 Safety layers, safe state, recovery

| # | Layer | Pilot | Covers |
|---|---|---|---|
| 0 | **Mechanical pressure-relief valve** (~2.5–3 bar, to waste) | A | Everything else failing, sensor wrong or missing |
| 1 | **Hardware E-stop cutting the 24 V actuator rail.** A latching mushroom switch with a DC rating above the total 24 V current can be wired in series directly, with no relay. A second contact goes to a Due input (P1). | A (switch), B (input) | Firmware hang, link down |
| 2 | Due **watchdog** | A | Stuck loops |
| 3 | Firmware **supervisor** `service()`: pressure cap, safety-sensor plausibility, timeouts, rotation interlock | A | Normal faults |
| 4 | **Software STOP** (console `stop`, host `STOP`), always accepted | A | Operator sees a problem |
| 5 | Host checks + **alerts** | B | Unattended operations |

**Safe state:** pump and micro pump off; all valves closed; manifold motor stopped; **the O-ring stays locked** if it is locked. If it is unlocked (e.g. mid-rotation), it is left as is.

**Recovery is enforced:** after any fault, STOP or boot, the Due refuses procedures until `RECOVER` passes: cause gone → encoder OK → O-ring state known (home if unknown) → valves to idle → IDLE. The host marks an interrupted slot `SUSPECT`.

### 3.3 Pressure: one safety value

`PRESSURE_CAP` = **2.5 bar** (the only safety value). The PID target is derived: `cap − 0.5` = 2.0 bar. Edits are accepted only in 0.5–3.0 bar, as a typo guard. The cap acts on **`P_in`** (upstream of the filter). An implausible `P_in` reading stops the pump.

### 3.4 Link: two ports

| Port | Connected to | Carries |
|---|---|---|
| **Native USB** (`SerialUSB`) | Host, permanently | Protocol v1 (JSON lines) + a copy of every log line |
| **Programming port** (`Serial`) | Laptop, when needed | Human console, test menus |

Why: *opening* the Programming port resets the Due (upload auto-reset). A host restart would reboot the Due mid-sample. The Native port doesn't reset, except on the 1200-baud "touch", which the host must never use. It is also full-speed USB, so the 9600-baud limit disappears (F33).

### 3.5 Controller modes (lean version for the pilots)

```
BOOT ─► self-check ─► RECOVERY_REQUIRED ─ RECOVER ok ─► IDLE (host commands)
IDLE ◄─► BUSY (a procedure runs; only STOP/STATUS/PING accepted)
IDLE ◄─► SERVICE (console on the Programming port: `service` to enter, `exit` to leave)
any  ─► FAULT (latched) / ESTOP ─► clear + RECOVER ─► IDLE
```

The boot default is **IDLE** (host mode) from FW7 onwards. Today, `setup()` ends in the blocking test menu, so the host path never runs after a reset (F23). That is fatal in the field.

### 3.6 Slots and sample records (pilot minimum)

Slot states on the host (SQLite): `EMPTY`, `READY`, `IN_PROGRESS`, `SAMPLED`, `FAILED`, `DISABLED`, `SUSPECT` (was in progress when power or the link was lost). A **sampling event** produces 1–3 filters. Each filter has a **role**:
- `sample`, with a replicate index;
- `equipment_blank` (NEW, P0): DNA-free water pumped through the machine from a bottle, using the normal cycle. It detects contamination from the machine itself.
- `negative_control` and `positive_control` (automated, P2).

**One row per filter in the CSV export** (the fields in HO2): `sample_id`, `deployment_id`, `device_id`, `slot`, `role`, `replicate`, planned/start/end time (UTC ISO 8601), site, lat/lon (entered by hand), depth (fixed intake depth), filtered volume, duration, `end_reason`, `P_in` max/mean, flow mean/min, purge volume, shield time, time from end of filtration to shield, water temperature (when installed), fault codes, warnings, firmware and host versions, operator notes. Match the column names to what the lab needs (LA1). The full FAIRe/MIxS mapping is P2.

No code hard-codes 14 or 15. The host sizes its inventory from the Due's `GET_INFO`.

### 3.7 eDNA sample-quality rules (NEW)

1. **Purge ≥ 3 × the dead volume** upstream of the manifold (intake hose + pump + sensors + tubing), computed from the hose's inner diameter and length. Example: 5 m of 6 mm ID hose = 141 mL, so purge ≥ ~424 mL. Today's value is 100 mL (F30).
2. **Filtered volume** from the calibrated flowmeter, **cross-checked by weighing** the outlet water in VA1. Record the `end_reason` for every filter: `volume_reached`, `clogged`, `timeout`, `pressure_cap`, `fault` or `stopped`.
3. **Residual water and shield volume** are measured by weighing filters (LA3). The target shield volume is defined by the lab (usually ~1–2 mL for a Sterivex).
4. **Blanks in every deployment:** one equipment blank at the start and one at the end (through the machine), plus one manual field blank.
5. **Manual comparison samples** in Pilot A: 2–3 manual Sterivex filtrations at the same place, time and volume. They validate the machine against the standard method; this is the key result of Pilot A.
6. **Filter handling:** load and unload with gloves, following a written cleaning SOP (LA2). Record the time from sampling to retrieval. After retrieval, cap the filters, label them, keep them cool and dark.
7. **Cross-talk between slots** is measured once with a dye (LA4), including over rotation while other filters sit in the manifold.
8. **Biosecurity:** Lake Geneva has quagga mussels. Clean and dry all wetted parts before using them in another water body.

### 3.8 Power strategy (battery only)

**Status:** confirmed: the lead screw self-locks. **Still unknown:** the pump driver's input range (measured in HW1; it decides what 0–100 % on the DAC really gives, F25) and every power figure below (measured in HW5). Don't buy the Pilot B battery before HW5. The Pilot A battery (#6 in §11.1) is sized with a large margin, so it can be ordered now.

Estimates, to be replaced by measurements in HW5:

| Load | Power | When | Energy |
|---|---|---|---|
| Pump | ≤ 12 W (24 V × ≤ 0.5 A, `POWER_PUMP` comment) | ~20–25 min per sample | ~5 Wh/sample |
| Stepper while locked (A4988) | **0 W**: the lead screw self-locks (confirmed), so the coils are off once locked (FW4) | — | ~0 (only the lock/unlock moves) |
| Valves, micro pump | a few W | minutes | < 1 Wh/sample |
| Due + shield logic | ~1–1.5 W, **estimate** | always | 24–36 Wh/day |
| Pi CM4 + TOFU (+ LTE) | ~3–5 W (+1–2 W), **estimate** | always, unless duty-cycled | 70–170 Wh/day |

- **Pilot A** (2 days, 14 samples, laptop or Pi): ~0.4–0.5 kWh. A **12 V 50 Ah LiFePO4** (~640 Wh) is enough with margin.
- **Pilot B** (14 days): the always-on load dominates. Always on: ~1.5–2.5 kWh. With the Pi duty-cycled (woken before each sample): ~0.6–0.9 kWh. Decide in HW5 from measurements: a bigger battery (simple) or duty cycling (P1/P2 work).
- **Voltage (important):** a 24 V LiFePO4 pack reaches **29.2 V** when charged. Check every 24 V load against its rating: 24 V solenoid valves are often rated ±10 %, i.e. 26.4 V max. The recommended topology: a **12 V LiFePO4 → a regulated 24 V boost** for the actuators, and a **regulated 5 V buck** for the Pi (≥ 3 A). Power the Due from a regulated supply (Vin 7–12 V recommended, 6–16 V absolute), never from raw 24 V. Fuse every branch.

### 3.9 Not now (P2, see ROADMAP_LATER.md)

A second pressure sensor and a second flowmeter with clogging analytics, the PID variants, runtime parameters, the mechanical restructure, the web app and API, full notifications, environmental sensors beyond water temperature, the spool/depth, automated controls, firmware updates from the Pi, the Jetson.

---

## 4. Findings (code and hardware)

"Step" = where it gets resolved. F1–F22 come from v1 (condensed). **F23–F36 are new.** **F37 onwards are found during the steps and tracked in [FIRMWARE_LOG.md](FIRMWARE_LOG.md)** (with their status); the most important so far are F50 (no pressure sensor installed) and F51 (V23 valve removed).

| # | Finding | Where | Severity | Step |
|---|---|---|---|---|
| F1 | Slot availability is RAM-only; after a reset, used filters look available | `Manifold.cpp` | **High** | HO2 (the host owns the inventory) |
| F2 | The pressure cap is not polled in button-panel pump mode | `main.cpp` | **High** | FW4 |
| F3 | The cap is 3.0 bar, and the setpoint `2` is hard-coded in 4+ places | `Settings.h`, `Step_functions.cpp` | **High** | FW4 |
| F4 | `step_DNA_shield()` uses `FILL_STERIVEX_TIME / 10.` (a test shortcut): 25 s instead of 250 s | `Step_functions.cpp` | **High** | FW4 (value from LA3) |
| F5 | `rotateMotor()` ignores the O-ring lock (slotN, clockMM, panel Man Slot) | `Manifold.cpp`, `Tests.cpp`, `main.cpp` | **High** | FW2, FW4 |
| F6 | `flow_sensor_small` is uncalibrated, and it is the volume stop for every sample | `Flow_sensor.h` | **High** | HW3, FW4 |
| F7/F22 | **Safety and PID read a sensor that is not installed**: `read_pressure2()` (A2, a floating pin). The installed sensor is the SPI ABP (`pressure1`, CS D8), bypassed because of an "invalid SPI response" error | `main.cpp`, `Step_functions.cpp` | **Blocking** | FW3 |
| F8 | `main_program()` blocks for a whole sample; there are no request ids, no acknowledgments, no timeout; the depth is parsed by string slicing | `main.cpp`, `cowas_loop.py` | High | FW7, HO1 |
| F9 | The Pi code is Linux-only (ZMQ `ipc://`, `/dev/ttyACM0`) and desktop-only (PyQt5) | `computer_interface/` | — | HO1–HO3 |
| F10 | The console and the Pi share one port at 9600 baud | `main.cpp` | Medium | FW7 |
| F11 | `DEBUG_MODE_PRINT` is a `const bool`, so the `#if` blocks never compile | `Settings.h` | Low | FW6 |
| F12 | The `uno` env can't build; `-I include/interfaces` is missing | `platformio.ini` | Low | FW0 |
| F13 | No automated tests; `Tests.cpp` is 1,878 lines | — | Maintainability | FW0; restructure P2 |
| F14 | `Pump::stop()` has `delay(500)`; many blocking waits | `Pump.cpp` | Low | FW5 |
| F15 | `step_sampling()` hard-codes a 3-minute runtime (too short for 2 L) and a 100 mL volume | `Step_functions.cpp` | **High** | FW4 |
| F16 | Hard-coded slot counts (`i < 14`, `% 15`, `FILL_ORDER[14]`) | many | Medium | FW2 (new code), P2 (the rest) |
| F17 | With the sensor out, every pumping test refuses to run | `Pump.cpp` | Blocking for tests | FW2 |
| F18 | The O-ring lock state is RAM-only (`false` at boot): after a reboot while locked, the firmware would rotate while locked | `Oring_lock.cpp` | **High** | FW2 |
| F19 | The analog pressure path (A2) has a hard-coded zero and 10-bit ADC | `Pressure_sensor.cpp` | Low | retired in FW3 |
| F20 | The comment says "all pins are claimed"; ~17 digital and ~8 analog pins are free in the code; the real limit is the PCB shield | `Settings.h` | Low | FW0, HW1 |
| F21 | ABP 150 psi: ±1.5 % FS = ±0.16 bar; too coarse for a pressure-drop measurement with 2 sensors. The 060 psi version gives ±0.06 bar | Hardware | Medium | HW0 (order), P2 (ΔP) |
| **F23** | **`setup()` ends by calling `test_all_components()`, which blocks in the console menu.** `main_program()` (the Pi link) only runs after someone types `stop`. After any reset in the field, the machine waits for a human. | `main.cpp:219` | **Blocking for the field** | FW6, FW7 |
| **F24** | **The legacy Pi link treats the first line it receives as "done".** The Due prints logs as soon as a sample starts, so the Pi believes the sample finished at once and can send the next command. The Due buffers it and runs it later. `sampleFunction<depth>` is sent without `\n`, so the Due waits for its 1 s serial timeout. | `cowas_loop.py`, `main.cpp` | **High** | FW7, HO1 (the legacy link is retired) |
| **F25** | **The Due DAC does not output 0–3.3 V.** It outputs ~0.55–2.75 V (1/6 to 5/6 of 3.3 V), so `analogWrite(DAC1, 0)` still gives ~0.55 V: the pump only stops because of the enable relay (D42). The pump driver's input range is unknown: 100 % may not be full speed. The Due DAC pins are also easily damaged by a short or a low-impedance load; buffer them (op-amp follower + series resistor). | `Pump.cpp`, hardware | **High** | HW1 |
| **F26** | The inlet flowmeter is on **D13**, and the code comment says "probably fried". The MW-FS-2.0 output swings to its **supply** voltage: powered at 5 V, it puts 5 V on a 3.3 V-only pin. | `Settings.h`, hardware | **High** | HW3 |
| **F27** | **ABP conversion constants are wrong for both sensors.** The code uses 7.63e-4 bar/count and a 1.25 bar offset, which assumes a **10.0 bar** full scale. The installed 150 psi part is 10.34 bar, so it reads ~3.4 % low. The planned 060 psi part (4.14 bar) would read **~2.4× too high** with these constants. Also, status bits `10` (stale data: polled faster than the sensor updates) are treated as a wiring error. That is a likely cause of the "invalid SPI response" message: stale data should mean "keep the last value" (with a staleness timeout). | `Trustability_ABP_Gage.cpp` | **Blocking** | FW3 |
| **F28** | **The air push can't empty a wet Sterivex membrane.** Air does not cross a wetted 0.22 µm membrane below its bubble point (≈ 3.4 bar or more for PES; check the datasheet), so the "empty the Sterivex" step only drains the housing down to the membrane. Its end condition (pressure < 0.8 bar) sits *above* the PID target (0.4 bar), so it is met almost immediately: in practice it is a ~5 s timed step. The residual water, which dilutes the shield, is unknown. | `step_sampling()` | **High (data quality)** | LA3, FW4 |
| **F29** | After the cycle, **the O-ring is unlocked and the used Sterivex stays open** on the manifold side and the outlet side for days. The shield may drain or evaporate, and the rotor face passes over used filters during later rotations. Retention and cross-talk are unmeasured. | `sample_process()` | **High (data quality)** | LA3, LA4 |
| **F30** | **The purge (100 mL) is smaller than the dead volume** of a realistic intake: 5 m of 6 mm ID = 141 mL, plus the pump, sensors and tubing. Each sample would include hours-old water from the hose and residue from the previous sample. | `Settings.h` | **High (data quality)** | FW4, LA4 |
| **F31** | `Flow_sensor::begin()` takes the calibration factor as `uint8_t`: a calibrated 81.7 becomes 81 (up to ~1.2 % volume error), and a factor ≥ 256 wraps around | `Flow_sensor.cpp/.h` | Medium | FW4 |
| **F32** | Power: the Due must not see raw 24 V or a 29 V LiFePO4 pack (§3.8). Some Due boards don't boot reliably on power-up without a reset press: test cold boots. The current power topology is not documented. | hardware | **High (field)** | HW1, HW5 |
| **F33** | 9600 baud limits telemetry (~1 line of 100 characters per 0.1 s at best). The Native USB port removes the limit. | `platformio.ini`, `C_output.cpp` | Medium | FW7 |
| **F34** | The A4988 stays **enabled for the whole time the O-ring is locked** (holding current), which costs energy and heat. **The lead screw self-locks (confirmed)**, so the coils can be disabled once locked. | `Oring_lock.cpp` | Medium (battery) | FW4 |
| **F35** | Field intake: no strainer. Debris, algae or quagga mussel larvae can jam the turbine flowmeter or the pump valves, and the intake biofouls over weeks. The outlet must discharge away from the intake. | hardware | **High (field)** | HW6 |
| **F36** | SPI bus: the ABP (≤ 800 kHz) and the AMT22 encoder (its own timing rules) share the bus; long or unshielded cables to a sensor on the fluid line cause bad frames. Also check the AMT22 MISO logic level against the Due's 3.3 V limit. | wiring | Medium | HW1, FW3 |

---

## 5. Test strategy

**Every step ships tests.** A step without tests is not done.

| Level | What | Tool | When it runs | Introduced in |
|---|---|---|---|---|
| 1. **Firmware unit tests** | Pure logic in `src/Core/` (no `Arduino.h`): math, decisions, state machines, parsers | PlatformIO `native` env + Unity: `pio test -e native` | Every commit (CI) | FW0 |
| 2. **Host unit tests** | Domain logic, persistence, the scheduler with an injectable clock | `pytest` | Every commit (CI) | HO1 |
| 3. **Contract tests** | The same `protocol/vectors.json` checked by the C++ and the Python code | both of the above | Every commit | FW1, FW7, HO1 |
| 4. **Simulation tests** | The host against `FakeController` (virtual time, injected faults) | `pytest` | Every commit | HO1 |
| 5. **On-device checks** | Console commands that exercise real hardware and print `TEST\|<name>\|<id>\|PASS/FAIL/SKIP\|k=v...` | `tools/hw_test_log.py` captures them to `test_logs/<date>_<step>.csv` | After every flash, per the step's hardware checklist | FW2 |
| 6. **Acceptance campaigns** | Scripted runs with pass criteria (VA1, VA3) | host CLI + checklists | Before each pilot | VA1 |
| 7. **Field checklists** | Pilot procedures | paper/markdown | On site | VA2 |

Conventions:
- The pure logic lives in `src/Core/`. Hardware classes stay thin and call it. When a safety decision is made (cap, interlock, stop condition), **the decision is a pure function with a truth-table test**.
- **Boundary tests** for every threshold: just below, at, and just above it; NaN; sensor absent.
- **Any slot count:** geometry-dependent tests also run with 16 and 24 positions.
- **Golden values:** before moving math out of a hardware file, record its current outputs and assert them bit for bit.
- **Hardware checklists** say what "PASS" looks like in numbers (e.g. "encoder error < 1°", "volume within ±5 % of the cylinder").
- CI (GitHub Actions): `pio run -e due`, `pio test -e native`, `pytest host/`.
- `docs/TESTS_PENDING.md` lists tests blocked by missing hardware, with the exact commands to run later.

---

## 6. Rules every step follows (copied into CLAUDE.md in FW0)

1. **Behavior changes only in steps that say so**, each listed in the PR description.
2. **Nothing is deleted without evidence and the user's approval.** Git tag `baseline-v0` keeps everything.
3. **Pure logic in `src/Core/`**, unit-tested with `pio test -e native`. Hardware code stays thin.
4. **Safety invariants are never weakened:**
   - The pump never runs above `PRESSURE_CAP`, and an implausible `P_in` stops it.
   - No manifold rotation while locked, or while the lock state is unknown.
   - Every procedure has a timeout.
   - STOP and E-stop are always accepted.
   - Safe state = pumps off, valves closed, O-ring stays locked.
   - Recovery is required after every stop and every boot (from FW6).
   - Placeholders stay marked `PLACEHOLDER`.
   - The only exception is FW2's sensorless pump test mode: explicit, test-only, low power, time-limited, confirmed at every run, unreachable from production code.
5. **Degraded modes are explicit.** When a non-safety sensor fails, keep working, raise a warning, and flag the sample record. Never fail silently.
6. **No hard-coded slot counts in new code.**
7. **Every step ends with:** `pio run -e due` and `pio test -e native` (and `pytest` once `host/` exists) passing; a hardware checklist with numeric pass criteria; `docs/TESTS_PENDING.md` updated.
8. **eDNA quality rules (§3.7)** are requirements, not options: purge volume, blanks, and recorded `end_reason`.
9. Claude does not flash, commit, push or delete files without asking.

---

## 7. Step plan

`Wk` = target week. Tick the "Done" column when merged and the hardware checklist has passed.

| ID | Title | Track | Pri | Depends on | Hardware needed | Main unit tests | Wk | Done |
|---|---|---|---|---|---|---|---|---|
| **FW0** | Baseline: CLAUDE.md, native test env, CI, PINOUT.md, command list | FW | P0 | — | — | Manifold math golden values | 1 | ☑ |
| **FW1** | Protocol v1 spec (docs + vectors) | FW | P0 | FW0 | — | (vectors used later) | 1–2 | ☐ |
| **FW2** | Mechanical test suite + lock tri-state + `TEST\|` logging | FW | P0 | FW0 | — | Slot iteration, stop decision, may-rotate, may-home | 2 | ☐ |
| **FW3** | Pressure sensor: fix the ABP driver, switch safety + PID to `P_in` | FW | P0 | FW0, HW1, HW4 (for the checklist) | an ABP on the line: **none installed as of FW0** (F50) | Count→bar (both ranges), status bits, plausibility | 3 | ☐ |
| **FW4** | Safety + sample-quality fixes: cap, auto-unlock, 2 L, timeout, purge = 3 × dead volume, shield time, flow factor | FW | P0 | FW2, FW3 | calibrated `Q_in` | Pressure decision, interlock, timeout, purge volume | 4 | ☐ |
| **FW5** | Supervisor `service()`, `wait_ms()`, watchdog | FW | P0 | FW4 | — | Supervisor with a fake clock and sensors | 5 | ☐ |
| **FW6** | STOP, safe state, fault latch, RECOVER, boot self-check, lean modes | FW | P0 | FW5 | E-stop input optional | Latch, recovery order, modes table, boot paths | 5 | ☐ |
| **FW7** | Protocol v1 in firmware on Native USB; boot into host mode | FW | P0 | FW1, FW6 | 2nd USB cable | Vectors, parser fuzzing, dispatcher, id dedupe | 6 | ☐ |
| **FW8** | Sample record (metrics + `end_reason`) + minimal clog end condition | FW | P0 | FW7 | — | Metrics accumulator, clog rule, phase gating | 6 | ☐ |
| **FW9** | Unattended hardening: E-stop input, leak sensor, water temperature, home switch | FW | P1 | FW8, HW7, HW8 | sensors | Debounce, sensor plausibility, absent-sensor behaviour | 9–10 | ☐ |
| **HO1** | Host skeleton: protocol client, transport, simulator, CLI, device logs | HO | P0 | FW1 | — | Vectors, client vs simulator, reconnect, dedupe | 2–3 | ☐ |
| **HO2** | Slots, deployments, sample records, CSV export | HO | P0 | HO1 | — | Slot transitions, persistence, export golden file | 3–4 | ☐ |
| **HO3** | Mission runner + scheduler + cold-boot resume | HO | P0 | HO2 | — | Simulated-time scenarios | 4–5 | ☐ |
| **HO4** | Deploy on the Pi (systemd, udev, clock, backups) | HO | P1 | HO3 | Pi + TOFU | Resume after power cut (integration) | 7–9 | ☐ |
| **HO5** | Remote access + alerts lite (Tailscale, LTE, ntfy/Telegram, daily report) | HO | P1 | HO4 | SIM, antennas | Alert rules, dedup, repeat until ack | 9–11 | ☐ |
| **LA1** | Lab alignment → `docs/LAB_INTERFACE.md` | LA | P0 | — | — | — | 1 | ☐ |
| **LA2** | SOPs: filter loading/unloading, cleaning, labels, chain of custody | LA | P0 | LA1 | — | — | 2 | ☐ |
| **LA3** | Shield volume, residual water, shield retention (weighing) | LA | P0 | FW4, LA1 | scale 0.01 g | Analysis script tests | 4–5 | ☐ |
| **LA4** | Carry-over and slot cross-talk (dye tracer ± qPCR) | LA | P0 | FW4, HW6 | dye, spectrophotometer or phone photometry | Analysis script tests | 5–6 | ☐ |
| **HW0** | Order everything (§11.1) | HW | P0 | — | — | — | 0–1 | ☐ |
| **HW1** | Electrical audit → `docs/PINOUT.md`, `docs/POWER.md` | HW | P0 | FW0 | multimeter, scope if available | — | 1 | ☐ |
| **HW2** | Relief valve, E-stop in the 24 V line, fuses | HW | P0 | HW1 | parts | — | 2–3 | ☐ |
| **HW3** | Flowmeter: level-safe input, calibration, real flow measurement | HW | P0 | HW1 | cylinder, scale | — | 2 | ☐ |
| **HW4** | Install and plumb the pressure sensor(s) | HW | P0 | HW0 | sensors, clamps | — | 3 | ☐ |
| **HW5** | Power: battery, regulated rails, measured budget, cold-boot test | HW | P0 | HW1 | battery, DC-DCs, meter | — | 4 | ☐ |
| **HW6** | Field fluidics + splash-proof housing for Pilot A | HW | P0 | HW3 | strainer, hose, box | — | 5 | ☐ |
| **HW7** | O-ring lock endurance (coils off while locked); "unlocked" home switch (P1) | HW | P0/P1 | FW2 | switch | — | 3 / 9 | ☐ |
| **HW8** | Pilot B: enclosure, leak sensor, temperature probe, mounting | HW | P1 | HW6 | parts | — | 8–12 | ☐ |
| **VA1** | Bench acceptance campaign (go/no-go for Pilot A) | VA | P0 | FW8, HO3, LA3, LA4, HW2–HW6 | full machine, battery | — | 7 | ☐ |
| **VA2** | **Pilot A** | VA | P0 | VA1 | site | — | 8 | ☐ |
| **VA3** | 72 h soak on battery + Pilot B readiness | VA | P1 | all P1 | full machine | — | 12–14 | ☐ |
| **VA4** | **Pilot B** | VA | P1 | VA3 | site | — | 14–16 | ☐ |

---

## 8. Prompts

> Copy each block into a **new** Claude Code session, on a new branch. Fill in `<...>`. After FW0, every prompt starts by reading `CLAUDE.md`.

### 8.1 Firmware track

#### FW0: Baseline, test harness, CI, pin map (P0)

```text
Project: CoWaS, an autonomous aquatic eDNA sampler (GenoRobotics), heading to a supervised pilot on
Lake Geneva in ~8 weeks. A diaphragm pump (24 V, driven from DAC1 + an enable relay on D42) pushes
lake water through Sterivex filters on a rotating manifold (slot 0 = purge, 1..14 = samples, FILL_ORDER
in Settings.h; AMT22 SPI encoder, CS D38; DC motor on a VNH5019 shield). An O-ring seals the manifold,
locked/unlocked open-loop by a NEMA17 + A4988 linear actuator (D9/D10/D11). A micro pump (D48) adds
DNA/RNA Shield. An Arduino Due (PlatformIO env `due`) controls everything. A legacy Raspberry Pi
script (computer_interface/, PyQt5 + ZMQ) sends text commands. The spool (depth) is broken
(SPOOL_USE=false).

Read docs/ROADMAP.md fully first (especially §3, §4 findings, §5 test strategy, §6 rules).
This step must NOT change firmware behavior.

1. CLAUDE.md at the repo root: a 10-line project summary, the hardware/pin list, build and test
   commands, the rules of ROADMAP §6 verbatim, the known hazards (placeholders such as
   LINEAR_ACT_LOCKED_STEPS; uncalibrated flowmeter; safety reads the uninstalled A2 sensor, F7/F22;
   the setup() test-menu block, F23; spool disabled), and a pointer to docs/ROADMAP.md.
2. A PlatformIO `native` env with Unity and build_src_filter so src/Core/ builds natively without
   Arduino.h. Move ONLY pure manifold math into src/Core/ (clock_minutes_for_slot,
   slot_for_clock_minutes, scale_angle, the slot-angle table from Manifold::begin() as a pure
   function). Record the current outputs as golden values BEFORE moving anything and assert them
   bit for bit. Tests: the golden angle table (purge at index 0, the no-hole position never
   produced), the slot<->clock round trip, FILL_ORDER is a permutation of the sample slots, NB_SLOT
   matches the table, and the same builders with 16- and 24-position geometries.
3. .github/workflows/ci.yml: `pio run -e due` + `pio test -e native` (+ a placeholder pytest job
   that is skipped while host/ doesn't exist).
4. docs/PINOUT.md from Settings.h and the code: every Due pin (D0-D53, A0-A11, DAC0/1, SPI, I2C,
   CAN), what uses it (file:line), direction, the expected voltage level, and EMPTY columns for me:
   "reachable on the PCB shield?", "measured level", "notes". Flag declared-but-unused and
   used-but-undeclared pins. Replace the "all pins are claimed" comments in Settings.h with a
   pointer to PINOUT.md (comment-only change).
5. docs/COMMANDS.md: every console command of test_all_components() (including the prefix
   commands slotN/clockMM), every button_control() mode and every main_program() command:
   name | function (file:line) | hardware touched | blocking? | needs the pressure sensor?
6. docs/TESTS_PENDING.md (empty template: test | blocked by | exact commands | pass criteria).
7. Report only (no fix): does `pio run -e uno` build; does include/interfaces exist (F12)?

Done when: `pio run -e due` builds, `pio test -e native` passes, and the firmware diff is only the
math extraction + comments. Give me a 5-minute hardware smoke test to run before I tag `baseline-v0`.
```

#### FW1: Protocol v1 specification (docs only) (P0)

```text
Read CLAUDE.md, docs/ROADMAP.md §3 (architecture, safety, link, modes, slots, sample record, eDNA
rules), findings F8, F23, F24, F33, and docs/COMMANDS.md.
Write docs/PROTOCOL.md and protocol/vectors.json. NO code. The host team starts from this document
next week, so keep it small, precise and extensible. This is v1 for the lake pilots.

- Transport (DECIDED, ROADMAP §3.4): HOST = Native USB (SerialUSB), CONSOLE = Programming port
  (Serial). One JSON object per line, UTF-8, max line length (propose one). On the host channel,
  every human log line is ALSO sent as {"t":"log","lvl":..,"msg":..}. Never open the Native port
  at 1200 baud.
- Message types: req {t:"req", id, cmd, args}; ack {t:"ack", id} sent at once for long commands;
  res {t:"res", id, ok, data | err:{code,msg}}; evt {t:"evt", type, ...} (mode change, fault,
  warning, procedure step start/end, phase change); tel {t:"tel", ...} at ~1 Hz while BUSY
  (P_in, flow, volume, phase, pump %, step); log.
- Commands (P0): HELLO (protocol version, firmware git hash, mode, recovery_required + cause,
  capabilities {p_in, p_out, q_in, q_out, estop_input, spool, water_temp, ...}), PING, STATUS,
  STOP (always accepted, any mode), CLEAR_FAULT, RECOVER {confirm_moving: bool}, GET_INFO (the
  manifold geometry: positions, sample slots, purge slot, FILL_ORDER, clock labels; the host must
  never assume 14), ACTUATOR_HOME, ROTATE {slot}, LOCK, UNLOCK, PRIME_SHIELD, and the main one:
  RUN_CYCLE {slot, purge_ml, volume_ml | null, max_duration_s, stages: {purge, sample, shield,
  empty}} = the production sequence purge -> sample -> shield -> empty -> unlock for one filter.
  Also PURGE, SAMPLE, DNA_SHIELD, EMPTY as separate commands for service use.
  Reserved (answer ERR not_available): SPOOL_*, NEG_CONTROL, POS_CONTROL, LEAK_TEST.
- RUN_CYCLE result (every field nullable when its sensor is absent): slot, volume_ml,
  duration_s, end_reason (volume_reached | clogged | timeout | pressure_cap | fault | stopped),
  p_in {max, mean, final}, flow_ml_min {mean, min, final}, purge_ml_measured, phases [{name,
  duration_s}], shield_ms, t_end_to_shield_s, faults[], warnings[], sensors[] {name, value, unit,
  stat}, and the reserved nullable fields p_out, q_out, dp, r_filter, clog_index (for P2).
- Rules: which commands each mode accepts (table); error codes (table); request ids are
  remembered (last 16) so a retried id never runs a cycle twice (it returns the stored result);
  link loss: the Due finishes or aborts the current procedure safely and never starts a new one;
  a host heartbeat (PING) period and what the Due does when it stops.
- Timing: expected durations per command and the host timeouts to use.
- A mapping table: every legacy main_program() command -> its v1 equivalent (or "dropped: why").
- protocol/vectors.json: 30+ request/response examples, including malformed JSON, unknown
  commands, wrong arg types, commands refused in the wrong mode, a retried id, and results with
  null fields. Both the C++ and the Python tests will load this file.
Ask me the open questions at the end instead of guessing.
```

#### FW2: Mechanical test suite, lock tri-state, test logging (P0)

```text
Read CLAUDE.md, docs/ROADMAP.md §5 (test strategy), findings F5, F16, F17, F18, docs/COMMANDS.md.
Also read src/Tests.cpp (the existing tests: linear_actuator, sample_cycle, sample_cycle_full,
multi_sample, multi_sample_full, calibrate_actuator, slotN/clockMM), src/Step_functions.cpp
(load_slot, load_slot_full, goto_slot_locked, sample_process, step_*) and src/Hardware/Oring_lock.cpp.

Current hardware: pressure sensor <NOT trusted yet (FW3 pending) | working>. Flowmeter <uncalibrated
| calibrated factor X>.
Goal: a reliable set of serial test commands to validate locking, sealing and rotation NOW, and
prepare the pumping tests. ADDITIVE: existing commands keep working, EXCEPT the lock tri-state.

0. Audit first: print a table "required test | exists / partial / missing | what it reuses". REUSE
   the existing procedure functions; don't duplicate logic.
1. Test output convention (ROADMAP §5): every test prints one machine-readable line per result:
   `TEST|<name>|<id>|<PASS/FAIL/SKIP>|<key=value;...>` plus a human summary table at the end.
   tools/hw_test_log.py (pyserial, cross-platform): opens a port, lets me type commands,
   mirrors everything to the screen, and appends the TEST lines to test_logs/<date>_<tag>.csv.
   pytest for its line parser.
2. Sensorless pump test mode (F17): PRESSURE_SENSOR_AVAILABLE in Settings.h. When false, every
   procedure that needs pressure REFUSES with a clear message. pump_open_loop_test(power,
   volume_ml, max_ms): a fixed power clamped to TEST_OPENLOOP_MAX_POWER (PLACEHOLDER, ask me);
   it stops on volume, max_ms, no flow pulses for 5 s, or any keypress, and returns the reason +
   volume. Detaching the pressure safety is allowed ONLY inside this helper, after I type NOSENSOR
   at every run, with a loud warning; it is re-attached on every exit path. Prove by grep that
   no production path can reach it.
3. O-ring lock state (F18): tri-state LOCKED / UNLOCKED / UNKNOWN, UNKNOWN at boot. `actuator_home`:
   only if the manifold is aligned to a slot (encoder within tolerance), run the overdrive lock
   (a stall against the physical stop = a known reference), then the normal unlock -> known
   UNLOCKED. Otherwise refuse. rotateMotor() and every rotating test refuse while LOCKED or
   UNKNOWN (the automatic unlock comes in FW4). Tell me how production paths behave on the first
   rotation after a boot; ask me before changing sample_process().
4. Test commands. Each one: 'x' aborts at every pause; optional `auto` (no pauses);
   `from=N count=M`; FILL_ORDER by default or `order=index`; no hard-coded 14/15; a summary table.
   a. lock_cycle [n]: lock/unlock n times at the current slot (default 5). Also measure and print
      the time per lock and unlock.
   b. lock_endurance [n] (for HW7): n cycles (default 200), with the encoder angle checked every
      10 cycles, and the driver temperature asked every 50 (y/n "too hot to touch?").
   c. rotate_all [dry]: per slot: rotate -> encoder error -> lock -> hold N s -> unlock.
   d. encoder_repeat [slot] [n]: go away and come back n times; print the angle spread.
   e. valve_check: each valve on/off with a y/n confirmation of click/flow (V1 and Vman; V23 is
      physically removed, F51 - its code goes in FW4).
   f. leak_hold [mL|s]: lock -> pump (sensorless mode, or normal once FW3 is done) -> "leak? y/n +
      where" -> air push -> unlock.
   g. multi_sample (extend the existing one, default behaviour unchanged): sensorless variant when
      PRESSURE_SENSOR_AVAILABLE is false; volume per slot; the options above.
   h. full_cycle [slot] purge=y/n shield=y/n empty=y/n volume=<mL>: the REAL production step
      functions in order; full_all runs it over the slots. They refuse while the sensor is not
      available, but must compile and be complete.
   i. shield_only [slot] [ms]: micro pump only (time-based); CAN run sensorless.
   Add all of them to the menu and to `help`.
5. Pure logic in src/Core/ + native tests: the slot iteration list builder (from/count/order, any
   slot count, FILL_ORDER validated as a permutation), the open-loop stop decision, the "may
   rotate?" rule, and the "may home?" rule. Every combination; 15 and 24 positions.
6. docs/TESTS_PENDING.md: every test that needs the pressure sensor, with exact commands and pass
   criteria.
Don't touch the pump PID. Don't change sample_process() without asking.
Hardware checklist (in order, safe first), each with a numeric PASS: actuator_home -> lock_cycle 5
-> rotate_all dry (encoder error < <1>°) -> encoder_repeat (spread < <0.5>°) -> valve_check ->
leak_hold small volume -> multi_sample auto count=3 -> shield_only.
```

#### FW3: Pressure sensor, correct driver, safety on `P_in` (P0, critical path)

```text
Read CLAUDE.md, docs/ROADMAP.md §3.3, findings F7/F22, F21, F27, F36, docs/TESTS_PENDING.md,
include/Trustability_ABP_Gage.h, src/Hardware/Trustability_ABP_Gage.cpp,
src/Hardware/Pressure_sensor.cpp, main.cpp (read_pressure2, set_pressure_safety), Pump.cpp,
docs/FIRMWARE_LOG.md (F44, F50).
Installed: P_in = Honeywell <ABPDANV150PGSA3 | ABPDANV060PGSA3 | NONE YET> on SPI, CS D8, 3.3 V,
between the pump and the manifold. Cable length to the sensor: <N> cm. P_out: <not installed |
ABPDANV060PGSA3 on CS D45>. (As of FW0 no sensor is installed at all, F50: the A2 analog sensor was
removed. If still NONE, do items 1-3 and 6 with native tests and leave the checklist in
docs/TESTS_PENDING.md.) Before starting, ask me for the result of the pending "Is the spare 150 psi
ABP alive?" check in docs/TESTS_PENDING.md (pin identification + wiring + `pressure_sensors`); if it
hasn't been done, walk me through it first.

0. Naming and dead code (F50, rule 2: the user approved the removal on 2026-09-24): `pressure1`
   (Trustability ABP, SPI) becomes `p_in` everywhere. The analog sensor on A2 is physically gone:
   delete BigPressure/SmallPressure (Pressure_sensor.*), `pressure2`, `read_pressure2()`,
   `pressure_2_pin`, `pressure_3_pin` and the `calibrate_pressure2` command (F44), after listing
   every call site. The `pressure` / `pressure_sensors` commands show P_in only.

1. Driver correctness (F27):
   - The range comes from the part number, as a constructor argument (060PG = 0-60 psi, 150PG =
     0-150 psi). Transfer function A: 10-90 % of 2^14 counts (1638..14745). psi -> bar
     (1 psi = 0.0689476 bar). Remove the hard-coded 7.63e-4 / 1.25 (they assume 10.0 bar).
   - Status bits: 00 normal; 10 STALE = no new conversion since the last read -> keep the last
     value, and only if nothing fresh arrives for STALE_TIMEOUT_MS is it a failure; 01 (command
     mode) and 11 (diagnostic fault) -> IMPLAUSIBLE. Mask the status bits before converting.
   - Diagnose the "invalid SPI response" message BEFORE blaming the sensor: log the raw status-bit
     histogram over 1000 reads at the current poll rate; try 100/400/800 kHz; check the CS
     timing and that the AMT22 transactions (encoder) never overlap. Give me a wiring checklist
     (3.3 V at the sensor pins, a common GND, MISO/SCK on the SPI header, cable length, a
     100 nF decoupling capacitor close to the sensor).
   - `calibrate_pressure`: pump off, lines vented, a 5 s average -> zero offset, printed as a
     Settings.h value (PRESSURE_ZERO_OFFSET_BAR).
2. Switch ALL safety and control to P_in (F7/F22): set_pressure_safety(...) and every
   CtrlPumpFlow/CtrlPumpNoWater::begin() call get a read_p_in() function. List every call site.
   Nothing may read A2 any more (item 0 deletes it).
3. Plausibility window for the installed range (below -0.3 bar or above range + 10 % =
   implausible -> the pump stops). NaN = implausible.
4. If P_out is installed: read-only display in the console `pressure` command (P_in, P_out, dP,
   raw counts, status). Not used for control or safety (P2).
5. PRESSURE_SENSOR_AVAILABLE = true. FW2's sensorless mode must refuse while a sensor is
   available, unless there is an explicit override.
6. Native tests: the count -> bar conversion at the datasheet points for BOTH ranges (1638 -> 0,
   14745 -> full scale, the midpoint), the status-bit handling including the stale timeout, the
   plausibility bounds (just below/at/above), and that the old constants are gone.
Then walk me through docs/TESTS_PENDING.md in a safe order: first a cap-trip test with a
restrictor/clamp at the filter outlet, pump at low power, CAP temporarily at 1.0 bar (it must trip
at 1.0 ± 0.1 bar); then full_cycle on one slot. Update the file with the results I report.
```

#### FW4: Safety and sample-quality fixes (P0)

```text
Read CLAUDE.md, docs/ROADMAP.md §3.3, §3.7, findings F2, F3, F4, F5, F6, F15, F28, F30, F31.
SMALL, listed behavior changes. Nothing else.

1. One safety value (§3.3): PRESSURE_CAP = 2.5. pressure_target() = PRESSURE_CAP -
   PRESSURE_TARGET_MARGIN (0.5), in src/Core/. static_assert 0.5 <= cap <= 3.0. STX_MAX_PRESSURE
   becomes a deprecated alias. Replace every hard-coded setpoint `2` in the CtrlPumpFlow/
   CtrlPumpNoWater begin() calls with pressure_target(); keep the 0.4 emptying setpoint. List the
   call sites.
2. F2: in loop(), while pump.is_running(), call pump.enforce_pressure_safety() every iteration.
   List any other place where a pump can run without polling (they get fixed properly in FW5).
3. The cap decision (reading, cap, bounds -> OK / OVER_CAP / IMPLAUSIBLE) is a pure src/Core/
   function with boundary tests.
4. F5, automatic unlock before rotation: LOCKED and safe (pumps off; P_in < UNLOCK_MAX_PRESSURE)
   -> unlock, log, rotate. LOCKED and not safe -> wait up to N s, then refuse. UNKNOWN -> refuse,
   ask for actuator_home. "Already at target" must NOT unlock (step_DNA_shield relies on it).
   Check EVERY caller (goto_slot_locked, slotN, clockMM, slot0, panel Man Slot, calibrations, the
   FW2 tests); report before/after. The pure rule (ROTATE / UNLOCK_THEN_ROTATE / WAIT / REFUSE) is
   tested for every combination.
5. Volume and timeout (F15, F6, F31):
   a. STX_SAMPLE_MILLILITERS = 2000; SAMPLE_VOLUME_CAP_ENABLED = true.
   b. The Flow_sensor calibration factor becomes a float (pulses per litre as float); fix the
      uint8_t/uint16_t truncation. Keep the behaviour identical for the current 77.0 factor
      (a test proves it).
   c. step_sampling()'s timeout = sample_max_duration_ms(volume_ml) in src/Core/:
      volume / SAMPLE_MIN_EXPECTED_FLOW_ML_PER_MIN x 1.5, bounded by SAMPLE_MAX_DURATION_ABS_MS.
      SAMPLE_MIN_EXPECTED_FLOW_ML_PER_MIN = <value measured in HW3, or PLACEHOLDER>.
   d. CtrlPump::run() returns WHY it stopped (condition met / timeout / pressure cap /
      implausible / aborted). step_sampling() warns loudly on a timeout; it must never look like
      a normal completion. Keep the reason available for FW8.
6. Purge volume (F30, §3.7): INTAKE_HOSE_ID_MM, INTAKE_HOSE_LENGTH_M, INTERNAL_DEAD_VOLUME_ML
   (PLACEHOLDER, measured in LA4), PURGE_DEAD_VOLUME_FACTOR = 3. purge_volume_ml() = max(
   PURGE_MIN_ML, factor x (pi x (ID/2)^2 x L + internal)). step_purge() uses it, and prints it.
   Check the purge timeout still fits the purge volume at the minimum expected flow.
7. DNA shield (F4): remove "/ 10." and use DNA_SHIELD_FILL_MS = <value from LA3 calibration, or
   FILL_STERIVEX_TIME until then>. Check nothing relied on the short time.
8. Air push (F28): do NOT change the logic yet, but log the pressure and duration of each
   emptying phase so LA3 can decide. Add EMPTY_STX_MIN_MS / EMPTY_STX_MAX_MS params (defaults =
   today's behaviour).
9. Stepper coils off while locked (F34; the lead screw self-locks, confirmed): lock_oring()
   disables the A4988 after the lock move; unlock_oring() re-enables it before moving. A param
   ORING_HOLD_WHILE_LOCKED = false (true restores today's behaviour). Tell me which callers
   assumed the driver stays enabled.
10. Don't touch DEBUG_MODE_PRINT (FW6).
11. V23 removal (F51, rule 2: the user approved it on 2026-09-24): the 3/2 valve on D40 is
    physically gone (only its relay remains). List every call site of `valve_23` (Step_functions,
    maintenance, Tests, the panel mode V23, test_valves), then delete them and VALVE_23_PIN; D40
    becomes free in docs/PINOUT.md. Report which procedures used it to route water (the
    container era: step_fill_container, purge_pipes_manifold) and ask me whether to delete those
    too (P2 otherwise). Also read docs/FIRMWARE_LOG.md findings F40-F43 and F45: fix the ones this
    step's items already touch, and log the rest.
Native tests: the cap decision; the interlock rule; sample_max_duration_ms (2000 mL at the expected
flow finishes before the timeout; no volume cap -> the absolute maximum; bounds); purge_volume_ml
(the worked example of §3.7 ≈ 424 mL; zero length -> the minimum); the flow factor round trip.
Hardware checklist: calibrate_flow (if HW3 isn't done yet); lock then slot5 auto-unlocks and
rotates; with the pump running, slot5 waits then refuses; after a reboot, slot5 asks for
actuator_home; one full_cycle volume=500: the printed purge volume matches the formula, and the
shield runs DNA_SHIELD_FILL_MS.
```

#### FW5: Supervisor, `wait_ms()`, watchdog (P0)

```text
Read CLAUDE.md and docs/ROADMAP.md §3.2 (safety layers), finding F14.
Goal: what must ALWAYS run keeps running during long procedures.

1. A Supervisor (logic in src/Core/, hardware glue outside) with service(), called at least every
   50 ms whenever anything blocks. It checks the pressure cap and P_in plausibility for any
   running pump, checks the per-procedure deadline, kicks the watchdog, and exposes
   should_abort(). Its time and sensor sources are injected so native tests can fake them.
2. bool wait_ms(ms) calls service() and returns false on abort. Replace delay() in the
   procedure-level code (Step_functions, maintenance, calibrations, tests); keep
   delayMicroseconds and the stepper pulse timing. Add service() calls in: CtrlPump::run, the
   rotateMotor wait loop, Linear_actuator::step_pulses (every N steps, with N chosen so a pulse
   train is not disturbed), Micro_Pump::start(ms), Pump::start(ms), Pump::stop (the 500 ms delay),
   Button::waitPressedAndReleased/waitReleasedAndPressed, and the keypress helpers. List every
   change.
3. Abort propagation: procedures return early when should_abort() is true. For now, stop the pumps
   and log; the full safe state comes in FW6 (leave one on_abort() hook).
4. The Due hardware watchdog: the Arduino SAM core disables it by default through a weak
   watchdogSetup(); override it, and enable once with a <4 s> timeout (it can be configured only
   once per reset). Measure the longest gap between service() calls first (instrument it, print
   the max) and report it. A hidden console command `wdt_test` deliberately hangs.
5. Report the reset cause at boot (the SAM3X RSTC status: power-on / watchdog / software / user).
6. Native tests: the cap trips at the exact threshold; NaN and implausible values trip; the deadline
   trips; the latch holds until cleared; the waits return false once should_abort() is true.
Hardware checklist: wdt_test resets the board and the boot prints "watchdog"; a cap trip during
full_cycle (clamp at the outlet, CAP at 1.0 bar) stops the pump within <200 ms> and the procedure
returns; full_cycle timing unchanged within 5 %; the max service() gap printed < 50 ms.
```

#### FW6: STOP, safe state, fault latch, recovery, boot, lean modes (P0)

```text
Read CLAUDE.md, docs/ROADMAP.md §3.2 (DECIDED safe state and recovery), §3.5 (lean modes), F11, F23.
Hardware: E-stop <switch in series with the 24 V actuator rail, second contact on pin <D37>, NC to
GND with INPUT_PULLUP | switch only, no input yet: ESTOP_INPUT_INSTALLED=false>.

1. enter_safe_state(), idempotent, callable from anywhere: pump + micro pump off, ALL valves
   closed, manifold motor stopped, spool stopped, O-ring STAYS LOCKED if locked (the lead screw
   self-locks; the A4988 stays disabled); if unlocked, don't drive it into a lock; record it.
   A snapshot at stop time: slot, lock state, procedure, step, volume so far.
2. FaultCode enum replacing the Critical_error.cpp stub: OVER_PRESSURE, IMPLAUSIBLE_PRESSURE,
   ESTOP, USER_STOP, TIMEOUT, ENCODER, ROTATION_REFUSED, WATCHDOG_RESET, SENSOR_DEAD, ... A latched
   current fault + a 16-entry ring buffer with millis timestamps. Warnings (non-safety) have their
   own severity and never latch.
3. Lean ModeMachine in src/Core/ (ROADMAP §3.5): BOOT, RECOVERY_REQUIRED, IDLE, BUSY, SERVICE,
   FAULT, ESTOP, with an explicit transition table; illegal transitions are rejected and logged.
   - SERVICE = today's console test menu, entered with `service` (and at boot, see 6), left with
     `exit`. The old `stop` (which left the menu) becomes the emergency STOP; `exit` replaces it.
   - IDLE runs the legacy text protocol (main_program) unchanged until FW7 replaces it.
   - The button panel (button_control) is only active in SERVICE.
4. The E-stop input (if installed): debounced; the ISR only sets a flag; service() acts on it.
   Pressed OR a broken wire = stop (fail-safe).
5. Software STOP: `stop` typed on the console is detected from ANY blocking wait (service() peeks
   at the serial buffer without consuming other input) -> enter_safe_state() + FAULT(USER_STOP).
   The same entry point will serve the protocol STOP (FW7).
6. Boot (fixes F23): setup() no longer ends in test_all_components(). Every boot:
   RECOVERY_REQUIRED; O-ring UNKNOWN; valves driven closed; non-moving self-checks (encoder
   reads, P_in plausible near 0, flow pulses = 0 with the pump off); E-stop pressed -> ESTOP.
   BOOT_DEFAULT = SERVICE for now (the console is shown, as today); FW7 switches it to IDLE.
   Moving recovery steps wait for confirmation (console) or the host's RECOVER {confirm_moving}.
   F11: DEBUG_MODE_PRINT becomes a real #define; before_start_program()/system_checkup() stay
   off at boot (SYSTEM_CHECKUP_AT_BOOT=false) and system_checkup() is exposed as `checkup`.
7. `clear` is refused while the cause persists. `recover` runs the §3.2 checklist step by step,
   printing PASS/FAIL, stops at the first FAIL (if the O-ring is UNKNOWN and the manifold is
   aligned, it runs actuator_home; if not aligned, it re-homes the manifold to the nearest slot
   first). While recovery is required, every procedure and moving command is refused with
   "RECOVERY REQUIRED (cause: ...)", with a console reminder every 30 s. `faults` prints the ring.
8. Native tests: every legal and illegal mode transition; the latch; clear refused while the
   cause persists; recovery ordering and stop-on-FAIL with fake hardware; procedures refused
   while recovery is required; every boot path (normal, E-stop pressed, watchdog reset); abort
   latency < 100 ms in simulation.
Hardware checklist: console STOP and E-stop during: pump, rotation, actuator move, DNA shield,
emptying -> valves closed, O-ring still locked, TEST lines logged; after release, commands are
refused; clear + recover -> normal run; power-cycle mid-lock -> the boot asks for recovery;
20 cold boots from the battery supply (F32) -> 20/20 reach the console.
```

#### FW7: Protocol v1 in firmware, Native USB, boot into host mode (P0)

```text
Read CLAUDE.md, docs/PROTOCOL.md, protocol/vectors.json, docs/ROADMAP.md §3.4-3.5, F8, F10, F24, F33.
1. ArduinoJson (pin the version). The parser/serializer + the dispatch table live in src/Core/
   (native-tested against vectors.json, plus malformed, oversized and binary-garbage lines).
2. Two channels (§3.4): HOST_PORT = SerialUSB (protocol + {"t":"log"} copies of every log line),
   CONSOLE_PORT = Serial (human text, service menu). Route all output through one small Log API
   (level + channel). Settings: HOST_PORT can be set to Serial for a single-cable bench setup
   (then JSON lines start with '{', and the host ignores other lines). Test both.
3. Handlers call the EXISTING procedures with explicit parameters. RUN_CYCLE takes the slot from
   the host (the internal slot search stays only for the legacy commands). Long commands: an
   immediate ack, events for each step, then the result with the same id. BUSY accepts only STOP,
   STATUS, PING, HELLO.
4. STOP from the protocol uses the FW6 path. Request-id dedupe (last 16 ids + their results).
   Link watchdog: no PING for LINK_TIMEOUT_S -> warning event; the current procedure finishes
   safely; nothing new starts until the host is back.
5. Telemetry at ~1 Hz while BUSY via service() (P_in, flow, volume, phase, pump %, step).
6. BOOT_DEFAULT = IDLE (the host mode). The console stays reachable: `service` on the
   Programming port (refused while BUSY). Legacy main_program() commands: keep them behind
   LEGACY_PROTOCOL_ENABLED (default false), and remove them in P2.
7. tools/protocol_smoke.py (pyserial, cross-platform, VID/PID auto-detect of the Native port):
   HELLO, STATUS, GET_INFO, ROTATE 3, LOCK/UNLOCK, STOP mid-PURGE, CLEAR_FAULT, RECOVER, a retried
   request id, NEG_CONTROL -> not_available. A pass/fail summary + TEST lines.
8. Native tests: every vector; fuzzing (random bytes, truncated lines, 10 kB lines) never crashes
   and never runs a command; the mode/command acceptance table; dedupe; link-timeout logic.
Hardware checklist: protocol_smoke.py over the Native USB port: all PASS; restarting the host
program during a PURGE does NOT reset the Due; with a laptop on the Programming port, the console
still works; boot -> RECOVERY_REQUIRED -> host RECOVER -> IDLE.
```

#### FW8: Sample record and minimal clog end condition (P0)

```text
Read CLAUDE.md, docs/PROTOCOL.md (the RUN_CYCLE result), docs/ROADMAP.md §3.6-3.7, F15, F28, F30,
docs/LAB_INTERFACE.md (the fields the lab needs), and HW3's measured flow: <mL/min clean filter>.
1. A SampleMetrics accumulator in src/Core/ fed by service(): volume, duration, p_in
   {max, mean, final}, flow {mean, min, final}, per-phase durations, measured purge volume,
   shield_ms, t_end_to_shield_s, faults, warnings. Phase-aware: a pumping phase variable
   (PRIME, PURGE, WATER, AIR_PUSH, SHIELD, EMPTY, IDLE) is set by every step function. Volume
   and flow statistics count ONLY in WATER (turbine meters spin on air). Stats ignore a short
   blanking window after each phase change. Fill the RUN_CYCLE result (FW7); nullable fields
   stay null.
2. Minimal clog end condition (WATER phase only; the volume stop and the timeout stay
   independent): after a settling time, if the flow < CLOG_FLOW_MIN_ML_MIN (a param,
   PLACEHOLDER = <30 %> of the flow measured in the first minute of this sample, with an absolute
   floor) for CLOG_T_S (30 s) while the pump is at >= 95 % or P_in >= target - 0.2 bar ->
   end_reason = clogged. If the flowmeter goes silent (below its rated minimum) while P_in is at
   target, that also counts as clogged, and the warning says so. The first condition met sets
   end_reason.
3. The console `last_sample` prints the last record (it survives until the next cycle only;
   the host is the store).
4. Native tests: the accumulator on synthetic series (water -> air push -> shield) where air adds
   no volume; stats per phase; each end_reason (volume, clogged, timeout, pressure cap, stopped);
   no false clog in the first minute; a slow, steady flow is not clogged.
Hardware checklist: RUN_CYCLE 500 mL clean filter -> end_reason=volume_reached, volume within 5 % of
the weighed outlet water; RUN_CYCLE with the outlet clamp slowly closed -> end_reason=clogged;
the air push adds < 5 mL to the volume.
```

#### FW9: Unattended hardening (P1)

```text
Read CLAUDE.md, docs/ROADMAP.md §3.2, §3.8, F35, docs/POWER.md, HW7/HW8 results:
home switch on <D39>, leak sensor on <D41/A6>, DS18B20 water probe on <D43>.
1. E-stop input fully wired (if not done in FW6); capability flag in HELLO.
2. Enclosure leak sensor: debounced; FAULT LEAK_ENCLOSURE (safe state + event).
3. O-ring home switch at "unlocked" (Hall or micro-switch, INPUT_PULLUP): actuator_home becomes
   "move toward unlocked until the switch, then back off"; lock/unlock verify the switch state
   (unlock must reach it within N steps, else FAULT ACTUATOR). ORING_CONTACT_BUTTON_INSTALLED
   stays for a future contact sensor.
4. Water temperature (DS18B20, 1-Wire at 3.3 V): the per-sample mean in sensors[]; an absent or
   implausible sensor -> warning only; also logged every 30 min between samples.
5. Native tests: debounce, fail-safe on a broken wire, the home-switch logic (found / not found /
   stuck), the temperature conversion and plausibility (-5..40 °C), absent-sensor behaviour.
Hardware checklist: each sensor unplugged -> the expected warning or fault; 50 lock cycles with the
home switch; a water-temperature reading vs a reference thermometer (within 0.5 °C).
```

### 8.2 Host track

#### HO1: Host skeleton, protocol client, simulator, CLI (P0)

```text
Read CLAUDE.md, docs/ROADMAP.md §3.1, §3.4, §5, docs/PROTOCOL.md, protocol/vectors.json, and
computer_interface/ (legacy; do not modify it, findings F9 and F24 explain why it is replaced).
host/: a Python 3.11+ package `cowas`, cross-platform (Windows laptop for Pilot A, Raspberry Pi
later). No PyQt, no ZMQ ipc://, no Pi-specific imports outside host/src/cowas/adapters/.
1. pyproject.toml, src layout, ruff, pytest, logging config. CI runs pytest (edit ci.yml).
2. cowas.protocol: pydantic models from PROTOCOL.md; tested against every vector.
3. cowas.transport: pyserial, auto-detects the Due's Native USB port by VID/PID (never 1200 baud),
   reconnects. '{' lines -> protocol; other lines and {"t":"log"} -> the device log (timestamped
   UTC, rotated files under data/logs/).
4. cowas.client: request/response with ids, acks, per-command timeouts (from PROTOCOL.md),
   retry-safe resend (the same id), an event/telemetry subscription, a heartbeat PING.
5. cowas.sim.FakeController: implements the protocol with virtual time and injectable faults:
   pressure trip, clog, slow procedure, link drop, a reboot mid-cycle (RECOVERY_REQUIRED after),
   a flowmeter dead, a watchdog reset. It emits human log lines too and plugs in as a transport.
6. CLI `cowas`: ports, hello, status, stop, recover, rotate, lock, unlock, run-cycle --slot
   --volume|--no-volume-cap, monitor (live telemetry + events + logs), logs. `--sim` everywhere.
7. Tests: vectors; client vs simulator (timeouts, reconnect, dedupe on retry, STOP during
   RUN_CYCLE, the log separation, a reboot mid-cycle is detected via HELLO).
Bench check (after FW7): `cowas hello` and `cowas monitor` against the real Due.
```

#### HO2: Slots, deployments, sample records, CSV export (P0)

```text
Read CLAUDE.md, docs/ROADMAP.md §3.6, §3.7, finding F1, docs/LAB_INTERFACE.md, host/.
1. cowas.domain: SlotState (EMPTY, READY, IN_PROGRESS, SAMPLED, FAILED, DISABLED, SUSPECT) with an
   explicit transition table; the inventory is sized from GET_INFO (never 14). Deployment (id,
   site name, lat/lon entered by hand, intake depth m, intake hose ID/length, operator, filter lot,
   shield lot, start/end). SamplingEvent (planned time, volume or none, replicates 1-3, role:
   sample | equipment_blank). Filter (event, role, replicate index, slot, sample_id).
2. sample_id format: <device>-<deployment>-<slot>-<yyyymmddThhmm> (or what LA1 decided); printed
   labels (tools/labels.py -> a PDF or CSV for a label printer) from the same ids.
3. cowas.store: SQLite, a repository layer, a schema version + migrations, one transaction per
   transition. At startup, IN_PROGRESS -> SUSPECT.
4. The reload flow: the operator confirms which slots got fresh filters (and the filter lot);
   the previous deployment is archived; those slots become READY; DISABLED keeps its reason.
5. A SampleRecord per filter = the RUN_CYCLE result + event + deployment + versions + notes.
   Export ONE CSV with all selected filters (a deployment, a date range, or all): one row per
   filter, the columns of ROADMAP §3.6 in the order of docs/LAB_INTERFACE.md, a header comment
   with schema_version and the export time, times in UTC ISO 8601 + a local-time column.
   `cowas export --deployment X -o file.csv`. sensors[] -> <name>_<stat>_<unit> columns.
6. Tests: every legal and illegal transition; persistence across restarts; IN_PROGRESS ->
   SUSPECT; replicate allocation follows FILL_ORDER; 15- and 24-slot geometries; an export golden
   file; an unknown sensor adds a column with no code change; a blank row has role
   equipment_blank.
```

#### HO3: Mission runner, scheduler, cold-boot resume (P0)

```text
Read CLAUDE.md, docs/ROADMAP.md §3.1, §3.6, §3.7, host/ (HO1-HO2), docs/PROTOCOL.md.
1. Plans: a YAML/CSV plan file (events: time, role, replicates, volume) + a recurring form
   (start, interval, count). Import the legacy formats (Samples_planned "dd/mm/yy HH:MM depth",
   CSV "date;time;depth") with a warning that depth is ignored (fixed intake). Validation:
   enough READY slots (replicates included), no overlap given the expected cycle duration
   (from the measured flow), times in the future, the clock is sane.
2. Runner: one procedure at a time. For each event, allocate filters to the next READY slots in
   FILL_ORDER (from GET_INFO), run RUN_CYCLE for each, store the record. Policies (config):
   - late event: run if late < X min, else skip + log;
   - sample fault (end_reason fault/timeout): slot FAILED, retry on the next slot up to N times;
   - system fault (STOP, ESTOP, OVER_PRESSURE, LEAK, RECOVERY_REQUIRED after a reboot): pause the
     mission, never auto-retry, wait for the operator (`cowas recover`, `cowas resume`);
   - warnings: continue, flag the record;
   - end_reason clogged: NOT a fault, the sample is kept.
3. Before the first event, and before each event, check the Due: HELLO (firmware hash as
   expected), STATUS IDLE, no recovery pending.
4. Cold-boot resume (the host restarts or loses power): reload the mission from SQLite;
   IN_PROGRESS -> SUSPECT; recompute the next event; apply the late policy.
5. `cowas mission load|validate|start|pause|resume|abort|status`, and `cowas mission dry-run`
   (runs the whole plan against FakeController in accelerated time and prints the timeline).
6. Tests (injectable clock + FakeController): 14 samples over 2 days; 4 events x 3 replicates +
   2 blanks; a power loss mid-cycle (SUSPECT, then continue); a link drop; a clog on sample 3 (kept,
   not a fault); STOP; a Due reboot mid-cycle; out of filters; overlapping events rejected.
Bench check: a 3-event plan every 30 min against the real Due on a bucket.
```

#### HO4: Deploy on the Pi (P1)

```text
Read CLAUDE.md, host/, docs/ROADMAP.md §3.8 (power), docs/POWER.md.
Hardware: Raspberry Pi CM4 on a TOFU carrier, Raspberry Pi OS <version>; RTC battery <present?>.
1. Scripts + docs/DEPLOY_PI.md: a venv install, a systemd service (auto-restart, starts after the
   USB device is up), journald limits, a udev rule giving the Due's Native port a stable name
   (/dev/cowas-due), the SQLite on the SD card with a nightly backup copy, a read-mostly setup to
   survive power cuts (fsync'd writes; consider overlayfs for the OS), an update procedure.
2. Clock: refuse to start a mission unless the time is trustworthy (NTP synced, or the RTC valid
   and set within the last N days); log the source. Document how to set the time in the field
   with no internet (from the laptop over SSH).
3. A Wi-Fi hotspot fallback so a phone/laptop can SSH in on site.
4. A power-cut test script: start a mission against the simulator, cut power, reboot -> the
   service resumes, SUSPECT is correct. Measure the Pi's idle and busy current (for HW5).
5. docs: how to swap between the laptop and the Pi as the host (the same CLI, the same DB file).
```

#### HO5: Remote access and alerts lite (P1)

```text
Read CLAUDE.md, docs/ROADMAP.md §3.2 (layer 5), host/, docs/DEPLOY_PI.md.
1. Tailscale on the Pi + the team's phones/laptops (docs + scripts). LTE through the TOFU modem
   (ModemManager/NetworkManager, APN <...>), Wi-Fi first, LTE fallback.
2. cowas.alerts: rules -> severity (critical: STOP/E-stop/over-pressure/leak/fault pausing the
   mission/controller link lost > 10 min/recovery pending; warning: sample ended clogged or
   timeout, few READY slots left, low disk, clock not synced; info: sample done, mission complete).
   Critical alerts repeat every <15> min until acknowledged (`cowas alerts ack <id>`). Dedup and
   rate limiting. Every alert stored with its ack (who, when).
3. Channels behind one interface: ntfy (self-hosted or ntfy.sh with a private topic) or a
   Telegram bot (config), + a daily status message (slots used, last sample, battery voltage if
   known, disk, temperature).
4. Bandwidth: telemetry is not pushed over LTE; logs on demand (`cowas logs --since`).
5. If the TOFU modem has GNSS, record the position in the deployment automatically.
6. Tests: the rule table with the simulator, repeat-until-ack, dedup, rate limits, a delivery
   failure + retry, and the daily report content.
```

### 8.3 Lab / eDNA track

These prompts produce documents, tools and analysis scripts. The lab work itself is done by people.

#### LA1: Lab alignment (P0, week 1)

```text
Read docs/ROADMAP.md §3.6, §3.7. Draft docs/LAB_INTERFACE.md as a meeting template with the lab
<lab name, contact>, with an empty "answer" column that we fill in during the meeting:
- extraction kit and whether it supports Sterivex + DNA/RNA Shield directly (e.g. a
  Sterivex-specific kit) or needs a protocol change;
- the target shield volume per Sterivex and the minimum acceptable; the max acceptable residual
  water (the shield/water ratio);
- the target filtered volume (2 L?) and the minimum usable volume;
- storage: how long filters may stay in the machine (temperature range), the transport
  conditions after retrieval, storage until extraction;
- labelling: the sample_id format, label type (freezer- and ethanol-proof), who prints them;
- blanks: equipment blanks per deployment (proposal: start + end), field blank, extraction
  blanks, PCR negatives; the DNA-free water source for equipment blanks;
- positive control policy (later: automated; now: none, or a lab mock community?);
- the metadata columns the lab needs and their names (start from ROADMAP §3.6), units, formats;
- chain of custody (who hands over what, a form);
- the markers/assays planned (e.g. 12S fish, COI, 16S) -> do they change the volume target?
- what a successful Pilot A means for the lab (e.g. "blanks clean, machine vs manual replicates
  give comparable detections").
Also add a one-page "Sterivex handling in CoWaS" summary for the lab.
```

#### LA2: SOPs: filter loading/unloading, cleaning, labels, chain of custody (P0)

```text
Read docs/ROADMAP.md §3.7, docs/LAB_INTERFACE.md (filled in), docs/COMMANDS.md.
Write docs/SOP_FILTERS.md, a checklist people can print:
1. Before loading: clean the manifold contact faces and the wetted parts (bleach at the % agreed
   with the lab, a contact time, DNA-free water rinse, then dry), gloves, a clean surface, the
   filter lot recorded.
2. Loading: which slots, orientation, luer tightness check, cap removal order, the host command
   to mark the slots READY (`cowas slots load ...`), a photo of the loaded manifold.
3. Unloading: the order, capping both ends, labelling from the printed labels, into a cooler with
   ice packs, the time recorded, a photo.
4. Cleaning after a deployment + biosecurity (quagga mussels: clean, rinse, dry all wetted parts
   before any other water body).
5. The chain-of-custody form (a printable table).
6. The equipment-blank procedure: DNA-free water bottle, intake hose tip moved to the bottle, the
   purge volume of blank water first, the host event with role equipment_blank.
Each step has a checkbox and a "who" field.
```

#### LA3: Shield volume, residual water, shield retention (P0)

```text
Read CLAUDE.md, docs/ROADMAP.md §3.7, findings F4, F28, F29, docs/LAB_INTERFACE.md (target shield
volume <x> mL, max residual water <y> mL), and the FW4 emptying-phase logs.
Goal: the numbers we need to trust each filter. We have a 0.01 g scale.
1. A console procedure `shield_calibrate`: dispense the micro pump into a tared tube for T s, 3
   repeats at 2-3 durations -> mL/s (density ~1.0; ask me the product density if known) and the
   repeatability; prints DNA_SHIELD_FILL_MS for the target volume (TEST lines).
2. docs/LA3_PROTOCOL.md: a weighing protocol per filter:
   dry filter mass -> after RUN_CYCLE without shield (water + air push) = the residual water ->
   after the full cycle = water + shield -> after 1, 3, 7 days in the manifold (O-ring unlocked
   as in production, at room temperature and at <25-30> °C if possible) = retention.
   3 filters per condition. Also test the air-push variants: today's settings, and a longer
   EMPTY_STX_MAX_MS at a higher air pressure still below the membrane bubble point.
3. tools/analyze_la3.py: reads a CSV of masses, computes residual water, shield volume, the
   shield/water ratio, retention (% per day), mean ± SD, and prints PASS/FAIL against the lab's
   thresholds. pytest with a synthetic dataset.
4. Recommend: DNA_SHIELD_FILL_MS, the air-push settings, and whether the O-ring should stay
   locked or the filter outlets need caps/check valves after the shield. Ask me before changing
   code; any change goes into a small follow-up PR.
```

#### LA4: Carry-over and slot cross-talk (P0)

```text
Read CLAUDE.md, docs/ROADMAP.md §3.7, findings F29, F30, and HW6 (intake hose ID/length).
Goal: prove that a sample doesn't contain the previous one, and that slots don't leak into each
other. Use a harmless tracer: food dye (absorbance) or salt (conductivity), whichever we have
instruments for: <spectrophotometer | conductivity meter | phone camera photometry>.
1. docs/LA4_PROTOCOL.md:
   a. Dead volume: fill the intake + pump + sensors with dye, then pump clear water, sampling the
      outlet every 25 mL -> the washout curve -> the internal dead volume (the INTERNAL_DEAD_VOLUME_ML
      of FW4) and the purge factor needed for < 0.1 % carry-over.
   b. Carry-over into a filter: run a dyed "sample" on slot A, then a clear-water cycle on slot
      B with the FW4 purge -> measure the dye in slot B's outlet water and its filter.
   c. Cross-talk: fill slots A with dye-shield-like liquid, rotate through all other slots with
      clear-water cycles -> check the neighbours of A and the slots the rotor passed over.
   d. Optional with the lab: the same with a DNA spike (e.g. a species absent from the lake)
      and qPCR on the following filter.
2. tools/analyze_la4.py: the washout curve fit (exponential), the purge factor for a target
   carry-over, the cross-talk table; pytest on synthetic data.
3. Recommend the PURGE_DEAD_VOLUME_FACTOR and INTERNAL_DEAD_VOLUME_ML values for Settings.h, and
   whether a decontamination flush (bleach) is needed before Pilot B (a P2 item otherwise).
```

### 8.4 Hardware / field track (with Claude's help)

#### HW1: Electrical audit (P0, week 1) — prompt

```text
Read CLAUDE.md, docs/PINOUT.md, docs/ROADMAP.md §3.8, §11, findings F20, F25, F26, F32, F36, and
include/Settings.h. I will measure things on the machine; you prepare and then record the audit.
1. Write docs/POWER.md: a block diagram (ASCII) of how power flows today (supply -> 24 V rail ->
   pump driver, VNH5019 shield, A4988, valves, micro pump; -> the Due; -> the Pi), with empty
   fields for what I don't know yet, and the proposed pilot topology of ROADMAP §3.8.
2. Give me a measurement checklist, one line per item, with what to measure, how, and the
   acceptable value:
   - the pump driver: model, input range, what DAC1 0 % and 100 % give (F25), pump speed at
     0.55 V ("0 %") with the relay on; is the DAC buffered?
   - the Due's power input (Vin/jack/5V pin, which voltage), and the USB power path;
   - the flowmeter output high level on D13 (F26) and the flowmeter supply voltage;
   - the AMT22 MISO high level (F36), the ABP supply at its pins (3.3 V);
   - the VNH5019 EN/DIAG pull-up voltage seen by the Due pins;
   - flyback diodes on every valve and relay coil;
   - the 24 V rail current: idle, pump at 50/90 %, stepper moving, valves on;
   - the A4988 Vref vs the motor's rated current (42SHD034-20B).
3. After I send the results, update PINOUT.md and POWER.md, and list the fixes (divider, buffer,
   pin move, fuse) with the parts to add to ROADMAP §11.
```

#### HW2–HW8: tasks and checklists (people)

| Task | Checklist (tick when done) |
|---|---|
| **HW2** Safety hardware | ☐ Relief valve set to 2.8 bar ±0.2 (check with the sensor and a clamp) and plumbed to waste · ☐ E-stop: latching mushroom switch in series with the 24 V actuator rail (DC rating ≥ 2 × max current), or through a relay if not · ☐ second contact to a Due pin (P1) · ☐ fuses per branch (pump, VNH5019, A4988, valves), rated ~1.5× the measured current · ☐ E-stop test: pump running → press → everything stops < 0.5 s |
| **HW3** Flowmeter | ☐ Output level safe for 3.3 V (sensor powered at 3.3 V if its spec allows, or a divider/open-collector + 3.3 V pull-up) · ☐ pin chosen (keep D13 if a pulse test passes, else D36) · ☐ `calibrate_flow` against a measuring cylinder **and** by weighing: 3 runs × 500 mL, spread < 3 % · ☐ **real sampling flow** measured with a clean Sterivex at 2 bar (mL/min) → FW4's `SAMPLE_MIN_EXPECTED_FLOW` and the outlet meter decision (P2) · ☐ flow with a half-clogged filter: does the meter still count (MW-FS-2.0 minimum 0.15 L/min)? |
| **HW4** Pressure sensor | ☐ Pin 1 identified before powering (6-pin DIP, not reverse-polarity protected; procedure in docs/TESTS_PENDING.md) · ☐ powered from 3.3 V, SS on D8 · ☐ Mounted on a tee between the pump and the manifold, port facing down or sideways (no trapped air) · ☐ barbs clamped (the pop-out issue) · ☐ short cable (< 30 cm) or twisted pairs with GND · ☐ FW3 checklist passed |
| **HW5** Power | ☐ Battery (Pilot A: 12 V LiFePO4 ≥ 50 Ah with BMS) · ☐ regulated 24 V boost (≥ 1.5 × the peak current) for actuators · ☐ 5 V buck ≥ 3 A for the Pi · ☐ Due supply per HW1 · ☐ main switch + main fuse · ☐ measured: idle W, one full cycle Wh → `docs/POWER.md` → battery sized for Pilot A and B · ☐ **20 cold boots** on the battery supply, 20/20 OK · ☐ a pump start doesn't brown out the Due (watch the reset cause in the boot log) |
| **HW6** Field fluidics (Pilot A) | ☐ Intake: strainer (~200–500 µm mesh) + weight + rope marked every metre, at a fixed depth `<1–5 m>` · ☐ hose ID and length recorded → FW4's dead volume · ☐ suction lift: the pump primes from the water level to the machine (test at the real height) · ☐ outlet discharges ≥ 5 m downstream of the intake · ☐ quick-disconnects for transport · ☐ splash-proof box (IP54+) with the electronics above the fluidics, shade cover (sun heats the box and the filters) · ☐ everything fits on one trolley/boat |
| **HW7** O-ring lock | ☐ `lock_endurance 200`: no drift in the encoder angle, no leak at the end (`leak_hold`) · ☐ with the coils off while locked (FW4): pressurize to 2.5 bar with the outlet clamped for 10 min → no leak, no backlash (confirms the self-locking under pressure) · ☐ P1: "unlocked" home switch (sealed Hall + magnet) on D39 |
| **HW8** Pilot B hardware | ☐ Enclosure IP65+, cable glands, desiccant, internal temperature logger · ☐ enclosure leak sensor at the lowest point · ☐ DS18B20 water probe on the intake · ☐ mounting/mooring plan, anti-theft, a "scientific equipment + contact" label · ☐ access agreed with the pier/harbour owner · ☐ site access and a swap plan for the battery |

### 8.5 Validation

#### VA1: Bench acceptance campaign (P0, week 7) — prompt

```text
Read CLAUDE.md, docs/ROADMAP.md §3, §5, §9, docs/COMMANDS.md, docs/PROTOCOL.md, host/ (HO1-HO3),
docs/LA3_PROTOCOL.md and docs/LA4_PROTOCOL.md results.
Write docs/VALIDATION_VA1.md + tools/va1_run.py (drives the host CLI, collects logs, test_logs and
the export into test_logs/va1_<date>/). Each test: purpose, setup, steps, numeric pass criteria,
a results table. Tests:
1. A full 14-slot mission from the host on a bucket (2 L each or 500 mL for speed, decide with
   me), on the pilot battery, with the laptop as host: every record complete, volumes within 5 %
   of the weighed outlet water.
2. A 24 h unattended run (e.g. 6 events) with the machine alone in a room.
3. A power cut (main switch) in each phase: purge, water, air push, shield, empty, rotation, lock
   -> boot -> RECOVERY_REQUIRED -> recover -> the host marks SUSPECT, the mission resumes.
4. STOP from the CLI and the E-stop in each phase -> the safe state, then recovery.
5. A host restart (kill the process) mid-cycle -> the Due is NOT reset, the cycle ends, the host
   reconnects and gets the result by request id.
6. A clogged filter (outlet clamp) -> end_reason clogged; the cap is never exceeded (max P_in
   printed).
7. A pressure cap trip with the relief valve isolated -> trips at 2.5 ± 0.1 bar.
8. The USB cable pulled mid-cycle -> the cycle finishes safely, nothing new starts.
9. The export CSV opened by the lab (LA1 columns) -> accepted.
10. The battery: measured Wh per cycle and idle W -> the Pilot A margin ≥ 50 %.
Go/no-go rule: all tests 1-8 PASS; a FAIL on 9-10 needs a written workaround.
```

---

## 9. Pilot procedures

### 9.1 Pilot A (supervised, 1–2 days): VA2

**Sampling plan** (14 slots; adjust with the lab):

| Slot use | Count | When |
|---|---|---|
| Equipment blank (DNA-free water through the machine) | 1 | Start, before any lake water |
| Lake samples: 4 timepoints × 3 replicates | 12 | e.g. every 3–4 h |
| Equipment blank | 1 | End |
| *Manual* comparison samples (syringe or separate pump, same volume, same shield) | 2–3 timepoints × 1–3 | Same time and place as machine timepoints |
| *Manual* field blank (DNA-free water through a manual filter on site) | 1 | Any time |

**Checklist:**
- [ ] VA1 passed; firmware hash and host version recorded.
- [ ] Site access confirmed; weather checked (wind, thunderstorms); life jackets if on a boat.
- [ ] Filters loaded and labelled per SOP (LA2); photo of the manifold; slots READY in the host.
- [ ] Battery charged (voltage recorded); spare fuses; tools; spare Sterivex; DNA-free water (≥ 3 L for blanks + purge); cooler + ice packs; gloves; bleach; the paper field log.
- [ ] Intake at `<depth>` m, strainer on, outlet downstream; hose ID and length entered in the deployment.
- [ ] Time synced on the host; site coordinates entered.
- [ ] Equipment blank run → OK. Then the mission starts.
- [ ] During the run: log every event on paper (time, what you saw, leaks, noises, weather, water temperature with a thermometer).
- [ ] End blank → STOP the mission → unload per SOP → cooler → chain-of-custody form → lab.
- [ ] Export the CSV + the logs + test_logs → `data/pilots/A_<date>/`.
- [ ] Debrief within 48 h: what failed, what to change for Pilot B (issues in GitHub).

**Success criteria:** ≥ 12 of 14 cycles completed without a fault; blanks clean (lab); machine and manual samples give comparable detections (lab); no safety event.

### 9.2 Pilot B (unattended, 1–2 weeks): VA3 + VA4

Before: all P1 steps done; **VA3**: a 72 h soak on battery with the Pi, remote access and alerts, including a power cut and a forced fault (the alert reaches the phones and repeats until acked). Plan: 1 blank at the start + 12 samples (e.g. 1 per day or 2 per day, per the lab) + 1 blank at the end. A visit on day 3–4 (check leaks, battery, biofouling of the strainer). Same retrieval and data rules as Pilot A.

---

## 10. Future work

See [ROADMAP_LATER.md](ROADMAP_LATER.md): P2 steps with prompts (2 pressure + 2 flow hydraulics and clogging analytics, the pump tuning campaign and PID variants, runtime parameters, the firmware restructure, the web app + API, full notifications, environmental sensors, the spool/depth, automated controls, firmware updates from the Pi, duty-cycled power, Jetson) and the "prepared for, not built" table.

---

## 11. Parts, pins, electrical rules

### 11.1 Order now (HW0)

Specs, not brands. Check each part against your fittings, tube sizes and measured currents. **Buy spares** of anything wet or hard to source. Lead times decide the critical path: order in week 0.

| # | Part | Qty | Spec / notes | For |
|---|---|---|---|---|
| 1 | **Pressure sensors ABPDANV060PGSA3** | 3 | Same family as the 150 psi part (not installed as of FW0, F50) (SPI, 3.3 V, same code once FW3 is done). **Critical path now: FW3's checklist and every pumping test wait for it**; 0–4.1 bar, ±0.06 bar. P_in + spare now; P_out in P2. Barb clamps too. **FW3 must be merged before swapping** (F27). | FW3 |
| 2 | **Mechanical pressure-relief valve** | 1 | Adjustable ~2.5–3 bar, water-compatible, outlet to waste | HW2 |
| 3 | **E-stop** | 1 | Latching mushroom switch, 2 NC contacts: one DC-rated ≥ 2 × the 24 V rail current (series cut), one for the Due input | HW2 |
| 4 | Relay (only if #3 can't carry the DC load) | 1 | 24 V coil, DC-rated contacts, flyback diode | HW2 |
| 5 | Fuses + holders | per branch + spares | Pump, VNH5019, A4988, valves, main | HW2 |
| 6 | **Battery 12 V LiFePO4 ≥ 50 Ah** with BMS + charger | 1 | Pilot A. Pilot B sized after HW5 | HW5 |
| 7 | **DC-DC boost 12 → 24 V, regulated** | 1 + 1 | Current ≥ 1.5 × the measured peak (pump start + stepper + valves) | HW5 |
| 8 | DC-DC buck 5 V ≥ 3 A (Pi) + the Due supply per HW1 | 1 + 1 | Low-noise; common ground | HW5 |
| 9 | Main switch, main fuse, a DC energy meter (shunt) | 1 | The shunt meter measures Wh per cycle | HW5 |
| 10 | Rail-to-rail op-amp + resistors for a DAC buffer; dividers, 10 kΩ / 4.7 kΩ, 100 nF | kit | Only if HW1 confirms (F25, F26) | HW1 |
| 11 | Micro-USB data cables, short, good quality | 2 | Due Native port ↔ laptop/Pi | FW7 |
| 12 | **Intake kit** | 1 | Strainer (200–500 µm mesh), weight, 10 m hose (same ID as today, food-grade), rope; optional foot valve (helps priming; the purge handles the retained water) | HW6 |
| 13 | Quick-disconnect fittings | 4 | Intake, outlet, purge | HW6 |
| 14 | Splash-proof box IP54+ + shade cover | 1 | Pilot A | HW6 |
| 15 | **Lab consumables** | — | Sterivex ≥ 40 (tests + pilot), DNA/RNA Shield (≥ 14 × target volume + ~100 mL for calibration), DNA-free water ≥ 5 L, luer caps for both ends, cryo labels, gloves, bleach, a cooler + ice packs | LA2–LA4, VA2 |
| 16 | **Test bench** | — | Scale 0.01 g (≥ 200 g), 1 L measuring cylinder, bucket, tube clamp or needle valve, food dye (+ a conductivity meter or photometer if available) | LA3, LA4, HW3 |
| 17 | Spare Arduino Due | 1 | A bench board + a field spare | all |
| 18 | O-ring spare sets | 3 | | HW7 |

**Should (P1), order by week 6:** a Hall switch + magnet (O-ring "unlocked" home), an enclosure leak sensor, DS18B20 waterproof probes ×2 (+1), INA226 current monitors ×2, an IP65 enclosure + cable glands + desiccant, SIM + LTE antenna (outside any metal), an RTC battery for the CM4 carrier if absent, the Pilot B battery (after HW5).

**Before you order:** confirm the fitting threads and tube sizes (#1, #2, #12, #13) and measure the 24 V currents (#3, #5, #7).

### 11.2 Pin budget

The code uses about 31 digital pins, 3–4 analog inputs and 1 DAC. The real limit is **what the PCB shield brings out** (HW1 fills in `docs/PINOUT.md`).

| New item | Pin | Wiring | Pri |
|---|---|---|---|
| `P_in` (ABP, SPI) | SPI + CS **D8** (unchanged) | 3.3 V, GND, MISO, SCK; read-only, no MOSI needed | P0 |
| `Q_in` (turbine) | **D13** if a pulse test passes with a 3.3 V-safe level; otherwise **D36** | Sensor at 3.3 V if its spec allows, or open-collector + pull-up to 3.3 V, or a divider | P0 |
| E-stop state | **D37** | NC contact to GND, `INPUT_PULLUP`: pressed **or a broken wire** = HIGH = stop | P0/P1 |
| O-ring "unlocked" home switch | **D39** | Same wiring as the E-stop | P1 |
| Enclosure leak sensor | **D41** (digital module) or **A6** (analog probe) | 3.3 V logic | P1 |
| Water temperature (DS18B20, 1-Wire) | **D43** | 4.7 kΩ pull-up to 3.3 V; probes powered at 3.3 V | P1 |
| Battery voltage | **A0** | Divider 47 k / 10 k (14.6 V → 2.56 V) + 100 nF; or the INA226 | P1 |
| Current monitors (INA226) | **SDA 20 / SCL 21** | The Due has on-board pull-ups on 20/21 | P1 |
| `P_out` (ABP, SPI) | SPI + CS **D45** | Same as P_in | P2 |
| `Q_out` | **D12** if the old `flow_sensor_big` wiring exists, else **D47** | As Q_in | P2 |
| Turbidity | **A5** | Divider to 3.3 V | P2 |
| Still spare | A1–A4, A7–A9, D4, D14–17, D49, D50–D53 | | |

### 11.3 Electrical rules

- **The Due is 3.3 V only and not 5 V tolerant.** Pull-ups to 3.3 V; sensor outputs that swing to their supply need a divider or a 3.3 V supply; check every module's output level before connecting it.
- **The DAC outputs ~0.55–2.75 V**, not 0–3.3 V, and is fragile: buffer it, and never rely on "0 %" to stop the pump (the relay does).
- **Never feed the Due raw 24 V**, or a charged LiFePO4 pack above 16 V.
- **One ground star point** for the power return; keep the motor currents out of the sensor grounds.
- **Flyback diodes** on every coil (valves, relays).
- **Short SPI/I²C cables** (< 30 cm); 1-Wire works over a few metres; for long runs, use 4–20 mA or RS-485.
- **I²C addresses:** list them in `docs/PINOUT.md` before ordering an I²C device (the MS5837 is fixed at 0x76; INA226 0x40–0x4F by strapping; SHT4x 0x44).

---

## 12. Old step IDs → new step IDs

| v1 | New | Note |
|---|---|---|
| S0.0 | FW2 | + endurance test, TEST logging |
| S0.1 | FW0 | the ledger became `docs/COMMANDS.md`; the dead-code audit is P2 |
| S0.2 | FW4 | + purge volume, flow factor, air-push logging |
| S0.3 | **FW3** | now on the critical path; fixes the driver first |
| S1.1 | LATER P2-1 | restructure postponed |
| S1.2 | FW5 | |
| S1.3 + S1.4 | FW6 | lean modes |
| S2.1 | LATER P2-2 | calibrations stay in `Settings.h` |
| S2.2 | **FW1** | moved to week 1 to unblock the host |
| S2.3 | FW7 | + boot into host mode (was I2) |
| S3.1–S3.5 | LATER P2-3…P2-7 | FW8 keeps a minimal clog end condition |
| S3.6 | FW9 (water temperature) + LATER P2-8 | |
| H1 | HO1 | |
| H2 + H3 | HO2 | FAIRe/MIxS + merge → LATER P2-12 |
| H4 + H5 | HO3 | commissioning workflow → LATER P2-13 |
| H6, H7 | LATER P2-9, P2-10 | CLI over SSH for the pilots |
| H8 | HO5 (lite) + LATER P2-11 | |
| H9 | HO4 | |
| H10 | HO5 | |
| I1 | VA1 | |
| I2 | FW7 | |
| I3 | LATER P2-14 | |
| I4 | LATER P2-18 | |
| HW0–HW10 | HW0–HW8 | reorganized; power and field fluidics added |
