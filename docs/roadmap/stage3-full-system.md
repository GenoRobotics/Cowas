# CoWaS roadmap, stage 3: the V2 machine (new design)

**Goal:** design and build the next CoWaS from what the prototype taught us: a machine that holds its alignment, doses and measures what it pumps, survives the field by design, and can be deployed for weeks to months.

**Scope rule (user, 2026-11-12).** Stage 2 holds every upgrade of the current machine, in software or hardware. **Stage 3 holds what needs a new design:** a new frame, manifold, fluidics, spool, electronics or enclosure, and the software that only makes sense on them. The rules of [stage 2 §6](stage2-pilots.md#6-rules-every-step-follows) apply here too. The version before this split is in [the archive](../archive/roadmap-stage3-2026-11-12.md); the mapping of the old P2-n steps is in §5.

**When.** V2-0 (requirements) can start at any time and should start early: it is a document, and it is the best way for a new person to learn the machine. The design steps start once the stage 2 P0 hardware is done, so the bench and field data they need exist. Building V2 waits for the Pilot B debrief.

**Order (default).**
1. V2-0 requirements and lessons (now, in parallel with stage 2).
2. V2-2 fluidics and V2-1 frame and manifold: the two biggest limits of the prototype.
3. V2-4 electronics and power, V2-3 spool and depth.
4. V2-5 enclosure and deployment, V2-6 compute and data.
5. Build, then the V2 bench and field validation (V2-7).

---

## 1. Steps

| ID | Step | Needs | Main lessons from the prototype |
|---|---|---|---|
| V2-0 | Requirements, lessons and architecture | stage 2 field data | All of §2 |
| V2-1 | Frame and manifold | V2-0 | F164, F186, F217, F187, F216, F144, F162, F179 |
| V2-2 | Fluidics: pump, damper, sensing, reservoirs, controls | V2-0, A15 traces (stage 2, Opt) | F137, F141, F146, F249, F150, F254, F255 |
| V2-3 | Spool and depth | V2-0, B6 results | F240, F241, F246, F250, F251, F213 |
| V2-4 | Electronics, controller and power | V2-0, PW1 results | F257, F258, F215, F36, F125, F174, F32 |
| V2-5 | Enclosure and deployment | V2-0, Pilot B | F35, H10 (rain), biosecurity |
| V2-6 | Compute, data and fleet | stage 2 A16 | — |
| V2-7 | V2 validation | V2-1 … V2-5 built | — |

**V2-0 · Requirements, lessons and architecture**
- Write the V2 requirements from §2, the field reports and Pilot B: deployment length, samples per deployment, volume per filter and flow range, depth range, pressure range, power source, maintenance interval, cleaning, cost, and the sensor set (§2b).
- Take the intended uses into account (stage 2 §8.0; intentions, not plans): lakes with depth, river stations at a fixed interval for weeks, **several identical units** in an array (build and calibrate copies, one time base), and possibly caves (humidity near 100 %, no mains power, hard access, large volumes per filter). State for each which requirement it drives, and which ones V2 will not cover.
- Decide the architecture with numbers: how many slots, cartridge-based or fixed manifold, which pump class, which controller, which host.
- Done when: a requirements document under `docs/` reviewed by the project and the lab, with each requirement traced to a finding or a field observation.

**V2-1 · Frame and manifold**
- **Concept under study: moving filters, fixed inlet and outlet** (user, 2026-11-12). The filters move to a fixed inlet and outlet, instead of a manifold that rotates under a seal. This would remove the rotating seal and its alignment with the frame, the two main sources of leaks (F144, F162, F164, F217). **Needs a bench test** of a mock-up (seal at 1.2 × the cap, positioning repeatability, cross-contamination) before it is chosen over an improved rotating manifold.
- **Self-sealing valves** on the filters and every connection that is unplugged (filters, reservoirs, intake), so nothing leaks when a part is removed (user, 2026-11-12).
- A frame that holds the manifold axis square to the cartridge plates and the outlet without hand realignment (F164, F186, F217: about 1.15° drift was enough to leak).
- An absolute manifold reference that cannot slip: an index on the rotor, and a coupling that is locked by design (F187, F216).
- A seal tested to the cap with margin (F144, F162), a lock state that is sensed, not assumed (F179: the FSR failed; look for a sensor that is not temperature-sensitive), and the drain step before the unlock designed in (F205).
- Filter cartridges as part of the design (stage 2 B13 is the retrofit); more slots if V2-0 asks for them (the code is geometry-driven).
- Done when: 100 swaps and 1000 rotations with no realignment and no `cal` change beyond 1 count; no drop at the seal at 1.2 × the cap for 10 min.

**V2-2 · Fluidics**
- **Pump** with controllable output, close to linear from near zero (its own speed or flow control, or control on calibrated flow). At least 10 bits over the useful range. Check the first-stroke pressure before buying. Never command below the pump's real minimum (F137: nothing below 10 % of the DAC, 1 % ≈ 0.2 bar).
- **A pulsation damper** after the pump (F141: ±0.25 bar every 2–3 s; F146: the PID turns it into a relay oscillation; F249: up to 113 pauses in a sample).
- **Safety sensing: two pressures and two flows, at the inlet and the outlet of the filter path** (user, 2026-11-12; §2b). Both flowmeters accurate at low flow (about 10 to 1000 mL/min). Named channels with a status (OK, DEGRADED, ABSENT); the cap on the higher pressure; volumes only in the water phase (old P2-3).
  - **Flow difference → automatic leak detection.** In the water phase, after a settling window, `Q_in − Q_out` above a tolerance for a set time is a leak: the pump stops, end reason `leak`. The tolerance comes from the two sensors' accuracy, measured on a leak-free bench run.
  - **Pressure difference → clogging.** `P_in − P_out` against the flow gives the filter's resistance through the run: the clog end condition uses it, and every record reports the start and end resistance and the clog index.
  - **Cross-checks** (old P2-7): a dead flowmeter (zero while the other counts), `P_out` = `P_in` (a torn or missing filter), a rising `P_out` (a blocked outlet), a stuck sensor while the pump command changes. `LEAK_TEST` (pressurise, stop, watch the decay) as a protocol command.
- **Pump campaign and control** (old P2-5, P2-6): measure the candidate pumps on lake water and on a silty-water recipe ([archive/roadmap-v1.md](../archive/roadmap-v1.md) §6); compare control variants on a fitted plant model (never above the cap, overshoot under 0.2 bar, no sustained oscillation); the full clog end condition from the data.
- **DNA Shield line designed for a measured dose** from the start (stage 2 B7 is the retrofit), with a prime and no residue in the common path (F111).
- **Pluggable reservoirs** (bleach, ethanol, DNA-free water, waste) that plug on and off without reconnecting tubing by hand (user, field 2026-11-06). **Controls and decontamination** (old P2-16): negative and positive control reservoirs with their own path, a decontamination flush between samples down to the C3 threshold; a positive control can never share a path with a sample without a flush (tested).
- A relief valve above the cap, sized against the weakest part's rating (F143).
- Done when: on a clogging filter the pressure holds within ±0.2 bar of the target with no cap pause; volumes within 3 % of the weighed water from 20 to 1000 mL/min; a calibrated leak of 5 % of the flow (PLACEHOLDER) stops the pump within 30 s, and 10 leak-free runs give no false leak; a dose within 5 % of the target.

**V2-3 · Spool and depth**
- A spool module with no absorbent material, a level-wind that does not crack or jam, a top stop that cannot be mistaken for the origin (F246, F250, F251).
- **A depth sensor at the intake**, so the sample depth is measured, not inferred from the tube length (F240: the current pulls the tube at an angle).
- The drain path and the tube-length accounting designed in (F213); the purge length follows the deployed hose; the planned and measured depth in the record (old P2-14).
- Done when: 50 dives and roll ups over the full range with no stall; the measured depth within 0.2 m of a reference at 1, 5 and 20 m in a current.

**V2-4 · Electronics, controller and power**
- A custom board (or a shield redesign) with the stage 2 safety built in: actuator inputs with safe defaults, a heartbeat-held actuator rail, an E-stop, per-channel current limits, input protection, separate logic and motor rails, an isolated host link, supply monitoring.
- Choose the controller with numbers (the Due's limits: no non-volatile memory, 3.3 V-only I/O, a shared SPI bus, F36, F125). Non-volatile storage for the journal and the records on board.
- **Duty-cycled power** (old P2-15): compare a Pi power and wake board, the controller switching the Pi supply, or a bigger battery; a missed wake is detected and alerted. Solar if V2-0 asks for months.
- Done when: the stage 2 PW1 dropout test passes by design, with no added parts; the power budget meets the V2-0 deployment length with 50 % margin.

**V2-5 · Enclosure and deployment**
- An enclosure for weeks to months on a platform or a mooring: IP67 for the electronics, drainage, condensation control, shade, a mooring and a recovery plan.
- Biosecurity: every wetted part can be cleaned and dried before another water body (quagga mussels).
- Environmental sensors as plug-ins (old P2-8): the site and enclosure sensors of §2b (water temperature, conductivity, turbidity, water level, enclosure climate). A missing sensor never blocks sampling.

**V2-6 · Compute, data and fleet**
- The host as plain Python with adapters, so a move to another computer (old P2-18: the Jetson, and sequencing on board as its own project) is a deployment task.
- Fleet operation (MQTT or similar) only with several devices. GNSS for a moored device.

**V2-7 · V2 validation**
- A bench acceptance like stage 2 C6, then a supervised field run, then an unattended deployment at least as long as Pilot B, with manual comparison samples and blanks.

---

## 2. What the prototype taught us (inputs to V2-0)

| Area | Lesson | Source |
|---|---|---|
| Pump | Nothing moves below 10 % of the DAC; at 11–12 % the pressure reaches 1.5–2 bar; 1 % of control ≈ 0.2 bar. The useful band is thin | F137 |
| Pump | The pressure pulses ±0.25 bar every 2–3 s at constant power; a PID makes it a relay oscillation; up to 113 pauses at the cap margin in one sample | F141, F146, F249 |
| Pump | The DAC was 8-bit (fixed to 12-bit in software); a 4 s blocking soft start missed the cap | F142, F132, F139 |
| Flow | The turbine flowmeter reads almost nothing below 25 mL/min and 20–31 % low at 60–80 mL/min | F150, F254 |
| Shield | A timed micro pump gives an unknown dose that varies with clogging; the first run after a bottle change loses the tube volume | F255, F156, F81 |
| Seal | Droplets at the seal around 3 bar on a misaligned frame; 1.15° of misalignment is enough | F144, F162, F164 |
| Frame | The frame and the outlet go out of square and need hand realignment before a campaign | F164, F186, F217 |
| Manifold | The encoder coupling slipped (three slots, twice in a day); no independent index | F187, F191, F216 |
| Lock | Open-loop lock works but its state is RAM-only; the force sensor failed | F18, F179 |
| Spool | Wet wood swelled; glue and zip ties held only as a repair; the drum stops at the switch holder; the tube is pulled at an angle by the current | F246, F250, F251, F240 |
| Electronics | A frozen controller left the manifold motor driven until it tangled; the pump relay closes at reset; a shared SPI bus lets one sensor kill another | F257, F215, F234, F235 |
| Power | The platform supply has outages and generator starts | F257 |
| Operations | Cleaning between filter sets takes sampling time; reservoirs are refilled by reconnecting tubing by hand | field log §27, user 2026-11-06 |
| Operations | Operator input errors cost samples (a wrong slot list, a missing prefilter) | F244, F248 |
| What worked | The pressure cap, the end rules and records, the schedule, the dive-prime-rise spool cycle, the vent before the unlock, the concentric double O-ring | field and revision reports |

---

## 2b. Physical sensors for V2

What V2 should measure, why, and the sensor class to evaluate (no brand chosen). **Role:** *safety* = stops the machine (rule 4); *core* = needed for a valid sample record; *site* = describes the water sampled, never blocks sampling (rule 5); *optional* = if the requirements (V2-0) ask for it. Every channel has a conversion, a plausibility check and a status (OK, DEGRADED, ABSENT), as in V2-2.

| Measurement | Why (lesson or use) | Sensor class to evaluate | Role | Step |
|---|---|---|---|---|
| **Pressure at the inlet (`P_in`)**, before the filter | Cap and control; today the only pressure (F126, F234). Each of the two pressures checks the other | Media-isolated gauge sensor, 0–6 bar, digital, temperature-compensated, on I²C or its own bus | safety | V2-2 |
| **Pressure at the outlet (`P_out`)**, after the filter | The **pressure difference** `P_in − P_out` is the filter's resistance: clogging detected as it builds and reported per sample (clog index, end resistance ÷ start resistance); `P_out` = `P_in` means a torn or missing filter, a rising `P_out` a blocked outlet. The cap acts on the higher of the two (old P2-3, P2-7) | Same class as `P_in`, so the two compare without a conversion error | safety | V2-2 |
| **Flow at the inlet (`Q_in`)** | Sets the volume; must be accurate where the turbine fails: 20–31 % low at field flow and nothing below 25 mL/min (F150, F254) | Liquid flow sensor accurate from about 10 to 1000 mL/min (thermal or ultrasonic class), not a turbine | safety | V2-2 |
| **Flow at the outlet (`Q_out`)** | The **flow difference** `Q_in − Q_out` is **automatic leak detection**: water that enters and does not leave is leaking between the two (seal, fittings, filter housing). Also a second volume, so one dead flowmeter is flagged instead of trusted | Same class as `Q_in` | safety | V2-2 |
| Suction pressure at the pump inlet | Intake blocked or out of the water, air leak, lost prime (F247, F252) | Compound (vacuum and pressure) sensor | core | V2-2 |
| Air or water in the line | Counts volume only in the water phase; ends the emptying and the prime on what is in the tube, not on a pressure threshold (F155, F188, F193) | Ultrasonic or capacitive bubble detector at the filter inlet and on the spool line | core | V2-2, V2-3 |
| DNA Shield delivered | Measured dose (F255) | Low-flow liquid sensor, or a load cell under the bottle | core | V2-2 |
| Reservoir levels (shield, bleach, ethanol, DNA-free water, waste) | No silent empty bottle on an unattended run; pluggable reservoirs | Load cells or non-contact level sensors | core | V2-2 |
| Manifold position at the rotor | No coupling to slip (F187, F216) | Absolute encoder on the rotor shaft itself, plus an independent index (Hall or optical) at the purge slot | safety | V2-1 |
| O-ring lock position and seal contact | Lock state sensed, not RAM-only (F18, F60); the FSR failed (F179) | Limit switches or a linear encoder on the actuator; a load cell or a strain gauge for the contact force | safety | V2-1 |
| Motor current per channel | Stall and tangle (F257), a slack belt (F250) | Current sense per driver channel | safety | V2-4 |
| Supply voltage and current per rail, battery charge | Dropouts (F257, F258), the power budget | Rail monitors; a battery fuel gauge | safety (dips), core (budget) | V2-4 |
| Water inside the enclosure | Leak or flooding | Conductive probe at the lowest point | safety | V2-5 |
| Enclosure temperature, humidity and barometric pressure | Condensation (a cave is near 100 % humidity); the air pressure corrects the absolute depth and level sensors | Combined digital sensor | core | V2-5 |
| Tilt and shock | A unit knocked over, moved by a flood, or tampered with | Accelerometer (IMU) | optional | V2-5 |
| Intake depth | The tube is pulled at an angle by the current (F240); a measured, not inferred, depth | Absolute pressure sensor at the intake, corrected by the enclosure barometer | core (with a spool) | V2-3 |
| Spool line out and end positions | Position lost on a reset; wooden switch holder jammed (F241, F246) | Encoder on the drum plus sealed magnetic (reed or Hall) end switches | safety | V2-3 |
| Water temperature at the intake | Required metadata; eDNA decay depends on it | Waterproof digital probe | site | V2-5 |
| Conductivity | Identifies the water mass: lake layers, karst sources mixing, a rain event | Conductivity cell with temperature compensation | site | V2-5 |
| Turbidity | Predicts clogging; marks rain peaks; a relative index unless calibrated | Optical backscatter sensor | site | V2-5 |
| Water level (stage) at a river site | Discharge from a rating curve, so eDNA concentration becomes a flux (§8.0 of stage 2) | Vented or barometer-corrected pressure transducer | site (river) | V2-5 |
| Time and position | One time base across several units (sample times known to about a minute); position of a moored unit | GNSS with a pulse-per-second output, or network time | core (arrays) | V2-6 |
| Dissolved oxygen, pH | Context for the ecology; not needed for the sampling | A multiparameter sonde, shared with the site if one exists | optional | V2-5 |

**Rules for the sensor set.** A safety sensor that fails stops the machine; any other sensor that fails is flagged in the record and sampling goes on. Sensors go on buses that one faulty sensor cannot take down (F36, F234, F235). Each sensor has a calibration procedure and an interval in the commissioning checklist (stage 2 A17).

---

## 3. Prepared for, not built

| Item | Ready | Missing |
|---|---|---|
| Negative and positive controls | Roles in the data model, reserved commands | V2-2 reservoirs and paths |
| Lock contact sensing | `ORING_CONTACT_BUTTON_INSTALLED` hook | A sensor that is not temperature-sensitive (V2-1) |
| More than 14 slots | Geometry-driven code; host sized from the Due (`GET_INFO`) | New mechanics and `FILL_ORDER` (V2-1) |
| Environmental sensors | `sensors[]` in the protocol and export | Drivers (V2-5) |
| GNSS | Lat and lon fields in the deployment | A modem or GNSS module (V2-6) |
| `P_out`, `Q_out`, clog index | Reserved fields in protocol v1; pins in stage 2 §11.2 | V2-2 |
| Better pressure resolution | Channel table (V2-2) | A 16-bit ADC, only if needed |
| Fleet (MQTT) | API-first host design | Only with several devices |
| Quieter stepper driver (TMC2209) | — | Only if B10 shows lock problems |

Sensor pre-selection and checks before ordering: [archive/roadmap-v1.md](../archive/roadmap-v1.md) §8.5.

---

## 4. Software that belongs to V2

Most software is an upgrade of the current machine and is in stage 2 (A14 runtime parameters, A15 trace telemetry, A19 restructure, A16 API and web app, A17 commissioning and metadata, A18 updates). What stays here needs the V2 hardware:

- Two-pressure, two-flow sensing and the clog index (V2-2).
- Leak and dead-sensor signatures (V2-2).
- Pump control variants and the full clog end condition, from the pump campaign (V2-2).
- Measured depth and the depth-based dive (V2-3).
- Controls and decontamination sequencing (V2-2).
- Duty-cycled power and wake scheduling (V2-4).
- Environmental sensor drivers (V2-5) and the Jetson integration (V2-6).

---

## 5. Old step IDs (P2-n)

| Old | Now | Why |
|---|---|---|
| P2-1 Firmware restructure | Stage 2 A19 (Opt) | Software upgrade of the current machine |
| P2-2 Runtime parameters | Stage 2 A14 (Opt) | Same |
| P2-3 Two pressures and two flows | V2-2 | Needs new plumbing and sensors |
| P2-4 Trace telemetry and analysis | Stage 2 A15 (Opt) | Works on the current machine; feeds V2-2 |
| P2-5 Pump measurement campaign | V2-2 | Chooses the V2 pump |
| P2-6 Pump control variants and end conditions | V2-2 (stage 2 A13 tunes the current pump) | |
| P2-7 Leak and dead-sensor detection | V2-2 | Needs `P_out`, `Q_out` |
| P2-8 Environmental sensors | V2-5 (water temperature stays in stage 2 B12) | |
| P2-9 Host API, P2-10 Web app, P2-11 Notifications | Stage 2 A16 (Opt) | Software upgrade |
| P2-12 Metadata and merged exports, P2-13 Commissioning | Stage 2 A17 (Opt) | Software upgrade |
| P2-14 Spool and depth | Done in stage 1 (spool in field use); the redesign is V2-3; the permanent repair is stage 2 B6 | |
| P2-15 Duty-cycled power | V2-4 | |
| P2-16 Automated controls and decontamination | V2-2 | Needs reservoirs and paths |
| P2-17 Firmware updates from the Pi | Stage 2 A18 (Opt) | Software upgrade |
| P2-18 Jetson migration | V2-6 | |
| P2-19 Records kept on the device | Stage 2 B15 (Opt) | A small add-on to the current board |
