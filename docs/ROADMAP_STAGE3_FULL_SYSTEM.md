# CoWaS roadmap, part 2: after the first lake data (P2)

These steps start **after Pilot A** (some after Pilot B). They follow the same rules as [ROADMAP.md](ROADMAP.md) §0 and §6: one step = one session = one PR, and every step ships unit tests and a hardware checklist.

Many of them were designed in detail in v1. Each prompt below is complete, and it also points to the v1 prompt in [archive/ROADMAP_v1.md](archive/ROADMAP_v1.md) §5 for background. **Section numbers inside the v1 prompts refer to the v1 file.** Where v1 and this file disagree, this file wins.

**Order.** Choose it from the pilot results. A sensible default:

1. P2-3 → P2-4 → P2-5 → P2-6: understand clogging and the pump.
2. P2-2: runtime parameters.
3. P2-9 → P2-10: API and app.
4. The rest by need.

Re-plan after the Pilot A debrief.

| ID | Title | v1 | Depends on | Hardware |
|---|---|---|---|---|
| P2-1 | Firmware restructure + dead-code audit | S1.1, S0.1 (audit) | FW8 | — |
| P2-2 | Runtime parameters: calibration without reflashing | S2.1 | FW7, HO3 | — |
| P2-3 | Hydraulic sensing: 2 pressures + 2 flows, phase-aware | S3.1 | FW8 | P_out, Q_out |
| P2-4 | 10 Hz trace telemetry + analysis tools | S3.2 | P2-3 | — |
| P2-5 | Pump measurement campaign (people) | §6 | P2-4 | test bench |
| P2-6 | Pump control variants + full end conditions | S3.4 | P2-5 | — |
| P2-7 | Leak, dead-sensor and pressure-signature detection | S3.5 | P2-3 | — |
| P2-8 | Environmental sensors (turbidity, compartment temperature/humidity) | S3.6 | FW9 | sensors |
| P2-9 | Host REST + WebSocket API | H6 | HO5 | — |
| P2-10 | Web app (PWA) | H7 | P2-9 | — |
| P2-11 | Full notifications (web push, SMS) | H8 | P2-9 | — |
| P2-12 | Metadata standards (FAIRe/MIxS) + export merge | H3 | HO2, lab feedback | — |
| P2-13 | Commissioning workflow in the host | H5 | P2-2 | — |
| P2-14 | Spool / depth reintegration | I3 | spool repaired | spool, depth sensor |
| P2-15 | Duty-cycled power (Pi woken per sample) | §7 | HO4, HW5 data | power controller |
| P2-16 | Automated controls + decontamination flush | §7 | P2-2 | pumps, reservoirs, valves |
| P2-17 | Firmware updates of the Due from the Pi | §7 | HO4 | — |
| P2-18 | Jetson migration + sequencing | I4 | — | Jetson |

---

## Prompts

### P2-1: Firmware restructure (no behavior change)

```text
Read CLAUDE.md, docs/COMMANDS.md, docs/ROADMAP.md §6. Background: docs/archive/ROADMAP_v1.md §5 S1.1.
Pure restructuring, ZERO behavior change (same commands, prints, timing). Bugs found go into
docs/BUGS_FOUND.md, not fixed.
1. Split src/Tests.cpp into src/Service/ (console.cpp, tests_<subsystem>.cpp,
   calibration_<subsystem>.cpp, service_io.cpp).
2. Replace the if-chain of the console with a command table {name, is_prefix, help, handler};
   the prefix-command ordering quirks (slot0 vs slotN, clocklist vs clockMM) behave as before.
3. include/Hardware.h replaces the copy-pasted extern blocks.
4. Slot-count independence everywhere (F16): one manifold-geometry block; golden values
   unchanged; tests with 16 and 24 positions.
5. docs/DEAD_CODE_AUDIT.md: unused functions/files/globals, large commented blocks, with grep
   evidence and a recommendation (keep / delete / ask me). Delete nothing.
6. Remove the legacy text protocol (LEGACY_PROTOCOL_ENABLED) and computer_interface/ -> legacy/
   only after I confirm.
Native test: the command table contains every command of docs/COMMANDS.md.
Hardware checklist: 10 representative commands + the panel modes compared against the baseline.
```

### P2-2: Runtime parameters

```text
Read CLAUDE.md, docs/PROTOCOL.md, host/. Background: docs/archive/ROADMAP_v1.md §5 S2.1 (why not
reflash from the Pi).
1. A parameter registry in src/Core/: name, type, unit, default (Settings.h), valid range,
   editable-only-in-IDLE, a "calibration" tag. It includes PRESSURE_CAP (0.5-3.0) and the margin,
   the volume cap, the flow expectations, the purge parameters, the timeouts,
   LINEAR_ACT_LOCKED_STEPS and the overdrive, the flow calibration factors, the pressure zero
   offsets, the shield times, the manifold angle offsets, the clog thresholds. Code reads the
   registry.
2. Console `param list|get|set|dump` + a params hash in HELLO/STATUS. Protocol GET_PARAMS /
   SET_PARAMS (update PROTOCOL.md + vectors).
3. Every calibration ends with a structured result {param: value} + "apply now?"; values live
   in RAM and are marked unsaved until the host stores them.
4. The host stores versioned calibration sets and pushes them at every connect (HELLO says
   "params: defaults|pushed"); a mismatch blocks missions.
5. tools/export_calibration_header.py writes include/Calibration_defaults.h from the host store,
   for a human to review and commit.
Tests: range validation, types, hash stability, the derived target follows the cap, a calibration
result updates exactly its tagged params; host: store, push on connect, mismatch blocks a mission.
```

### P2-3: Hydraulic sensing: 2 pressures + 2 flows, phase-aware

```text
Read CLAUDE.md, docs/PROTOCOL.md (the reserved p_out/q_out/dp/r_filter/clog_index fields), the FW3
pressure code, Flow_sensor.cpp, the FW8 phase/metrics code, the pilot results in data/pilots/.
Background (read it): docs/archive/ROADMAP_v1.md §1.6 (roles, air phases, the phase table) and
§5 S3.1.
Installed: P_in <model/CS>, P_out <ABPDANV060PGSA3, CS D45>, Q_in <model/pin/factor>, Q_out
<model/pin/factor>. The layout: <Q_in -> pump -> P_in -> manifold -> filter -> P_out -> Q_out -> outlet>.
1. A channel table (named channels P_in, P_out, Q_in, Q_out; conversion, calibration, range,
   plausibility, status OK/DEGRADED/ABSENT). No code says "pressure2" any more.
2. Roles: P_in = safety + PID; the cap acts on max(P_in, P_out); P_out = the clog measurement
   (a failure -> warning + flag, the fallback P_in/Q flagged, never a stop); flows: fused when
   both agree within FLOW_AGREE_TOL, flagged when not, one dead -> use the other, flagged.
3. Phase-aware exactly as the v1 §1.6 table (volumes only in WATER, dP/R "undefined" below
   Q_min/dP_min, a blanking window, the cap always on).
4. Derived: dP, R_filter = dP/Q, the clog index (R end / R start), low-pass filtered; fill the
   reserved RUN_CYCLE fields + quality flags.
5. Calibration: an automatic zero before every sample (pump off, vented); calibrate_pressure_pair
   (outlet closed, both sensors at ~2 bar -> a relative gain so dP = 0); calibrate_flow per meter;
   flow_crosscheck (V_in/V_out on a leak-free run).
6. Console `hydro`: live values + channel status.
Native tests: conversions, the role truth table, the cap on max(), fusion (agree/disagree/dead),
dP/R with Q ~ 0, the clog index, a synthetic full cycle where air adds no volume and no clog.
Hardware checklist: as v1 S3.1 (clean filter, restrictors before/after the filter, unplug P_out
-> warning, unplug P_in -> the pump stops, air adds no volume).
```

### P2-4: 10 Hz trace telemetry and analysis tools

```text
Read CLAUDE.md, docs/ROADMAP_LATER.md P2-5. Background: docs/archive/ROADMAP_v1.md §5 S3.2.
No control changes.
1. A 10 Hz trace mode over the Native USB port: t, phase, P_in, P_out, dP, Q_in, Q_out, Q_fused,
   V, R_filter, pump command, PID P/I/D terms, setpoint.
2. Console/protocol `pump_open_loop <power%> <s>` (the cap stays ACTIVE; refuse without P_in) and
   `pump_step_series` (20/40/60/80 %, 20 s each).
3. tools/record_trace.py -> data/pump_traces/<date>_<run>.csv + a JSON sidecar (filter, water,
   restrictor, notes, firmware hash, params).
4. tools/analyze_trace.py (max P_in, overshoot, margin to the cap, oscillation, settle time, plant
   gain K per step, R_filter vs volume, flow agreement, end reason, plots) and
   tools/compare_runs.py.
5. pytest on synthetic traces with known answers.
```

### P2-5: Pump measurement campaign (people)

Use the procedure in [archive/ROADMAP_v1.md §6](archive/ROADMAP_v1.md): the preparation, runs R1a–R4, the results table, and the decision tree that picks variant A, B or C. The one change: use real **lake water** from the pilots as well as the silty-water recipe, because that is the clogging that matters. Record the results in `docs/PUMP_RESULTS.md`.

### P2-6: Pump control variants and full end conditions

```text
Read CLAUDE.md, docs/PUMP_RESULTS.md (my results and the variant chosen: <A|B|C>), data/pump_traces/,
the FW8 clog rule. Background: docs/archive/ROADMAP_v1.md §5 S3.4 (common part + variants A/B/C).
1. Replace FW8's minimal clog rule with the full one: Q_fused < CLOG_Q_MIN for CLOG_T_S at
   P_in >= target - margin, OR R_filter > CLOG_R_MAX, OR clog index > CLOG_INDEX_MAX (the fallback
   P_in/Q, flagged, if P_out is degraded). Thresholds from my results.
2. SAMPLE_MIN_EXPECTED_FLOW from the measured 2 L runs (timeout ~1.5x).
3. A fixed, measured 10 Hz control period.
4. A plant model fitted to the traces (pump curve + R(V)) in tools/ and test/.
5. The variant: A = keep the gains; B = anti-windup + an output ramp limit, tuned in simulation;
   C = gain scheduling blended continuously by the filtered R_filter between the CLEAN and CLOGGED
   gain sets, plus B's protections, bumpless (no output jump > 5 %).
Native tests on the simulated plant from clean to fully clogged: never above the cap, overshoot <
0.2 bar, no sustained oscillation, each end condition fires.
Hardware checklist: repeat runs R2-R4 and compare with compare_runs.py.
```

### P2-7: Leak, dead-sensor and pressure-signature detection

```text
Read CLAUDE.md, the P2-3 channel code. Background: docs/archive/ROADMAP_v1.md §1.6 and §5 S3.5.
WATER phase only, after a settling window, with params:
1. Leak: V_in - V_out > max(abs_tol, rel_tol x V_in) -> FAULT LEAK.
2. A dead flowmeter vs a leak (one meter at exactly 0 while the other counts with normal
   pressures = a dead meter, a warning). The full truth table in docs/DIAGNOSTICS.md, implemented
   in src/Core/.
3. Pressure signatures: a sudden P_out drop with P_in stable -> a leak between them; P_out ~ P_in
   -> filter torn or missing (slot FAILED); P_out rising -> a blocked outlet; P_in stuck while the
   pump command changes -> SENSOR_DEAD (fault).
4. LEAK_TEST{slot}: lock, pressurize, stop the pump, measure the P_in decay over T s; enable it in
   the protocol (it was not_available).
5. Native tests: leak ramp, sudden leak, each dead meter, each signature, and a normal full cycle
   with air phases -> no false positive.
```

### P2-8: Environmental sensors

```text
Read CLAUDE.md, docs/PROTOCOL.md (sensors[]), the FW9 sensor code. Background: docs/archive/
ROADMAP_v1.md §1.9 (the tiers) and §8.5 (the sensor pre-selection and pre-order checks).
Installed: <compartment temperature/humidity SHT4x on I2C | turbidity on A5 via a divider in a
flow-through, light-tight housing | ...>.
1. A Sensor interface (read, name, unit, plausibility, status) + a driver table; a missing sensor
   never blocks sampling.
2. Per-sample stats over WATER in sensors[]; the compartment temperature logged between samples.
3. Turbidity as a relative index (not NTU) unless calibrated.
4. HELLO capability flags; the host export gets the columns with no code change.
Native tests: conversions, plausibility, absent-sensor behaviour.
```

### P2-9: Host API

```text
Read CLAUDE.md, host/. Background: docs/archive/ROADMAP_v1.md §5 H6.
FastAPI: /status; /mission (+ load/start/pause/resume/abort); /plan (CRUD + CSV import); /slots
(+ disable/enable with a reason); /samples + export CSV/JSON; /faults (+ clear, recover); POST
/stop (always allowed, highest priority); /alerts (+ ack); /logs; WebSocket /ws (telemetry,
events, alerts). Token auth (operator + read-only). The OpenAPI descriptions are the interface
spec. Tests with httpx against FakeController, including STOP during RUN_CYCLE and permissions.
```

### P2-10: Web app

```text
Read CLAUDE.md, the host API. Background: docs/archive/ROADMAP_v1.md §5 H7.
A static PWA served by FastAPI (plain HTML/JS, mobile-first): a dashboard (mission state, next
event, a big STOP always visible, live P_in and flow, alerts with ack, a "Recovery required"
banner with the RECOVER flow), a slots grid drawn from GET_INFO, plan editing, samples + export,
alerts history, settings (params, calibration history, a hidden service page). A reconnecting
WebSocket. Smoke tests.
```

### P2-11: Full notifications

```text
Read CLAUDE.md, host/cowas/alerts (HO5). Background: docs/archive/ROADMAP_v1.md §1.10 and §5 H8.
Add Web Push (VAPID) for the PWA, per-user channel configuration, SMS for critical alerts through
the TOFU modem, and a "send a test alert" endpoint. Tests: delivery failure + retry, per-user
routing.
```

### P2-12: Metadata standards and merge

```text
Read CLAUDE.md, host/ export code, the lab's feedback on the pilot CSVs.
Background: docs/archive/ROADMAP_v1.md §1.9 and §5 H3.
1. docs/METADATA.md maps every export column to FAIRe and MIxS (water) terms. VERIFY the exact
   current term names from the official sources and flag uncertain ones instead of inventing them.
2. A JSON export alongside the CSV; an optional per-sample trace file referenced by sample_id.
3. `cowas export merge a.csv b.csv -o all.csv`: checks and upgrades schema versions, builds the
   union of columns, removes identical duplicates, reports conflicts.
4. Site weather covariates (air temperature, recent rainfall) from a public weather API.
Tests: schema round trip, merge with different sensor columns, duplicates and conflicts.
```

### P2-13: Commissioning workflow

```text
Read CLAUDE.md, host/. Background: docs/archive/ROADMAP_v1.md §5 H5.
A mission state machine IDLE -> COMMISSIONING -> CONFIGURED -> RUNNING <-> PAUSED -> COMPLETED |
ABORTED. RUNNING requires: commissioning passed, a synced clock, a valid plan, enough READY slots,
params pushed with a matching hash, no recovery pending. A commissioning checklist recorded in
the DB (link and firmware version, params, manifold check, actuator home + lock/unlock, flow
check, pressure zero, LEAK_TEST, a dry purge, an equipment blank). Tests: the full workflow in
simulation, including a cold boot in every state.
```

### P2-14: Spool / depth reintegration

```text
Read CLAUDE.md, docs/COMMANDS.md (spool entries), src/Hardware/Motor.cpp, Step_functions.cpp
(step_dive, step_rewind). Background: docs/archive/ROADMAP_v1.md §5 I3 and §8.5 (the depth sensor
row: 4-20 mA for long cables). Hardware changes: <describe the repair>.
DIVE{depth_cm}/REWIND as protocol procedures: endstop interrupts reviewed (no prints inside ISRs;
the ISR only sets flags), a max-depth param, timeouts, cable-length accounting, supervisor
integration, a longer purge for the longer hose (FW4's formula with the deployed length). The
depth goes into RUN_CYCLE and the record (planned vs measured, if a depth sensor exists). The spool
capability flag. Native tests for the depth/encoder math. A careful hardware checklist (no water
first, low speed, a hand on the E-stop).
```

### P2-15: Duty-cycled power

```text
Read CLAUDE.md, docs/POWER.md (the HW5 and VA3 measurements), host/, docs/DEPLOY_PI.md.
Goal: cut the always-on load for multi-week battery deployments. Compare options with numbers:
(a) a Pi power/RTC-wake HAT that powers the Pi only around each sample; (b) the Due (or a small
always-on MCU with an RTC) switching the Pi's supply; (c) a bigger battery and no change.
For the chosen option: the host writes its next wake time; the firmware or the HAT wakes it N min
early; a missed wake is detected and alerted; a cold boot resume always works.
Tests: the scheduler computes wake times with an injectable clock; a missed wake; clock drift.
```

### P2-16: Automated controls and decontamination flush

```text
Read CLAUDE.md, docs/ROADMAP.md §3.7, the LA4 results. Background: docs/archive/ROADMAP_v1.md §7.
Hardware: <negative-control reservoir + pump + valve | positive-control | bleach flush reservoir +
valve>. Implement NEG_CONTROL/POS_CONTROL{slot} (they were not_available), an optional
decontamination flush step between samples (bleach, then a rinse with enough volume to reach
< the residual threshold from LA4), plan options on the host, and the metadata roles.
Tests: plan validation, procedure ordering, contamination-safe routing (a positive control can
never share a path with a sample without a flush).
```

### P2-17: Firmware updates of the Due from the Pi

```text
Background: docs/archive/ROADMAP_v1.md §7 (row "Firmware updates"). bossac on the Pi; binaries
built and checksummed in CI (never compiled on the Pi); only when IDLE with no mission and the
O-ring state known; rollback to the previous binary; a post-flash HELLO check of the git hash.
```

### P2-18: Jetson migration

The host is pure Python with adapters, so the migration is a deployment task: the same package, a Jetson-specific adapters folder, and the sequencing integration as its own project.

---

## Prepared for, not built yet

| Item | What is prepared | What's missing |
|---|---|---|
| Negative / positive controls | Roles in the data model, reserved commands | Hardware + P2-16 |
| More than 14 slots | Geometry-driven code (FW0/FW2), host sized from GET_INFO | New mechanics + FILL_ORDER |
| Environmental sensors | sensors[] in the protocol and the export | Drivers (P2-8) |
| GNSS | Lat/lon fields in the deployment | HO5 (TOFU modem) or a u-blox module on the Pi |
| O-ring contact confirmation | `ORING_CONTACT_BUTTON_INSTALLED` hook | A contact sensor |
| Better pressure resolution | Channel table (P2-3) | A 16-bit I²C ADC, only if needed |
| MQTT / fleet | API-first design | Only if there are many devices |
| Stepper driver upgrade (TMC2209: quiet, stall detection) | — | Only if HW7 shows lock problems |

For sensor pre-selection and the checks to do before ordering (conductivity isolation, depth over long cables, I²C addresses), see [archive/ROADMAP_v1.md §8.5](archive/ROADMAP_v1.md).
