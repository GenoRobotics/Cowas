# CoWaS roadmap, stage 3: full system (P2)

Work after the first lake data. Every step starts **after Pilot A**, some after Pilot B. The rules are in [stage 2 §6](stage2-pilots.md). One step = one branch = one PR, with unit tests and a hardware check. The old step prompts are in [the archive](../archive/roadmap-stage3-prompts-2026-10-23.md); the useful ideas are below.

**Order.** Choose it from the pilot results. Default:
1. Understand clogging and the pump: P2-3 → P2-4 → P2-5 → P2-6.
2. Runtime parameters: P2-2.
3. Remote operation: P2-9 → P2-10.
4. The rest, by need. Re-plan after the Pilot A debrief.

| ID | Step | Needs | Hardware |
|---|---|---|---|
| P2-1 | Firmware restructure and dead-code audit | FW8 | — |
| P2-2 | Runtime parameters: calibration without reflashing | FW7, HO3 | — |
| P2-3 | Two pressures and two flows, phase-aware | FW8 | P_out, Q_out |
| P2-4 | 10 Hz trace telemetry and analysis tools | P2-3 | — |
| P2-5 | Pump measurement campaign (with people) | P2-4 | Test bench |
| P2-6 | Pump control variants and full end conditions | P2-5 | — |
| P2-7 | Leak, dead-sensor and pressure-signature detection | P2-3 | — |
| P2-8 | Environmental sensors | FW9 | Sensors |
| P2-9 | Host API | HO5 | — |
| P2-10 | Web app | P2-9 | — |
| P2-11 | Full notifications | P2-9 | — |
| P2-12 | Metadata standards and merged exports | HO2, lab feedback | — |
| P2-13 | Commissioning workflow | P2-2 | — |
| P2-14 | Spool and depth | Spool repaired | Spool, depth sensor |
| P2-15 | Duty-cycled power | HO4, HW5 data | Power controller |
| P2-16 | Automated controls and decontamination flush | P2-2 | Pumps, reservoirs, valves |
| P2-17 | Firmware updates of the Due from the Pi | HO4 | — |
| P2-18 | Jetson migration | — | Jetson |
| P2-19 | Sample records kept on the device: an SD card on the Due, or a slave microcontroller, so a `run_sample` without the PC log still saves its metadata (today it only warns; the PC session log saves it, `tools/sample_slots.py`) | P2-9 or a decision on the storage | SD card or slave MCU |

---

## Steps

**P2-1 · Restructure, no behaviour change**
- Split `src/Tests.cpp` into service modules. Replace the console if-chain with a command table. One hardware header in place of the copied `extern` blocks.
- Slot-count independence everywhere (F16), tested with 16 and 24 positions.
- A dead-code audit with evidence for each item (keep, delete, or ask). Delete nothing without approval. Remove the legacy text protocol only after confirmation.
- Done when: every command in [commands.md](../software/commands.md) is in the table; 10 representative commands and the panel modes match the baseline.

**P2-2 · Runtime parameters**
- A parameter registry in `src/Core/`: name, unit, default, valid range, "only when idle", calibration tag. The cap, the margins, the purge and timeouts, the lock steps, the flow factors, the pressure zeros, the shield times, the clog thresholds.
- Console `param list|get|set|dump`, a parameter hash in `HELLO`. Each calibration ends with a structured result and "apply now?". Values stay unsaved until the host stores them.
- The host keeps versioned calibration sets, pushes them on connect, and blocks a mission on a hash mismatch. A script writes a header for a human to review and commit.
- Done when: range checks and hash stability are tested; a calibration updates exactly its tagged parameters.

**P2-3 · Hydraulic sensing**
- Named channels: `P_in`, `P_out`, `Q_in`, `Q_out`, each with a conversion, a plausibility check and a status (OK, DEGRADED, ABSENT).
- Roles: `P_in` is the safety and control pressure. The cap acts on the higher of `P_in` and `P_out`. `P_out` gives the clog measurement; if it fails, warn and fall back to `P_in`/flow, flagged, never a stop. Two flows are fused when they agree; a dead one is flagged.
- Volumes count only in the water phase; pressure drop and filter resistance are "undefined" below a minimum flow. A clog index (end resistance ÷ start resistance) fills the reserved fields of the protocol.
- Calibration: an automatic zero before each sample, a pressure pair check with the outlet closed, a flow cross-check on a leak-free run.
- Done when: the role table and the fusion rules pass native tests. Bench: unplugging `P_out` gives a warning; unplugging `P_in` stops the pump; air adds no volume.

**P2-4 · Trace telemetry and analysis**
- A 10 Hz trace over the Native port: time, phase, pressures, flows, pump command, the PID terms and the setpoint. Dump it afterwards as `CSV|` lines, because the console runs at 9600 baud.
- Test commands: a fixed-power run (the cap stays active) and a step series (20, 40, 60, 80 %).
- `tools/record_trace.py` saves each run with a JSON sidecar (filter, water, notes, firmware hash, parameters). `tools/analyze_trace.py` gives maximum pressure, overshoot, margin to the cap, oscillation, settling time, gain per step. `tools/compare_tune.py` compares two settings on the same slots.
- Deferred from stage 1: a `pump_step` test (+10 % from a steady point). Include it.
- Done when: the analysis gives known answers on synthetic traces.

**P2-5 · Pump measurement campaign (with people)**
- Follow the procedure in [archive/roadmap-v1.md](../archive/roadmap-v1.md) §6: preparation, runs R1a to R4, a results table, and a decision tree that picks variant A, B or C.
- Change: use **real lake water** from the pilots, as well as the silty-water recipe. Clogging from lake water is what matters.
- Record results in a new file under `docs/` when the campaign runs.

**P2-6 · Pump control and end conditions**
- Lessons from stage 1: `MiniPID` applies `ki` and `kd` per update and `kp` without a bias. Gains of 200 / 100 / 100 turn the pulsation into a relay oscillation (F146). Rank settings by their worst case over several noise sequences. On the bench nothing moves below about 10 % of the DAC output, and the pressure rises steeply above about 12 % (F137, F141).
- Replace the minimal clog rule of FW8 with the full one: low flow at high pressure for a set time, or high filter resistance, or a high clog index, with the thresholds taken from P2-5.
- Three variants to compare on the simulated plant from clean to fully clogged, then on the bench: **A** keep the gains. **B** anti-windup and an output ramp limit. **C** gain scheduling between a clean and a clogged gain set, blended continuously, with no output jump above 5 %.
- Native tests on a fitted plant model: never above the cap, overshoot under 0.2 bar, no sustained oscillation, each end condition fires.

**P2-7 · Leak and dead-sensor detection**
- In the water phase only, after a settling window: an inlet–outlet pressure difference above a set tolerance is a leak.
- A dead flowmeter (zero while the other counts at normal pressure) is a warning. A sudden drop in `P_out` with `P_in` steady is a leak between the two; `P_out` equal to `P_in` is a torn or missing filter; a rising `P_out` is a blocked outlet; a stuck `P_in` while the pump command changes is a dead sensor.
- `LEAK_TEST` (lock, pressurise, stop the pump, watch the decay) becomes a protocol command.
- Done when: a normal full cycle with air phases gives no false positive; each signature is detected in a native test.

**P2-8 · Environmental sensors**
- A sensor interface (read, unit, plausibility, status). A missing sensor never blocks sampling.
- Compartment temperature and humidity between samples. Turbidity as a relative index, not NTU, unless calibrated.
- Done when: conversions and plausibility tests pass; the export gains the columns with no code change.

**P2-9 · Host API**
- HTTP and WebSocket: status, mission control, plan, slots, samples and export, faults and recovery, alerts, logs, live telemetry.
- `POST /stop` is always allowed and has the highest priority. Token access for an operator and a read-only viewer. The API description is the interface spec.
- Done when: tests against the simulator show a STOP during a cycle is honoured, and permissions hold.

**P2-10 · Web app**
- Mobile-first page served by the host: a dashboard with a large STOP, live pressure and flow, alerts with acknowledgement, a "recovery required" banner, a slot grid from `GET_INFO`, plan editing, samples and export, and a hidden service page.
- Done when: smoke tests pass on a phone-sized screen.

**P2-11 · Full notifications**
- Web push for the app, SMS for critical alerts through the modem, per-user routing, a test-alert button.
- Done when: a failed delivery is retried and each user gets only their routed alerts.

**P2-12 · Metadata and merged exports**
- Map every export column to FAIRe and MIxS terms. Check the current term names against the official sources, and flag any term that is uncertain instead of inventing it.
- A JSON export next to the CSV. `cowas export merge` joins files with different sensor columns, removes identical duplicates and reports conflicts.
- Site weather (air temperature, recent rain) from a public source as covariates.
- Done when: a round-trip and a merge with conflicting columns pass tests.

**P2-13 · Commissioning workflow**
- A mission state machine: idle → commissioning → configured → running ↔ paused → completed or aborted.
- Running needs: commissioning passed, a trusted clock, a valid plan, enough READY slots, parameters matching the pushed hash, no recovery pending.
- The commissioning checklist (firmware and link, parameters, manifold check, actuator home and lock test, flow check, pressure zero, leak test, a dry purge, an equipment blank) is stored in the database.
- Done when: a full simulated workflow passes, including a cold boot in every state.

**P2-14 · Spool and depth**
- Depth moves as protocol procedures, with the end-stop interrupts reviewed (interrupts only set flags), a maximum depth, timeouts, and cable-length accounting.
- The purge length follows the deployed hose. The depth goes into the record (planned and measured).
- Hardware check: no water first, low speed, a hand on the E-stop.

**P2-15 · Duty-cycled power**
- Compare with numbers: a Pi power/RTC-wake board that powers the Pi only around each sample; the Due (or a small always-on controller) switching the Pi's supply; or a bigger battery with no change.
- The host writes the next wake time. A missed wake is detected and alerted. A cold-boot resume always works.
- Done when: wake times are tested with a fake clock, including a missed wake and clock drift.

**P2-16 · Automated controls and decontamination**
- Negative and positive control reservoirs with their own pump and valve, and a decontamination flush (bleach, then enough rinse to reach the residual threshold from LA4) between samples.
- Done when: a positive control can never share a path with a sample without a flush (tested).

**P2-17 · Firmware updates from the Pi**
- Build and checksum the binaries in CI, never on the Pi. Flash only when idle, with no mission and a known lock state. Keep the previous binary for rollback. Check the git hash after flashing.

**P2-18 · Jetson migration**
- The host is plain Python with adapters, so the move is a deployment task: the same package, a Jetson adapters folder, and the sequencing integration as its own project.

---

## Prepared for, not built

| Item | Ready | Missing |
|---|---|---|
| Negative and positive controls | Roles in the data model, reserved commands | Hardware and P2-16 |
| Lock contact sensing | — (the FSR was removed: not accurate, temperature-sensitive, F179) | A sensor that is not temperature-sensitive |
| More than 14 slots | Geometry-driven code; host sized from the Due | New mechanics and `FILL_ORDER` |
| Environmental sensors | `sensors[]` in the protocol and export | Drivers (P2-8) |
| GNSS | Lat and lon fields in the deployment | A modem or GNSS module (HO5) |
| O-ring contact confirmation | `ORING_CONTACT_BUTTON_INSTALLED` hook | A contact sensor |
| Better pressure resolution | Channel table (P2-3) | A 16-bit I²C ADC, only if needed |
| Fleet (MQTT) | API-first design | Only with many devices |
| Quieter stepper driver (TMC2209) | — | Only if HW7 shows lock problems |

Sensor pre-selection and checks before ordering: [archive/roadmap-v1.md](../archive/roadmap-v1.md) §8.5.

---

## Notes for the next machine version

Not for the current machine. What the prototype taught us, for a redesign (bench, 2026-10-15, slot 11, new Sterivex):

- At 10 % of the DAC, nothing moves (0.01–0.15 bar). At 11–12 %, the pressure reaches 1.5–2 bar. At 17 %, 1.5–2.4 bar. The useful band is thin: 1 % of control ≈ 0.2 bar (F137).
- The DAC was 8-bit, whole percent (F142). The reading pulses ±0.25 bar every 2–3 s at constant power (F141). Software added a 12-bit output, a floor, a rate limit and a low-pass (F139, F140).
- **For the redesign:** a pump and driver with controllable output, close to linear from near zero (its own speed or flow control, or control on calibrated flow). A pulsation damper after the pump. At least 10 bits over the useful range. Check the first-stroke pressure before buying. Never command below the pump's real minimum while it runs.
