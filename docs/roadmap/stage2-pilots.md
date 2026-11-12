# CoWaS roadmap, stage 2: upgrade the prototype to an unattended pilot

**Goal:** upgrade the current machine so that it survives the real field (power loss, sensor faults, wet mechanics), then run it **unattended for 1–2 weeks (Pilot B)** with remote monitoring. Stage 2 changes the machine we have; it never needs a new design. Anything that needs a new design is in [stage 3](stage3-full-system.md) (the V2 machine).

**Where we are (2026-11-12).**
- Stage 1 is done, and the field campaigns are done: Bois Chamblard (26–29 Oct) and LéXPLORE on Lake Geneva (3–6 Nov). The supervised lake run at LéXPLORE counts as **Pilot A** (user, 2026-11-12): 48 machine filters, 18 manual comparison samples, blanks. The lab results are pending. See the [robustness update report](../reports/robustness-update-report-2026-11.md).
- **The machine is out of service** since the incident of 6 Nov (F257): the manifold over-travelled and tangled, and the SAM3X no longer answers. Phase 0 brings it back.
- The field showed what to fix: nothing stops the motors if the Due itself fails (F257, F258), sensor faults go unnoticed (F234, F236), the spool mechanics are wet wood and glue (F246, F250, F251), the flowmeter reads low (F254), the DNA Shield dose is not measured (F255), and the session tool cannot resume or edit a list (F228, F239).

**Stages**
- Stage 1 = [stage1-FINISHED-prototype.md](stage1-FINISHED-prototype.md): everything done from August to November 2026, including the field campaigns and Pilot A.
- **Stage 2 = this file:** upgrades of the current machine, up to Pilot B.
- Stage 3 = [stage3-full-system.md](stage3-full-system.md): the V2 machine (new design), and what only makes sense on it.

"ROADMAP §N" in the code and older notes means a section of this file. §3 (design decisions), §4 (findings), §6 (rules) and §11 (parts and pins) keep their numbers. Findings (F…) are in [findings.md](../logs/findings.md), decisions (D…) in [decisions.md](../logs/decisions.md). The version before this reorganisation is in [the archive](../archive/roadmap-stage2-2026-11-12.md); old step IDs map to new ones in §10.

---

## 0. How to use this file

- **Two lanes.** **Lane A = software** (firmware without new hardware, the session tool, the host). It starts now and never waits for a part. **Lane B = hardware upgrades**, each with its **firmware follow-up**. Lane B takes longer and can be delegated (see "Organisation of the work" below). Lane C = lab and validation.
- **Read §1 first:** it gives the order of every step in one table.
- One step = one branch = one PR, with its unit tests and its hardware check (rule 7). Before a step, read the sections it names and the findings it fixes. Ask before changing anything outside the step.
- **Priorities.** **P0** = before the next field run. **P1** = before Pilot B (unattended). **Opt** = a useful upgrade that blocks nothing; do it when there is time or a need.
- **Size** (estimate, one person): **S** ≤ 3 days, **M** ≤ 3 weeks, **L** > 3 weeks.
- **Organisation of the work** (by kind of work, not by person; who does what is decided as people join).
  - **Lane A** needs a computer and the repository, not the machine. It is built and unit-tested offline (AI-assisted coding fits it well). Its bench checks are batched (§7a) and need someone at the machine.
  - **Lane B** needs the machine, tools and parts. Each card is self-contained (findings to read, firmware follow-up, numeric check), so it can be handed to a newcomer.
  - **Lane C** needs the lab (extraction, sequencing, weighing) and the field.
  - **Across lanes:** decisions on behaviour changes and placeholders (rule 1), the bench checks, and the field runs.
- The rules in §6 apply to every step. Rule 10 is new: **the prototype stays usable after every step.**

---

## 1. The order, in one view

**Phase 0: back in service (now).** Nothing else can be checked on the bench before it.

| # | Step | What | Size |
|---|---|---|---|
| R1 | Diagnose F257 | The tests of [field log §39.3](../logs/field-lexplore-geneva-2026-11-03.md#393-tests-that-separate-the-hypotheses-not-done): the Due alone on USB, ERASE and upload, the rails, burn marks and shorts on the shield, the motor and the connector (photos first), the platform power log | S |
| R2 | Repair | Replace the Due if the SAM3X is dead. Untangle the manifold, check the tubing and cables. Ethanol-purge the three slots used last at LéXPLORE (schedule 3). Do B1 (pull-downs) while the board is out: it is cheap and fixes F215 | S |
| R3 | Flash and check | Flash the current build. Run the bench checks of §7a, including every item marked "built, not flashed" | S |
| R4 | Commit | Commit the stage 1 and field work, and fix the report wording listed in [LOG 2026-11-12](../logs/LOG.md#session-2026-11-12-checks-of-the-revision-report-host-protocol-simulator-core-claims) | S |

**Then the lanes run in parallel.** Order within a lane = the order to do them.

| Order | Lane A: software (now) | Pri | Size | Lane B: hardware (delegable) | Pri | Size |
|---|---|---|---|---|---|---|
| 1 | **A1** Safe boot, reset cause, watchdog | P0 | S | **B1** Safe pin defaults (pull-downs) | P0 | S |
| 2 | **A2** Sensor-fault rules from the field | P0 | M | **B2** Manifold encoder coupling lock | P0 | S |
| 3 | **A3** Spool software from the field | P0 | S | **B3** Pressure sensor mount, bus and range | P0 | S |
| 4 | **A4** Session tool field fixes | P0 | M | **B4** Independent stop and power protection | P1 | M |
| 5 | **A5** One implementation per action, clean-up | P1 | M | **B5** Flowmeter at field flow | P1 | S |
| 6 | **A6** Supervisor `service()` | P1 | M | **B6** Spool mechanics, permanent | P1 | M |
| 7 | **A7** Stop, safe state, RECOVER, modes | P1 | M | **B7** Measured DNA Shield dose | P1 | M |
| 8 | **A8** Motion journal and anti-tangle | P1 | M | **B8** Power at the pilot site | P1 | M |
| 9 | **A9** Sample record v2, flow correction | P1 | M | **B9** Enclosure, leak sensor, water temperature | P1 | M |
| 10 | **A10** Protocol v1 on Native USB, beside the console | P1 | L | **B10** O-ring endurance and home switch | P1 | M |
| 11 | **A11** The host package takes over the session tool's logic | P1 | L | **B11** Pi and LTE modem on site | P1 | S |
| 12 | **A12** Remote monitoring and alerts | P1 | M | **B12** Firmware for B9/B10 (lane B follow-up) | P1 | M |
| 13 | Opt: A13 pump tuning, A14 runtime parameters, A15 trace telemetry, A16 API and web app, A17 commissioning and metadata, A18 updates from the Pi, A19 firmware restructure | Opt | — | Opt: **B13** field cartridges, B14 manifold index sensor, B15 on-device storage, B16 relief valve (a low-flow flowmeter is an option of B5) | Opt | — |

| Lane C: lab and validation | Pri | Needs |
|---|---|---|
| **C1** Pilot A lab results: machine vs manual, blanks | P0 (science) | the lab |
| **C2** Shield dose and residual water (with B7) | P1 | B7 |
| **C3** Carry-over and cross-talk (decides the purge rule) | P1 | A5 |
| **C4** SOPs, two-person dry run | P1 | A4 |
| **C5** Supervised field run with the P0 fixes | P0 gate | G1 |
| **C6** Bench acceptance and 72 h soak (go or no-go for Pilot B) | P1 gate | all P1 |
| **C7** Pilot B | P1 | C6 |

**Gates**
- **G0 Back in service:** R1–R4 done; the bench checks of §7a pass.
- **G1 Field-safe (before the next field run, C5):** G0 + A1, A2, A3, A4 (P0 part) + B1, B2, B3 + the dropout test of PW1 (§7.0) for the phases idle, pump, rotation. **An unattended night run also needs B4** (a stop that does not depend on the Due).
- **G2 Pilot B ready:** every P1 step + C6 passed.

**Critical path to Pilot B:** A1 → A5 → A6 → A7 → A8, and A10 → A11 → A12, with B4 and B8 on the hardware side. **If the schedule slips, cut scope, not safety.** You can cut every Opt step, B13, the Pi (a laptop with a modem can host), and A10/A11 (the session tool can stay the host for Pilot B if A12's alerts work from it). You cannot cut A1, A2, A7, A8, B1, B4, B3, C3.

---

## 2. Why this plan

1. **The field decides the order.** The first lake campaign worked: no seal leak, the cap held, unattended series ran. What failed was what a lab bench does not test: the power, wet mechanics, a sensor coming off its fitting, operator input. Those come first.
2. **Software first, because it does not wait.** Lane A needs no part and no machine to be built and unit-tested. The bench checks are batched once the machine is back. The safety value is high: a watchdog plus pull-downs alone covers the "frozen Due drives the motor" case of F257.
3. **Hardware is delegable, in small cards.** Each lane B step is a short card with numeric pass criteria and its firmware follow-up, so a new person can own it.
4. **Every step keeps the prototype usable** (rule 10). The field workflow (console, `run_sample`, session tool, schedule) is never broken between two steps. New hardware is a setting (`*_INSTALLED`), and the firmware runs without it.
5. **The host grows out of what worked** (user decision, 2026-11-12). The session tool ran every field sample. Its logic moves into the `host/` package step by step. A10 adds the protocol on Native USB *beside* the console, so nothing is switched over in one go.

---

## 3. Design decisions

### 3.1 Who does what

- **The Due:** every sensor and actuator, the safety checks, the procedures. No clock. Today it forgets everything on reset; from A8 it keeps a small motion journal.
- **The host:** today the session tool (`tools/session_log.py`) on the laptop: schedule, slot book, sample records, logs. From A11 the `host/` package holds this logic and the session tool is its console front end. A Pi is optional (A12).
- **Rule:** the Due never starts a sample on its own, and the host never switches a valve directly.

### 3.2 Safety layers and recovery

| # | Layer | Step | Covers |
|---|---|---|---|
| 0 | Mechanical pressure-relief valve | Opt (the user accepted running without it, 2026-10-23, F144) | Everything else failing, including a wrong sensor |
| 1 | **Safe pin defaults:** pull-downs on every actuator input, a pull-up on the A4988 ENABLE | B1 | A reset, a frozen or unpowered Due leaves every actuator OFF |
| 2 | **Independent stop:** the 24 V actuator rail through a relay held by a Due heartbeat and an external watchdog; an E-stop in the same line | B4 | A frozen Due, a firmware hang, an operator emergency |
| 3 | Due hardware watchdog and brown-out reset | A1 | Stuck loops, a supply dip |
| 4 | Firmware supervisor `service()`: cap, sensor plausibility, timeouts, rotation interlock, over-travel guard | A6, A2, A8 | Normal faults |
| 5 | Software STOP (console `stop`/`x`, host `STOP`), always accepted | A7 | The operator sees a problem |
| 6 | Host checks and remote alerts | A12 | Unattended operation |

- **Safe state:** pump and micro pump off, valves closed, manifold and spool stopped, **O-ring stays locked** if it is locked.
- **Rest state:** the O-ring is locked whenever the machine waits. It is unlocked only to rotate: unlock, rotate, lock ([protocol §5.1](../software/protocol.md)).
- **Power dropout (F257, F258):** a loss of power at any moment, for any length, must end with nothing moving and a checked restart. Details in §7.0 (PW1).
- **Recovery:** after any fault, STOP or boot, procedures are refused until `RECOVER` passes: cause gone, everything off, encoder steady, O-ring state known (home it if unknown), spool homed. The interrupted slot is `SUSPECT`. Nothing resumes by itself.

### 3.3 Pressure cap

The only safety value is the cap: **3.6 bar** (`STX_MAX_PRESSURE`, user decision 2026-10-19). It acts on `P_in`, the pressure before the filter. The pump pauses at 3.35 bar and resumes at 3.15 bar. An implausible reading stops the pump; from A2 a **flat** reading while the pump runs at power also stops it (F234). The sensor is calibrated to 2.0 bar only (F136) and its range is unresolved (F126): B3 fixes both. Highest pressure seen in 155 field records: 3.44 bar.

### 3.4 Two serial ports

| Port | Used by | Carries |
|---|---|---|
| Native USB | The host, always connected (from A10) | Protocol v1 (JSON lines), a copy of every log line |
| Programming port | The console and the session tool (today); a service console (from A10) | The test menu, `run_sample` |

Opening the Programming port **resets the Due**, so a host restart reboots it mid-sample. The Native port does not reset, except on the 1200-baud "touch", which the host must never use. Until A10, the session tool stays on the Programming port.

### 3.5 Controller modes

```
BOOT -> safe pins -> self-check -> RECOVERY_REQUIRED -> (RECOVER ok) -> IDLE
IDLE <-> BUSY      (a procedure runs; only STOP, STATUS, PING, HELLO, GET_INFO are accepted)
IDLE <-> SERVICE   (console on the Programming port; `service` to enter, `exit` to leave)
any  -> FAULT (latched) or ESTOP -> clear + RECOVER -> IDLE
```

Until A10 the boot default is SERVICE (today's console, so the field workflow is unchanged); then IDLE (host mode). Today `setup()` ends in the blocking test menu (F23).

### 3.6 Slots and sample records

- **Slot states (host):** `EMPTY`, `READY`, `IN_PROGRESS`, `SAMPLED`, `FAILED`, `DISABLED`, `SUSPECT`. The session tool's slot book is the first version of this.
- **Filter roles:** `sample` (with a replicate number), `equipment_blank`. Later `negative_control` and `positive_control` (stage 3).
- **One record per filter** (today a JSON file per sample, from the session tool): sample ID, slot, role, start and end time, filtered volume (plain and trusted), duration, `end_reason`, pressure max and mean, purge volume and end, shield time (and from B7 the shield volume), step times (D96), spool and depth, reset cause, flags, firmware version. A9 adds what is missing. Column names agreed with the lab.
- No code hard-codes 14 or 15 slots. The count comes from the Due (`GET_INFO`).

### 3.7 eDNA quality rules

These are requirements, not options.

1. **Purge large enough for the dead volume.** Stage 1 uses the user's values: **500 mL without the spool, 3 L with it** (D50; the dead volume with the spool is 2.57 L). The stage 2 rule is set by **C3** (washout curve) and **C1** (the blanks and the machine vs manual comparison).
2. **Filtered volume** from the flowmeter, **corrected at field flow** (B5, A9: it reads 20–31 % low at 60–80 mL/min, F254). Every filter gets an `end_reason`: `volume`, `clogged`, `stalled`, `pressure`, `timeout`, `cap`, `sensor`, `fault`, `stopped`.
3. **Shield volume measured** (B7, C2), not timed (F255).
4. **Blanks in every deployment:** one equipment blank at the start and one at the end, plus one manual field blank. The tip stays in the DNA-free water for the whole blank (F252).
5. **Manual comparison samples** at the same place, time and volume. Done in Pilot A (18 samples); repeat a few in Pilot B.
6. **Handling:** gloves, the cleaning SOP (ethanol purge per used slot), the prefilter check (F248), filter batch and expiry on the sheet (F253), caps on, cool and dark.
7. **Cross-talk between slots** measured with a dye (C3).
8. **Biosecurity:** quagga mussels are in Lake Geneva. Clean and dry all wetted parts before use in another water body.

### 3.8 Power

- **Field reality:** LéXPLORE gave platform power with a generator that starts on its own; the incident of 6 Nov came at a generator start (F257). Pilot B will see the same kind of supply, or a battery.
- **Known:** the lead screw self-locks, so the stepper can be off while the O-ring is locked (F34).
- **Not yet measured:** the pump driver's input range (F25) and every power figure (B8).
- **Estimates:** pump ≤ 12 W, about 20 min per sample without the spool and 29 min with it (field medians). Due and logic 1–1.5 W always on. Pi and modem 3–7 W always on unless duty-cycled.
- **Plan:** a DC-UPS or a LiFePO4 battery between the supply and the CoWaS (B8), so a platform cut does not reach it. Logic and motors on separate rails (B4). **Never power the Due from raw 24 V** (F32).

### 3.9 Not in stage 2 (stage 3, the V2 machine)

A new frame, a new manifold, a new pump and pulsation damper, a second pressure and flow sensor on the plumbing, a redesigned spool with a depth sensor, pluggable reservoirs, controls and decontamination, a custom PCB or another controller, duty-cycled power, the Jetson. See [stage 3](stage3-full-system.md).

---

## 4. Findings the steps must fix

Only the findings a step still has to fix. Their status is in [findings.md](../logs/findings.md). The full F1–F36 texts are in [the archive](../archive/roadmap-stage2-details-2026-10-23.md).

| Finding | What | Step |
|---|---|---|
| F257, F258 | The manifold over-travelled when the Due stopped; no defined behaviour for a power dropout | R1, B1, B4, A1, A8 |
| F215 | The pump turns on at a flash or reset (D42 not driven before `setup()`) | B1, A1 |
| F49 | `Serial.begin` twice hides the boot lines | A1 |
| F238, F82, F190 | Console froze on the SPI bus; micro pump and O-ring moves cannot be stopped | A1, A6 |
| F234 | A dead pressure sensor (flat reading) went unnoticed for 3.5 h while the pump ran | A2, B3 |
| F235, F236, F36 | Encoder bad frames from the shared SPI bus; a 2-bit checksum passes 1 bad frame in 4 | A2, B3 |
| F233 | A purge stopped by the cap goes on to the sampling | A2 |
| F252, F249 | The pause resumes on the raw reading, not the filtered one; the reading drops 2.3 bar at each stop | A2, B3 |
| F221 | The applied pump power is not printed | A2 |
| F251, F241, F245 | The spool takes a stop at the switch holder as the origin; top-switch presses at wrong lengths | A3, B6 |
| F243 | Panel spool moves are not logged | A3 |
| F224, F223 | Spool time limits too long (20/30 min for a 2 min move); spool height in RAM only | A3, A8 |
| F228, F229, F239, F222, F244, F248, F210 | Session tool: no resume, erase hides the old sample, no edit of times, notes asked too early, no filter-loaded confirmation, no prefilter check, `run_sample` outside the tool has no record | A4, A10 |
| F208 | The timed list does not check that the samples fit | A4 (timing spec, §7.5) |
| F1 | Slot availability is RAM-only on the Due (the session tool's slot book covers it) | A11 |
| F2, F40, F41, F42, F63, F122, F202 | Pump loops and rotations that bypass the cap or the lock | A5 |
| F4, F28, F80, F81, F156, F159, F255 | Shield dose timed, not measured; no prime; the micro pump drive is unverified | B7, C2 |
| F5, F16 | Each rotation command checks the lock; no hard-coded slot counts | A5 |
| F8, F10, F23, F24, F33, F53 | Console and Pi share one port; `setup()` ends in the test menu; output can block | A7, A10 |
| F25, F32, F125, F170, F173, F174 | DAC range, no raw 24 V to the Due, reset when the A4988 enables, ENABLE pull-up, the filter capacitor, the shared 24 V rail | B1, B4, B8 |
| F34 | The stepper stays enabled while locked | A5 |
| F51 | V23 relay still switched | A5 |
| F71, F60 | RECOVER can deadlock when the lock is UNKNOWN and the manifold is between slots | A7, B10 |
| F126, F136, F143 | Pressure range (×2.4 disagreement); calibrated only to 2.0 bar; cap checks | B3 |
| F150, F254 | Flowmeter 20–31 % low at field flow | B5, A9 |
| F187, F191, F203, F216 | Manifold encoder coupling slips; two zeros 3 slots apart | B2 |
| F213, F240, F246, F250 | Spool drain path, tube angle, wet wood, belt, guide | B6 |
| F237 | Panel LEDs go off with the pressure sensor unplugged | B3 |
| F30, F110, F111, F130 | Purge rule and carry-over | C3 |
| F35 | No strainer on the field intake | B9 |
| F146, F151 | Pump pauses at the cap margin (up to 113 per sample) | A13 (Opt); the pump itself is stage 3 |
| F164, F186, F217 | Frame and outlet out of square | **stage 3** (V2 frame). Until then: check by hand before each campaign |

---

## 5. Test strategy

**Every step ships tests. A step without tests is not done.**

| Level | What | When |
|---|---|---|
| 1. Firmware unit tests | Pure logic in `src/Core/` (`pio test -e native`) | Every commit |
| 2. Host and tool unit tests | Domain logic, storage, scheduler with a fake clock (`pytest host/`, `pytest tools/`) | Every commit |
| 3. Contract tests | The same `protocol/vectors.json` checked by C++ and Python | Every commit (from A10) |
| 4. Simulation tests | The host against `FakeController` with injected faults | Every commit |
| 5. On-device checks | Console commands print `TEST\|<name>\|<id>\|PASS/FAIL/SKIP\|k=v`; the session tool saves them | After each flash (batched in §7a) |
| 6. Fault-injection bench tests | Power dropouts in every phase, a forced hang, a sensor unplugged | PW1, A7, C6 |
| 7. Acceptance campaigns | Scripted runs with numeric pass criteria | C5, C6 |
| 8. Field checklists | The operator tutorial and the paper sheet | On site |

Conventions: a safety decision is a pure function with a truth-table test. Every threshold has boundary tests (just below, at, just above, NaN, sensor absent). Before moving math out of a hardware file, record its outputs (golden values) and assert them exactly. Checklists give PASS in numbers. **Field logs are test inputs:** a rule built from a field finding (F234, F251) is tested against the real log lines of that event.

---

## 6. Rules every step follows

These are copied into [CLAUDE.md](../../CLAUDE.md).

1. Behaviour changes only in steps that say so, each listed in the PR description.
2. Nothing is deleted without evidence and the user's approval. Git tag `baseline-v0` keeps everything.
3. Pure logic in `src/Core/`, unit-tested. Hardware code stays thin.
4. Safety is never weakened:
   - the pump never runs above the cap; an implausible pressure stops it;
   - no rotation while locked or while the lock state is unknown;
   - every procedure has a timeout;
   - STOP and E-stop are always accepted;
   - safe state = pumps off, valves closed, O-ring locked;
   - recovery is required after every stop and boot (from A7);
   - placeholders stay marked `PLACEHOLDER`;
   - the one exception is the sensorless pump test mode: explicit, test-only, low power, time-limited, never reachable from production.
5. A failing non-safety sensor: keep working, warn, flag the record. Never fail silently.
6. No hard-coded slot counts in new code.
7. Every step ends with: `pio run -e due` and `pio test -e native` passing (and `pytest` for `host/` and `tools/`), a hardware checklist with numeric pass criteria, and the checks in §7a updated.
8. The eDNA rules (§3.7) are requirements.
9. Claude does not flash, commit, push or delete files without asking.
10. **The prototype stays usable after every step.** The console commands, `run_sample`, the session tool and the timed schedule keep working, or the step names their replacement and updates the [operator tutorial](../field/operator-tutorial.md) in the same PR. Hardware that is not installed yet is a setting (`*_INSTALLED false` in `Settings.h`), and the firmware runs without it.

---

## 7. Steps

Status: ☑ done, ◐ partly done, ☐ open. Done steps from earlier plans:

| ID | Step | Status | Where |
|---|---|---|---|
| FW0 | Baseline: tests, CI, pin map, command list | ☑ 2026-09-24 | LOG |
| FW1 | Protocol v1 spec and test vectors | ☑ 2026-09-25 (firmware side = A10) | [protocol.md](../software/protocol.md) |
| FW2 | Mechanical test suite, lock tri-state, `TEST\|` logging | ☑ 2026-09-28; bench rows 1–5 PASS 2026-10-12; the rest in §7a | LOG |
| FW3 | Pressure sensor on `P_in`: driver, cap, PID | ☑ in stage 1 (S1-1, 2026-10-12); the range and calibration left to B3 | LOG |
| HO1 | Host skeleton: client, transport, simulator, CLI | ☑ 2026-09-28 (364 tests, simulator only; the bench check needs A10) | [host/](../../host/README.md) |
| LA1 | Lab meeting | dropped 2026-10-23 (values set with the lab directly) | — |
| VA2 | **Pilot A** | ☑ LéXPLORE 3–6 Nov 2026; lab results pending (C1) | [stage 1 §4.2](stage1-FINISHED-prototype.md#42-léxplore-lake-geneva--pilot-a-3--6-nov) |
| — | Parts of A5 and A9 (cap in every operation loop, end reasons, clog rule, trusted volume, step log, sample record) | ☑ in stage 1 | [report](../reports/robustness-update-report-2026-11.md) §2 |

### 7.0 Power-dropout robustness (PW1)

The plan spans both lanes: **B1** and **B4** (hardware), **A1**, **A6**, **A7** and **A8** (firmware), **A4** (host).

**Why.** On 2026-11-06 (LéXPLORE) the manifold turned further than a slot change needs and tangled itself, and the computer lost the Due, at the moment the platform generator started ([field log §38–39](../logs/field-lexplore-geneva-2026-11-03.md#39-the-incident-reviewed-against-the-final-log-and-the-hypotheses-weighed-2026-11-10), F257). The leading reading: the Due stopped running code during the rotation and the motor driver kept its last command; the SAM3X no longer answers. Whatever the cause, the CoWaS runs on supplies it does not control. **It must survive a dropout of any length, at any moment, and never be left moving.**

**Aim.** Two promises. (1) While power is lost, flaky or coming back, no actuator moves unless the Due is alive and commanding it. (2) After any restart the CoWaS checks that everything is off and in a known state before it does anything, and says which sample was hit.

**Hardware, does not rely on the Due**
1. **Safe defaults (B1).** Pull-downs on every actuator control line, so that a reset, a frozen Due or an unpowered Due means OFF: VNH5019 INA, INB, PWM of both channels; the pump relay D42 and the DAC (F215); valves D44, D46, D40; micro pump D48. A pull-up on the A4988 ENABLE D11 (F170, pinout !4). Check the shield's EN/DIAG pull-ups (pinout !3).
2. **A stop that works when the Due is frozen (B4).** The 24 V actuator rail (manifold motor, spool, pump, valves, A4988) through a relay or MOSFET switch held on only by a heartbeat from the Due, through an external watchdog or retriggerable timer (a 100–500 ms window). A hung Due stops the pulses and the rail drops. A latching E-stop in the same line, with a second contact to a Due input (D37).
3. **Separate logic from motors (B4).** The Due and the sensors on their own regulated rail with a hold-up (a capacitor, a small battery or a DC-UPS), so a dip of the motor rail does not reset the Due, and the Due can see the dip.
4. **Ride through the supply (B8).** A small battery or DC-UPS between the platform outlet and the CoWaS.
5. **Protection (B4).** A fuse or current limit per motor channel, a TVS and reverse protection on the input, and an isolated USB link (a USB isolator) between the laptop and the Due, so a ground difference cannot reach the board. A splash-proof box (B9).
6. **Brown-out (A1).** The SAM3X brown-out reset enabled; a voltage supervisor on the logic rail if it is not enough.

**Firmware**
1. **First lines of `setup()`** drive every actuator pin to its safe level before anything else, including D42 and the DAC to 0 (A1, F215).
2. **Reset cause** read and printed at boot, and written into the next sample record: power-on, brown-out, watchdog, software, user (A1).
3. **Hardware watchdog** at 4 s (A1). With the pull-downs, a frozen core resets and every actuator goes off within 4 s.
4. **Motion journal** in non-volatile memory: run, slot, step, "motion in progress", O-ring state, last manifold angle, net turns, spool length. On boot, "motion in progress" means a dropout hit that step (A8).
5. **Boot checklist** (the RECOVER of A7): everything off; pressure near ambient; encoder read twice and steady, compared with the journal; O-ring UNKNOWN until `actuator_home`; spool homed first; the interrupted sample flagged `SUSPECT`; no motion resumed by itself.
6. **Manifold anti-tangle that does not depend on time:** keep the net turns (from the journal) and refuse a rotation that would leave an absolute angle window; refuse when the encoder jumps faster than the motor can turn (A8). The stall and over-travel guard (`core::manifold_guard`, built 2026-11) stays.
7. **Heartbeat** to the B4 watchdog from `service()` only, so a stuck loop stops it (A6).
8. **Supply log:** the 24 V and logic rails read every second on an analog input; a dip below a threshold is logged with the time in the session log and the record (B4 follow-up).
9. **Host:** after a restart the list does not resume a half-done sample. It waits for RECOVER, marks the sample `SUSPECT`, and goes on with the next one only if RECOVER passes (A4 now from the boot banner; A11 later).

**Bench test (pass criteria fixed with the user before the test)**
- Dropouts of the 24 V alone, the logic alone, and both, for 0.1, 0.5, 1 and 5 s, in each phase: idle, pump running, manifold rotating, O-ring moving, spool moving, shield step.
- Pass: no actuator moves after the power returns until RECOVER passes; a frozen Due (a forced hang in `rotateMotor`, a hidden test command) stops the motor within the watchdog window (4 s with A1; 0.5 s with B4); the reset cause and the interrupted step are in the log; the sample is `SUSPECT`; the O-ring and the manifold angle are checked, not assumed.
- 20 cold boots and 20 dropouts in a row.

### 7.1 Lane A: software (no hardware change)

Each step is built and unit-tested without the machine. Its bench check goes into §7a and is run with the next flash.

**A1 · Safe boot, reset cause, watchdog (P0, S)** · was FW5a · F215, F49, F238, F258
- First lines of `setup()`: every actuator pin to its safe level (§7.0 firmware 1). One `Serial.begin` (F49).
- Reset cause from the SAM3X reset controller, printed in the boot banner and kept for the next sample record (`reset_cause`).
- Brown-out detector on. Hardware watchdog at 4 s, kicked from the existing wait helpers and pump loops. Print the longest gap between kicks at the end of each procedure.
- Bound every SPI read loop (encoder, ABP) with a retry limit and a timeout, so the bus cannot freeze the console (F238).
- A hidden `wdt_test` command (service only) that hangs on purpose.
- Prototype stays usable: no command changes; the boot banner gains two lines.
- Done when: native tests for the reset-cause decoding; bench: `wdt_test` resets the board and the banner says `watchdog`; a reset with the pump relay wired shows no relay click (with B1); the longest gap in a full `run_sample` is under 1 s.

**A2 · Sensor-fault rules from the field (P0, M)** · was FW11 · F234, F236, F233, F252, F249, F221
- **Flat pressure while pumping** (F234): a pure rule in `src/Core/`. The pump at or above a power for N seconds with `P_in` below a floor (or a spread below a minimum) means a dead or detached sensor: stop, end reason `sensor`. Thresholds from the field logs (normal runs vs the 2026-11-04 00:58 event), marked PLACEHOLDER until the bench check.
- **Encoder: two reads that agree** (F236): a rotation target, a home and an alignment check accept an angle only after two consecutive reads within 0.5°. `oring_state` reads once.
- **Purge on the cap is a failure** (F233): `purge_end=` on the TEST line; `pressure` or `timeout` → `failed_at=purge` (the filter stays free, D75).
- **Pause resume on the filtered reading, with a raw-reading artefact check** (F252, F249; decided by the user 2026-11-12, D99). The resume compares the **filtered** pressure with the resume level, as the `Settings.h` comment says. The raw reading is checked alongside it as a safety measure, so that a sensor artefact can neither restart nor hold the pump:
  - a raw sample that jumps further from the filtered value than the pump can physically change the pressure in one read (a rate limit in bar/s, from the field traces) is an artefact: counted, logged, and not used;
  - artefacts in a row (N, PLACEHOLDER) or over a time window make the reading implausible, and the pump stops with end reason `sensor` (the existing dead-sensor path);
  - the cap keeps acting on the raw reading (a real spike must still stop the pump), but only after the same check, so one bad frame does not trip it (F120);
  - the pause still starts on the raw reading, so the pump reacts to a real rise at once.
  The rule is a pure function in `src/Core/` with boundary tests, replayed on the 2026-11-05 pauses (the 3.37 → 1.70 bar drop in 0.2 s) and on the night-run sensor failure of 2026-11-04. Behaviour change: every pump run (listed in the PR).
- **Print the applied pump power** on the purge and prime lines (F221).
- Prototype stays usable: the end reasons are new values; the session tool and `run_report.py` accept them.
- Done when: the flat-pressure rule fires on the replayed 2026-11-04 trace and never on 10 replayed normal runs (native test on extracted log lines); bench: the sensor unplugged during a pump run stops it within the rule's window.

**A3 · Spool software from the field (P0, S)** · was FW13 · F251, F243, F224, F241
- **Stricter origin rule** (F251): a hold stop more than `SPOOL_ORIGIN_TOL_CM` from the zero is not an origin; the roll up fails and asks for a check, instead of zeroing. Keep the two-pass roll up (D95).
- **Time limits from the field** (F224, D72, D79): a full 40 m rewind takes about 2 min at 31–37 cm/s (F220), so the limit becomes length ÷ minimum speed × margin, bounded, with a fixed cap well under the current 20/30 min.
- **Log every panel spool move** (F243): direction, length before and after.
- Done when: native tests on the origin rule with the 2026-11-06 08:00 and 12:58 move logs (both rejected as origin); bench: a hold at the switch holder 1 m out fails the roll up; a panel move prints a line.

**A4 · Session tool field fixes (P0 part, then P1)** · was HO7 · F228, F229, F239, F222, F244, F248, F208 · tool only, no firmware
- **P0:** resume an interrupted list in the same run folder: show done and left, choose the row (F228). Archive a sample under its slot instead of erasing it (F229). Confirm the loaded filters against the start list, slot by slot (F244). A prefilter question at `session` (F248). **Detect a Due reboot** (the boot banner during a run): stop the list, mark the running sample `SUSPECT` with the time, never restart it by itself (§7.0 firmware 9).
- **P1:** `schedule time <row> <HH:MM>` and depth for waiting rows, `schedule reload` (F239). The notes question at the end of a sample, without holding the next row (F222). The timing estimator (spec in §7.5, was HO6) and its planning warnings (F208).
- Prototype stays usable: the same commands and files; new commands only add.
- Done when: `pytest tools/` covers each case with fixtures from the field folders (a stopped list, an erase, a reboot mid-sample); a dry run with the simulator script or the bench.

**A5 · One implementation per action, and the clean-up (P1, M)** · was FW4 · F2, F5, F34, F40, F41, F42, F51, F63, F122, F202, F45, F16
- Each physical action written once, as a brick with a result struct: `pump_through_filter`, `purge_line`, `empty_line`, `empty_filter`, `shield`, `rotate_to_slot`, `spool_move`. Options: `log`, `pause`, `interactive`. The test menu and the run call the same bricks; `fill_test` and `full_cycle` become callers.
- Remove or fix every path that bypasses the cap or the lock: panel Pump mode (F2), `calibrate_flow` (F41), `step_fill_container` (F42), `purge_pipes_manifold` (F63), `cal_DNA` (F202), the bare `rotateMotor()` after `step_purge` (F40). The cap decision and the rotation rule (ROTATE / UNLOCK_THEN_ROTATE / WAIT / REFUSE) as pure functions.
- Stepper coils off while locked (F34). Remove the V23 relay calls (F51, approved 2026-09-24). Procedures end **locked** (§3.2).
- The deferred renames (D67): `actuator_home` → `init`, `valve_1` → `valve_air`, `valve_manifold` → `valve_water`. Keep the old names as aliases for one release, so the tutorial and the operator's habits still work.
- A dead-code list with evidence for each item (keep, delete, ask). Delete nothing without approval (rule 2).
- Done when: native tests for the bricks' decisions; bench: `fill_test volume=200` before and after gives the same end reason and mL within 5 %; a lock then a rotation auto-unlocks; a rotation with the pump running is refused.

**A6 · Supervisor `service()` (P1, M)** · was FW5b · needs A5 · F47, F82, F190
- A `service()` call at least every 50 ms from every wait: cap check, deadline, watchdog kick, B4 heartbeat, keys (`x`, `s`), VNH5019 diagnostics (F47). `wait_ms()` returns false on abort. The micro pump and the O-ring moves become stoppable (F82, F190).
- Done when: the maximum gap is below 50 ms in every procedure (printed); a cap trip during a cycle stops the pump within 200 ms; `x` stops an O-ring move within one step.

**A7 · Stop, safe state, RECOVER, modes (P1, M)** · was FW6 · needs A6 · F23, F71
- `enter_safe_state()`; one fault enum, a latched fault and a 16-entry ring buffer; warnings never latch.
- Modes as in §3.5. The console test menu is SERVICE (`exit` to leave); `stop` is the emergency stop. The panel works only in SERVICE.
- Boot goes to RECOVERY_REQUIRED. `recover` runs the boot checklist of §7.0 step by step and stops at the first failure.
- F71: decide the manual procedure for UNKNOWN + not aligned (a hand turn with the O-ring released, or homing off-slot if safe).
- E-stop input (D37) read when `ESTOP_INSTALLED` (B4): debounced; a broken wire counts as a stop.
- Prototype stays usable: until A10, boot lands in SERVICE after RECOVER, so the session tool works as today.
- Done when: every mode transition is tested natively; bench: STOP in each phase leaves the valves closed and the O-ring locked; 20 cold boots reach the console.

**A8 · Motion journal and anti-tangle (P1, M)** · was FW12 · needs A7 · F258, F257, F223
- The journal of §7.0 (firmware 4) in the Due flash, wear-levelled, written at step boundaries and before each motion (not in a loop). If flash wear or write time is a problem, use a FRAM on I²C (B15).
- The spool height and the last known positions survive a reset (F223), but are only *hints*: RECOVER still checks them.
- The absolute turn window and the encoder-jump check (§7.0 firmware 6).
- Done when: native tests for the journal format, the wear levelling and the turn window; bench: a dropout in a rotation (PW1 test) boots with "motion in progress: rotation, slot N" and the sample is `SUSPECT`.

**A9 · Sample record v2 and the flow correction (P1, M)** · was FW8 · needs B5 data for the correction · F150, F254, F204
- Add to the record: `reset_cause`, `purge_end`, `spool`, `depth_m`, the flags (`SUSPECT`, sensor degraded), the corrected volume.
- **Flow correction:** a pure function `corrected_ml(pulses, flow)` from the B5 bucket-check table, unit-tested; the stop volume uses it. The plain and trusted volumes stay in the record.
- Throttle the `End cond` lines of a clogged emptying (F204).
- Done when: native tests on the correction with the B5 table; bench: three 1 L runs at field flow end within 5 % of the weighed water.

**A10 · Protocol v1 on Native USB, beside the console (P1, L)** · was FW7 · needs FW1, A7 · F8, F10, F24, F33, F53, F210, F37
- Parser and dispatch in `src/Core/`, tested against `protocol/vectors.json` and garbage input (never a crash, never a command run).
- The host channel on Native USB; the console stays on the Programming port. Output to the host never blocks (drop and count, F53).
- `RUN_CYCLE` takes the slot from the host; ack, events, result with the same id; link watchdog.
- **The session tool moves to Native USB** and keeps its commands: one operator console with records for every sample (F210). Legacy Pi text commands behind a flag, off by default.
- Done when: `tools/protocol_smoke.py` passes on the Native port; a host restart during a purge does not reset the Due; the session tool runs a schedule over the protocol.

**A11 · The host package takes over the session tool's logic (P1, L)** · was HO2/HO3 · needs A10, A4
- Move the slot book, the records, the schedule and the resume from `tools/` into the `host/` package (SQLite, migrations, slot states of §3.6, CSV export agreed with the lab). The session tool becomes the console front end of `host/`.
- Mission policies (from the old A11): a late event runs only within a set delay; a sample fault marks the slot `FAILED` and retries on the next filter up to N times; a system fault pauses the mission and never retries by itself; blanks pause for the operator.
- Done when: the field folders of Bois Chamblard and LéXPLORE import into `host/` and export the same records (golden files); a 14-sample plan runs in simulated time with a power cut, a link drop, a clog, a STOP and a reboot mid-cycle.

**A12 · Remote monitoring and alerts (P1, M)** · was HO4/HO5 · needs A4 (minimal) or A11 · hardware B11
- A12: the host on site (laptop or Pi) as a service that restarts after a fault, a trusted clock synced to UTC (its offset written in each record, so several units and the lab share one time base, §8.0), the data backed up nightly. A12: alert levels (critical: stop, E-stop, cap, sensor, leak, recovery needed, link lost 10 min; warning: clogged, few READY slots, disk; info: sample done), repeats until acknowledged, one channel interface (for example ntfy), a daily status, remote access over a VPN.
- A minimal version from the session tool is allowed for Pilot B if A11 slip.
- Done when: a simulated critical alert reaches the phone and repeats until acknowledged; a delivery failure is retried; a power cut on the host resumes correctly.

**Optional software (Opt):** do when there is a need.
- **A13 · Pump control tuning on the current pump** · was FW14. The pauses at the cap margin (F249, F151, up to 113 per sample) and the ±0.25 bar pulsation (F141, F146). Fit a plant model from the field logs, compare anti-windup and an output ramp limit in simulation, then on the bench. The pump and damper themselves are stage 3.
- **A14 · Runtime parameters** · was FW15 (old P2-2): a parameter registry in `src/Core/` (name, unit, default, range, "only when idle", calibration tag), `param list|get|set|dump`, a hash in `HELLO`; the host keeps calibration sets. Ends the reflash for each calibration.
- **A15 · Trace telemetry and analysis** · was FW16 (old P2-4): 10 Hz traces over Native USB, `tools/record_trace.py`, `analyze_trace.py`, `compare_tune.py`, a `pump_step` test. Feeds A13 and the stage 3 pump choice.
- **A16 · Host API and web app** · was HO8 (old P2-9, P2-10, P2-11): HTTP and WebSocket, `POST /stop` always allowed, a mobile page with a big STOP, live pressure and flow, the slot grid, alerts; web push.
- **A17 · Commissioning and metadata** · was HO9 (old P2-12, P2-13): a commissioning checklist stored in the database; FAIRe and MIxS column mapping (check the term names against the official sources); merged exports.
- **A18 · Firmware updates from the Pi** · was FW17 (old P2-17): binaries built in CI, flashed only when idle, rollback, git hash checked.
- **A19 · Firmware restructure** · was FW18 (old P2-1): split `src/Tests.cpp`, a command table, one hardware header, slot-count independence (F16) tested with 16 and 24 positions.

### 7.2 Lane B: hardware upgrades of the current machine

Each card: what to do, its **firmware follow-up**, and the sign-off check. A card can be done by someone new to the project; read the findings it names first. Tick it in the log when done.

**B1 · Safe pin defaults (P0, S)** · was PW1a · F215, F170, F258 · do it during R2
- Pull-downs (10–100 kΩ, 3.3 V side) on the VNH5019 INA, INB and PWM of both channels, D42, the valve drivers (D44, D46, D40), D48, and the DAC line. A 10 kΩ pull-up to 3.3 V on A4988 ENABLE D11. Check the EN/DIAG pull-ups of the shield. Update [pinout.md](../hardware/pinout.md) and the schematic (F183).
- Firmware follow-up: A1.
- Check: with the Due held in reset (RESET pressed) and with the Due unpowered, 24 V on: no relay click, no motor or stepper movement, valve lines at 0 V (meter).

**B2 · Manifold encoder coupling lock (P0, S)** · was HW9 · F187, F191, F203, F216
- Check the encoder cable and connector first (F203). Lock the coupling mechanically (clamp or set screw with thread lock) and put a witness mark across it.
- Firmware follow-up: `encoder_repeat`, then `cal` on slot 7 and a new `purge_angle`.
- Check: `encoder_repeat` spread under 0.3°; after 100 rotations (`rotate_all dry`), the witness mark has not moved and `cal` gives the same encoder value within 1 count. **Opt B14:** a Hall sensor and magnet at the purge slot as an independent index (D39-class free pin); firmware checks it at every pass of slot 0.

**B3 · Pressure sensor mount, bus and range (P0, S)** · was HW4 · F234, F235, F36, F126, F136, F237
- Bond the sensor to its tubing and add a strain relief on the cable (user's proposal, 2026-11-04). Keep the connector dry (heat-shrink or a sealed connector).
- SPI bus: shorten the sensor cable, add series resistors (about 33–100 Ω) on SCK and MOSI if the cable stays long; check that the sensor cannot hold MISO low when disconnected (F234: `00 00 00`). Trace the LED fault (F237).
- **Range (F126):** read the marking, then a water column check (1 m water = 0.098 bar; 311 counts on 60 psi, 124 on 150 psi). **Calibration to 3.6 bar (F136):** a 0–4 bar or 0–6 bar reference manometer, three points between 2 and 3.6 bar.
- Firmware follow-up: `ABP_FULL_SCALE_PSI` and the gain and offset from the checks (removes a PLACEHOLDER); A2's flat-pressure thresholds checked.
- Check: `abp_raw` 20 lines within ±20 counts at rest; `encoder_read 200` with no bad frame while the pump runs; reading within 0.1 bar of the reference at 2.0, 3.0 and 3.5 bar.

**B4 · Independent stop and power protection (P1, M; before any unattended night run)** · was PW1b · F257, F258, F125, F173, F174 · also covers the old HW1 (part 2) and HW2
- The heartbeat-held 24 V actuator rail with an external watchdog (§7.0 hardware 2). A latching E-stop (DC-rated above the measured 24 V current) in the same line, with an NC contact to D37.
- Logic on its own regulated rail with a hold-up; supply sense dividers on two free analog pins (24 V and logic rail).
- Fuses per branch (about 1.5 × the measured current), a TVS and reverse protection on the input, a USB isolator to the laptop.
- Finish the HW1 electrical audit part 2 ([power.md](../hardware/power.md) §4, M1–M44), including F125 (reset when the A4988 enables), F173 (the leaky capacitor), F174 (the shared 24 V rail).
- Firmware follow-up: the heartbeat output from `service()` (A6), the E-stop input (A7), the supply log (§7.0 firmware 8), behind `ESTOP_INSTALLED` and `HEARTBEAT_INSTALLED`.
- Check: the PW1 bench test (§7.0); a forced hang drops the 24 V within 0.5 s; the E-stop stops a running pump within 0.5 s; every line of the electrical audit is "Acceptable".

**B5 · Flowmeter at field flow (P1, S)** · was HW3 · F150, F254
- Bucket checks: five flows from 30 to 600 mL/min (set by a clamp on a clean filter), three runs each, weighed on the 0.01 g scale. Record pulses, time and mass.
- Firmware follow-up: A9's correction table.
- Check: the correction brings each run within 5 % of the weighed water. **Opt:** if the error below 50 mL/min stays above 10 %, a low-flow flowmeter (stage 3 also needs one).

**B6 · Spool mechanics, permanent (P1, M)** · was HW10 · F246, F250, F251, F240, F213
- Replace every wooden part (frame, level-wind support, top-switch holder) with a material that does not take up water (aluminium, POM, HDPE, or PETG/ASA prints). Level-wind guide in a tougher material with metal fixings. A belt tensioner. A switch holder that the drum cannot stop against (F251).
- Measure the drum diameter (`spool_calibrate`) and the friction at the up-button guide (F240). Watch a first roll up from 2 m with a container under the purge outlet (F213, the drain path).
- Firmware follow-up: `SPOOL_DRUM_DIAMETER_CM` (removes a PLACEHOLDER); A3's origin rule checked against the new holder.
- Check: the guide turns freely by hand over the full tube length; 5 roll ups and 5 dives over 40 m with no stall and no `stalled` line; the encoder length matches a tape mark at 10 m within 10 cm.

**B7 · Measured DNA Shield dose (P1, M)** · was HW13 · F255, F156, F159, F81, F80, F4 · with the lab (C2)
- First the drive: meter on D48, the transistor and the pump supply (F159). A shield prime command for a new bottle or deployment (F81).
- Measure the dose: a load cell under the shield bottle (HX711 class, 0.1 g) is simple and reads the real volume leaving the bottle; an inline low-flow liquid sensor is the alternative. Choose with the lab.
- Firmware follow-up: the shield step stops on the measured volume (with the timed run as a timeout), the record gets `shield_ml`, and the dose rule takes the end reason and the trusted volume into account (F255: less on clean filters, enough on clogged ones). The rule's numbers come from C2.
- Check: ten doses within 10 % of the weighed target; the record shows `shield_ml`; an empty bottle gives a warning, not a silent sample.

**B8 · Power at the pilot site (P1, M)** · was HW5 · F25, F32
- A DC-UPS or a LiFePO4 battery between the supply and the CoWaS (§3.8). Regulated 24 V and 5 V rails. A DC energy meter.
- Size two cases: a platform supply with outages (LéXPLORE), and an **off-grid** site (a river station, §8.0) running on the battery between visits (for example 4 samples a day for 3–4 days, plus the always-on load). Solar charging only if the budget asks for it.
- The measured budget: idle W and Wh per cycle, with and without the spool. 20 cold boots from the battery, 20 of 20 reach the console; a pump start does not reset the Due.
- Check: a platform cut of 1 min during a sample does not reach the CoWaS (the UPS holds); the budget gives 50 % margin for Pilot B.

**B9 · Enclosure and field kit (P1, M)** · was HW6 and HW8 · F35
- An IP65 enclosure (or IP54 with a shade for a platform), cable glands, desiccant, a leak sensor at the lowest point (D41 or A6), a waterproof temperature probe on the intake (DS18B20 on D43). The intake strainer (200–500 µm, sized for river debris as well as lake water) and weight, quick-disconnects. For a river site: mounting above the flood level, the intake anchored against the current.
- Firmware follow-up: B12.
- Check: a hose test on the closed box with no water inside; the leak sensor trips on a wet cloth.

**B10 · O-ring endurance and home switch (P1, M)** · was HW7 · F60, F71, F172
- Check the A4988 current is unchanged since the last calibration (F172). `lock_endurance 200` with no encoder drift. Pressurise to 3.0 bar with the outlet clamped for 10 min, coils off. Keep the O-ring locked for 7 days, then check for leaks.
- A Hall switch at "unlocked" (D39), so every unlock is confirmed.
- Firmware follow-up: B12.

**B11 · Pi and LTE on site (P1, S)** · was HO4-hw. A Pi (or the laptop), an LTE modem and antenna, a SIM, an RTC battery. Measure the idle and busy current for B8.

**B12 · Firmware for B9 and B10 (P1, M)** · was FW9 · the lane B follow-up
- Leak sensor = a fault that stops everything. The O-ring home switch: each unlock must reach it within a set number of steps. Water temperature logged per sample and every 30 min. Each behind its `*_INSTALLED` setting.
- Done when: each sensor unplugged gives the expected warning or fault; 50 lock cycles with the home switch are clean; the water temperature is within 0.5 °C of a reference thermometer.

**Optional hardware (Opt)**
- **B13 · Field cartridges** · was HW14 (user, 2026-11-06; kept in stage 2 on 2026-11-12; the first optional step if Pilot B runs at 6 h for more than 3 days, §8.0). Three cartridges: one always carries hollow tubes for the ethanol cleaning; two carry the filters, so the next one is cleaned and loaded while the other runs. The cleaning then takes no sampling time. Firmware and tool follow-up: a cartridge ID in the session (`session` question), a slot book per cartridge, the ethanol purge on the cleaning cartridge. Check: a cartridge swap in under 5 min with no tubing reconnected by hand; the manifold calibration holds after 10 swaps (`cal` within 1 count).
- **B15 · On-device storage** · was HW15 (old P2-19): a FRAM (journal) or an SD card (records), so a `run_sample` without the PC still saves its metadata.
- **B16 · Relief valve** · was part of HW2. Set above the 3.6 bar cap and below the weakest part's rating (F143: the rating is unchecked; the old 2.5–3 bar plan would open below the cap). The user accepted running without it (F144). Reconsider for Pilot B, where nobody watches.

### 7.3 Lane C: lab

**C1 · Pilot A lab results (P0 for the science)** · was LA0 · the lab
- **Record corrections first:** mark E1_T5_01M_R1 as not valid (empty slot, F244); flag E1_T7_20M_R1 (expired filter, F253); correct the label of the E2 T3 1 m sample (typed E1_T3_01M_R1); note the hand-added DNA Shield of E2_T7_05M_R1.
- Extract ([extraction-protocol.md](../lab/extraction-protocol.md)) and sequence the LéXPLORE filters. Record per batch how the DNA/RNA Shield in the CoWaS filters was handled (§6 of that protocol). Compare machine vs manual at the same time and depth (18 pairs), and read the blanks.
- Defines "clean" (blanks) and "comparable" (machine vs manual) in numbers, and decides whether the purge rule (§3.7.1) or the shield handling must change before Pilot B.
- Check: a short results note in `docs/lab/` with the pass criteria for Pilot B.

**C2 · Shield dose and residual water (P1)** · was LA3 · with B7
- `shield_calibrate` (exists): durations, three repeats, weighed. Per filter: dry mass → after the water cycle (residual water) → after the shield → after 1, 3 and 7 days (retention). `tools/analyze_la3.py` gives PASS/FAIL against the thresholds in [shield-protocol.md](../lab/shield-protocol.md) §8.
- Done when: the measured dose is within 10 % of the target, and the dose rule for B7 is written.

**C3 · Carry-over and cross-talk (P1)** · was LA4 · needs A5
- Dye or conductivity tracer. The dead volume from a washout curve; the purge needed for under 0.1 % carry-over, with and without the spool; cross-talk on a dyed slot while the rotor passes the others. `tools/analyze_la4.py` fits the curve.
- Report the carry-over against the most sensitive assay in view (qPCR of a synthetic tracer, §8.0), not only metabarcoding: a carry-over invisible to one can be visible to the other.
- Done when: the stage 2 purge rule is decided (§3.7.1) and the 13 min spool purge timeout is checked against it.

**C4 · SOPs (P1)** · was LA2 · draft [filter-sop.md](../lab/filter-sop.md)
- Update for the field: the prefilter check, filter batch and expiry, the ethanol cleaning, the cartridges (if B13). Remove the "until A11" notes once A4 has the commands.
- Done when: the two-person dry run of §7a passes.

### 7.4 Lane C: validation

**C5 · Supervised field run with the P0 fixes (P0 gate)** · was VA5 · needs G1
- A one- or two-day supervised run (LéXPLORE or a pier) with the timed schedule, the spool, two blanks.
- Pass: no lost sample to a machine cause; no motion after a deliberate power cut of the CoWaS supply during a rotation (the PW1 test in the field, supervised); the records complete with `reset_cause` and `purge_end`.

**C6 · Bench acceptance and 72 h soak (P1 gate, go or no-go for Pilot B)** · was VA1 and VA3 · needs every P1 step
1. A full 14-slot mission from the host on the pilot power (B8): every record complete, volumes within 5 % of the weighed water (corrected).
2. A 72 h unattended soak with remote alerts, including a power cut and a forced fault.
3. A power cut in each phase (§7.0): the interrupted slot is `SUSPECT`, the mission resumes only after RECOVER.
4. STOP and E-stop in each phase: valves closed, O-ring locked, nothing new starts.
5. A host restart mid-cycle: the Due is not reset, and the result arrives by request id.
6. A clogged filter: end `clogged`; the cap is never exceeded (maximum `P_in` printed).
7. A forced hang: the 24 V drops within 0.5 s (B4).
8. The USB cable pulled mid-cycle: the cycle ends safely and nothing new starts.
9. The export opened by the lab: columns accepted.
10. Power: measured Wh per cycle and idle W give a margin of at least 50 %.

Go / no-go: 1–8 all pass. A failure in 9 or 10 needs a written workaround.

**C7 · Pilot B (P1)** · was VA4 · see §8.2.

### 7.5 Spec kept: sample timing from field records (part of A4, P1; was HO6)

The full specification of 2026-10-26 (D55) is unchanged and kept in [the archive](../archive/roadmap-stage2-2026-11-12.md) (§7.2, HO6). In short: the host reads the last 10 session folders at each start, groups complete samples by the key `spool;depth_m;target_ml;fw;calib`, takes the 90th percentile × 1.05 of a repeatable group (at least 3 samples, each within ±15 % of the median: PLACEHOLDER), stores the largest value ever seen per key in `timings.json`, and warns (never blocks) when a planned slot is shorter than its key's value. The field folders of 2026-10-26 to 2026-11-06 are the first real inputs; the step times (D96) give the breakdown. Prerequisite: `spool` and `depth_m` in the record (A9).

---

## 7a. Bench checks still to run

Run with R3 (the first flash after the repair), then after each lane A flash.

| Step | Check | Pass when |
|---|---|---|
| R3 | Every item marked "built, not flashed" in [findings.md](../logs/findings.md) and [decisions.md](../logs/decisions.md): at least the manifold guard (stall and over-travel), the two-pass roll up (D95), the 100 % prime (D94), the pause and resume messages (F249), the step log (D96), the spool origin rules (F214, F226, F227, F232) | Each one's own check in its log entry passes; the log shows the new lines |
| R3 | `encoder_repeat` and `cal` after the repair | Spread under 0.3°; `slot7` lands on slot 7 |
| R3 | Pressure reading after a pump stop (F249): 100 ms trace with the pump off and the line shut, against a manometer | The drop after a stop is explained (sensor or line) |
| FW2 | `leak_hold`, `multi_sample`, `full_cycle`, `full_all` in sensor mode | Each prints `TEST\|…\|PASS`, no SAFETY line, O-ring locked at the end |
| FW2 / A1 | After a reset, `sample1m` is refused while the lock is UNKNOWN | `REFUSED`, nothing moves; after `actuator_home` it runs |
| FW3 / A5 | Every pump command runs under the cap | Commands that do not poll the cap are not used until A5: `cal_DNA` (F202), panel Pump (F2); see [commands.md](../software/commands.md) §7 |
| B3 | Spare 150 psi sensor alive (optional) | Steady 0.0–0.1 bar at rest, a clear rise when blown into. Check pin 1 first |
| B3 | Manifold alignment by airflow (F131) | Maximum flow within about 1° of the table value |
| Maintenance | Ethanol manifold prime, slot flush and final purge | With no filters installed, at least 50 mL actual volume through slot 0 before slot 1; at least 50 mL actual through each sample slot; a final 50 mL actual through slot 0 |
| HO1 | Host against the real Due (after A10) | Native port found; reconnect after a cable pull with the same boot id; opening the port does not reset the Due |
| C4 | SOP dry run, two people, 14 Sterivex | Loading ≤ 30 min, unloading ≤ 20 min, no leftover caps, no drips, no label mismatch |
| C2 | Shield calibration | CV ≤ 5 % per duration, R² ≥ 0.995 (shield protocol) |

Dropped: PID tuning by measurement for stage 1. The gains stay at 200 / 100 / 100 (user decision 2026-10-19); A13 (Opt) reopens it.

---

## 7b. Items carried from stage 1 and the field: where they went

Every item of the old §7b list (stage 1 operator test and the field work of 26 Oct – 6 Nov) now belongs to a step. The full old text is in [the archive](../archive/roadmap-stage2-2026-11-12.md#7b-carried-from-the-stage-1-operator-test).

| Item | Ref | Now in |
|---|---|---|
| Vent: the O-ring unlocks even if the line stays above 0.3 bar | F192 | A5 (a venting step with a real stop, if seen on the bench) |
| Stop rules read the flowmeter, which reads almost nothing below 25 mL/min; confident-volume threshold | F150, F194 | B5, A9 |
| Manifold encoder coupling, manifold sometimes does not turn, two zeros | F187, F191, F203, F216 | B2 |
| Water drips from the filter after the unlock | F205 | A5 |
| ~940 `End cond` lines on a clogged emptying | F204 | A9 |
| Purge rule, 13 min spool timeout, 3 L purge pressure check | F30, F110, F130, F207 | C3, R3 (pressure check) |
| Ethanol cleaning volumes | — | §7a (Maintenance) |
| Filter ID, site, weights on paper only | A11 | A4, A11 |
| `cal_DNA` without a cap check | F202 | A5 |
| Code clean-up: two sample paths, `fill_test`'s loop, `maintenance.cpp`, `step_dive` | — | A5 |
| Timed list does not check the timeline | F208 | A4 (timing spec, §7.5) |
| `run_sample` outside the session tool has no record | F210 | A10 |
| Renames `init`, `valve_air`, `valve_water` | D67 | A5 |
| Plate orthogonality off, redone on site | F164, F217 | **stage 3** (V2 frame) |
| Pump turns on at flash or reset | F215 | B1, A1 |
| Spool origin interrupts, origin limits, hold rule | F214, F224, F232 | R3, A3 |
| Fill dive keeps the deployment valve shut | F219 | R3 (first dry fill on the bench) |
| Purge and prime at 100 %: print the applied power | F221 | A2 |
| Purge stopped by the cap goes on to sampling | F233 | A2 |
| Pressure sensor shorts the SPI bus; mount; flat reading | F234, F235, F36 | B3, A2 |
| Panel LEDs off with the sensor unplugged | F237 | B3 |
| Console froze after the sensor and encoder faults | F238 | A1 |
| Encoder: accept an angle after two reads agree | F236 | A2 |
| Schedule times cannot be changed in place | F239 | A4 |
| Spool tube pulled sideways; friction at the up-button guide | F240 | B6 (friction); stage 3 (depth sensor at the intake) |
| Home the spool between samples, log panel moves | F241, F243, D88 | A3 (D90 already rolls up after every sample) |
| Sample notes at the end | F222 | A4 |
| Field cartridges | field log §27 | B13 (Opt) |
| Pump and PID tuning, the pauses | F249, F252, F150 | A2 (resume rule), A13 (Opt) |
| Spool mechanics permanent, drum stop at the holder, wooden up-switch holder | F246, F250, F251 | B6 |
| DNA Shield dose from the filter state | F255, F156 | B7, C2 |
| Flowmeter at low flow | F254, F150 | B5, A9 |
| Relief valve, E-stop and leak at 3 bar: accepted 2026-10-23 | F144 | B4 (E-stop); relief valve Opt |
| Power dropout (PW1) | F257, F258 | B1, B4, A1, A6, A7, A8 (§7.0) |

---

## 8. Pilot procedures

### 8.0 Intended uses that shape the pilots (context, not plans)

Two research master projects are in preparation (2026-11). Neither is confirmed, and the CoWaS takes on no extra work just for them. They show **where the machine is meant to go**, so Pilot B tests what those uses would need first. The uses are summarised here, not adopted.

| Intended use | What it would ask of the machine | Already in a step | Gap, and where it goes |
|---|---|---|---|
| **Rivers, at a fixed point:** a sample every 6 h for several weeks, in two seasons; blanks inserted in the sequence | Unattended for weeks; 14 slots last 3.5 days at 6 h, so a visit every 3 days to swap filters; turbid water and rain peaks (clogging, debris); no platform power; no spool (shallow water) | A12 alerts, B8 power, B9 strainer, A4 schedule, the end rules | Fast filter swaps on a visit (B13 cartridges); off-grid power (B8); more slots (stage 3 V2-1) |
| **Rivers, an array:** three units at 0, 3 and 10 km on one stream; a synthetic DNA tracer followed by qPCR | Several identical machines; one time base across units (UTC, synchronised clocks); sample times known to about a minute; carry-over far below the qPCR detection limit; possibly denser sampling while the tracer passes (limited by the cycle time, about 20 min without the spool) | A12 trusted clock, the step log (D96), C3 carry-over | Copies of the machine and a design for building them (stage 3 V2-0, V2-7); discharge and water level beside each unit (stage 3 V2-5 sensors) |
| **Caves (karst):** a pilot in a show cave, if conditions allow | Near 100 % humidity, cold (about 8–10 °C), possibly no mains power, hard access and transport; low DNA concentration, so large volumes per filter and a prefilter | B9 enclosure, B8 power, the volume and clog rules | Portability, humidity-proof electronics and large-volume filtration are V2 requirements (stage 3 V2-0, V2-5); not for the current machine |

**What this changes in stage 2 (small):**
- Pilot B (§8.2) may run on a **river or a lake**; the plan below covers both.
- **B8** sizes power for an **off-grid** site as well as a platform.
- **A12** syncs the clock to UTC and records the offset in each record, so units and the lab share one time base.
- **B9** adds flood-safe mounting and a strainer sized for river debris.
- **C3** reports carry-over against the most sensitive assay in view (qPCR of a tracer), not only metabarcoding.
- **B13** (cartridges) stays optional, but becomes the first optional step if Pilot B runs at 6 h for more than 3 days.

### 8.1 Pilot A (supervised): done

Run at LéXPLORE, Lake Geneva, 3–6 Nov 2026. Goals, results and the success criteria are in [stage 1 §4.2](stage1-FINISHED-prototype.md#42-léxplore-lake-geneva--pilot-a-3--6-nov). Open here: the lab results and the record corrections (C1).

### 8.2 Pilot B (unattended, 1–2 weeks): C7

- **Before:** G2 (every P1 step, C6 passed, including the 72 h soak). The site, its power and its access are agreed: a lake (LéXPLORE platform or a mooring, with the spool) or a river station (no spool, off-grid power, flood-safe mounting). Choose the one closest to the intended use at the time (§8.0).
- **Plan:** a fixed interval (for example every 6 h). Per filter set: an equipment blank at the start, the samples, an equipment blank at the end; on longer runs a blank in the middle as well, so carry-over over time is measured. 14 slots last about 3.5 days at 6 h, so either a visit every 3 days to reload (and clean), or a longer interval. A few manual comparison samples on the visit days. Each visit checks leaks, power, the strainer and the alerts log.
- **Records:** UTC times with the clock offset, water temperature (B12), the step times, every warning; the paper sheet holds the filter batch and expiry and the visit notes.
- **Remote:** alerts to the phone (A12), a daily status, a person on call who can reach the site.
- **After:** unload per the SOP, chain of custody, export the records and logs to `data/pilots/B_<date>/`, debrief within 48 h.
- **Success:** at least 90 % of the planned cycles complete without a fault (12 of 14 per filter set); blanks clean and machine vs manual comparable by the C1 criteria; no unrecovered fault; no motion after any power event; the alerts reached the person on call within 15 min of each critical event.

---

## 9. Future work

[Stage 3](stage3-full-system.md) holds the V2 machine: what needs a new design, and the lessons that drive it.

---

## 10. Old step IDs

Since 2026-11-12 the stage 2 steps are named by lane and in the order of §1: **R** = Phase 0, **A** = lane A (software), **B** = lane B (hardware), **C** = lane C (lab and validation). The logs, the findings, the decisions and the code comments written before use the old IDs; this table maps them. The version before is in [the archive](../archive/roadmap-stage2-2026-11-12.md).

| Old | Now | Note |
|---|---|---|
| FW0, FW1, FW2, FW3, HO1, LA1, VA2 | unchanged, done | Stage 1 work; see [stage 1](stage1-FINISHED-prototype.md) |
| FW4, FW4a | A5 | Merged; plus the cap and lock bypasses and the renames |
| FW5 | A1 (watchdog, reset cause, safe boot) + A6 (`service()`) | Split so the safe boot comes first (A1 was FW5a, A6 was FW5b) |
| FW6 | A7 | Plus the boot checklist of §7.0 |
| FW7 | A10 | The session tool moves to Native USB; console kept |
| FW8 | A9 | Plus the flow correction and the field record fields |
| FW9 | B12 | Lane B follow-up |
| FW11, FW12, FW13 (named 2026-11-12) | A2, A8, A3 | |
| FW14, FW15, FW16, FW17, FW18 (named 2026-11-12) | A13, A14, A15, A18, A19 | Optional |
| HO2, HO3 | A11 | One step: the host package takes over the session tool |
| HO4, HO5 | A12; the hardware is B11 (named HO4-hw on 2026-11-12) | |
| HO6 | A4, spec in §7.5 | |
| HO7, HO8, HO9 (named 2026-11-12) | A4, A16, A17 | |
| HW0 | §11.1 | Ordering is part of each card |
| HW1 | Part 1 done (stage 1); part 2 in B4 | |
| HW2 | B4 (E-stop, fuses); B16 (relief valve) | |
| HW3 | B5 | |
| HW4 | B3 | |
| HW5 | B8 | |
| HW6, HW8 | B9 | Merged |
| HW7 | B10 | |
| HW9, HW9b | B2, B14 | |
| HW10 | B6 | |
| HW13, HW14, HW15 (named 2026-11-12) | B7, B13, B15 | |
| PW1 | §7.0 (the plan); PW1a = B1, PW1b = B4; its firmware in A1, A6, A7, A8 | |
| LA0 (named 2026-11-12), LA3, LA4, LA2 | C1, C2, C3, C4 | |
| VA5 (named 2026-11-12), VA1 + VA3, VA4 | C5, C6, C7 | |
| R1–R4 | unchanged | Phase 0 |
| Stage 3 P2-1, P2-2, P2-4, P2-9 … P2-13, P2-17, P2-19 | A19, A14, A15, A16, A17, A18, B15 | Upgrades of the current machine (Opt) |
| Stage 3 P2-3, P2-5 … P2-8, P2-14 … P2-16, P2-18 | Stage 3 (V2) | See [stage 3 §5](stage3-full-system.md#5-old-step-ids-p2-n) |

---

## 11. Parts and pins

### 11.1 Parts for lane B

Specifications, not brands. Check each part against the fittings and the measured currents. Order early: lead times set the hardware lane.

| Part | Spec | For |
|---|---|---|
| Arduino Due (×2) | One to replace the board of F257, one spare | R2 |
| Resistor kit | 10–100 kΩ pull-downs, 10 kΩ pull-up, 33–100 Ω series | B1, B3 |
| External watchdog and switch | A retriggerable watchdog IC or timer driving a relay or MOSFET rated above the 24 V peak current | B4 |
| E-stop | Latching mushroom, 2 NC contacts: one DC-rated in the 24 V line, one to D37 | B4 |
| Fuses and holders | Per branch: pump, VNH5019 (both channels), A4988, valves, main. Spares | B4 |
| TVS, reverse-polarity protection, USB isolator | Input protection; full-speed USB isolator for the laptop link | B4 |
| Logic rail with hold-up | A regulated 5 V supply for the Due and sensors, separate from the motor rail; a hold-up capacitor or a small DC-UPS | B4 |
| DC-UPS or LiFePO4 battery with BMS | Between the platform supply and the CoWaS; sized after the B8 budget | B8 |
| DC energy meter (shunt) | Wh per cycle | B8 |
| Pressure sensor spare (ABP, SPI, 3.3 V) and a 0–4 or 0–6 bar reference manometer | Spare `P_in`; calibration to 3.6 bar (F136) | B3 |
| Encoder coupling with clamp or set screw; thread lock | Manifold encoder | B2 |
| Hall sensor and magnet (×2) | Manifold index (Opt), O-ring home switch | B14, B10 |
| Spool parts | Non-absorbent frame material, metal fixings, belt tensioner, a tougher level-wind guide | B6 |
| Load cell and amplifier (0.1 g class) or a low-flow liquid sensor | Shield dose | B7 |
| Scale 0.01 g, 1 L cylinder, buckets, clamp, food dye | Bucket checks, shield, carry-over | B5, C2, C3 |
| IP65 enclosure, cable glands, desiccant, leak sensor, DS18B20 probe, intake strainer (200–500 µm), quick-disconnects | Field kit | B9 |
| Pi, LTE modem and antenna, SIM, RTC battery | Remote monitoring | B11 |
| FRAM (I²C) or SD module | On-device journal or records (Opt) | B15, A8 |
| Sterivex ≥ 40, DNA/RNA Shield, DNA-free water ≥ 10 L, prefilters, luer caps, labels, gloves, cooler | Lab consumables | C4–C3, VA |

### 11.2 Pin plan

The code uses about 31 digital pins and a few analog ones. The real limit is what the PCB shield brings out ([pinout.md](../hardware/pinout.md)).

| Item | Pin | Step |
|---|---|---|
| Pressure `P_in` (ABP, SPI) | SPI CS D8 (unchanged) | — |
| Flow `Q_in` (turbine) | D13 (unchanged); D36 fallback | — |
| E-stop state | D37 (NC to GND, `INPUT_PULLUP`; a broken wire reads as stop) | B4, A7 |
| Heartbeat to the external watchdog | D49 | B4, A6 |
| 24 V rail sense / logic rail sense | A3 / A4 (dividers to under 3.3 V) | B4 |
| O-ring home switch | D39 | B10, B12 |
| Manifold index (Opt) | D51 | B14 |
| Enclosure leak sensor | D41 or A6 | B9, B12 |
| Water temperature (DS18B20) | D43 (4.7 kΩ pull-up to 3.3 V) | B9, B12 |
| Shield load cell (HX711 clock, data) | D50, D53 | B7 |
| FRAM, INA226 current monitors | SDA 20 / SCL 21 (I²C keeps them off the SPI bus, F36) | B15, B4 |
| `P_out`, `Q_out`, turbidity | SPI D45; D12 or D47; A5 | stage 3 |

### 11.3 Electrical rules

- The Due is **3.3 V only**, not 5 V tolerant. Check every module's output level before connecting it.
- **Every actuator input has a defined level when the Due is off or in reset** (B1).
- The DAC outputs about 0.55–2.75 V and is fragile: buffer it. The relay stops the pump, not the zero setting.
- Never feed the Due raw 24 V or a charged 24 V pack (above 16 V).
- One ground point for the power return. Keep motor current out of the sensor grounds. Isolate the USB link to the laptop (B4).
- Flyback diodes on every coil and relay.
- Short SPI and I²C cables (under 30 cm). New sensors go on I²C where possible, to keep the shared SPI bus short (F36). List every I²C address in [pinout.md](../hardware/pinout.md) before ordering.
