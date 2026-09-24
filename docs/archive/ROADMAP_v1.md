# CoWaS development roadmap

The path from today's bench prototype (Arduino test menu + button panel + a Linux-only PyQt/ZMQ Raspberry Pi script) to a field-deployable sampler: **calibrate → set up → run autonomously**, controlled and monitored from a phone.

## 0. How to use this document

- **One step = one fresh Claude Code session = one branch/PR.** Copy the step's prompt (section 5) into a new session. The prompts point back to this file and to `CLAUDE.md` (created in S0.1), so they stay short.
- **Before a step:** the tree is clean, and the previous step is merged, flashed and has passed its hardware checklist.
- **After a step:** tests pass, you flash, you run the step's **hardware checklist**, then fill in the "verified on hardware" column of `docs/FUNCTIONALITY_LEDGER.md` and merge.
- **Start with S0.0** (the mechanical test suite). It works with the pressure sensor out.
- Section 6 is a **hands-on procedure for you** (pump measurements → decision → which prompt to paste next).
- Section 7 lists **future updates**: prepared for, but not built yet.
- Section 8 is the **parts list** and the **pin budget**.

### 0.1 Why the steps are in this order

1. **Tests first** (S0.0). You need them today, and they protect every later refactor.
2. **Safety net and quick safety fixes** (S0.1, S0.2) come before any restructuring.
3. **Firmware foundations** (Phase 1: structure → supervisor → modes → stop/recovery) come before features, because every later feature relies on them. Modes come before stop/recovery, because a stop *is* a mode transition.
4. **Parameters → protocol spec → protocol** (Phase 2). The protocol exposes the parameters and calibrations, so the parameters come first. The protocol also comes before the hydraulics, because it needs no new hardware while the hydraulics wait for parts, and the host work can start from the protocol spec.
5. **Hydraulics** (Phase 3) come when the sensors are installed, and you **measure before tuning** the pump control.
6. **Host software** (Phase 4) runs **in parallel** from the protocol spec (S2.2) onwards, against a simulator, then against the real Due once S2.3 is done.
7. **Integration** (Phase 5) comes last. The machine boots into operational mode by default only after the bench campaign has passed.
8. **Floating step S0.3:** as soon as the first new pressure sensor arrives, S0.3 installs it on the existing code path and runs the pending tests. There's no need to wait for Phase 3.

### 0.2 Milestones

| Milestone | Reached after | What you can do |
|---|---|---|
| **M1: Bench-testable** | S0.0–S0.3 | Test locking, sealing, rotation and full cycles from the console, safely |
| **M2: Safe controller** | Phase 1 | STOP and E-stop from anywhere, enforced recovery, no blocking loops, clean modes |
| **M3: Remote-controllable** | Phase 2 + H1–H6 | Drive the machine from the Pi's API; calibrate without reflashing |
| **M4: Autonomous on the bench** | Phase 3 + H7–H9 + I1 | Full missions with metadata, clog detection, leak detection, notifications |
| **M5: Field-ready** | H10 + I2 (+ power, enclosure) | Deploy, run from the phone over LTE, collect the filters and the export file |

---

## 1. Target architecture

```
 Phone / browser  ──HTTPS + WebSocket (+ push notifications)──►  HOST  (Raspberry Pi CM4 on TOFU now, Jetson later)
                                          Python, cross-platform
                                          mission · schedule · slot inventory · sample metadata
                                          calibration store · device logs · API · notifications
                                               │
                                               │  Native USB port: protocol v1 (JSON lines) + forwarded logs
                                               │  (the Programming port stays free for a laptop: human console/debug)
                                               ▼
                                          CONTROLLER  (Arduino Due)
                                          all sensors/actuators · safety interlocks
                                          atomic procedures (purge, sample slot N, DNA shield, empty, calibrations)
                                          service modes (console + button panel)
                                               │
          pump · micro pump · valves · manifold + encoder · O-ring actuator · P_in/P_out · Q_in/Q_out · spool
```

### 1.1 Who owns what

| Concern | Controller (Due) | Host (Pi/Jetson) | Why |
|---|---|---|---|
| Actuators, sensors, timing-critical loops (PID, stepper) | ✅ | | Real-time; must not depend on a serial link |
| Safety: pressure cap, E-stop, watchdog, leak cut-off, timeouts, rotation interlock | ✅ | monitors + **notifies** | Must work even if the host crashes or the link drops |
| Sampling *procedures* (the sequence of valves, pump and manifold moves) | ✅ | | Keeps the host simple; the host says *what*, the Due knows *how* |
| **Which slot**, **when**, **what depth**, replicates/controls | | ✅ | Needs a clock, persistence, policy |
| Slot/filter inventory, **sample metadata** | measures and reports | ✅ stores and exports | The Due has no RTC or EEPROM and **forgets everything on reset** |
| Calibration values | runs the calibration, applies it live, reports it | ✅ stores it, pushes it at every boot | No code editing, no reflash, no interruption (S2.1) |
| Device logs (all Due prints) | emits | ✅ records | The host sees everything the Due says |
| Remote access, UI, notifications, export | | ✅ | |
| Future: sequencing, firmware updates of the Due | | ✅ | Section 7 |

Rule of thumb: **the controller never starts a sample on its own, and the host never toggles a valve directly.**

### 1.2 Controller modes (merges the two test loops without mixing them)

```
BOOT ──► boot self-check (1.4) ──► RECOVERY_REQUIRED ── RECOVER passes ──► default mode
         └─ E-stop pressed ──► ESTOP                   (default = SERVICE_CONSOLE until step I2, then IDLE)

IDLE ◄──────► BUSY              (host procedure running; only STOP / STATUS / PING accepted)
  │  ▲
  │  └── exit ── SERVICE_CONSOLE (today's serial test menu, every command kept)
  │  └── exit ── SERVICE_PANEL   (today's 3-button control)
  ▼
any ──► FAULT  (latched: over-pressure, leak, timeout, implausible safety sensor, software STOP)
any ──► ESTOP  (hardware button)
FAULT / ESTOP ── cause gone + clear ──► RECOVERY_REQUIRED ── RECOVER ──► IDLE
```

The service modes and host commands exclude each other by construction: a host command sent while in a service mode gets `ERR busy_service`.

### 1.3 "Things running in parallel" on a single-core Arduino

- One function, `service()`, does everything that must *always* happen: pressure cap, E-stop, software STOP, leak check, watchdog kick, telemetry.
- `wait_ms()` replaces `delay()` in procedures and calls `service()` while it waits. Every existing blocking loop also calls it: `CtrlPump::run`, the `rotateMotor` wait, stepper moves, `Micro_Pump::start(ms)`, `Button::waitPressedAndReleased`, and the keypress waits.
- When `service()` latches a fault, the waits return "abort" and the procedures unwind to the safe state.

Proof it's needed: today, in button-panel pump mode, `loop()` never calls `enforce_pressure_safety()`.

### 1.4 Safety layers, STOP buttons, recovery

**Both stop buttons exist**: a hardware button and a software button (app and console).

| # | Layer | Covers |
|---|---|---|
| 1 | **Hardware E-stop**: a latching button that cuts 24 V actuator power through a relay. The Due reads it on an input pin. | Firmware hang or bug, link down |
| 2 | Due hardware **watchdog** | Stuck loops |
| 3 | Firmware **supervisor** (`service()`): pressure cap, implausible safety sensor, leak, per-procedure timeouts, rotation interlock (unlocks automatically before rotating, only when that is safe) | Normal faults |
| 4 | **Software STOP**: app → host → controller `STOP`, or `stop` in the console. Accepted at any time. Same safe state as the E-stop, minus the power cut. | Operator sees a problem remotely |
| 5 | Host checks and **notifications**: plan sanity, inventory, link heartbeat; every fault is pushed to the app | Operations |

**Safe state (decided):** pump and micro pump off; **all valves closed**; manifold motor and spool stopped; **O-ring stays locked**, to avoid leaks. If the O-ring is *unlocked* at that moment (e.g. mid-rotation), it is left as is and not driven into a misaligned lock; recovery handles it.

**Recovery procedure (decided):** `RECOVER` runs an explicit, reported checklist and stops at the first FAIL:
1. The cause is gone (E-stop released, pressure near 0, safety sensor plausible).
2. The encoder reads and the O-ring state is known.
3. If the manifold was mid-rotation, it re-homes to a known slot and locks.
4. If a sample was interrupted, it optionally drains (`step_empty`) before unlocking.
5. Every valve and actuator goes back to the normal idle state, one by one, each confirmed.
6. The mode becomes IDLE.

The host marks the interrupted slot `FAILED` or `SUSPECT`.

**You can't forget the recovery: it is enforced automatically (decided).**
- **Stop while powered:** after an E-stop or a STOP, the controller stays latched even once the button is released and 24 V is back. It refuses every procedure until `RECOVER` passes. The app shows a "Recovery required" banner with the cause.
- **Every boot is treated as possibly unclean.** RAM is lost, so the valve, O-ring and manifold states are unknown:
  - the O-ring lock becomes `UNKNOWN`, and the valves are driven closed;
  - the non-moving checks run automatically (the boot self-check);
  - an E-stop pressed at boot goes straight to ESTOP;
  - moving steps (actuator homing, manifold re-home) wait for operator confirmation or for the host's `RECOVER`.
- **The host remembers too:** the last fault or stop is persisted in SQLite. After its own reboot the host still shows it, and it won't resume a mission before recovery passes.

### 1.5 Pressure: one safety value

There is **one safety value: `PRESSURE_CAP`** (default **2.5 bar**, user-editable). Above it, the pump stops and a FAULT latches. There is no other limit.
- The **PID target** is a control setting, not a safety value. It is derived as `cap − 0.5` (2.0 bar at a 2.5 cap), so you only ever set one number.
- **Input validation:** an edit to the cap is accepted only within 0.5–3.0 bar, to catch typos like `25`. It is not a second cap.

### 1.6 Hydraulic measurements: 2 pressures + 2 flows (planned)

Assumed layout, to confirm in S3.1: `Q_in` → pump → `P_in` → manifold → **filter** → `P_out` → `Q_out` → outlet.

| Sensor | Role | If it fails |
|---|---|---|
| **`P_in`** (upstream of the filter) | **Safety sensor**: the cap and the PID act on it. It sits where the pressure is highest. | The pump stops (FAULT). |
| **`P_out`** (downstream of the filter) | **Clogging measurement**: `ΔP = P_in − P_out` is the pressure drop *across the filter*. It separates a clogging filter from a restriction elsewhere (tubing, manifold, a closed valve). Also detects a blocked outlet. | Sampling continues, but the clog measurement is **degraded**: a fallback estimate `P_in / Q`, explicitly flagged as lower quality in the metadata. It is never a pump stop. |
| **`Q_in`, `Q_out`** (inlet and outlet flowmeters) | Together: a **better flow measurement**, **leak detection**, and **more robust clog detection** (below) | Continue on the remaining meter, flagged |

What the two flowmeters add:
- **Better flow measurement.** When both are valid and agree within a tolerance, the fused flow is their average. Turbine meters are noisy at low flow, so averaging helps. When they disagree beyond the tolerance, the controller does not guess: the value is flagged and the leak and sensor checks decide what happened. Metadata stores `V_in`, `V_out`, the fused filtered volume, and the in/out agreement (%), so the lab can judge the volume's quality.
- **Leak detection.** Water that goes in but doesn't come out (`V_in − V_out` beyond a tolerance, after a settling window) means a leak between the meters.
- **Telling a leak from a dead meter.** One meter reading exactly 0 while the other counts, with normal pressures, means that meter is dead, not a leak.
- **Robust clog detection.** Clogging shows as **rising ΔP *and* falling flow**: two independent signals, far fewer false positives than either alone. The filter resistance `R_filter = ΔP / Q_fused` and the clog index (R at the end / R at the start) feed the end condition and the metadata.
- **Cross-calibration.** During a leak-free reference run, the ratio `V_in / V_out` tracks the meters' relative drift. The absolute calibration stays anchored to a measuring cylinder.

**Pressure signatures**:

| Signature | Meaning |
|---|---|
| `P_out` drops suddenly while `P_in` is stable | Leak between the pressure sensors |
| `P_out ≈ P_in` during a sample | Filter torn or missing |
| `P_out` rising | Blocked outlet |

**Caveat:** if the filter outlet drains freely to the air, `P_out` stays near 0 and ΔP ≈ `P_in`. That still works, but choose a `P_out` range that gives good resolution near 0 (section 8).

**Air phases must not fool this logic.** The sequence alternates *water* phases (priming, filtering) and *air* phases (air pushed through to empty the filter; air pushing the DNA shield). In air: pressures fall near 0, may read slightly negative, and spike at transitions; ΔP and Q ≈ 0, so R is meaningless; and **turbine flowmeters spin on air and count false "water"**. So every computation is **phase-aware**. The controller tracks the phase (`PRIME`, `WATER`, `AIR_PURGE`, `SHIELD`, `IDLE`):

| Computation | WATER | AIR_PURGE / SHIELD |
|---|---|---|
| Pressure cap on `max(P_in, P_out)` (in practice `P_in`) | ✅ | ✅ **always on** |
| `P_in` plausibility (stops the pump) | ✅ | ✅, with a lower bound allowing small negative readings near 0 |
| `P_out` checks (warnings only) | ✅ | ❌ |
| ΔP, `R_filter`, clog index, clog end condition | ✅ only when Q > Q_min and ΔP > ΔP_min; otherwise "undefined" (never 0 or infinite) | ❌ frozen |
| Volume counting (`V_in`, `V_out`, filtered volume) | ✅ | ❌ frozen: air must not add volume |
| In/out flow comparison (leak) | ✅ after a settling window | ❌ |
| Air detection (existing `CtrlPumpNoWater`) | — | ✅ uses `P_in`, as today |
| Metadata statistics | computed over WATER only | the air phase is logged separately (duration, max `P_in`) |

A short blanking window after each phase change ignores the transition spikes.

**Firmware design:** sensors are **named channels** (`P_in`, `P_out`, `Q_in`, `Q_out`) in a table, each with its conversion, calibration, range and plausibility window. The code never says "pressure2", so adding, moving or swapping a sensor is a configuration change. This is implemented in S3.1; S0.3 is the minimal early version.

### 1.7 Pump control under clogging

As the filter clogs, R rises, so the plant gain (Δpressure per Δpump power) rises. A PID tuned on a clean filter becomes too aggressive on a clogged one, which causes spikes. **Measure first** (section 6), then pick the cheapest fix the data supports:
- **A.** Keep the current PID.
- **B.** Add anti-windup and an output ramp limit.
- **C.** Gain scheduling. This is your colleague's two PIDs, but *blended continuously* by `R_filter` instead of switched, so there is no switching moment to make safe. A discrete switch is only a fallback, and needs bumpless transfer plus hysteresis.

**Sample end conditions (decided)**, whichever comes first:
- **Clogging: always on.** Falling flow at near-target pressure, or `R_filter` / clog index above its threshold. Evaluated in the WATER phase only.
- **Pressure cap trip: always on.**
- **Volume: optional, on by default** (2000 mL). It can be switched off per plan or per sample; filtering then runs until clogged or timed out.
- **Maximum duration: always on**, as a safety timeout.

The end reason is recorded in the metadata.

### 1.8 Slots, sampling events, replicates, controls

**Slot states** live on the host, persisted in SQLite:

| State | Meaning | Allowed next states |
|---|---|---|
| `EMPTY` | No filter loaded | `READY` |
| `READY` | Sterile filter loaded | `IN_PROGRESS`, `DISABLED` |
| `IN_PROGRESS` | Being used now | `SAMPLED`, `FAILED` |
| `SAMPLED` | Used; has metadata | `EMPTY` (collected) |
| `FAILED` | Procedure faulted | `EMPTY`, `DISABLED` |
| `DISABLED` | Operator: out of order (with a reason) | `READY` |
| `SUSPECT` | Was `IN_PROGRESS` when power or the link was lost | `EMPTY`, `DISABLED` |

A **sampling event** (one timepoint in the plan) produces one or more **filters**, each with a *role*:
- `sample`, with **replicates** (1 by default, optionally 2–3). These are consecutive `SAMPLE` runs on the next `READY` slots, so they are **implemented**.
- `negative_control` (sterile water) and `positive_control` (known DNA). They are **modeled in the data and the plan, but disabled** until their pumps exist: plan validation rejects them, and the firmware answers `ERR not_available` (section 7).

Replicates and controls are off by default and set per timepoint. Plan validation checks that enough `READY` slots exist.

**More than 14 slots in future designs:** no code hard-codes 14 or 15. One manifold-geometry block (positions, positions without a hole, purge position, calibration reference, `FILL_ORDER`) drives everything: `NB_SLOT`, loops, validation, host inventory size, UI grid. The host reads the geometry via `GET_INFO`. Tests also run with 16 and 24 positions.

### 1.9 Sample metadata (for bioinformatics)

One record per **filter**. An **export is a single file with all selected samples** (a campaign, a date range, or everything): CSV with one row per filter, or JSON. Every row has `deployment_id`, `device_id` and a globally unique `sample_id`, and the columns are fixed and versioned, so files from several campaigns or devices **merge** without collisions (`cowas export merge`). Columns are mapped to eDNA standards (**FAIRe**, **MIxS** water) where equivalents exist.

- **Identity:** sample id, deployment id, event id, role, replicate index, slot, clock label, filter type and lot.
- **When and where:** planned time, actual start and end (ISO 8601 UTC + site time zone), latitude/longitude (GNSS if available, otherwise entered per deployment), site, depth.
- **Process:**
  - target volume (or "off"); **filtered volume (fused)**, `V_in`, `V_out` and the in/out agreement; duration; **end reason**
  - `P_in`, `P_out`, **ΔP** (max/mean/final); flow (mean/min/final); **`R_filter` at the start and end, and the clog index**
  - DNA shield dispensed; time from the end of filtration to preservation
- **Quality flags:** clog measurement degraded (`P_out` missing), flow disagreement, single-flowmeter volume, sensor warnings.
- **Provenance:** firmware and host versions, calibration-set id, fault codes, operator notes.
- **Environmental sensors:** a generic, extensible `sensors[]` block (`name, value, unit, stat`). New sensors need **no schema or protocol change**.
- Optionally, the 10 Hz hydraulic trace of each sample is saved as a separate file referenced by the sample id.

**Recommended environmental measurements** (tier 1 first; don't over-commit):

| Tier | Measurement | Why for eDNA | Effort |
|---|---|---|---|
| Free | Volume, clog index, time to preservation, depth, GPS, time | Normalize DNA per litre; interpret the yield | Planned |
| Free (host, online) | Site weather: air temperature, recent rainfall | Runoff DNA and turbidity covariates | Public weather API; no hardware |
| **1** | **Water temperature** | eDNA degrades faster when warm; standard covariate | Cheap waterproof digital probe |
| **1** | **Storage-compartment temperature** | DNA stability between sampling and collection; the lab will ask first | Same probe, inside the housing |
| **1** | **Turbidity** (relative index) | Predicts clogging and PCR inhibition | Cheap analog sensor; reported as an index, not NTU |
| 2 | Conductivity / salinity | Fresh vs brackish vs marine; DNA persistence | Moderate; needs calibration solutions |
| 3 | pH, dissolved oxygen | Useful, but the probes need maintenance | Postpone |

Tier 1 is step S3.6.

### 1.10 Notifications (safety and operations)

| Severity | Examples | Delivery |
|---|---|---|
| **Critical** | Leak, over-pressure trip, E-stop, software STOP, safety sensor fault, mission paused by a fault, controller link lost > X min | Immediate; repeated until **acknowledged**; SMS fallback once LTE exists |
| Warning | Sample ended "clogged" early, `P_out` or a flowmeter degraded, few `READY` slots left, late sample, low disk | Immediate, once |
| Info | Sample done, mission complete | Summary / optional |

Channels: in-app (WebSocket + PWA web push), then ntfy or Telegram, then SMS through the TOFU modem. Every alert is stored along with its acknowledgment (who and when).

### 1.11 Pi ↔ Due link: two ports, two audiences (decided)

| Port | Connected to | Carries |
|---|---|---|
| **Native USB** (`SerialUSB`) | **Pi / mini-computer, permanently** | Protocol v1 (JSON lines), telemetry, events, **and a copy of every log line** as `log` messages |
| **Programming port** (`Serial`) | **Laptop, when needed** | Human service console, debug prints, test menus |

**Why this way round:**
- *Opening* the Programming port resets the Due. This is the upload auto-reset; check it once on your board. With the Pi on that port, every Pi restart could reset the Due mid-sample.
- Opening the Native port does not reset the board, except the 1200-baud "touch" used for uploads, which the host must never use.
- The corollary: plugging a laptop into the Programming port may reset the Due, so connect it only when the machine is idle.

**Routing:** every print goes through one `Log` API with a level and a channel. All log lines are also forwarded to the host.

**Transition:** until the cable is moved, both channels are bound to the same port with a one-line setting.

### 1.12 Remote access ladder

1. **Local**: the Pi serves the API and web UI; the phone connects on the same Wi-Fi or to the Pi's hotspot.
2. **Remote**: **Tailscale** over Wi-Fi or LTE (TOFU). Same API; no port forwarding; no cloud server.
3. **Later, if needed**: MQTT for many devices or very poor links.

The app is a **PWA** (an installable web app): one codebase for phone and browser.

### 1.13 Power (batteries, hopefully solar)

- The controller turns everything off when idle (pump relay, stepper driver disabled while unlocked, LEDs).
- The host survives a cold power-off at any moment and resumes after booting. SQLite is the source of truth, and the clock is checked at boot.

True sleep between samples (section 7) comes after a power budget has been measured.

---

## 2. Findings from reading the code

| # | Finding | Where | Severity | Resolved in |
|---|---|---|---|---|
| F1 | Slot availability is RAM-only; after a reset, used filters look available | `Manifold.cpp` | **High** | H2, H4 |
| F2 | The pressure cap is not polled in button-panel pump mode | `main.cpp` | **High** | S0.2 |
| F3 | The cap is 3.0 bar, and the setpoint `2` is hard-coded in 4+ places. **Decision:** one cap of 2.5 bar, target = cap − 0.5 (§1.5). | `Settings.h`, `Step_functions.cpp` | **High** | S0.2, S2.1 |
| F4 | `step_DNA_shield()` uses `FILL_STERIVEX_TIME / 10.` (temporary), so production gets 25 s instead of 250 s. **Decision: remove `/10`.** | `Step_functions.cpp` | **High** | S0.2 |
| F5 | `rotateMotor()` ignores the O-ring lock (`slotN`, `clockMM`, panel Man Slot). `step_DNA_shield()` calls it while locked, expecting a no-op. **Decision:** unlock automatically before rotating when safe, otherwise refuse. | `Manifold.cpp`, `Tests.cpp`, `main.cpp` | **High** | S0.0, S0.2 |
| F6 | `flow_sensor_small` works but is **to be calibrated**; it is the volume stop for every sample | `Settings.h`, `Flow_sensor.h` | Medium | S0.2 checklist |
| F7 | One pressure sensor does control and safety; pressure1 (SPI) is faulty. **Resolution:** `P_in` = safety/control, `P_out` = clogging, plus 2 flowmeters (§1.6). | `main.cpp` | Medium | S0.3, S3.1 |
| F8 | `main_program()` blocks for a whole sample, so it can't be stopped. The Pi busy-waits with no timeout. Depth is parsed by string slicing. There are no request ids or acknowledgments. | `main.cpp`, `cowas_loop.py` | High | S1.2–S2.3 |
| F9 | The Pi code is Linux-only (ZMQ `ipc://`, `/dev/ttyACM0`) and desktop-only (PyQt5) | `computer_interface/` | — | H1–H9 |
| F10 | The test menu and the Pi share one serial port at 9600 baud. **Decision:** Native port → Pi, Programming port → laptop (§1.11). | `main.cpp`, `C_output.cpp` | Medium | S1.1, S2.2, S2.3 |
| F11 | `DEBUG_MODE_PRINT` is a `const bool`, so the `#if` blocks never compile; fixing it would change the boot behavior | `Settings.h`, `main.cpp` | Low | S1.3 |
| F12 | The `uno` env likely can't build; `-I include/interfaces` points to a missing folder | `platformio.ini` | Low | S0.1 (report only) |
| F13 | No automated tests; `Tests.cpp` is 1,878 lines; copy-pasted `extern` blocks | — | Maintainability | S0.1, S1.1 |
| F14 | `Pump::stop()` has `delay(500)`; other blocking waits matter for the watchdog timeout | `Pump.cpp` | Low | S1.2 |
| F15 | `step_sampling()` hard-codes a 3-minute runtime, which is incompatible with 2 L samples (the volume default is 100 mL). **Resolution:** S0.2 sets 2000 mL and a timeout derived from volume ÷ expected flow × 1.5, and reports a timeout loudly; S3.3 measures the real flow; S3.4 adds the clog end condition. | `Step_functions.cpp`, `Settings.h` | **High** | S0.2 → S3.4 |
| F16 | Hard-coded slot counts (`i < 14`, `FILL_ORDER[14]`, `% 15`, "1-14" checks, `MAX_FILTER_NUMBER`) | many | Medium | S0.0 (new code), S1.1 |
| F17 | With the pressure sensor unplugged, the cap reads implausible values and refuses to run the pump; every pumping test uses the pressure PID | `Pump.cpp`, `Step_functions.cpp` | Blocking for tests | S0.0 (sensorless test mode), S0.3 |
| F18 | The O-ring lock state is RAM-only (`false` at boot): after a reboot while physically locked, the firmware would rotate while locked | `Oring_lock.cpp` | **High** | S0.0 |
| F19 | The analog pressure conversion has a hard-coded zero (`3.105 V`) and 2.5 bar/V; the sensor is powered at 5 V (ratiometric, so it drifts with the rail); the ADC runs at 10 bits although the Due supports 12 | `Pressure_sensor.cpp` | Medium | S0.3, S3.1 |
| F22 | **The firmware reads a sensor that is no longer installed.** The safety cap and all pump controllers use `read_pressure2()` (analog A2), but pressure2 has been removed. The real sensor is the SPI ABP (`pressure1`, CS D8), which was bypassed because of an "invalid SPI response" fault. The cap therefore reads a floating pin, which explains the "implausible reading, pump refuses to run" behavior. | `main.cpp`, `Step_functions.cpp` | **High** | S0.3 |
| F21 | The pressure sensor is a Honeywell **ABPDANV150PGSA3**: 0–150 psi (0–10.3 bar), SPI, 3.3 V, liquid-media option, barbed port (the `Trustability_ABP_Gage` class, chip select on D8). Its range covers the cap, but its accuracy is ±1.5 % of 10.3 bar = **±0.16 bar**, which is coarse at our 2–2.5 bar operating point and too coarse for a ΔP (clogging) measurement between two such sensors. Its barbed port is the likely cause of the pop-outs. **Proposal:** 0–60 psi versions of the same family (ABPDANV060PGSA3: same package, bus, code; ±0.06 bar) + clamped barbs. | Hardware | Medium | HW4, S0.3, S3.1 |
| F20 | `Settings.h` says "all pins are claimed", but ~17 digital and ~8 analog pins are unused in the code; the real limit is the PCB shield | `Settings.h` | Low | S0.1 (pin map) |

---

## 3. Step plan

```
Phase 0  Now          S0.0 ─► S0.1 ─► S0.2        S0.3 (floating: as soon as the new pressure sensor arrives)
Phase 1  Foundations  S1.1 ─► S1.2 ─► S1.3 ─► S1.4
Phase 2  Host link    S2.1 ─► S2.2 ─► S2.3
Phase 3  Hydraulics   S3.1 ─► S3.2 ─► S3.3 (you measure) ─► S3.4 (A|B|C) ─► S3.5 ─► S3.6        needs HW3 + HW4
Phase 4  Host         (from S2.2)  H1 ─► H2 ─► H3 ─► H4 ─► H5 ─► H6 ─► H7 ─► H8 ─► H9 ─► H10    [parallel]
Phase 5  Integration  (needs S3.5 + H9)  I1 ─► I2 ─► I3 (after the spool repair) ─► I4 (later)
Hardware track (you, anytime; see section 8)  HW0..HW9
```

| Step | Title | Behavior change? | Needs hardware | Main tests |
|---|---|---|---|---|
| **S0.0** | **Mechanical test suite**: lock tri-state + homing, lock cycles, lock + pump leak test, rotate through all slots, full cycle with optional stages, explicit sensorless pump test mode | Additive (+ lock tri-state) | — | Slot iteration, stop decisions, homing rule |
| **S0.1** | Baseline: CLAUDE.md, functionality ledger, dead-code audit, pin map, FUTURE.md, native tests, CI | None | — | Manifold math, FILL_ORDER |
| **S0.2** | Quick safety fixes: F2, F3, F4, F5 (auto-unlock), F15 | Yes, listed | — | Pressure decision, interlock, timeout |
| **S0.3** | *Floating:* new pressure sensor as `P_in` on the existing code path (+ `P_out` read-only if installed); 12-bit ADC; run the pending tests | Yes (sensor swap) | 1–2 pressure sensors | Conversion |
| **S1.1** | Mechanical restructure: split Tests.cpp, command table, `Hardware.h`, `Core/`, two-port log routing, no hard-coded slot counts | None | — | Command table ⊇ ledger; slot-count independence |
| **S1.2** | Supervisor + `wait_ms()` + watchdog | Yes: aborts on fault | — | Supervisor with fake clock and sensors |
| **S1.3** | Mode manager (console/panel become service modes; `exit`; RECOVERY_REQUIRED state) | Yes: `exit` | — | Transition table |
| **S1.4** | STOP + E-stop, safe state, fault latch, RECOVER, boot self-check | Yes | E-stop (HW1), optional at first | Latch, abort latency, recovery, boot paths |
| **S2.1** | Runtime parameters; calibration applied live, no reflash | Yes | — | Bounds, derivation, hash |
| **S2.2** | Protocol v1 spec + golden vectors | Docs only | — | — |
| **S2.3** | Protocol v1 in firmware; results, events, telemetry, logs forwarded | Yes | Native USB cable (HW2) | Vectors, fuzzing |
| **S3.1** | **Hydraulic sensing**: `P_in`/`P_out`/`Q_in`/`Q_out` channels, phases, flow fusion, ΔP, R_filter, quality flags | Yes | 2 pressure sensors + outlet flowmeter | Channel table, fusion, phase truth table |
| **S3.2** | Hydraulic trace telemetry + record, plot and analysis tools | None | — | Analyzer on synthetic traces |
| **S3.3** | **You: pump measurement campaign + decision** (section 6) | — | Test bench | — |
| **S3.4** | Pump control A, B or C + sample end conditions | Yes | — | Plant simulation |
| **S3.5** | Leak, dead-sensor and pressure-signature detection; LEAK_TEST | Yes | — | Synthetic series |
| **S3.6** | Tier-1 environmental sensors | Additive | Temperature probes, turbidity | Conversions, absent-sensor behaviour |
| **H1** | Host skeleton: transport, protocol client, device logs, simulator, CLI | New | — | Vectors, simulator |
| **H2** | Inventory: slot states, events, replicates, controls (flagged off) | New | — | Transitions, persistence |
| **H3** | Sample metadata + single-file export + merge | New | — | Schema, export, merge |
| **H4** | Scheduler + mission runner | New | — | Simulated-time scenarios |
| **H5** | Mission workflow: commission → set up → run | New | — | Workflow state machine |
| **H6** | REST + WebSocket API | New | — | API with simulator |
| **H7** | Minimal web UI (PWA, logic only) | New | — | Smoke |
| **H8** | Notifications and alarms | New | — | Alert rules |
| **H9** | Deploy on the Pi; retire the legacy code | Removes old code | Pi + TOFU | Parity checklist |
| **H10** | Remote access (Tailscale, LTE) + SMS | New | SIM, antennas | — |
| **I1** | Bench validation campaign | — | Full machine | Test plan |
| **I2** | Operational defaults (boot to IDLE) | Yes | — | Ledger re-verified |
| **I3** | Spool/depth reintegration | Yes | Repaired spool | Spool tests |
| **I4** | Jetson migration (later) | — | Jetson | — |

---

## 4. Rules every step follows (copied into CLAUDE.md in S0.1)

1. **Refactor steps change no behavior.** Behavior changes happen only in steps that say so, each listed in the PR description.
2. **The ledger is the contract.** Every entry in `docs/FUNCTIONALITY_LEDGER.md` must still work after each step. Removing or renaming one needs an explicit note and the user's approval.
3. **Nothing is deleted without evidence and approval.** Git tag `baseline-v0` keeps everything. If in doubt, keep it.
4. **Pure logic lives in `src/Core/`** (no `Arduino.h`) and is unit-tested with `pio test -e native`. Hardware code stays thin.
5. **Safety invariants must never be weakened:**
   - The pump never runs above the pressure cap, and an implausible *safety* sensor (`P_in`) stops it.
   - No manifold rotation while locked, or while the lock state is unknown.
   - Every procedure has a timeout.
   - The hardware E-stop and software STOP are always accepted.
   - The safe state is: pumps off, valves closed, O-ring stays locked.
   - Recovery is required after every stop and every boot.
   - Placeholders stay marked.
   - The **only** exception is S0.0's sensorless pump test mode: explicit, test-only, low power, time-limited, confirmed every run, and unreachable from production code.
6. **Degraded modes are explicit.** When a *non-safety* sensor fails (`P_out`, one flowmeter, an environmental sensor), keep working on what remains, raise a warning, and flag the sample's metadata. Never fail silently, and never abort a sample for a diagnostic-only sensor.
7. **No hard-coded slot counts.** Use the manifold geometry config; tests also run with other slot counts.
8. **Every step ends with:** `pio run -e due`, `pio test -e native` (and `pytest` once `host/` exists) all passing, plus a **hardware checklist** for the user.
9. **Prepared-but-not-built items** go in `docs/FUTURE.md`. **Tests blocked by missing hardware** go in `docs/TESTS_PENDING.md`, with the exact commands to run later.
10. Claude does not flash, commit, push or delete files without asking.

---

## 5. Prompts

> Copy each block into a **new** Claude Code session, on a new branch. Fill in `<...>` where present.

### S0.0: Mechanical test suite (lock, rotate, pump, full cycle), pressure sensor currently out

```text
Project: CoWaS, an autonomous aquatic eDNA sampler. An Arduino Due (PlatformIO, env `due`)
controls a pump that pushes water through Sterivex filters on a rotating manifold (slot 0 =
purge, 1..14 = samples, FILL_ORDER in Settings.h). The manifold is sealed by an O-ring that a
NEMA17 linear actuator locks and unlocks, open-loop (src/Hardware/Oring_lock.cpp: the lock
overdrives into stall against a physical stop, and the unlock moves LINEAR_ACT_LOCKED_STEPS
back). Read docs/ROADMAP.md: sections 1.4 and 1.8, findings F5, F16, F17, F18, and rules in
section 4.

CURRENT HARDWARE STATE: the pressure sensor is OUT (to be replaced). The flow sensor
(flow_sensor_small) works but is not yet calibrated.

Goal: a complete, reliable set of serial test commands to validate locking, sealing and rotation
TODAY, and to prepare the tests that need the pressure sensor. This step is ADDITIVE: existing
commands keep working exactly as before, EXCEPT the lock tri-state described in step 2.

0. Audit first. Read src/Tests.cpp (test_all_components and the existing tests: linear_actuator,
   sample_cycle, sample_cycle_full, multi_sample, multi_sample_full, calibrate_actuator,
   slotN/clockMM), src/Step_functions.cpp (load_slot, load_slot_full, goto_slot_locked,
   sample_process, step_purge, step_sampling, step_DNA_shield, step_empty) and Oring_lock.cpp.
   Print a table: required test (from the list below) | exists / partial / missing | what it
   reuses. REUSE the existing functions; do not duplicate procedure logic.

1. Sensorless pump test mode (F17). With the sensor out, Pump::enforce_pressure_safety() sees an
   implausible reading and refuses to run, and every CtrlPump uses the pressure PID. Add:
   - PRESSURE_SENSOR_AVAILABLE (Settings.h, false for now). When false, every procedure that
     needs pressure (CtrlPumpFlow/CtrlPumpNoWater users: step_purge, step_sampling, load_slot,
     step_empty, load_slot_full) REFUSES with a clear message.
   - pump_open_loop_test(power, volume_ml, max_ms): a fixed power, clamped to
     TEST_OPENLOOP_MAX_POWER (a placeholder; ask me which power is known to stay well below
     2.5 bar). It stops on flow volume reached, max_ms, flow sensor dead (no pulses for 5 s
     while running), or any keypress, and returns the reason and the volume.
   - Detaching the pressure safety is allowed ONLY inside this helper, and only after the
     operator types NOSENSOR at a prompt, every run. Print a loud warning ("pressure NOT
     monitored - hand on the power switch"). The safety is re-attached on every exit path,
     including aborts. No production code path can reach it (grep and prove it).
   - A sensorless equivalent of load_slot(): lock → open-loop water (volume/time) → open-loop
     air for TEST_AIR_PURGE_MS with V1 open → unlock. It uses the same valve sequence as
     load_slot(), so extract the valve setup into shared helpers rather than copying it. The
     flow volume counts only during the water part (air spins turbine flowmeters).

2. O-ring lock state (F18): tri-state LOCKED / UNLOCKED / UNKNOWN, UNKNOWN at boot.
   - `actuator_home` (ensure_unlocked): only if the manifold is aligned to a slot (encoder
     within ANGLE_TOLERANCE of a slot angle), run the overdrive lock (stall against the
     physical stop = a known reference), then the normal unlock. This gives a KNOWN UNLOCKED
     state. Otherwise refuse and tell the operator to align first.
   - rotateMotor() and every rotating test refuse while LOCKED or UNKNOWN, with a message
     pointing to actuator_home/unlock. (The automatic unlock comes in S0.2.)
   - Check that production paths (goto_slot_locked, sample_process) still work from boot.
     Explain what happens on the first rotation after a boot, and propose the least surprising
     behaviour; ask me before changing sample_process.

3. New or extended test commands. Every one has: a 'x' abort at every pause; an optional `auto`
   argument (no pauses); `from=N count=M`; FILL_ORDER (default) or `order=index`; NO hard-coded
   14/15 (F16); a SUMMARY TABLE at the end (slot, clock label, result, encoder angle error,
   volume, operator leak answer, notes); and one machine-parsable line per result:
   `TEST|<name>|<slot>|<PASS/FAIL/SKIP>|<key=value...>`.
   a. lock_cycle [n]: at the current slot, lock/unlock n times (default 5).
   b. leak_hold [volume|seconds]: at the current slot, lock → sensorless pump (step 1) →
      ask "leak? y/n + where" → air purge → unlock.
   c. rotate_all [dry]: per slot: unlock → rotate → check the encoder error → lock → hold N s →
      unlock. No pumping.
   d. multi_sample: extend the existing command (keep its default behaviour); it uses the
      sensorless variant automatically when PRESSURE_SENSOR_AVAILABLE is false; selectable
      volume per slot; the options above.
   e. full_cycle [slot] purge=y/n shield=y/n empty=y/n volume=<mL>: the real production step
      functions in order (purge → sampling → DNA shield → empty → unlock), each stage
      switchable; full_all runs it over the slots. Both REFUSE while PRESSURE_SENSOR_AVAILABLE
      is false, but must be complete and compile. shield_only [slot] (micro pump, time-based)
      CAN run sensorless.
   f. Anything else relevant you find missing (e.g. valve_check with operator confirmation,
      or encoder_repeat to measure the angle spread). List them before implementing; keep
      them small.
   Add all commands to the menu and to a `help` listing.

4. Pure logic in src/Core/ (no Arduino.h), with a minimal PlatformIO `native` env + Unity
   (extended in S0.1): the slot-iteration list builder (from/count/order, any NB_SLOT, FILL_ORDER
   validated as a permutation), the open-loop stop decision, the "may rotate?" rule, and the
   "may home?" rule. Test them with 15 AND 24 slot configurations.

5. docs/TESTS_PENDING.md: every test that needs the pressure sensor (full_cycle, full_all,
   sample_cycle, sample_cycle_full, multi_sample_full, the pressure-cap trip), with the exact
   commands and what to look for. These will be run in S0.3.

Do not change sample_process() without asking me. Do not touch the pump PID.
Give me a hardware test sequence for today, in order and starting safe: actuator_home →
lock_cycle → rotate_all dry → leak_hold (small volume) → multi_sample auto count=3 →
shield_only. For each, say what "pass" looks like.
```

### S0.1: Baseline, ledger, audit, pin map, unit tests, CI

```text
Project: CoWaS, an autonomous aquatic eDNA sampler (GenoRobotics). A pump pushes water through
Sterivex filters (14 sample slots + 1 purge slot on a rotating manifold, sealed by an O-ring that a
NEMA17 linear actuator locks and unlocks), then a micro pump adds DNA/RNA Shield. An Arduino Due
(PlatformIO, env `due`) controls all hardware. A Raspberry Pi (legacy code in computer_interface/,
PyQt5 + ZMQ, Linux-only) sends high-level commands over serial. The spool (depth) module is broken
(SPOOL_USE=false).

Read docs/ROADMAP.md fully first (architecture, findings F1-F20, rules in section 4, future
updates in section 7, pin budget in 8.4).
This step must NOT change firmware behavior. The goal is a safety net before any refactor.

1. CLAUDE.md at repo root: a short project summary, hardware and pin list, build/test commands,
   the rules from ROADMAP section 4 verbatim, the known hazards (placeholders like
   LINEAR_ACT_LOCKED_STEPS, flow sensor to be calibrated, pressure sensor status, spool
   disabled), and a pointer to docs/ROADMAP.md.
2. docs/FUNCTIONALITY_LEDGER.md: an exhaustive table of EVERY entry point: name | trigger |
   function (file:line) | what it does | hardware touched | blocking? | notes | verified on
   hardware (date), left empty for me. Cover every test_all_components() command (including
   the S0.0 ones and the prefix commands slotN/clockMM), every button_control() mode, every
   main_program() command, every calibration, every computer_interface/*.py command, and the
   file formats (Samples_planned, Samples_executed, CSV `date;time;depth`). Add a second table
   of all constants marked placeholder/uncalibrated/experimental/temporary.
3. docs/DEAD_CODE_AUDIT.md: functions never called, files never included, large commented-out
   blocks, unused globals, each with grep evidence and a recommendation (keep / delete / ask
   me). Delete nothing.
4. docs/FUTURE.md from ROADMAP section 7.
5. docs/PINOUT.md from ROADMAP 8.4 and Settings.h: every Due pin (D0-D53, A0-A11, DAC0/1, I2C,
   SPI, CAN), what uses it (file:line), direction, voltage level, and an EMPTY column
   "reachable on the PCB shield? (y/n/connector)" for me. Flag pins that are declared but
   unused, and pins that are used but undeclared. Replace the outdated "all pins are claimed"
   comments in Settings.h with a pointer to PINOUT.md (comment-only).
6. Extend the `native` env from S0.0. Move ONLY pure math into src/Core/: clock_minutes_for_slot,
   slot_for_clock_minutes, scale_angle, and the slot-angle table building from Manifold::begin()
   as a pure function, with bit-identical results. Record the current outputs as golden values
   BEFORE moving anything. Tests: the golden angle table (purge at index 0, the no-hole
   position never produced), slot<->clock round trip, FILL_ORDER is a permutation of the
   sample slots, NB_SLOT matches the table size. Use build_src_filter to separate the two envs.
7. .github/workflows/ci.yml: `pio run -e due` + `pio test -e native`.
8. Check whether `pio run -e uno` builds and whether include/interfaces exists. Report only.

Done when: due builds, native tests pass, the docs exist, and the firmware diff is only the math
extraction + comments. Give me (a) a hardware smoke checklist to run before I tag `baseline-v0`,
and (b) the "ask me" items from the audit.
```

### S0.2: Quick safety fixes

```text
Read CLAUDE.md and docs/ROADMAP.md (sections 1.5 and 1.7, findings F2, F3, F4, F5, F6, F15).
SMALL, listed behavior changes for safety. Nothing else.

1. One pressure safety value (ROADMAP 1.5): PRESSURE_CAP = 2.5 is the ONLY safety value. The
   PID target is derived: pressure_target() = PRESSURE_CAP - PRESSURE_TARGET_MARGIN (0.5), in
   src/Core/. A static_assert that the cap lies in 0.5-3.0 (typo guard; the runtime check comes
   in S2.1). STX_MAX_PRESSURE becomes a deprecated alias. Replace every hard-coded setpoint
   literal `2` passed to CtrlPumpFlow/CtrlPumpNoWater begin() (step_purge, step_sampling,
   load_slot, step_empty, tests, calibrations) with pressure_target(). Keep the 0.4 emptying
   setpoint. List every call site.
2. F2: in loop(), while pump.is_running(), call pump.enforce_pressure_safety() every iteration.
   List any other place where a pump can run without polling.
3. Extract the decision in Pump::enforce_pressure_safety() into a pure src/Core/ function
   (reading, cap, min_plausible -> OK / OVER_CAP / IMPLAUSIBLE; NaN = implausible) with
   boundary tests.
4. F5, automatic unlock before rotation (DECIDED). When a move is actually needed:
   - LOCKED and safe (pump and micro pump off; pressure < UNLOCK_MAX_PRESSURE when the sensor is
     available, otherwise PRESSURE_SETTLE_MS since the pump stopped) -> unlock, log, rotate.
   - LOCKED and not safe -> wait up to N s, then refuse loudly.
   - UNKNOWN -> refuse and ask for actuator_home.
   The "already at target" case must NOT unlock (step_DNA_shield relies on it). Check EVERY
   caller (goto_slot_locked, slotN, clockMM, slot0, panel Man Slot, test_manifold,
   calibrations, S0.0 tests) and report before/after behaviour. The pure decision rule
   (-> ROTATE / UNLOCK_THEN_ROTATE / WAIT / REFUSE) goes in src/Core/ with a test for every
   combination.
5. F4: remove "/ 10." in step_DNA_shield() and use a named constant DNA_SHIELD_FILL_MS =
   FILL_STERIVEX_TIME. Check that nothing relied on the short time.
6. F15, the 3-minute timeout that makes 2 L samples impossible:
   a. STX_SAMPLE_MILLILITERS = 2000. SAMPLE_VOLUME_CAP_ENABLED = true (when false, the volume
      stop is ignored; the cap and timeout still apply; the clog stop comes in S3.4).
   b. Replace set_max_runtime(3 * 60 * 1000) in step_sampling() with
      sample_max_duration_ms(volume_ml) in src/Core/:
      volume / SAMPLE_MIN_EXPECTED_FLOW_ML_PER_MIN x SAMPLE_TIMEOUT_MARGIN (1.5), bounded by
      SAMPLE_MAX_DURATION_ABS_MS; with no volume cap, use the absolute maximum.
      SAMPLE_MIN_EXPECTED_FLOW_ML_PER_MIN is a PLACEHOLDER (<~65 mL/min, i.e. ~45 min for 2 L>)
      until S3.3.
   c. CtrlPump::run() reports WHY it stopped (condition met / timeout / pressure cap), and
      step_sampling() warns loudly when a sample ends on the timeout. It must never look like
      a normal completion.
   d. List every other hard-coded runtime too short for the new volume, with a
      recommendation. Change only step_sampling's timeout unless I agree.
   e. Native tests: 2000 mL at the expected flow finishes before the timeout; no volume cap ->
      absolute maximum; bounds.
7. Do NOT touch DEBUG_MODE_PRINT (F11).

Update the ledger notes and docs/FUTURE.md if relevant. Hardware checklist: calibrate_flow
first (F6); lock, then slot5 auto-unlocks and rotates; with the pump running, slot5
waits/refuses; after a reboot, slot5 asks for actuator_home; the DNA shield pump runs ~250 s.
Pressure-dependent checks go into docs/TESTS_PENDING.md.
```

### S0.3 (floating): New pressure sensor on the existing code path

Run this as soon as the first new pressure sensor is installed. It is independent of the Phase 1–2 progress.

```text
Read CLAUDE.md, docs/ROADMAP.md (sections 1.5 and 1.6, findings F7, F17, F19, F21), docs/TESTS_PENDING.md,
include/Trustability_ABP_Gage.h, src/Hardware/Trustability_ABP_Gage.cpp, src/Hardware/Pressure_sensor.cpp,
main.cpp (read_pressure2, set_pressure_safety).
Hardware: P_in = Honeywell <ABPDANV060PGSA3 | ABPDANV150PGSA3> (SPI, 3.3 V, liquid media, barbed
port) on the SPI bus, chip select D8, between the pump and the filter. P_out: <ABPDANV060PGSA3 on
chip select D45 / not yet installed>.

Minimal step. The full channel design comes in S3.1; do not build it here.
1. Make the ABP SPI class correct for the installed part: pressure range from the part number
   (060PG = 0-60 psi, 150PG = 0-150 psi) as a constructor argument, transfer function A
   (10-90 % of 2^14 counts), psi -> bar, and the 2 status bits (normal / command mode / stale
   data / diagnostic fault). Stale or fault status counts as IMPLAUSIBLE (stop the pump). Find
   the cause of the existing "invalid SPI response" error (SPI mode/clock, CS timing, wiring:
   list the checks for me) before blaming the sensor. A zero offset from a console
   `calibrate_pressure` (pump off, lines vented, 5 s average), printed as a Settings.h value
   (runtime params come in S2.1).
2. pressure2 (analog, A2) is NOT installed any more (confirmed by me), but the firmware still
   uses it for the safety cap (set_pressure_safety(read_pressure2, ...) in main.cpp) and for
   every CtrlPumpFlow/CtrlPumpNoWater in Step_functions.cpp: they read a floating analog pin.
   Switch ALL of them to the SPI ABP sensor (P_in) and list every call site you changed. Keep
   the BigPressure class compiling, but nothing may read A2 for control or safety.
3. The safety cap and the PID use P_in (replace read_pressure2's body; keep the function name
   for now). Plausibility window updated for the sensor's range.
4. If P_out is installed: read it (same class, CS D45) and show it in the console `pressure`
   command (P_in, P_out, dP, raw counts, status) ONLY. It is not used for control or safety yet.
5. Set PRESSURE_SENSOR_AVAILABLE = true. Check that the S0.0 sensorless mode still refuses to
   detach the safety while a sensor is available (or require an explicit override).
6. Native tests: the count -> bar conversion against the datasheet points (both ranges), the
   status-bit handling, and the plausibility bounds.
Then walk me through docs/TESTS_PENDING.md in a safe order (the pressure-cap trip test first,
with a restrictor), and update the file with the results I report.
```

### S1.1: Mechanical restructure (no behavior change)

```text
Read CLAUDE.md, docs/ROADMAP.md (section 1.11, rule 7), docs/FUNCTIONALITY_LEDGER.md.
Pure restructuring. ZERO behavior change: same commands, prints and timing. If you find a bug,
write it in docs/BUGS_FOUND.md and do NOT fix it.

1. Split src/Tests.cpp into src/Service/: console.cpp (the menu loop), tests_<subsystem>.cpp,
   calibration_<subsystem>.cpp, service_io.cpp (wait_for_keypress_or_abort, jog_*, ask_yes_no).
2. Replace the if-chain in test_all_components() with a command table
   {name, is_prefix, help, handler}, plus `help`. The prefix commands and their ordering quirks
   (slot0 vs slotN, clocklist vs clockMM) must behave exactly as before.
3. include/Hardware.h holds the extern declarations of all global hardware objects and replaces
   the copy-pasted extern blocks. Objects stay defined in main.cpp.
4. Move button_control() to src/Service/panel.cpp and main_program() to
   src/Comm/legacy_protocol.cpp, unchanged.
5. Two-port output routing (ROADMAP 1.11): every Serial.print/println/read/available goes
   through one Log/IO API with a level (debug/info/warn/error) and a channel (CONSOLE, HOST).
   Settings.h: CONSOLE_PORT = Serial, HOST_PORT = Serial (the same port for now = today's
   behaviour; output byte-identical). Later, HOST_PORT = SerialUSB is the only change.
   Document the Programming-port reset-on-open and the Native-port 1200-baud rule.
6. Slot-count independence (F16): replace every hard-coded 14/15 (loops, `% 15`, "1-14" checks,
   FILL_ORDER[14], MAX_FILTER_NUMBER) with values derived from one manifold-geometry block
   (positions, no-hole positions, purge offset, angle between positions, FILL_ORDER). Today's
   behaviour must be identical: compare against the S0.1 golden values, and also test 16- and
   24-position geometries.
7. Native test: the command table contains every ledger command (a hardcoded list from it).
8. Do not delete anything.
Show a before/after file map. Hardware checklist: 10 representative ledger commands + the three
panel modes, compared against the baseline output.
```

### S1.2: Supervisor, `wait_ms()`, watchdog

```text
Read CLAUDE.md and docs/ROADMAP.md (sections 1.3 and 1.4, finding F14).
Goal: things that must ALWAYS run keep running during long procedures.

1. A Supervisor (logic in src/Core/, hardware glue outside) with service(), called at least every
   50 ms whenever anything blocks. It checks the pressure cap and safety-sensor plausibility for
   any running pump, kicks the watchdog, and exposes should_abort(). Its time and pressure
   sources are injected so native tests can fake them.
2. bool wait_ms(ms) calls service() and returns false on abort. Replace delay() in
   procedure-level code (Step_functions, maintenance, calibrations, tests); keep
   delayMicroseconds and the stepper pulse timing. Add service() calls in CtrlPump::run, the
   rotateMotor wait loop, Linear_actuator::step_pulses (every N steps), Micro_Pump::start(ms),
   Pump::start(ms), Button::waitPressedAndReleased/waitReleasedAndPressed, and the keypress
   helpers. List every change.
3. Abort propagation: procedures return early when should_abort(). For now, stop the pump and
   log; the full safe state comes in S1.4 (leave one on_abort() hook).
4. The Due hardware watchdog (watchdogSetup/watchdogEnable; configurable once per reset).
   Timeout <4 s>. Measure or estimate the longest gap between service() calls first (F14).
   A hidden console command `wdt_test` deliberately hangs.
5. Native tests: the cap trips at the exact threshold; NaN and implausible values trip; the
   latch holds until cleared; the waits see should_abort().
Hardware checklist: wdt_test resets the board; a pressure trip during sample_cycle stops the pump
and the procedure returns; sample_cycle_full timing is unchanged (within a few %).
```

### S1.3: Mode manager

```text
Read CLAUDE.md and docs/ROADMAP.md (section 1.2, finding F11).
1. A pure ModeMachine in src/Core/: BOOT, RECOVERY_REQUIRED, IDLE, BUSY, SERVICE_CONSOLE,
   SERVICE_PANEL, FAULT, ESTOP, with an explicit transition table. Illegal transitions are
   rejected and logged. FAULT/ESTOP/RECOVERY_REQUIRED exist now but are wired in S1.4 (for
   now, BOOT goes straight to the default mode).
2. SERVICE_CONSOLE = today's test menu. The command that leaves it is renamed `exit`. The old
   `stop` (which also left the test menu) remains an alias of `exit` with a deprecation notice;
   S1.4 turns `stop` into the emergency STOP. SERVICE_PANEL = button_control(): enter it with
   the console `panel` or by holding START 3 s at boot; leave it with a 3 s long-press on START.
3. IDLE runs the legacy Pi text protocol (main_program), unchanged, until H9. While a legacy
   command runs, the mode is BUSY.
4. BOOT_DEFAULT_MODE = SERVICE_CONSOLE (today's behaviour) until step I2.
5. The LEDs show the mode (a pattern table).
6. F11: make DEBUG_MODE_PRINT a real #define, but gate before_start_program()/system_checkup()
   behind SYSTEM_CHECKUP_AT_BOOT = false (boot behaviour unchanged). Expose system_checkup() as
   the console `checkup`.
7. Native tests: every legal and illegal transition, and service modes reject host commands.
Update the ledger (exit, panel, checkup). Hardware checklist included.
```

### S1.4: STOP + E-stop, safe state, fault latch, recovery, boot self-check

```text
Read CLAUDE.md and docs/ROADMAP.md (section 1.4: the DECIDED safe state and recovery; 1.2 modes).
Hardware: E-stop <installed on pin N (NC/NO) with the 24 V relay | NOT YET: implement behind
ESTOP_INPUT_INSTALLED=false, software STOP only>.

1. enter_safe_state(), idempotent, callable from anywhere: pump and micro pump off, ALL valves
   closed, manifold motor and spool stopped, O-ring STAYS LOCKED if locked. If unlocked (e.g.
   mid-rotation), do NOT drive it into a lock; record it. The stepper driver stays enabled if
   locked (holding; check heat/current), otherwise disabled. Snapshot at stop time: slot, lock
   state, procedure, step.
2. FaultCode enum replacing the Critical_error.cpp stub (OVER_PRESSURE, IMPLAUSIBLE_PRESSURE,
   ESTOP, USER_STOP, TIMEOUT, ENCODER, ROTATION_REFUSED, LEAK, SENSOR_DEAD, ...), a latched
   current fault, and a 16-entry ring buffer with timestamps. Every fault emits an event line
   (a protocol event in S2.3, a notification in H8). Warnings (non-safety, rule 6) get their
   own severity and never latch.
3. E-stop input: debounced; the ISR only sets a flag; service() acts on it. Review the existing
   ISR_emergency_stop_up/down (spool endstops) and document whether they are safe.
4. Software STOP: `stop` is now the emergency stop (remove the S1.3 alias; `exit` leaves the
   console). It calls enter_safe_state() from ANY blocking wait (service() peeks at the serial
   buffer). The same entry point will serve the protocol STOP (S2.3).
5. `clear` is refused while the cause persists. `recover` runs the ROADMAP 1.4 checklist step by
   step, printing PASS/FAIL, and stops at the first FAIL.
6. Automatic enforcement (ROADMAP 1.4):
   - After a stop while powered: latched until RECOVER passes, even after the button is
     released. Every procedure and moving test command is refused with "RECOVERY REQUIRED
     (cause: ...)"; a console reminder every 30 s.
   - Every boot: RECOVERY_REQUIRED. The O-ring is UNKNOWN (S0.0), valves driven closed, the
     non-moving checks run automatically, an E-stop pressed at boot goes to ESTOP, and moving
     steps wait for console confirmation or the host's RECOVER.
   - Report the reset cause if the SAM3X reset controller exposes it (power-on / watchdog /
     software / user).
7. `faults` prints the ring buffer.
8. Native tests: the latch, clear refused while the cause persists, abort latency < 100 ms in
   simulation, recovery ordering and stop-on-FAIL (fake hardware), procedures refused while
   recovery is required, and every boot path.
Hardware checklist: E-stop and software stop during pump, rotation, actuator move, DNA shield;
valves closed and O-ring still locked; after release, commands are refused; clear + recover,
then a normal run; power-cycle mid-lock, and the boot asks for recovery.
```

### S2.1: Runtime parameters: calibration without reflashing

Why not "the Pi edits Settings.h and reflashes the Due"? It needs a compiler on the Pi, a failed flash can leave the Due without firmware in the field, reflashing resets the Due, and the code ends up differing per machine. Runtime parameters give the same result (calibrate → applied → continue, no human editing) without any of that. Firmware *code* updates from the Pi stay a future item (section 7).

```text
Read CLAUDE.md, docs/ROADMAP.md (sections 1.1, 1.5, 1.6, 1.11).
Goal: calibration without editing code or reflashing. The Due runs a calibration → applies it
live → (from S2.3) reports it to the host → the host stores it and pushes it at every boot.

1. Parameter registry in src/Core/: name, type, unit, default (Settings.h), valid range (input
   validation), editable-only-in-IDLE flag, and a "calibration" tag. Include: PRESSURE_CAP
   (the single safety value, range 0.5-3.0) + PRESSURE_TARGET_MARGIN (target derived); the
   volume cap + enabled flag; SAMPLE_MIN_EXPECTED_FLOW + the timeout margin; purge volume;
   timeouts; LINEAR_ACT_LOCKED_STEPS; the overdrive factor; the flow calibration factors (one
   per flowmeter, including the future outlet meter); the pressure zero offsets (one per
   channel); DNA shield times/power; purge_angle; angle_offset_pos/neg; the manifold geometry
   (read-only). Code reads the registry instead of the constants.
2. Console `param list|get|set|dump`. A param hash, for HELLO/STATUS later.
3. Every calibration (cal, calibrate_actuator, calibrate_flow, calibrate_dna_volume,
   calibrate_pressure, calibrate_all) keeps its human prompts AND ends with a structured result
   {param: value, ...} plus "apply now? y/n". Applied values are live immediately and marked
   "unsaved" (until a host stores them, from S2.3/H5).
4. Values live in RAM, with compile-time defaults as fallback (no EEPROM; flash is wiped on
   reflash). Interim until the host exists: `param dump` prints a ready-to-paste
   include/Calibration_defaults.h (included by Settings.h), so values survive a deliberate
   reflash. tools/export_calibration_header.py produces the same file from the host's store
   later. A human reviews and commits it; this is never automatic reflashing.
5. Native tests: range validation, types, hash stability, the derived target follows the cap,
   and a calibration result updates exactly the tagged params.
```

### S2.2: Protocol v1 specification (docs only)

```text
Read CLAUDE.md, docs/ROADMAP.md (sections 1.1, 1.2, 1.4, 1.6, 1.7, 1.8, 1.9, 1.11), the ledger
(legacy main_program commands + computer_interface/).
Write docs/PROTOCOL.md and protocol/vectors.json. NO code.

- Transport (DECIDED, 1.11): HOST channel = Native USB (SerialUSB); CONSOLE = Programming port
  (Serial); both can be bound to one port during the transition. Machine lines are JSON starting
  with '{'; on a shared port, other lines are human text. On the host channel every log line is
  ALSO sent as {"t":"log","lvl":..,"msg":..}. Never open the Native port at 1200 baud; opening
  the Programming port resets the board. Baud rates.
- Types: req {id, cmd, args}, ack, result {id, ok, data}, err {id, code, msg}, evt {type, ...}
  (mode change, fault, warning, procedure step start/end, phase change), tel {...} (~2 Hz while
  BUSY; the 10 Hz trace comes in S3.2).
- HELLO: protocol version, firmware hash, param hash + "params: defaults|pushed", mode,
  recovery_required + cause, capability flags (p_in, p_out, q_in, q_out, estop_input, spool,
  neg_control, pos_control, sensors[]).
- Commands: PING, STATUS, STOP (always accepted), CLEAR_FAULT, RECOVER{drain?,
  confirm_moving_steps}, ACTUATOR_HOME, GET_PARAMS/SET_PARAMS, CALIBRATE_*{...} (the result
  carries the values), GET_INFO (the manifold geometry: positions, NB_SLOT, purge, FILL_ORDER,
  clock labels; the host must not assume 14), ROTATE{slot}, LOCK/UNLOCK, PURGE{volume_ml},
  SAMPLE{slot, volume_cap_ml|null, max_duration_s, depth_cm}, DNA_SHIELD{slot}, EMPTY,
  PURGE_PIPES, PRIME_DNA, LEAK_TEST{slot} (ERR not_available until S3.5), RELOAD (define what it
  means now), SPOOL_* (ERR disabled while SPOOL_USE=false), NEG_CONTROL/POS_CONTROL{slot}
  (reserved: ERR not_available). Map every legacy command to its new equivalent.
- SAMPLE result (every hydraulic field nullable until S3.1 provides it): filtered_ml (fused),
  v_in_ml, v_out_ml, flow_agreement_pct, duration_s, end_reason (volume_reached / clogged /
  pressure_cap / timeout / fault / stopped), p_in/p_out/dp {max, mean, final}, flow {mean, min,
  final}, r_filter {start, end}, clog_index, quality_flags[], air_phase {duration_s, p_in_max},
  shield_ms, t_end_to_shield_s, faults[], warnings[], sensors[] {name, value, unit, stat}.
- Idempotency: a retried request id never runs a sample twice; how long ids are remembered.
- Link loss: finish or abort the current procedure safely; never start a new one. Heartbeat.
- Error codes; the modes in which each command is allowed.
- vectors.json: 30+ examples, including malformed input and null hydraulic fields, used by both
  C++ and Python tests.
Ask me the open questions at the end instead of guessing.
```

### S2.3: Protocol v1 in firmware

```text
Read CLAUDE.md, docs/PROTOCOL.md, protocol/vectors.json.
1. ArduinoJson. The parser/serializer + dispatch table live in src/Core/ (native-tested against
   vectors.json, plus malformed and oversized lines).
2. Handlers call the EXISTING procedures with explicit parameters. SAMPLE takes the slot from the
   host (the internal slot search stays for the legacy commands only). A SampleMetrics
   accumulator fills today's available fields (P_in stats, Q_in volume/flow, duration,
   end_reason from S0.2's CtrlPump stop reason); the fields for sensors not yet installed are
   null. volume_cap_ml = null disables the volume stop. The legacy commands are unchanged.
3. GET/SET_PARAMS and CALIBRATE_* map onto the S2.1 registry and calibration results.
4. Events (mode, fault, warning, procedure step, phase) and telemetry via service(). The protocol
   STOP uses the same path as the console stop.
5. Channel routing from S1.1: protocol + structured logs on HOST_PORT, human text on
   CONSOLE_PORT. Test both a shared port and separate ports (HOST_PORT = SerialUSB).
6. Request-id dedupe; capability flags in HELLO.
7. tools/protocol_smoke.py (pyserial): HELLO, STATUS, ROTATE 3, LOCK/UNLOCK, STOP mid-PURGE,
   CLEAR_FAULT, RECOVER, SET_PARAMS round trip, NEG_CONTROL -> not_available. A pass/fail
   summary.
Hardware checklist: protocol_smoke.py over the Native USB port; the legacy Pi commands still work.
```

### S3.1: Hydraulic sensing: 2 pressures + 2 flows, phase-aware

```text
Read CLAUDE.md, docs/ROADMAP.md (section 1.6 in full, rule 6), docs/PROTOCOL.md (SAMPLE result),
the S0.3 pressure code, Flow_sensor.cpp/.h.
Hardware installed: P_in <model/range/pin>, P_out <model/range/pin>, Q_in <model/pin/factor>,
Q_out <model/pin/factor>. Confirm or correct the layout assumed in ROADMAP 1.6:
<Q_in -> pump -> P_in -> manifold -> filter -> P_out -> Q_out -> outlet>.

1. Channel table (src/Core/ logic + thin drivers): named channels P_in, P_out, Q_in, Q_out
   (extendable), each with pin, conversion, calibration (S2.1 params), range, plausibility
   window, and status (OK / DEGRADED / ABSENT). Replace every direct pressure2/read_pressure2/
   flow_sensor_small use with channel reads (keep the old classes compiling for the service
   tests). The flowmeters use interrupts on any Due pin; update the pull-ups to 3.3 V
   (ROADMAP 8.4).
2. Roles (DECIDED, 1.6): P_in = safety + PID (implausible P_in -> stop). The cap is evaluated on
   max(P_in, P_out). P_out = the clogging measurement (dP); a P_out failure -> WARNING +
   quality flag, the clog measure degrades to P_in/Q (flagged), never a stop. Flowmeters:
   fused Q = the mean when both are valid and within FLOW_AGREE_TOL; on disagreement, flag
   and let S3.5 decide leak vs dead meter; if one is dead, use the other (flagged).
3. Phase awareness: a pumping-phase variable (PRIME, WATER, AIR_PURGE, SHIELD, IDLE) set by
   every step function and test that pumps. Implement the ROADMAP 1.6 phase table EXACTLY,
   with a blanking window after each transition. Volumes (V_in, V_out, filtered) count only in
   WATER. dP/R are "undefined" (not 0 or inf) below Q_min/dP_min. Check that the existing air
   detection (CtrlPumpNoWater) behaves as before, and that the P_in lower plausibility bound
   tolerates near-zero or slightly negative air-phase readings. List every place where the
   phase is set.
4. Derived signals in src/Core/: dP, R_filter = dP / Q_fused, clog index (R end / R start),
   low-pass filtered. Fill every SampleMetrics field from PROTOCOL.md (no more nulls for
   installed sensors), plus quality_flags.
5. Calibration: calibrate_pressure zeros both P channels, AND an automatic zero runs before
   every sample (pump off, lines vented; it removes the ABP offset and its thermal drift);
   calibrate_pressure_pair (at commissioning: outlet closed, pump to ~2 bar, both sensors see
   the same pressure) computes a relative gain so that dP reads 0 there. Both are recommended
   with the 060PG sensors and REQUIRED if 150PG sensors are used (F21); document the resulting
   dP uncertainty. calibrate_flow calibrates each meter
   against a measuring cylinder; flow_crosscheck runs a leak-free reference and reports the
   V_in/V_out ratio (a relative drift indicator, not an automatic correction). All return
   S2.1 params.
6. Console `hydro`: live P_in, P_out, dP, Q_in, Q_out, Q_fused, R, phase, and channel status.
7. Native tests: conversions; the role truth table (P_out/flowmeter problems -> warning, P_in
   problem -> stop); the cap on max(); fusion (agree / disagree / one dead); dP/R with Q ~ 0;
   the clog index; and a synthetic full-cycle trace (water -> air purge -> shield) where air
   adds no volume and triggers no clog, no leak and no fault, but the cap still trips in air
   if P_in exceeds it.
Hardware checklist: zero and calibrate; a clean filter (dP small, flows agree); a restrictor
after the filter (P_out rises); a restrictor at the filter outlet (dP rises, flow falls); unplug
P_out (warning, sampling continues); unplug P_in (pump stops); a full cycle where V does not
grow during the air purge.
```

### S3.2: Hydraulic trace telemetry and analysis tools

```text
Read CLAUDE.md, docs/ROADMAP.md (sections 1.7 and 6).
No control changes.
1. A 10 Hz trace mode (a param): t, phase, P_in, P_out, dP, Q_in, Q_out, Q_fused, V_filtered,
   R_filter, pump command, PID P/I/D terms, setpoint.
2. Console `pump_open_loop <power%> <seconds>` (reuses S0.0's helper WITH the pressure cap
   active; refuse if P_in is unavailable) and `pump_step_series` (20/40/60/80 %, 20 s each),
   both with the trace on.
3. tools/record_trace.py -> data/pump_traces/<date>_<run>.csv + a JSON sidecar (filter, water
   type, restrictor setting, notes, param snapshot).
4. tools/analyze_trace.py: max P_in, overshoot over target, cap trips, margin to the cap,
   steady-state oscillation, settle time, plant gain K per power step, R_filter vs volume,
   flow agreement, final flow and volume, end reason; plots.
   tools/compare_runs.py: a multi-run table including K_clogged / K_clean.
5. pytest for the analyzer on synthetic traces with known answers.
Then tell me to follow ROADMAP section 6.
```

### S3.4: Pump control + sample end conditions (paste the common part + the variant section 6 chose)

Common part:

```text
Read CLAUDE.md, docs/ROADMAP.md (sections 1.5, 1.6, 1.7 and 6), docs/PUMP_RESULTS.md (my results
and decision), data/pump_traces/.

Common to every variant:
1. Sample end conditions (1.7), src/Core/ logic, params, evaluated in the WATER phase only:
   - clogging (ALWAYS on): Q_fused < CLOG_Q_MIN for CLOG_T_S at P_in >= target - margin, OR
     R_filter > CLOG_R_MAX, OR clog index > CLOG_INDEX_MAX (falling back to P_in/Q, flagged,
     if P_out is degraded). Thresholds from my results.
   - volume (optional, default on, 2000 mL; volume_cap_ml = null disables it).
   - max duration (always on): set SAMPLE_MIN_EXPECTED_FLOW from my measured 2 L runs, so the
     S0.2 formula gives ~1.5x the measured duration.
   - the pressure cap trip stays independent.
   The first condition met sets end_reason.
2. A fixed 10 Hz control period, measured.
3. A plant model fitted to my traces (pump curve + R(V)) in tools/ (Python) and test/ (C++).
4. Native tests on the simulated plant from clean to fully clogged: never above the cap;
   overshoot < <0.2> bar; no sustained oscillation; each end condition fires; no volume cap ->
   runs until clog or timeout.
Hardware checklist: repeat runs R2-R4 (section 6) and compare with compare_runs.py.
```

Variant **A**, current PID is fine:
```text
Variant A: keep the current PID gains. Only the common part.
```

Variant **B**, cheap fixes:
```text
Variant B: add anti-windup (setMaxIOutput) and an output ramp limit (setOutputRampRate). Tune
them in simulation and show the before/after responses. If B still fails for my worst run, stop
and tell me before doing C.
```

Variant **C**, gain scheduling (the "two PIDs", blended):
```text
Variant C: two gain sets (CLEAN, CLOGGED) from my step tests (K_clean, K_clogged), interpolated
continuously by the low-pass-filtered R_filter between R_clean and R_clogged, plus the variant B
protections. Prove bumplessness in simulation (no output jump > <5>%). Only if continuous
blending fails: a discrete switch with bumpless transfer (integral re-init) + hysteresis, with
the reason explained.
```

### S3.5: Leak, dead-sensor and pressure-signature detection

```text
Read CLAUDE.md, docs/ROADMAP.md (section 1.6, rule 6), the S3.1 channel code.
All checks run in the WATER phase only, after a settling window, with params:
1. Leak: V_in - V_out > max(abs_tol, rel_tol * V_in) -> FAULT LEAK (safe state, event, later a
   critical notification).
2. Dead flowmeter vs leak: one meter at exactly 0 while the other counts AND the pressures are
   normal -> that meter is DEAD (warning, degraded mode), NOT a leak. Write the full truth table
   (Q_in, Q_out, P_in, P_out patterns -> diagnosis -> action) in docs/DIAGNOSTICS.md and
   implement it in src/Core/.
3. Pressure signatures: a sudden P_out drop with a stable P_in -> suspected leak between them
   (FAULT); P_out ~ P_in during a sample -> filter torn or missing (FAULT, slot FAILED); P_out
   rising -> blocked outlet (FAULT). P_in stuck (no change while the pump command changes) ->
   SENSOR_DEAD (FAULT, since it's the safety sensor).
4. LEAK_TEST{slot}: lock, pressurize to X, stop the pump, measure the P_in decay over T s; pass if
   the decay < threshold. Available via the protocol and the console (commissioning, optionally
   before each sample). Enable it in the protocol (it was not_available).
5. The enclosure leak sensor, if installed (pin <N>): FAULT LEAK_ENCLOSURE.
6. Native tests: a leak ramp, a sudden leak, each dead meter, each pressure signature, and a
   normal run plus a normal full cycle with air phases -> no false positive.
```

### S3.6: Tier-1 environmental sensors

```text
Read CLAUDE.md, docs/ROADMAP.md (section 1.9 table), docs/PROTOCOL.md (sensors[]).
Installed: <e.g. 2 one-wire waterproof temperature probes on pin N (water, storage compartment),
an analog turbidity sensor on pin M via a divider>.
1. A Sensor interface (read, name, unit, plausibility, status) + drivers in a table; a missing or
   failed sensor reports its status and never blocks sampling (rule 6).
2. Low-rate telemetry; per-sample stats (mean/min/max over WATER) in SAMPLE's sensors[]; the
   compartment temperature is logged every N minutes between samples.
3. Turbidity is exported as a relative index (not NTU) unless calibrated later.
4. HELLO capability flags list the sensors. The host needs no code change; check the export
   columns appear.
5. Native tests: conversions, plausibility, absent-sensor behaviour.
```

### H1: Host skeleton, transport, device logs, simulator, CLI [parallel from S2.2]

```text
Read CLAUDE.md, docs/ROADMAP.md (sections 1.1 and 1.11), docs/PROTOCOL.md, protocol/vectors.json,
and computer_interface/ (legacy; do not modify it).
host/: a Python 3.11+ package `cowas`, cross-platform (Windows, Pi, later a Jetson). No PyQt, no
ZMQ ipc://, no Pi-specific imports outside adapters/.
1. pyproject.toml, ruff, pytest, a src layout, logging.
2. cowas.protocol: pydantic messages from PROTOCOL.md, tested against vectors.json (including
   null hydraulic fields).
3. cowas.transport: pyserial, auto-detection by USB VID/PID (prefer the Native port; never
   1200 baud), reconnect. '{' lines -> protocol; other lines + {"t":"log"} messages -> device
   log store (timestamped, searchable, rotated).
4. cowas.controller_client: request/response with id, timeout, retry-safe resend, and an
   event/telemetry subscription.
5. cowas.sim.FakeController: the protocol with virtual time and configurable faults and sensor
   sets (pressure trip, clog, leak, dead meter, P_out missing, link drop, slow procedure). It
   emits human log lines too and works as a transport in tests.
6. CLI `cowas`: ping, status, stop, recover, rotate, sample --slot --volume|--no-volume-cap,
   params get/set, calibrate, monitor (live telemetry + logs), logs.
7. Tests: vectors; client vs simulator (timeouts, reconnect, dedupe on retry, STOP during
   SAMPLE, log separation). CI runs pytest.
Bench check (after S2.3): `cowas status` and `cowas monitor` against the real Due.
```

### H2: Inventory, sampling events, replicates, controls

```text
Read CLAUDE.md and docs/ROADMAP.md (section 1.8, finding F1).
1. cowas.domain: SlotState (EMPTY, READY, IN_PROGRESS, SAMPLED, FAILED, DISABLED, SUSPECT) with an
   explicit transition table; Slot (the inventory is sized from GET_INFO's geometry, never 14;
   a geometry change between deployments is detected and handled); Deployment (site, operator,
   filter lot, start/end); SamplingEvent (planned time, depth, volume cap or none, replicates
   1-3, include_negative_control / include_positive_control, default false); Filter (event,
   role, replicate index, slot).
2. Feature flags (from HELLO capabilities + host config): neg_control, pos_control, spool.
   Validation rejects events that need a missing capability, with a clear message.
3. cowas.store: SQLite, a repository layer, schema versioning, one transaction per transition.
   At startup, IN_PROGRESS -> SUSPECT.
4. Reload flow: the operator confirms which slots have fresh filters; the previous deployment is
   archived; those slots become READY; DISABLED keeps its reason.
5. Tests: every legal and illegal transition, replicate allocation following FILL_ORDER, controls
   rejected while flagged off, persistence, crash recovery, and 15- and 24-slot geometries.
```

### H3: Sample metadata and bioinformatics export

```text
Read CLAUDE.md, docs/ROADMAP.md (section 1.9), docs/PROTOCOL.md (SAMPLE result), host/.
1. SampleMetadata per filter with every field from ROADMAP 1.9, including V_in/V_out/fused
   volume, flow agreement, dP/R/clog index, and quality_flags. Times are ISO 8601 UTC + the site
   time zone. Location comes from the deployment (manual) or a GNSS adapter interface ("manual
   only" for now; GNSS goes in FUTURE.md). The generic sensors list is accepted without
   migration.
2. Filled automatically from the SAMPLE result + event + deployment + calibration set + versions.
   Operator notes are editable, with an audit trail.
3. Export = ONE file with all selected samples (a campaign, a date range, or all): flat CSV, one
   row per filter (controls and replicates included, grouped by event and replicate ids), or
   JSON. Every row has deployment_id, device_id and a globally unique sample_id; a header or
   sidecar gives schema_version and the export time. docs/METADATA.md maps the columns to FAIRe
   and MIxS (water): VERIFY the exact current term names from the official sources and flag
   uncertain ones instead of inventing them. Sensors become <name>_<stat>_<unit> columns;
   files with different sensor sets still merge.
4. `cowas export merge a.csv b.csv ... -o all.csv`: checks and upgrades schema versions, builds
   the union of columns, removes identical duplicates, reports conflicts.
5. Optional per-sample 10 Hz trace file, referenced by sample id.
6. Tests: schema round trip, export golden file, an unknown sensor needs no code change, an
   event with 3 replicates, a merge with different sensor columns, duplicates/conflicts, and a
   sample with a degraded P_out (the flag appears).
```

### H4: Scheduler and mission runner

```text
Read CLAUDE.md, docs/ROADMAP.md, host/ (H1-H3).
1. Plans: explicit SamplingEvents, recurring (start, interval, count, defaults), and the legacy
   imports (Samples_planned "dd/mm/yy HH:MM depth", CSV "date;time;depth"). The validation from
   computer_interface/cowas_interface.py (max depth 45 m, future dates) becomes reusable
   functions.
2. Runner: one procedure at a time. For each event, allocate the filters to the next READY slots
   in FILL_ORDER (from GET_INFO), run SAMPLE for each, and store the metadata. Policies
   (configurable): late event (run if late < X min, else skip and log); sample fault (slot
   FAILED, retry on the next slot up to N times); system fault (ESTOP / USER_STOP / LEAK /
   OVER_PRESSURE: pause the mission, alert, never auto-retry); a warning (degraded sensor):
   continue and flag; out of filters.
3. An injectable clock; tests in simulated time with FakeController.
4. Scenarios: 14 samples over 7 days; 4 events x 3 replicates; power loss mid-sample (SUSPECT,
   then continue); link drop; a clog on sample 3 (end_reason clogged, not a fault); P_out lost
   mid-mission (continue, flagged); STOP from the API; out of filters; overlapping events
   rejected.
```

### H5: Mission workflow (commission → set up → run)

```text
Read CLAUDE.md, docs/ROADMAP.md, host/.
1. Mission state machine: IDLE -> COMMISSIONING -> CONFIGURED -> RUNNING <-> PAUSED -> COMPLETED
   | ABORTED. RUNNING requires: passed commissioning, a synced clock, a valid plan, enough READY
   slots, params pushed with a matching hash, and no recovery pending.
2. The calibration store: calibration results from the Due (S2.1/S2.3) are stored as versioned
   calibration sets and pushed at every connect; the export header tool
   (tools/export_calibration_header.py) reads from here.
3. Commissioning checklist (pass/fail/skip + note + stored results): link and firmware version,
   params pushed, manifold check (purge + slot 1), actuator home + lock/unlock, flow calibration
   + cross-check, pressure zero, LEAK_TEST (when available), a dry purge. Interactive
   calibrations stay in the service console for now; the checklist records their values. List
   the ones to port to app-driven prompts in FUTURE.md.
4. Setup: plan editing with feasibility checks (measured 2 L duration vs interval, slots needed
   including replicates, depth ignored while the spool is disabled, with a warning).
5. Cold-boot resume: reload the mission, handle SUSPECT slots, recompute the next event, apply
   the late policy.
6. Tests: the full workflow in simulation, including a cold boot in every mission state.
```

### H6: API

```text
Read CLAUDE.md, host/.
FastAPI: GET /status; /mission (+ POST commission|start|pause|resume|abort);
/mission/commissioning; /plan (CRUD + CSV import); /slots and PATCH /slots/{n} (disable/enable
with a reason); /samples, /samples/export.csv|.json (query: deployment, date range);
/calibration (sets, history); /params; /faults (+ POST clear, POST recover); POST /stop (always
allowed, highest priority); /alerts (+ POST /alerts/{id}/ack); /logs (filterable device logs);
WebSocket /ws (telemetry, events, alerts). Token auth (operator + read-only). The OpenAPI docs are
the logical interface spec, so write good descriptions. Tests with httpx against FakeController,
including STOP during a SAMPLE and permissions.
```

### H7: Minimal web UI (logic, not design)

```text
Read CLAUDE.md, host/ API.
A static PWA served by FastAPI (no build step, plain HTML/JS, mobile-first): Dashboard (mission
state, next event, a big STOP always visible, live P_in/P_out/flows, active alerts with ack, a
"Recovery required" banner with the RECOVER flow); a Slots grid drawn from the controller's
geometry (state colours, tap to disable/enable with a reason); Plan (events with replicates;
control options greyed out while unavailable; CSV import); Commissioning checklist; Samples
(list, metadata detail with quality flags, export); Alerts history; Settings (PRESSURE_CAP and
other params, calibration history, a Service page hidden by default). A reconnecting WebSocket.
The design comes later.
```

### H8: Notifications and alarms

```text
Read CLAUDE.md, docs/ROADMAP.md (section 1.10), host/.
1. cowas.alerts: rules mapping controller events/warnings and host checks to severities exactly
   as in ROADMAP 1.10. Host checks: controller link lost > X min, mission paused, few READY
   slots, late event, low disk, clock not synced, recovery pending.
2. Lifecycle: raised -> delivered -> acknowledged (who, when) -> resolved. Critical alerts repeat
   every <10> min until acknowledged. Dedup and rate limiting.
3. Channels behind one interface: in-app (WebSocket), Web Push for the PWA (VAPID), ntfy or a
   Telegram bot (configurable); an SMS stub for H10.
4. Per-user channel config; a "send test alert" endpoint.
5. Tests: the rule table in simulation (leak -> critical, repeated until acked; P_out degraded ->
   one warning), dedup, rate limiting, delivery failure + retry.
```

### H9: Deploy on the Pi, retire the legacy code

```text
Read CLAUDE.md, host/, computer_interface/, docs/FUNCTIONALITY_LEDGER.md.
1. A deployment guide + scripts for Raspberry Pi OS on the CM4/TOFU: a venv, a systemd service
   with auto-restart, journald, a udev rule for the Due's Native port (a stable name), a Wi-Fi
   hotspot fallback, a clock check (refuse missions if not synced; check the TOFU RTC), a
   nightly SQLite backup, an update procedure, and a cold-boot resume test.
2. Parity checklist: every function of computer_interface/ and every legacy main_program command
   mapped to its new equivalent. Only after I confirm parity on hardware: move
   computer_interface/ to legacy/, and remove the legacy text protocol from the firmware (a
   separate commit).
```

### H10: Remote access

```text
Read CLAUDE.md, host/.
1. Docs + scripts: Tailscale on the Pi and the phones; LTE via the TOFU modem (ModemManager/
   NetworkManager, APN <...>) with automatic failover between Wi-Fi and LTE.
2. An SMS alert channel through the modem (critical only), filling the H8 stub.
3. Bandwidth mode: downsample telemetry on LTE; device logs on demand.
4. If the modem provides GNSS, implement the H3 GNSS adapter and remove it from FUTURE.md.
```

### I1: Bench validation campaign

```text
Read CLAUDE.md, docs/ROADMAP.md, docs/FUNCTIONALITY_LEDGER.md.
Write docs/VALIDATION_PLAN.md + scripts in tools/ that drive the host API and collect logs: a
full 14-slot dry run on a water bucket; a 48-72 h soak with events every 2-4 h, including a
3-replicate event; a power cut in each phase (purge, water, air purge, shield, empty, rotation,
lock); link loss; hardware E-stop AND app STOP in each phase, each followed by recover; a
clogged filter; a simulated leak; a dead flowmeter; P_out unplugged; every critical alert
reaching the phone and repeating until acked; and the export file checked against the lab's
needs. Each test has pass criteria and a results table, plus a report template.
```

### I2: Operational defaults

```text
Read CLAUDE.md. Flip BOOT_DEFAULT_MODE to IDLE (operational, after the boot recovery). The
service console and panel stay reachable (console `service`, holding START at boot, the app's
Settings > Service with the operator token). Update the ledger and CLAUDE.md. Hardware
checklist: a cold boot -> recovery -> IDLE; a Pi mission runs; the service modes are reachable.
```

### I3: Spool / depth reintegration (after the repair)

```text
Read CLAUDE.md, docs/FUNCTIONALITY_LEDGER.md (spool entries), src/Hardware/Motor.cpp,
Step_functions.cpp (step_dive, step_rewind). Hardware changes: <describe>.
DIVE{depth_cm}/REWIND as protocol procedures: endstop interrupts reviewed, a max depth param,
timeouts, cable-length accounting, supervisor integration, a PRIME phase for the longer tubing.
Wire the depth into SAMPLE and the metadata (planned vs achieved). Set the spool capability flag.
Native tests for the depth/encoder math. A careful hardware checklist (no water first, low speed,
a hand on the E-stop).
```

---

## 6. Pump measurement and decision procedure (S3.3, done by you)

Goal: decide from data whether the current PID is enough (A), needs protections (B), or needs gain scheduling (C), and get the thresholds for clog detection and the timeout.

### 6.1 Preparation

- [ ] S3.1 and S3.2 are flashed. Both flowmeters are calibrated, and the flow cross-check passes. Both pressure channels are zeroed.
- [ ] PRESSURE_CAP = 2.5 (PID target 2.0).
- [ ] Bench:
  - a water bucket
  - a **needle valve or tube clamp** at the filter outlet, for reproducible "fake clogging"
  - ≥ 6 new Sterivex filters
  - **silty water** made with a fixed recipe (e.g. X g of fine soil or clay per litre, stirred), recorded
  - a measuring cylinder
- [ ] Hand on the E-stop for every run. Use a spare manifold slot.
- [ ] Create `docs/PUMP_RESULTS.md` with the table from 6.3.

### 6.2 Runs

Each run: `python tools/record_trace.py --run <ID> --notes "..."`, then `python tools/analyze_trace.py <file>`.

| Run | What | How | Repeats | Why |
|---|---|---|---|---|
| **R1a** | Open-loop steps, clean filter | `pump_step_series` | 2 | Pump curve, K_clean |
| **R1b** | Open-loop steps, restrictor ½ closed | same | 1 | K at medium resistance |
| **R1c** | Open-loop steps, restrictor nearly closed (stop before the cap) | same | 1 | K_clogged |
| **R2** | Current PID, clean filter, 2 L of clean water | `full_cycle volume=2000` (or `sample_cycle`) | 2 | Baseline: overshoot, oscillation, **real sampling flow**, 2 L duration, flow agreement |
| **R3** | Current PID, clean filter; close the restrictor to ~¾ in one move at ~30 s | same | 2 | Sudden clog: spike size, ΔP response |
| **R4** | Current PID, silty water until clogged (volume cap off, max 45 min) | same, volume cap disabled | 3 (new filter each) | Real clogging: R_filter(V), clog signature |

Stop any run early if the pressure exceeds the cap without a trip. That is a safety bug: report it before continuing.

### 6.3 Results table (copy into docs/PUMP_RESULTS.md)

| Run | Max P_in | Overshoot | Margin to cap | Cap trips | Oscillation (±bar) | Settle (s) | K (bar/%) | Flow start → end (mL/min) | Flow agreement (%) | R_filter start → end | Volume (mL) | Duration | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|

Also record:
- **K_clogged / K_clean** (R1c / R1a);
- from R4, the **flow, ΔP and R_filter when filtering stops being useful** (e.g. the flow falls below 10–20 % of its initial value), and how long it took;
- the **lowest flow observed**. If it is below the flowmeter's rated minimum, note it: section 8, item 6.

### 6.4 Decision

```
Q0. Any run over the cap without a trip, or the pump unable to hold ~2 bar even at 100 %?
    └─ yes → STOP: hardware or safety issue. Tell Claude, with the traces. No tuning.
Q1. In R3 and R4: no cap trips, margin to the cap ≥ 0.2 bar, oscillation < ±0.1 bar?
    └─ yes → Variant A.
Q2. K_clogged / K_clean < 3, and spikes short (< 1 s within 0.2 bar of the cap)?
    └─ yes → Variant B.
Q3. Otherwise → Variant C.
```

Also write down, for the S3.4 common part:
- **CLOG_Q_MIN** ≈ the flow at which R4 stopped being worth continuing (e.g. 10–20 % of the initial flow)
- **CLOG_R_MAX** ≈ R_filter at that point
- **CLOG_INDEX_MAX** ≈ the clog index at that point
- **CLOG_T_S** ≈ 30–60 s
- **SAMPLE_MIN_EXPECTED_FLOW** ≈ the mean flow of the R2 runs (the timeout becomes ≈ 1.5 × the 2 L duration)

### 6.5 Next

Paste the **S3.4 common part + your variant** into a new session. Afterwards, repeat R2–R4 and compare with `tools/compare_runs.py`. If the targets are still not met, paste the comparison into a session with: *"S3.4 variant <X> results attached; ROADMAP 6.4 targets not met: <which>. Propose the next change."*

---

## 7. Future updates (prepared for, not built yet; kept in docs/FUTURE.md)

| Item | What is prepared now | What's missing |
|---|---|---|
| **Negative controls** (sterile water) | Data model, plan option, metadata role, reserved `NEG_CONTROL`, capability flag | Reservoir + pump + valve; firmware procedure |
| **Positive controls** (known DNA) | Same, with `POS_CONTROL` | Reservoir + pump; contamination-safe routing |
| **Environmental sensors, tier 2–3** (conductivity, pH, DO) | Generic `sensors[]` everywhere; the S3.6 driver table | Hardware + drivers only |
| **Site weather covariates** | Metadata schema | Host fetches them from a public weather API |
| **GNSS location** | Adapter interface (manual for now) | Check the TOFU modem; implemented in H10 if available |
| **Firmware (code) updates of the Due from the Pi** | Firmware hash in HELLO; the Native port is the Pi's link | `bossac` on the Pi; checksummed binaries built in CI (never compiled on the Pi); only when IDLE with no mission and a known O-ring state; rollback; a post-flash HELLO check. *Calibration never needs this (S2.1).* |
| **More than 14 slots** | Geometry-driven code and tests; host sized from `GET_INFO` | New mechanics + a new `FILL_ORDER` |
| **Low-power sleep** | Idle-off behavior, cold-boot resume | Power budget (HW9); Pi power controller / RTC wake |
| **Decontamination flush** between samples | `PURGE_PIPES` exists | Reservoir + valve; an optional plan step |
| **App-driven interactive calibrations** | Calibrations return structured results | Prompt/response events + UI |
| **MQTT / fleet** | API-first design | Only if many devices |
| **Jetson + nanopore** | Pure-Python host, adapters folder | I4 |
| **O-ring contact confirmation / true home switch** | `ORING_CONTACT_BUTTON_INSTALLED` hook | Limit switch (section 8, item 11) + a pin |
| **I²C ADC for better pressure resolution** | Channel table (S3.1) | Only if the 12-bit ADC proves insufficient |

---

## 8. Hardware: track, parts to order, pins

### 8.1 Hardware track (yours, in parallel)

| # | Task | Unblocks |
|---|---|---|
| **HW0** | Order the "must have" parts (8.2); fit the **mechanical pressure-relief valve** | Safety now |
| **HW1** | Hardware E-stop: button + relay cutting 24 V + one Due input | S1.4 (software STOP works without it) |
| **HW2** | Native USB cable Due ↔ Pi | S2.3 |
| **HW3** | Calibrate `Q_in` (`calibrate_flow`); **measure the real sampling flow** with a measuring cylinder *before* buying the outlet meter; install `Q_out` | S0.2, S3.1 |
| **HW4** | Install `P_in` (as soon as it arrives → S0.3), then `P_out` | S0.3, S3.1 |
| **HW5** | ENABLE pull-up, stepper power off the breadboard, a dedicated stepper supply or a verified rail budget, fuses | Reliability |
| **HW6** | Manifold/plate orthogonality, the PTFE/aluminum seal, the pressure-sensor pop-out | Leaks |
| **HW7** | Re-test the 7th capacitor; confirm the motor's rated current and Vref | Reliability |
| **HW8** | Fill in the "reachable on the shield" column of `docs/PINOUT.md` (after S0.1) | Pin decisions |
| **HW9** | Power budget (current monitors); size the battery and solar; check GNSS on the TOFU | Field autonomy |
| **HW10** | Spool repair | I3 |

### 8.2 Must have: order now

Specs, not brands. Check each part against your fittings and tube sizes. **Buy spares** of anything that gets wet or is hard to source.

| # | Part | Qty | Spec / notes | Unblocks |
|---|---|---|---|---|
| 1 | **Pressure sensors** | 3 (P_out, P_in replacement, spare) | **ABPDANV060PGSA3** (supplier found): the same family as the installed ABPDANV150PGSA3, in 0–4.1 bar. Same SPI bus, 3.3 V, code and tubing. Raw ΔP error ±0.12 bar; ≈ ±0.02 bar with auto-zero + cross-calibration (S3.1). The old 150 psi sensor becomes a bench spare, or the fallback if supply fails. Barb clamps too. See `docs/reports/note-capteur-pression.html`. | S0.3, S3.1 |
| 2 | **Hardware E-stop** | 1 | Latching mushroom button, **two contacts**: one NC in series with the relay coil, one to a Due input. IP65 if exposed. | S1.4 |
| 3 | **Relay / contactor for the E-stop** | 1 + 1 | 24 V DC coil; contacts ≥ 2× the total 24 V current; flyback diode | S1.4 |
| 4 | **Mechanical pressure-relief valve** | 1 | Adjustable ~2.5–3 bar, water-compatible, outlet to waste. Protects the tubing even when the sensors are missing or wrong. | Safety now |
| 5 | **Fuses + holders** | per branch + spares | One per 24 V branch (pump, motor shield, stepper) | Safety |
| 6 | **Outlet flowmeter** | 1 + 1 spare | **Same model as the inlet meter.** The inlet meter is a **MW-FS-2.0** (turbine, 0.15–1.0 L/min, 77 Hz per L/min ≈ 4,620 pulses/L, ±5 %, 2.5–24 V, output = supply voltage, so **power it at 3.3 V**: at 5 V it puts 5 V on D13). **Measure the real sampling flow first** (HW3). If the minimum flow is ≥ 0.2 L/min, keep MW-FS-2.0 (≈ 4 €). If it is < 0.15 L/min, use a BIO-TECH FCH-m-POM-LC with a 1.0 mm nozzle (0.015–1.0 L/min, open collector) **at both positions**. A Sensirion SLF3S-4000B is an outlet-only premium option. See `docs/reports/note-debitmetre.html`. | S3.1, S3.5 |
| 7 | **Micro-USB data cable**, short, good quality | 2 | Due Native port ↔ Pi | S2.3 |
| 8 | Wiring consumables | — | 10 kΩ resistors, perfboard or screw-terminal shield, ferrules, a crimp kit | HW5 |
| 9 | Sealing spares | — | O-rings for the double-O-ring design (×3 sets), PTFE-compatible adhesive, heat-set inserts | HW6 |
| 10 | Test-bench supplies | — | ≥ 20 test Sterivex filters, a needle valve or tube clamp, a measuring cylinder, a bucket | S0.0, S3.3 |

### 8.3 Should have (next 1–3 months) and nice to have

| # | Part | Priority | Notes | For |
|---|---|---|---|---|
| 11 | **Limit switch at the actuator's "unlocked" position** | Should | A *true* home for the O-ring actuator instead of stall dead-reckoning; cheap, big reliability gain; fills the `ORING_CONTACT_BUTTON_INSTALLED` hook | Recovery, homing |
| 12 | **Enclosure leak sensor** | Should | Water where the electronics are → critical alert | S3.5 |
| 13 | **Current/power monitors** (I²C, high-side, ~26 V) | Should | Power budget; later a stall/fault diagnostic | HW9 |
| 14 | Dedicated stepper supply or DC-DC | Should | Removes the shared-rail question | HW5 |
| 15 | **Spare Arduino Due** ×1–2 | Should | A bench board + a field spare | All |
| 16 | Stepper driver upgrade (TMC2209-class) | Should | Quieter, microstepping, electrical stall detection to confirm "locked" | Lock reliability |
| 17 | SIM + data plan, LTE (+ GNSS) antennas | Should | Antennas outside any metal enclosure | H10 |
| 18 | RTC backup battery for the CM4/TOFU (if absent) | Should | The mission clock must survive power cuts | H9 |
| 19 | **Waterproof temperature probes** (one-wire) ×2 + 1 | Should | Water + storage compartment; tier-1 metadata | S3.6 |
| 20 | Turbidity sensor (analog) | Should | Relative index; tier-1 metadata | S3.6 |
| 21 | Waterproof enclosure, cable glands, desiccant | Should | Once the layout is stable | Field |
| 22 | Battery + MPPT solar controller (with a data interface) + panel | Nice (after HW9) | Size from measurements; LiFePO4 preferred | Field autonomy |
| 23 | Pi power controller / RTC wake | Nice | Pi off between samples | Section 7 |
| 24 | Conductivity (EC) kit + solutions | Nice | Tier 2 | Metadata |
| 25 | pH / DO probes | Later | Maintenance-heavy | Metadata |
| 26 | 2 small peristaltic pumps + reservoirs + valves | Later | Negative and positive controls | Section 7 |
| 27 | Reservoir + valve for decontamination | Later | Line cleaning | Section 7 |
| 28 | Spare pump, micro pump, encoder | Later | Once the design is frozen | Field |
| 29 | 16-bit I²C ADC | If needed | Better pressure resolution, or more analog inputs | Section 7 |
| 30 | Jetson-class computer | Later | Sequencing | I4 |

**Before you order:**
- Confirm the fitting thread and tube size (items 1, 4 and 6).
- Confirm the total 24 V current (items 3 and 5).
- Measure the real sampling flow (item 6).

### 8.4 Pin budget: is the Due big enough?

**Yes, electrically.** The Due has 54 digital pins (all interrupt-capable), 12 analog inputs, 2 DACs, I²C and SPI. The code uses about 31 digital, 3–4 analog and 1 DAC (`Settings.h`, commit `d508f41`).

| Group | Used today | Free in the code |
|---|---|---|
| Digital | 0–1 (USB serial), 2, 3, 5–13, 22–35, 38, 40, 42, 44, 46, 48; 18–19 if the legacy `Serial1` link is kept | **4, 14–17, 36, 37, 39, 41, 43, 45, 47, 49, 50–53** (17), + 18–19 if `Serial1` is dropped; 20–21 = I²C |
| Analog | A2 (pressure), A10, A11 (motor current); A1 declared but apparently unused | **A0, A1, A3–A9** (8–9) |
| DAC | DAC1 (pump) | DAC0 |
| Buses | SPI (manifold encoder, pressure1), USB serial | **I²C (20/21)** |

**Planned needs:**

| New item | Pins | Type |
|---|---|---|
| `P_in`, `P_out` | 2 | Analog (`P_in` can reuse A2) |
| `Q_out` (`Q_in` already on pin 13) | 1 | Digital, interrupt |
| E-stop state | 1 | Digital input |
| Actuator limit switch | 1 | Digital input |
| Enclosure leak sensor | 1 | Digital or analog |
| 2 temperature probes | **1 shared** | Digital (one-wire bus) |
| Turbidity | 1 | Analog |
| 5 V rail monitor (ratiometric compensation) | 1 | Analog |
| Current monitors ×2 | 0 extra | I²C |
| **Total** | **~5 digital + ~4 analog + I²C** | Fits with margin |

**Proposed pin assignment** (check each one against the PCB shield before wiring):

| New hardware | Pin | Wiring notes |
|---|---|---|
| `P_in` (existing ABP, SPI) | SPI bus + chip select **D8** | Unchanged: MISO + SCK on the Due's SPI header, 3.3 V, GND (the ABP is read-only, so MOSI is unused) |
| `P_out` (new ABP, SPI) | Same SPI bus + chip select **D45** | Same 4 wires as `P_in`, plus its own chip-select line. 3.3 V part: no divider. |
| Turbidity | **A5** | Divider sized to its output range |
| Enclosure leak sensor | **D41** (digital module) or **A6** (analog probe) | 3.3 V logic |
| `Q_out` (second flowmeter) | **D12** if the existing, unused `flow_sensor_big` wiring is there; otherwise **D36** | Pull-up to **3.3 V** (not 5 V). If the sensor has an internal pull-up to 5 V, add a divider. |
| E-stop state | **D37** | Normally-closed contact between the pin and GND, with `INPUT_PULLUP`: pressed **or a broken wire** reads HIGH = stop (fail-safe) |
| Actuator "unlocked" limit switch | **D39** | Same as the E-stop: `INPUT_PULLUP`, switch to GND |
| Temperature probes (one-wire bus, both on one pin) | **D43** | 4.7 kΩ pull-up to 3.3 V; power the probes at 3.3 V |
| Current monitors | **SDA 20 / SCL 21** | The Due has on-board pull-ups on these pins |
| Still spare after all of this | A0–A4, A7–A9, D4, D14–17, D47, D49, D50–D53 | |

The new digital inputs are grouped on the long double-row header (D36–D53) to keep the wiring tidy.

**The real constraints:**
1. **The custom PCB shield.** A free pin helps only if the shield brings it out. Fill in `docs/PINOUT.md` (HW8).
2. **3.3 V logic.** The Due is **not 5 V tolerant**.
   - Switches: pull-ups to 3.3 V.
   - Hall flowmeters (5 V supply, open-collector output): pull-up to 3.3 V.
   - Temperature probes: powered at 3.3 V.
   - 0.5–4.5 V analog outputs: through a divider.
3. **Freeable pins:** 8 (pressure1 CS) once retired; 18–19 if `Serial1` is confirmed dead; A1. The spool pins stay reserved until I3.

**If you run short** (e.g. on a bigger machine): add I²C expanders, not a bigger board. A 16-bit I²C ADC adds analog inputs (and resolution); a 16-pin I²C GPIO expander adds slow digital I/O (valves, switches, LEDs). Keep flowmeters, stepper, encoder and E-stop on native pins.

### 8.5 Future sensors: pre-selection for later orders

This is a rough shortlist to prepare future orders, not a final choice. Each line gives the recommended candidate, how it connects to the Due, and **the check to do before ordering**, so we don't have to reorder. Prices are approximate and to be confirmed; specs come from memory of the usual references and must be checked against the datasheet at order time.

| Need (tier, §1.9) | Recommended candidate | Alternative | Interface / voltage → Due | Pin | ≈ Price | Check before ordering |
|---|---|---|---|---|---|---|
| **Water temperature** (tier 1) | DS18B20 in a waterproof stainless probe (±0.5 °C, −10 to +85 °C) | PT100/PT1000 + MAX31865 (±0.1–0.3 °C, costlier) | 1-Wire, 3.3 V, 4.7 kΩ pull-up to 3.3 V | D43 (shared bus) | 5–10 € | Many DS18B20 probes are fakes (unstable readings); buy from a known distributor. Cable length and gland. |
| **Storage-compartment temperature** (tier 1) | Same DS18B20 on the same bus | **Sensirion SHT4x** (temperature + humidity, ±0.2 °C, I²C 3.3 V); the humidity also flags condensation or a leak in the housing | 1-Wire / I²C | D43 or I²C 20/21 | 5–15 € | Placement next to the used filters, not next to the electronics (heat). |
| **Turbidity** (tier 1) | DFRobot SEN0189 class (analog, relative index) | Industrial optical probe (NTU, costly: later) | Analog 0–4.5 V at 5 V → **divider** to 3.3 V | A5 | ≈ 10 € | Not submersible as is (open electronics): needs a flow-through, light-tight housing. Sensitive to ambient light and bubbles, so place it after a settling point. Relative index only. |
| **Conductivity** (tier 2) | Atlas Scientific EZO-EC + K 1.0 probe (lab-grade, calibratable) | DFRobot Gravity EC (cheap, less stable) | I²C or UART, 3.3 V | I²C 20/21 | 150–250 € (Atlas) | **Galvanic isolation required** as soon as another probe shares the same water (the Atlas isolator board), otherwise the readings interfere. Calibration solutions to order with it. |
| **pH** (tier 3) | Atlas EZO-pH + industrial probe | — | I²C, 3.3 V, isolated | I²C | 150–250 € | Recalibration every few weeks; the probe must not dry out. Only if the lab asks for it. |
| **Dissolved oxygen** (tier 3) | Optical probe (no membrane) | Galvanic probe (cheaper, more maintenance) | Varies (RS-485/Modbus common) | via adapter | 300 € + | Postpone; expensive and maintenance-heavy. |
| **Actual sampling depth** (with the spool, I3) | Hydrostatic **4–20 mA** level transmitter, 0–5 or 0–6 bar (≈ 50–60 m), at the inlet | Blue Robotics Bar30 (MS5837, I²C, ±2 mm resolution) | 4–20 mA → shunt resistor → analog input | A7 | 50–150 € | **Cable length:** I²C does not work over the tens of metres of the spool without a dedicated extender; 4–20 mA does. Cable vented or not (gauge vs absolute sensor). |
| **Enclosure leak** | Resistive water-detection probe/strip on the bottom of the housing | SHT4x humidity (above) as an early warning | Digital or analog, 3.3 V | D41 / A6 | 2–10 € | Placed at the lowest point of the housing. |
| **Power consumption** (budget, stall diagnostics) | **INA226** (I²C, up to 36 V, 16-bit) | INA219: to avoid, its 26 V max is too close to our 24 V rail with transients | I²C, 3.3 V | I²C 20/21 | 5–10 € each | Size the shunt for the maximum current of each branch (pump, motors). |
| **Actuator home position** | Sealed Hall switch + magnet (no contact, no wear) | IP67 microswitch | Digital, 3.3 V | D39 | 2–10 € | Mounting to design (the magnet on the moving part). |
| **GNSS position** (if the TOFU modem lacks it) | u-blox M8/M9 class module | — | UART **on the Pi**, not on the Due | — | 15–40 € | First check the TOFU modem (HW9); the antenna must be outside any metal housing. |

**Keeping the I²C bus coherent (20/21):** several of these sensors share the bus, so check their default addresses before ordering. Usual values, to confirm on each datasheet:

| Device | Default I²C address | Configurable? |
|---|---|---|
| SHT4x | 0x44 | Depends on the reference |
| INA226 | 0x40–0x4F | Yes, by strapping |
| Atlas EZO | 0x63–0x64 | Yes |
| MS5837 | 0x76 | **Not configurable** |
| Sensirion SLF3S | 0x08 | Only via a procedure |

List all the planned addresses in `docs/PINOUT.md` before any order.

**General rules for any new sensor (so we don't have to reorder):**
- **3.3 V logic or a divider:** the Due is not 5 V tolerant.
- **Pin and bus** chosen in `docs/PINOUT.md` and checked on the shield *before* ordering.
- **Rated for the medium:** raw water with sediment, cold, sometimes salty.
- **Cable length vs interface:** I²C and SPI only work over short cables (< 30 cm, or with a dedicated extender); 1-Wire works over a few metres; 4–20 mA and RS-485 work over long distances.
- **Maintenance and calibration** compatible with an autonomous deployment of several weeks.
- **Buy from an official distributor** (fakes are common for cheap sensors).
