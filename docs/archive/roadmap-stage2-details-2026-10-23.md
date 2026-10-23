# Stage 2: detailed sections, kept on 2026-10-23

When [stage 2](../roadmap/stage2-pilots.md) was made concise on 2026-10-23, these sections were moved here unchanged in content. Use them when a step needs the full text.

Contents: §3.8 power table · §4 all findings F1–F36 · §7 step table with unit tests · §7a bench checks in full · §11 parts, pins and electrical notes in full.

---

## §3.8 Power estimates

| Load | Power | When | Energy |
|---|---|---|---|
| Pump | ≤ 12 W (24 V × ≤ 0.5 A, `POWER_PUMP` comment) | About 20–25 min per sample | About 5 Wh per sample |
| Stepper while locked (A4988) | **0 W.** The lead screw self-locks (confirmed), so the coils are off once locked (FW4) | — | About 0 (only lock and unlock moves) |
| Valves, micro pump | A few W | Minutes | Under 1 Wh per sample |
| Due and shield logic | About 1–1.5 W (**estimate**) | Always | 24–36 Wh per day |
| Pi CM4 and TOFU (+ LTE) | About 3–5 W (+1–2 W) (**estimate**) | Always, unless duty-cycled | 70–170 Wh per day |

- **Pilot A** (2 days, 14 samples, laptop or Pi): about 0.4–0.5 kWh. A 12 V 50 Ah LiFePO4 (about 640 Wh) is enough with margin.
- **Pilot B** (14 days): always on: about 1.5–2.5 kWh. With the Pi duty-cycled (woken before each sample): about 0.6–0.9 kWh. Decide in HW5.
- **Voltage:** a 24 V LiFePO4 pack reaches 29.2 V when charged. 24 V solenoid valves are often rated ±10 %, so 26.4 V maximum. Recommended: a 12 V LiFePO4 → regulated 24 V boost for the actuators, a regulated 5 V buck for the Pi (3 A or more). Power the Due from a regulated supply (Vin 7–12 V recommended, 6–16 V absolute), never from raw 24 V. Fuse every branch.

---

## §4 Findings F1–F36 (full text)

| # | Finding | Where | Severity | Step |
|---|---|---|---|---|
| F1 | Slot availability is RAM-only. After a reset, used filters look available | `Manifold.cpp` | **High** | HO2 (the host owns the inventory) |
| F2 | The pressure cap is not polled in button-panel pump mode | `main.cpp` | **High** | FW4 |
| F3 | The cap is 3.0 bar, and the setpoint `2` is hard-coded in 4+ places | `Settings.h`, `Step_functions.cpp` | **High** | FW4 |
| F4 | `step_DNA_shield()` uses `FILL_STERIVEX_TIME / 10.` (a test shortcut): 25 s instead of 250 s | `Step_functions.cpp` | **High** | FW4 (value from LA3) |
| F5 | `rotateMotor()` ignores the O-ring lock (slotN, clockMM, panel Man Slot) | `Manifold.cpp`, `Tests.cpp`, `main.cpp` | **High** | FW2, FW4 |
| F6 | `flow_sensor_small` calibrated, factor 77 (**closed 2026-10-23, user**) | `Flow_sensor.h` | closed | — |
| F7/F22 | **Safety and PID read a sensor that is not installed.** `read_pressure2()` (A2, a floating pin). The installed sensor is the SPI ABP (`pressure1`, CS D8), bypassed because of an "invalid SPI response" error | `main.cpp`, `Step_functions.cpp` | **Blocking** | FW3 |
| F8 | `main_program()` blocks for a whole sample. No request ids, no acknowledgements, no timeout. The depth is parsed by string slicing | `main.cpp`, `cowas_loop.py` | High | FW7, HO1 |
| F9 | The Pi code is Linux-only (ZMQ `ipc://`, `/dev/ttyACM0`) and desktop-only (PyQt5) | `computer_interface/` | — | HO1–HO3 |
| F10 | The console and the Pi share one port at 9600 baud | `main.cpp` | Medium | FW7 |
| F11 | `DEBUG_MODE_PRINT` is a `const bool`, so the `#if` blocks never compile | `Settings.h` | Low | FW6 |
| F12 | The `uno` env cannot build. `-I include/interfaces` is missing (corrected by F48) | `platformio.ini` | Low | FW0 |
| F13 | No automated tests. `Tests.cpp` is 1,878 lines | — | Maintainability | FW0; restructure P2 |
| F14 | `Pump::stop()` has `delay(500)`. Many blocking waits | `Pump.cpp` | Low | FW5 |
| F15 | `step_sampling()` hard-codes a 3-minute runtime (too short for 2 L) and a 100 mL volume | `Step_functions.cpp` | **High** | FW4 (fixed in stage 1, S1-C) |
| F16 | Hard-coded slot counts (`i < 14`, `% 15`, `FILL_ORDER[14]`) | many | Medium | FW2 (new code), P2 (the rest) |
| F17 | With the sensor out, every pumping test refuses to run | `Pump.cpp` | Blocking for tests | FW2 |
| F18 | The O-ring lock state is RAM-only (`false` at boot). After a reboot while locked, the firmware would rotate while locked | `Oring_lock.cpp` | **High** | FW2 (fixed: tri-state) |
| F19 | The analog pressure path (A2) has a hard-coded zero and a 10-bit ADC | `Pressure_sensor.cpp` | Low | retired in FW3 |
| F20 | The comment says "all pins are claimed". About 17 digital and 8 analog pins are free in the code. The real limit is the PCB shield | `Settings.h` | Low | FW0, HW1 |
| F21 | ABP 150 psi: ±1.5 % FS = ±0.16 bar, too coarse for a pressure-drop measurement with two sensors. The 060 psi version gives ±0.06 bar | Hardware | Medium | HW0 (order), P2 (ΔP) |
| F23 | `setup()` ends in `test_all_components()`, which blocks in the console menu. `main_program()` (the Pi link) runs only after someone types `stop`. After any reset in the field, the machine waits for a human | `main.cpp:219` | **Blocking for the field** | FW6, FW7 |
| F24 | The legacy Pi link treats the first line it receives as "done". The Due prints logs as soon as a sample starts, so the Pi believes the sample finished and can send the next command. The Due buffers it and runs it later. `sampleFunction<depth>` is sent without `\n`, so the Due waits for its 1 s serial timeout | `cowas_loop.py`, `main.cpp` | **High** | FW7, HO1 (legacy link retired) |
| F25 | The Due DAC does not output 0–3.3 V. It outputs about 0.55–2.75 V (1/6 to 5/6 of 3.3 V). `analogWrite(DAC1, 0)` still gives about 0.55 V. The pump stops only because of the enable relay (D42). The pump driver's input range is unknown: 100 % may not be full speed. The DAC pins are easily damaged by a short or a low-impedance load: buffer them (op-amp follower and series resistor) | `Pump.cpp`, hardware | **High** | HW1 |
| F26 | D13 wired at 3.3 V (**closed 2026-10-23, user**; not measured). The MW-FS-2.0 output swings to its supply voltage, so the 3.3 V wiring is what matters | `Settings.h`, hardware | closed | — |
| F27 | ABP conversion constants are wrong for both sensors. The code uses 7.63e-4 bar/count and a 1.25 bar offset, which assumes a **10.0 bar** full scale. The installed 150 psi part is 10.34 bar, so it reads about 3.4 % low. The planned 060 psi part (4.14 bar) would read **about 2.4 times too high** with these constants. Status bits `10` (stale data: polled faster than the sensor updates) are treated as a wiring error. This is a likely cause of "invalid SPI response". Stale data should mean "keep the last value", with a staleness timeout | `Trustability_ABP_Gage.cpp` | **Blocking** | FW3 (fixed for the installed part in S1-1) |
| F28 | The air push cannot empty a wet Sterivex membrane. Air does not cross a wetted 0.22 µm membrane below its bubble point (about 3.4 bar or more for PES; check the datasheet). The "empty the Sterivex" step only drains the housing down to the membrane. Its end condition (pressure < 0.8 bar) sits above the PID target (0.4 bar), so it is met almost at once. In practice it is a timed step of about 5 s. The residual water, which dilutes the shield, is unknown | `step_sampling()` | **High (data)** | LA3, FW4 |
| F29 | After the cycle, the O-ring is unlocked and the used Sterivex stays open on both sides for days. The shield may drain or evaporate. The rotor face passes over used filters during later rotations. Retention and cross-talk are unmeasured | `sample_process()` | **High (data)** | LA3, LA4 |
| F30 | The purge (100 mL) is smaller than the dead volume of a realistic intake. 5 m of 6 mm ID is 141 mL, plus the pump, sensors and tubing. Each sample would include hours-old water from the hose and residue from the previous sample | `Settings.h` | **High (data)** | FW4, LA4 |
| F31 | `Flow_sensor::begin()` takes the calibration factor as `uint8_t`. A calibrated 81.7 becomes 81 (up to about 1.2 % volume error). A factor of 256 or more wraps around | `Flow_sensor.cpp/.h` | Medium | FW4 |
| F32 | Power: the Due must not see raw 24 V or a 29 V LiFePO4 pack (§3.8). Some Due boards do not boot reliably on power-up without a reset press: test cold boots. The current power topology is not documented | hardware | **High (field)** | HW1, HW5 |
| F33 | 9600 baud limits telemetry (about one 100-character line per 0.1 s at best). Native USB removes the limit | `platformio.ini`, `C_output.cpp` | Medium | FW7 |
| F34 | The A4988 stays **enabled for the whole time the O-ring is locked** (holding current). That costs energy and heat. The lead screw self-locks (confirmed), so the coils can be disabled once locked | `Oring_lock.cpp` | Medium (battery) | FW4 |
| F35 | Field intake: no strainer. Debris, algae or quagga mussel larvae can jam the turbine flowmeter or the pump valves. The intake biofouls over weeks. The outlet must discharge away from the intake | hardware | **High (field)** | HW6 |
| F36 | SPI bus: the ABP (up to 800 kHz) and the AMT22 encoder (its own timing rules) share the bus. Long or unshielded cables to a sensor on the fluid line cause bad frames. Check the AMT22 MISO logic level against the Due's 3.3 V limit | wiring | Medium | HW1, FW3 |

---

## §7 Step table with unit tests (full)

| ID | Title | Track | Pri | Depends on | Hardware needed | Main unit tests | Wk | Done |
|---|---|---|---|---|---|---|---|---|
| FW0 | Baseline: CLAUDE.md, native test env, CI, pin map, command list | FW | P0 | — | — | Manifold math golden values | 1 | ☑ |
| FW1 | Protocol v1 spec (docs and vectors) | FW | P0 | FW0 | — | (vectors used later) | 1–2 | ☐ |
| FW2 | Mechanical test suite, lock tri-state, `TEST\|` logging | FW | P0 | FW0 | — | Slot iteration, stop decision, may-rotate, may-home | 2 | ☐ |
| FW3 | Pressure sensor: fix the ABP driver, switch safety and PID to `P_in`. Stage 1 S1-1 does the core, including the manometer calibration | FW | P0 | FW0, HW1, HW4 (for the checklist) | An ABP on the line. None installed as of FW0 (F50) | Count → bar (both ranges), status bits, plausibility | 3 | ☐ |
| FW4 | Stage 1 S1-2 does items 1, 5a/c/d, 6 and 7. Safety and sample-quality fixes: cap, auto-unlock, 2 L, timeout, purge = 3 × dead volume, shield time, flow factor | FW | P0 | FW2, FW3 | Calibrated `Q_in` | Pressure decision, interlock, timeout, purge volume | 4 | ☐ |
| FW4a | One brick per action: the test menu and the run loop call the same functions (`run_samples`). The menu adds logs; the run adds none. Spool-installed purge volume | FW | P0 | FW4 | — | Option rules (log on or off, pause on or off), purge volume with and without the spool | 4–5 | ☐ |
| FW5 | Supervisor `service()`, `wait_ms()`, watchdog | FW | P0 | FW4 | — | Supervisor with a fake clock and sensors | 5 | ☐ |
| FW6 | STOP, safe state, fault latch, RECOVER, boot self-check, lean modes | FW | P0 | FW5 | E-stop input optional | Latch, recovery order, modes table, boot paths | 5 | ☐ |
| FW7 | Protocol v1 in firmware on Native USB. Boot into host mode | FW | P0 | FW1, FW6 | Second USB cable | Vectors, parser fuzzing, dispatcher, id dedupe | 6 | ☐ |
| FW8 | Sample record (metrics and `end_reason`) and a minimal clog end condition | FW | P0 | FW7 | — | Metrics accumulator, clog rule, phase gating | 6 | ☐ |
| FW9 | Unattended hardening: E-stop input, leak sensor, water temperature, home switch | FW | P1 | FW8, HW7, HW8 | Sensors | Debounce, sensor plausibility, absent-sensor behaviour | 9–10 | ☐ |
| HO1 | Host skeleton: protocol client, transport, simulator, CLI, device logs | HO | P0 | FW1 | — | Vectors, client against simulator, reconnect, dedupe | 2–3 | ☐ |
| HO2 | Slots, deployments, sample records, CSV export | HO | P0 | HO1 | — | Slot transitions, persistence, export golden file | 3–4 | ☐ |
| HO3 | Mission runner, scheduler, cold-boot resume | HO | P0 | HO2 | — | Simulated-time scenarios | 4–5 | ☐ |
| HO4 | Deploy on the Pi (systemd, udev, clock, backups) | HO | P1 | HO3 | Pi and TOFU | Resume after power cut (integration) | 7–9 | ☐ |
| HO5 | Remote access and alerts lite (Tailscale, LTE, ntfy or Telegram, daily report) | HO | P1 | HO4 | SIM, antennas | Alert rules, dedup, repeat until ack | 9–11 | ☐ |
| LA2 | SOPs: filter loading and unloading, cleaning, labels, chain of custody. [filter-sop.md](../lab/filter-sop.md) drafted 2026-09-25 with the lab values as proposals | LA | P0 | LA1 | — | — | 2 | ☐ |
| LA3 | Shield volume, residual water, shield retention (weighing) | LA | P0 | FW4, LA1 | Scale 0.01 g | Analysis script tests | 4–5 | ☐ |
| LA4 | Carry-over and slot cross-talk (dye tracer, optional qPCR). The protocol document was dropped 2026-10-21 (never written) | LA | P0 | FW4, HW6 | Dye, spectrophotometer or phone photometry | Analysis script tests | 5–6 | ☐ |
| HW0 | Order everything (§11.1) | HW | P0 | — | — | — | 0–1 | ☐ |
| HW1 | Electrical audit → [pinout.md](../hardware/pinout.md), [power.md](../hardware/power.md) | HW | P0 | FW0 | Multimeter, scope if available | — | 1 | ☐ Part 1 done 2026-09-25 (power.md and checklist M1–M44). Part 2 waits for the measurements |
| HW2 | Relief valve, E-stop in the 24 V line, fuses | HW | P0 | HW1 | Parts | — | 2–3 | ☐ |
| HW3 | Flowmeter: level-safe input, calibration, real flow measurement | HW | P0 | HW1 | Cylinder, scale | — | 2 | ☐ |
| HW4 | Install and plumb the pressure sensor(s) | HW | P0 | HW0 | Sensors, clamps | — | 3 | ☐ |
| HW5 | Power: battery, regulated rails, measured budget, cold-boot test | HW | P0 | HW1 | Battery, DC-DCs, meter | — | 4 | ☐ |
| HW6 | Field fluidics and splash-proof housing for Pilot A | HW | P0 | HW3 | Strainer, hose, box | — | 5 | ☐ |
| HW7 | O-ring lock endurance (coils off while locked). "Unlocked" home switch (P1) | HW | P0/P1 | FW2 | Switch | — | 3 / 9 | ☐ |
| HW8 | Pilot B: enclosure, leak sensor, temperature probe, mounting | HW | P1 | HW6 | Parts | — | 8–12 | ☐ |
| HW9 | Manifold encoder coupling: mechanical lock (the encoder slipped three times in two days, F187, F191) | HW | P0 | HW6 | Tools | — | 2–3 | ☐ |
| VA1 | Bench acceptance campaign (go or no-go for Pilot A) | VA | P0 | FW8, HO3, LA3, LA4, HW2–HW6 | Full machine, battery | — | 7 | ☐ |
| VA2 | Pilot A | VA | P0 | VA1 | Site | — | 8 | ☐ |
| VA3 | 72 h soak on battery and Pilot B readiness | VA | P1 | All P1 | Full machine | — | 12–14 | ☐ |
| VA4 | Pilot B | VA | P1 | VA3 | Site | — | 14–16 | ☐ |

---

## §7a Bench checks in full

| Step | Check | Blocked by | Commands and pass criteria |
|---|---|---|---|
| FW2 | Sensor-mode `leak_hold` (PID water phase, PID air push) | Stage 1 §1 (R0–R3, the flash and the runs) | `actuator_home` → `slot5` → `leak_hold 100` → answer the leak question. Water phase ends on volume (about 100 mL on the flowmeter, factor 77, calibrated). No SAFETY line. `leak=no`. `TEST\|leak_hold\|5\|PASS` |
| FW2 | Sensor-mode `multi_sample` (the `load_slot` sequence per slot) | As above | `actuator_home` → `multi_sample auto count=3`. Three `TEST\|multi_sample\|<slot>\|PASS`. No SAFETY line. O-ring UNLOCKED at the end |
| FW2 | `full_cycle` (`step_purge` → `step_sampling` → `step_DNA_shield` → `step_empty`) | As above | `actuator_home` → `slot1` → `full_cycle 1 volume=100`. `TEST\|full_cycle\|1\|PASS`. `sampled_ml` about 100 (±20 % until HW3). O-ring UNLOCKED at the end. No SAFETY line |
| FW2 | `full_all` over three slots | As above | `full_all count=3 volume=100` (pauses between slots; `auto` for none). Three PASS. Stops at the first FAIL |
| FW2 / FW5 | The production path after boot: `sample1m` refuses while the lock is UNKNOWN | As above | Reset → `sample1m` (expect `REFUSED ... UNKNOWN`, nothing moves) → `actuator_home` → `sample1m` + START. The second completes. O-ring UNLOCKED at the end |
| FW3 / FW4 | Every pumping command runs under the cap (F201 fixed the three that did not poll; F202 `cal_DNA` and the purge pipes do not), as documented | The cap is live since 2026-10-12. The panel Pump mode, `calibrate_flow` and `fillContainerFunction` still do not poll it (F2, F41, F42) | Each command in [commands.md](../software/commands.md): refuses or runs as written. Do not use the three that do not poll the cap |
| HW1 | Electrical audit M1–M44 (supply, pump driver and DAC, Due power, flowmeter level, SPI levels, VNH5019 EN and DIAG, flyback diodes, 24 V currents, A4988 Vref, reachable pins) | A person at the machine with a multimeter (and a scope for M19). Pumping only with an open outlet (F50) | [power.md](../hardware/power.md) §4: one line per item, with commands. Pass: the "Acceptable" column of each item. Results into power.md, pinout.md and the HW1 entry of the log |
| HW3 | D13 peak check (F26) | A multimeter on MAX. The turbine spun by hand or with water | Peak ≤ 3.3 V under pulses. Deferred by the user 2026-10-12 |
| HW4 | The spare 150 psi ABP (ABPDANV150PGSA3): alive? | Only needed if a second sensor is wanted | (1) Find pin 1: datasheet Table 8 (6 pins, `1 2 3` / `6 5 4`; SPI: 1 GND, 2 Vsupply 3.3 V, 3 SS, 4 NC, 5 MISO, 6 SCLK; **not reverse-polarity protected**). Unplugged, multimeter in diode test: the pin that reads OL against all five others, both ways, is pin 4 (NC). Pin 1 is diagonally opposite. If no pin is fully open, stop. (2) Wire 1 → Due GND, 2 → Due **3.3 V** (never the 5 V pin), 3 → D8, 5 → MISO, 6 → SCK. (3) `pio device monitor`, `pressure_sensors`, the `Trustability` line for 30 s, blow gently into the port. Alive: steady about 0.0–0.1 bar at rest, a clear rise when blowing. A warm sensor is shorted: unplug. Record in the log (F50) |
| HW4 | Manifold alignment check: air flow, angle of maximum flow with V1 open (F131) | The pilot (user decision 2026-10-12) | Angle of maximum flow within about 1° of the table value. Otherwise `cal` and paste `purge_angle` |
| HO1 | The host against the real Due | FW7 (today's firmware speaks the legacy text protocol) | Due on the bench, both USB cables to the laptop, `cd host`, venv active. (1) `cowas ports`: the Native port (2341:003E) listed and auto-detected. The Programming port marked as such. (2) `cowas hello`, twice. (3) `cowas info`: 15 slots, `fill_order` as in `Settings.h`. (4) `cowas monitor --duration 120` while a `PURGE` runs: `tel` lines every 1 ± 0.2 s, step events, `log` lines in `data/logs/device.log`. (5) During `cowas run-cycle --slot 1 --volume 200` on a bucket, unplug the Native cable about 10 s, plug it back: reconnect, same `boot_id`, the result arrives, the command ran once. (6) `cowas logs -n 20` prints the Due's lines. Pass: 1: one port auto-detected, never the Programming port. 2: same `boot_id`, `proto` 1.x, `fw.git` = the flashed commit, and opening the port does **not** reset the Due (uptime keeps growing) |
| LA2 | SOP dry run: two people follow [filter-sop.md](../lab/filter-sop.md) §1–§3 and §6 with a printed copy, 14 Sterivex (bench filters are fine), tap water for the lake and DNA-free water | The values in [shield-protocol.md](../lab/shield-protocol.md) §8. The cap (now live). For the host steps, FW7 and HO2/HO3. Loading and unloading alone (§2, §3) can run earlier | §1 (faces only). §2 with every check. One `RUN_CYCLE`-equivalent per filled slot (or `sample_cycle_full` on the console before FW7, supervised). §6 once. §3. §5 filled with dummy data. §4.2 once. Time each section. Pass: loading 14 filters ≤ 30 min, unloading ≤ 20 min. **0** outlet caps left on. **0** drips at a luer thread. **0** label or slot mismatches. Every box and "Who" field filled. Each unclear step written down and fixed in the SOP |
| LA3 Part A | Shield micro pump calibration (density, rate, line volume, drain-back) | Needs this branch flashed, the Shield bottle, a 0.01 g scale, about 15 mL of Shield | [shield-protocol.md](../lab/shield-protocol.md) §2: A1 density (5 × 1 mL pipetted). A2 `shield_calibrate` (tube off the manifold, defaults 10/20/40 s × 3). A3 the same with `y` at "line empty". A4 one dose after ≥ 12 h with the line on the manifold. Pass: CV ≤ 5 % per duration, R² ≥ 0.995, \|offset\| < 0.1 mL. `line_ml` within ±20 % of π(ID/2)²L. A4 delivers ≥ 95 % of the calibrated dose. Results into shield-protocol §6 → `DNA_SHIELD_FILL_MS` (FW4 item 7), `PRIME_SHIELD_MS` (FW7 item 3) |
| LA3 Parts C–D | Residual water, Shield volume, retention (weighing campaign) | FW3 and HW4 (live cap), FW4 (rest locked, 2 L, emptying parameters and logs), FW2 `full_cycle`, a Shield-only console command (shield-protocol §7) | [shield-protocol.md](../lab/shield-protocol.md) §3: 12 filters (A-today, B-long, F-warm, S-room, H-handling). Day 0 and holds at 1, 3 and 7 days. Then `python tools/analyze_la3.py <csv> --thresholds tools/la3_thresholds.json`. Pass: the thresholds of shield-protocol §8 (B1–B5, still OPEN). Handling loss < 0.02 g per handling. F and S `water + shield` within 10 % of A |

---

## §11 Parts, pins and electrical notes (full)

### §11.1 Order now (HW0), full

| # | Part | Qty | Spec and notes | For |
|---|---|---|---|---|
| 1 | **Pressure sensors ABPDANV060PGSA3** | 3 | Same family as the 150 psi part (not installed as of FW0, F50). SPI, 3.3 V, same code once FW3 is done. 0–4.1 bar, ±0.06 bar. P_in and a spare now; P_out in P2. Barb clamps too. **Critical path: FW3's checklist and every pumping test wait for it.** **FW3 must be merged before the swap** (F27) | FW3 |
| 2 | **Mechanical pressure-relief valve** | 1 | Adjustable, about 2.5–3 bar, water-compatible, outlet to waste | HW2 |
| 3 | **E-stop** | 1 | Latching mushroom switch with 2 NC contacts: one DC-rated at 2 × the 24 V rail current (series cut), one for the Due input | HW2 |
| 4 | Relay (only if #3 cannot carry the DC load) | 1 | 24 V coil, DC-rated contacts, flyback diode | HW2 |
| 5 | Fuses and holders | per branch, plus spares | Pump, VNH5019, A4988, valves, main | HW2 |
| 6 | **Battery 12 V LiFePO4 ≥ 50 Ah** with BMS and charger | 1 | Pilot A. Pilot B sized after HW5 | HW5 |
| 7 | **DC-DC boost 12 → 24 V, regulated** | 1 + 1 | Current ≥ 1.5 × the measured peak (pump start, stepper, valves) | HW5 |
| 8 | DC-DC buck 5 V ≥ 3 A (Pi), plus the Due supply per HW1 | 1 + 1 | Low noise. Common ground | HW5 |
| 9 | Main switch, main fuse, DC energy meter (shunt) | 1 | The shunt meter measures Wh per cycle | HW5 |
| 10 | Rail-to-rail op-amp and resistors for a DAC buffer. Dividers 10 kΩ / 4.7 kΩ, 100 nF | kit | Only if HW1 confirms (F25, F26) | HW1 |
| 11 | Micro-USB data cables, short, good quality | 2 | Due Native port to laptop or Pi | FW7 |
| 12 | **Intake kit** | 1 | Strainer (200–500 µm mesh), weight, 10 m hose (same ID as today, food-grade), rope. Optional foot valve (helps priming; the purge handles retained water) | HW6 |
| 13 | Quick-disconnect fittings | 4 | Intake, outlet, purge | HW6 |
| 14 | Splash-proof box IP54+ and shade cover | 1 | Pilot A | HW6 |
| 15 | **Lab consumables** | — | Sterivex ≥ 40 (tests and pilot). DNA/RNA Shield (≥ 14 × target volume + about 100 mL for calibration). DNA-free water ≥ 10 L (about 6 L for 2 equipment blanks at 2 L plus their purges, the field blank, and DNA-free rinses before loading and after a deployment: [filter-sop.md](../lab/filter-sop.md) §4.2.6, §6.1; lab interface F2). New luer caps for both ends. Cryo labels (type: lab interface E2; Sterivex model: A3). Gloves, bleach, cooler and ice packs | LA2–LA4, VA2 |
| 16 | **Test bench** | — | Scale 0.01 g (≥ 200 g), 1 L measuring cylinder, bucket, tube clamp or needle valve, food dye (plus a conductivity meter or photometer if available) | LA3, LA4, HW3 |
| 17 | Spare Arduino Due | 1 | A bench board and a field spare | all |
| 18 | O-ring spare sets | 3 | | HW7 |

**Should (P1), order by week 6:** a Hall switch and magnet (O-ring "unlocked" home). An enclosure leak sensor. DS18B20 waterproof probes ×2 (+1). INA226 current monitors ×2. An IP65 enclosure, cable glands and desiccant. SIM and LTE antenna (outside any metal). An RTC battery for the CM4 carrier if absent. The Pilot B battery (after HW5).

**Before you order:** confirm the fitting threads and tube sizes (#1, #2, #12, #13). Measure the 24 V currents (#3, #5, #7).

### §11.2 Pin budget, full

The code uses about 31 digital pins, 3–4 analog inputs and 1 DAC. The real limit is **what the PCB shield brings out** (HW1 fills in [pinout.md](../hardware/pinout.md)).

| New item | Pin | Wiring | Pri |
|---|---|---|---|
| `P_in` (ABP, SPI) | SPI, CS **D8** (unchanged) | 3.3 V, GND, MISO, SCK. Read-only, no MOSI needed | P0 |
| `Q_in` (turbine) | **D13** if a pulse test passes with a 3.3 V-safe level; otherwise **D36** | Sensor at 3.3 V if its spec allows, or open-collector with pull-up to 3.3 V, or a divider | P0 |
| E-stop state | **D37** | NC contact to GND, `INPUT_PULLUP`. Pressed **or a broken wire** reads HIGH = stop | P0/P1 |
| O-ring "unlocked" home switch | **D39** | Same wiring as the E-stop | P1 |
| Enclosure leak sensor | **D41** (digital module) or **A6** (analog probe) | 3.3 V logic | P1 |
| Water temperature (DS18B20, 1-Wire) | **D43** | 4.7 kΩ pull-up to 3.3 V. Probes powered at 3.3 V | P1 |
| Battery voltage | **A0** | Divider 47 k / 10 k (14.6 V → 2.56 V) and 100 nF. Or the INA226 | P1 |
| Current monitors (INA226) | **SDA 20 / SCL 21** | The Due has on-board pull-ups on 20 and 21 | P1 |
| `P_out` (ABP, SPI) | SPI, CS **D45** | Same as P_in | P2 |
| `Q_out` | **D12** if the old `flow_sensor_big` wiring exists, else **D47** | As Q_in | P2 |
| Turbidity | **A5** | Divider to 3.3 V | P2 |
| Still spare | A1–A4, A7–A9, D4, D14–17, D49, D50–D53 | | |

### §11.3 Electrical rules, full

- **The Due is 3.3 V only and not 5 V tolerant.** Pull-ups to 3.3 V. Sensor outputs that swing to their supply need a divider or a 3.3 V supply. Check every module's output level before connecting it.
- **The DAC outputs about 0.55–2.75 V**, not 0–3.3 V, and it is fragile. Buffer it. Never rely on "0 %" to stop the pump: the relay does.
- **Never feed the Due raw 24 V**, or a charged LiFePO4 pack above 16 V.
- **One ground star point** for the power return. Keep motor currents out of the sensor grounds.
- **Flyback diodes** on every coil (valves, relays).
- **Short SPI and I²C cables** (under 30 cm). 1-Wire works over a few metres. For long runs use 4–20 mA or RS-485.
- **I²C addresses:** list them in [pinout.md](../hardware/pinout.md) before ordering an I²C device. The MS5837 is fixed at 0x76. INA226 is 0x40–0x4F by strapping. SHT4x is 0x44.
