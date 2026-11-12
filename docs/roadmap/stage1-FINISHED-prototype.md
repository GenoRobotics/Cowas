# CoWaS roadmap, stage 1: from the old prototype to the first lake pilot (done)

**Goal:** turn the prototype as it stood in August 2026 into a machine that an operator can run in the field, one command or one schedule per sample, with a record for every filter. Then take it to the field.

**Result (2026-11-12): done.** Two field campaigns (26 Oct – 6 Nov 2026) gave 66 filters from the CoWaS, including an overnight river series and two 24 h lake series on Lake Geneva. There was no seal leak, the pressure stayed under the cap, and no sample was lost to the sampling program. The LéXPLORE run is **Pilot A** (user, 2026-11-12). The field feedback is now the work of [stage 2](stage2-pilots.md).

**Stages**
- **Stage 1 = this file:** what was done from August to November 2026. Everything here is done; nothing is planned here.
- Stage 2 = [stage2-pilots.md](stage2-pilots.md): upgrades of the current machine, up to an unattended pilot (Pilot B).
- Stage 3 = [stage3-full-system.md](stage3-full-system.md): the V2 machine (new design).

This file used to be shortened each time a step was done. Since 2026-11-12 it lists the work done. Earlier versions are in the archive: [2026-10-15](../archive/roadmap-stage1-2026-10-15.md), [2026-10-23](../archive/roadmap-stage1-2026-10-23.md). The detail of every step is in [LOG.md](../logs/LOG.md); findings (F…) are in [findings.md](../logs/findings.md) and decisions (D…) in [decisions.md](../logs/decisions.md). The [robustness update report](../reports/robustness-update-report-2026-11.md) summarises it for readers outside the project (what changed and why, the field work, what comes next). The two earlier reports are in [the archive](../archive/reports/).

---

## 0. How to read this file

- §1 gives the six phases in one table. §2 compares the start and the end. §3 lists the steps of each phase. §4 covers the field campaigns: their goals and what they achieved, including Pilot A. §5 says where the leftover items went.
- Status: ☑ done, ◐ done in part (the rest is in stage 2), ✗ dropped or not met (with the reason).
- Each row cites the log entry, the findings and the decisions that hold the evidence. Dates are the dates of the log entry.

---

## 1. The phases in one view

| # | Phase | Dates | Outcome |
|---|---|---|---|
| 1 | Prototype hardware and the first tools | Aug – Sep 2026 | Motorised O-ring lock, manifold sealing parts, fill order, calibration commands, pressure cap in the pump |
| 2 | Foundations | 24 – 28 Sep | Tests and CI, pin map and command list, host protocol, mechanical test suite with the three-state lock, host with simulator, lab protocols, electrical audit (part 1) |
| 3 | Bench prototype (stage 1 proper) | 6 – 23 Oct | Pressure on a calibrated sensor, pump control on clogging filters, water through filters, end rules, records, the operator command `run_sample`, the session tool, the operator tutorial |
| 4 | Field preparation | 26 Oct – 2 Nov | 3 L purge at full power, timed schedule, slot book, ethanol cleaning, the depth spool enabled (dive, prime, rise, roll up) |
| 5 | Field campaigns | 26 Oct – 6 Nov | Bois Chamblard (river and lake), LéXPLORE (Lake Geneva, Pilot A); spool and schedule made robust on site |
| 6 | Analysis and handover | 6 – 12 Nov | Field logs, protocols, sample list, the incident of 6 Nov analysed, two reports, stage 2 and 3 reorganised |

---

## 2. Start and end

| | August 2026 | November 2026 |
|---|---|---|
| Firmware | 48 files, about 5,600 lines, no tests | 107 files, about 16,600 lines; pure logic in `src/Core/`, 221 unit tests in 22 suites, CI |
| Seal | Single O-ring, no motorised lock | Concentric double O-ring, motorised lock (stepper and lead screw) with a three-state lock (locked, unlocked, unknown) |
| Pressure | Read for thresholds only, at a clock that gave bad frames | Calibrated sensor, every frame checked; cap 3.6 bar, pause at 3.35, resume at 3.15; an implausible reading stops the pump |
| Pump | Fixed power, ended by timers | Regulated on pressure (10–60 % with water, 100 % for the purge), gentle start, pause by the relay |
| End of a sample | A container filled for a time | 1000 mL counted, or a defined early stop (`clogged`, `stalled`, `pressure`, `timeout`); the end reason recorded |
| Depth | Not in use | Spool to 40 m: dive, prime, rise to depth, roll up and empty for every sample |
| Operation | Commands one at a time | `run_sample`, a timed schedule, a slot book, a session tool that saves every session |
| Records | Console text | One JSON record per filter (volumes, end reason, pressures, step times, flags) |
| Host | None | A host program with protocol v1 and a simulator (364 tests; runs in simulation, the firmware side waits for stage 2 A10) |
| Documentation | None | Operator tutorial, field and manual protocols, lab protocols, pin map, 51 documented commands, findings and decisions registers, two reports |

---

## 3. Steps done

### 3.1 Phase 1: prototype hardware and the first tools (Aug – Sep 2026)

Source: [LOG, Prehistory](../logs/LOG.md#prehistory-linear-actuator-manifold-layout-and-calibration-tools-2026-09-before-fw0) and [archive/memo-prehistory.md](../archive/memo-prehistory.md).

| Step | What | Status | Refs |
|---|---|---|---|
| O-ring lock | NEMA17 and A4988 linear actuator (STEP/DIR/EN D9/D10/D11): loosen to rotate, tighten to seal. A DIR wire fault found and fixed; power wiring soldered | ☑ | F167, F171, F172 |
| Manifold sealing | O-rings sealing the PTFE rotor, metal inserts so no water passes through PLA, concentric double O-ring, a tighter sensor adapter | ☑ | F175–F178 |
| Force sensor | An FSR to detect the seal contact: added, then removed (temperature-sensitive, not accurate). The lock runs open-loop from a known reference | ✗ removed | F179 |
| Manifold layout | Purge port at the bottom, slots fill from the bottom up (a leak only reaches filled slots), `FILL_ORDER`, `NB_SLOT` fixed to 15 | ☑ | F168, F169 |
| Calibration commands | Encoder, actuator, flow, pressure, DNA pump | ☑ | F181 |
| Pressure cap in the pump | `CtrlPump`'s limit enforced in every pump path | ☑ | F180 |

### 3.2 Phase 2: foundations (24 – 28 Sep)

| ID | Step | Date | Status | Result |
|---|---|---|---|---|
| FW0 | Baseline: native tests, CI, manifold maths in `src/Core/` with golden values, pin map, command list | 09-24 | ☑ | Found: no pressure sensor installed (F50), V23 removed (F51) |
| FW1 | Host protocol v1: spec and shared test vectors | 09-25 | ☑ | [protocol.md](../software/protocol.md), `protocol/vectors.json`; the firmware side is stage 2 A10 |
| HW1 | Electrical audit, part 1 (checklist M1–M44) | 09-25 | ◐ | Measurements left to stage 2 B4 |
| LA1 | Lab interface (meeting template) | 09-25 | ✗ dropped 10-23 | Values set with the lab directly |
| LA2 | Filter SOP (loading, cleaning, biosecurity, chain of custody) | 09-25 | ◐ draft | [filter-sop.md](../lab/filter-sop.md); the dry run is stage 2 C4 |
| LA3 | Shield volume, residual water, retention: protocol and analysis tool | 09-25 | ◐ | No measurement yet; stage 2 C2 with B7 |
| LA4 | Carry-over and cross-talk: analysis tool | 09-28 | ◐ paused | Protocol dropped 10-21; stage 2 C3 |
| HO1 | Host skeleton: client, transport, simulator `FakeController`, CLI | 09-28 | ☑ | 364 tests; used in simulation only |
| FW2 | Mechanical test suite, lock tri-state, `TEST\|` logging, sensorless test mode | 09-28 | ☑ | Bench rows 1–5 PASS on 10-12; F38, F43, F46 fixed |
| — | Merge of the parallel steps into `cowas-v2` | 09-28 | ☑ | |

### 3.3 Phase 3: bench prototype (6 – 23 Oct)

Stage 1 was defined on 2026-10-06 (user): a working prototype first, even without every safety layer, for early water tests.

| ID | Step | Date | Status | Result | Refs |
|---|---|---|---|---|---|
| S1-0 | Bench facts: lock tests, `cal`, stop offsets, manometer | 10-09, 10-12 | ☑ | Lock tests PASS; garbage ABP frames fixed by the SPI clock (800 → 100 kHz); `purge_angle` wrap point fixed | F124, F127–F129 |
| S1-1 | Pressure on the ABP: cap, dead-sensor stop, PID, calibration command | 10-09, 10-12 | ☑ | Calibration against the manometer PASS (gain 1.0413, offset −0.0311, R² 0.999); dead-sensor stop PASS; cap trip at a temporary 1.0 bar. Range unresolved (×2.4 against the manometer, `ABP_FULL_SCALE_PSI = 60` placeholder) | F126, F136, D7 |
| S1-1b | Pump control on clogging filters: `pressure_ramp`, `fill_test`, controller trials | 10-15 – 10-19 | ☑ | Four controllers tried; back to the PID 200/100/100, ripple accepted; 12-bit output; cap raised step by step to 3.6 bar; dead controllers deleted | F132–F147, D16, D26, D27 |
| S1-A | Water flows | 10-19 | ☑ PASS | 1000 mL counted for 1006 g (slot 7); 100 mL through a filter in 52 s (slot 11) | F148, F149 |
| S1-B | One new filter, 1 L | 10-19 | ✗ not met, accepted | Dirty water clogged a new filter at 673 mL; the clog stop works | F153 |
| S1-C | One real cycle (purge, sample, shield, empty) and its fixes | 10-19 – 10-23 | ☑ | Skip and abort keys, emptying rule, purge volume, pause in operation, shield timing, every phase time-limited | F155–F161, F188 |
| — | Manual realignment and recalibration; `actuator_home` abort key | 10-20, 10-22 | ☑ | Frame and outlet realigned by hand; `purge_angle` from `cal` | F164–F166, F186 |
| — | Sample records and the console tool (session log) | 10-21, 10-23 | ☑ | One record per sample, sessions saved with the PC time | F198–F200, F206 |
| — | Outlet reprint | 10-22 | ☑ | Outlet thread sealed with PTFE tape | F189 |
| — | Encoder slip and slot offset | 10-23 | ☑ | Encoder re-tightened; `purge_angle` 344.27 (D40). The coupling lock is stage 2 B2 | F187, F191 |
| — | Clog floor, gentle pump start, relay pause | 10-23 | ☑ | A window under 30 mL per 60 s ends `clogged`; start from 0 % at 5 %/s; the pause cuts the relay. Max 3.379 bar on a dead-headed line | F194, F196, D43 |
| — | Vent before the unlock; confident (`trusted_ml`) volume; flowmeter factor 77 confirmed | 10-23 | ☑ | The vent released the pressure in all but one of about 240 cases in the field | F150, F192 |
| — | Operator command `run_sample` and the first run on the current build | 10-23 | ☑ | One command: purge, sample, shield, unlock; metadata in the record | F197–F199 |
| — | Operator tutorial, docs restructure, registers | 10-21 – 10-23 | ☑ | [operator-tutorial.md](../field/operator-tutorial.md), findings and decisions registers | — |
| S1-D | Several in a row | — | ☑ in the field | Replaced by the timed schedule; the overnight river series ran 6 of 6 | §4.1 |

### 3.4 Phase 4: field preparation (26 Oct – 2 Nov)

| Step | Date | Status | Result | Refs |
|---|---|---|---|---|
| Purge 3 L with the spool, 500 mL without; purge at full power | 10-26 | ☑ | About 7 min instead of 16 per spool sample | D50, D51 |
| Loading order and slot availability | 10-26 | ☑ | The tool keeps the used slots; from 11-03 the start question asks which are available | D64 |
| Timed sample list (`--schedule`) | 10-26 | ☑ | Runs only after `schedule start`; a late sample waits; restart keeps the list | D52–D54 |
| Sample timing estimator (spec only) | 10-26 | ☑ spec | Stage 2 A4 (timing spec, §7.5) | D55, F208 |
| Ethanol cleaning (`ethanol_purge`) | 10-26, 11-02 | ☑ | Prime, each used slot, final purge; 0.1 L per slot | D91 |
| Spool enabled: dive, sample, roll up, calibration | 11-02 | ☑ | New move loop with stall, wrong-way, time and end-stop checks; old spool bugs fixed | D56, F211, F212 |
| Water fill before each sample, 40 m total tube, emptying after each timepoint | 11-02 | ☑ | | D57–D61 |
| Commit of the field branch | 10-23, 10-26 | ☑ | PR #5 | — |

### 3.5 Phase 5: changes made during the field campaigns (26 Oct – 6 Nov)

Built and used on site, or built and left for the first flash after the repair (stage 2 R3). Details: [Bois Chamblard log](../logs/field-bois-chamblard-2026-10.md), [LéXPLORE log](../logs/field-lexplore-geneva-2026-11-03.md).

| Area | Change | Refs |
|---|---|---|
| Spool position | The origin switch acts only during a move and only near the zero; a held switch stops the drum within 300 ms; the depth is homed when in doubt; every sample dives, primes and rises on its own; the prime retries up to 3 times; the roll up runs twice | D65, D69, D70, D72, D78, D79, D83, D87, D88, D90, D93, D95; F214, F224, F226, F227, F232, F247, F251 |
| Spool power | The prime may run at 100 % | D94 |
| Schedule | Stops on a failed roll up, a refusal or an `x`; restarts at a row; retries a failure before the filter on the same filter | D71, D74, D75, D80, D81; F225 |
| Start list | Read back and applied only on `y` | F244 |
| Manifold | `purge_angle` and offsets set on site after the plate work; `encoder_read` diagnostic; stall and over-travel guard (F256, built, not flashed) | D62, D63, D76, D84 |
| Blanks | Procedure without spool movement | D92 |
| Records | Every timed step in the sample file; pause and resume messages | D96; F249 |
| Pressure sensor | Dried and refitted on site after it came off its tubing | F234, F235, D85 |
| Spool mechanics | Wooden switch holder trimmed; screw, belt and level-wind guide repaired (temporary) | F246, F250 |
| Analysis tool | `tools/run_report.py` (pauses, purge flow, prime tries, warnings) | field log §23 |

### 3.6 Phase 6: analysis and handover (6 – 12 Nov)

| Step | Status | Where |
|---|---|---|
| Field logs, plan versus what happened, problems counted per cause | ☑ | Field logs; D97 |
| Manual sampling and LéXPLORE protocols rewritten from what was done | ☑ | [manual-sampling-protocol.md](../field/manual-sampling-protocol.md), [lexplore-field-protocol.md](../field/lexplore-field-protocol.md); D98 |
| Sample list (automatic and manual, with the pairs) | ☑ | [lexplore-sample-list-2026-11.xlsx](../reports/lexplore-sample-list-2026-11.xlsx) |
| Flowmeter bucket checks (2) | ☑ | 20 % and 31 % low at field flow; F254 |
| The incident of 6 Nov analysed (hypotheses weighed, tests listed) | ☑ | Field log §38–39; F257, F258 |
| Power-dropout robustness proposed | ☑ | Stage 2 §7.0 (PW1) |
| Field report and revision report | ☑ | [reports/](../reports/README.md) |
| Stage 2 and 3 reorganised | ☑ | LOG 2026-11-12 |

---

## 4. Field campaigns

### 4.1 Bois Chamblard, river and lake (26 – 29 Oct)

**Goals.** Test the CoWaS without the spool on real water, beside the air/water eDNA project: a river series that includes an unattended overnight run on the schedule, lake samples, and blanks.

| Goal | Achieved |
|---|---|
| Run on river and lake water | ☑ 14 samples at Bois Chamblard (10 river, 4 lake), all taken |
| Unattended timed series | ☑ 6 of 6 scheduled samples over 27–28 Oct, 3.5–4 h apart, each 1001 mL |
| Blanks | ☑ one river and one lake blank (the river blank ended `clogged` at 414 mL; cause not recorded) |
| Clogging water handled | ☑ the lake filters clogged and ended cleanly on the clog rule (854–912 mL) |
| Time per sample | About 20 min without the spool |

Lessons for the operator (dummy rows to mark used slots, `s` versus `x`, the date check, `actuator_home` before the schedule) went into the tutorial; the tool now asks which slots are available ([log §3–4](../logs/field-bois-chamblard-2026-10.md)).

### 4.2 LéXPLORE, Lake Geneva = Pilot A (3 – 6 Nov)

**Goals.** The first use of the spool in the field; two 24 h diel series at three depths; a comparison of the autosampler with manual samples at the same time and depth; blanks; and the Pilot A test of the machine: supervised, 1–2 days, blanks and manual comparison, filters to the lab. The study design came from the project's first-step description ([archive](../archive/field-plans/lexplore-first-step-project-description.md)) and changed on site ([field log §32](../logs/field-lexplore-geneva-2026-11-03.md#32-plan-versus-what-happened-2026-11-06)).

| Goal | Plan | Achieved |
|---|---|---|
| Diel series | 2 runs of 24 h, every 4 h | ☑ E1 (3–4 Nov) and E2 (5–6 Nov), 7 time points each |
| Depths | 3 intakes (2, 20, 30 m) | ☑ one intake on the spool at 1, 5 and 20 m (40 m tube limit) |
| Samples | 48 automatic | ☑ 46 valid samples, 2 lost: E1_T5_01M_R1 (wrong start list, F244) and E2_T7_01M_R1 (incident of 6 Nov, F257) |
| Blanks | 2 automatic, 2 manual | ◐ 2 automatic (E1-B1-00M-R1, E2-Blank), 1 manual |
| Manual comparison | Paired samples | ☑ 18 manual samples within −4 to +15 min of the machine (mean 4.9 min) |
| Seal and pressure | No leak, under the cap | ☑ no manifold leak; highest 3.44 bar in 155 records (cap 3.6) |
| Spool in the field | Depth known for every sample | ☑ after the fixes of 3 Nov evening, no failed try used a filter |
| Time per sample | About 20 min planned | 28.7 min median (dive 4.4, purge 7.2, filtration 13.6, shield 4.0) |

**Pilot A against its success criteria** (old stage 2 §8.1):

| Criterion | Result |
|---|---|
| At least 12 of 14 cycles per filter set complete without a fault | ☑ 46 of 48 planned samples; neither loss came from the sampling program |
| Blanks clean | Pending: lab results (stage 2 C1) |
| Machine and manual samples comparable | Pending: lab results (stage 2 C1) |
| Filters, records and logs to the lab | ☑ sample list and records delivered; record corrections listed in stage 2 C1 |

**What the field showed** (details in the [report](../reports/robustness-update-report-2026-11.md) §4):
- **Validated:** the pressure cap, the end rules and the records, the timed schedule with automatic retries, the vent before the unlock, and the double O-ring seal.
- **Revised on site:** the spool software (§3.5).
- **Gaps:** nothing stops the motors if the Due itself fails (incident of 6 Nov). The spool mechanics are wet wood and glue. The pressure sensor came off its tubing. The encoder coupling slips. The flowmeter reads low at field flow. The shield dose is timed, not measured. Operator input errors cost samples.

---

## 5. Where the leftovers went

Every improvement from the field and every item left open here is a step of [stage 2](stage2-pilots.md): §4 lists the findings by step, and §7b maps the old stage 1 and field items. The last "Left to do" list of this file (2026-11-05):

| Item | Now |
|---|---|
| Clean the tubing before the next acquisition campaign | Done on site (ethanol purges, field log §22.5, §25); the three slots of schedule 3 → stage 2 R2 |
| DNA Shield dose and air timing | Stage 2 B7, C2 |
| Commit the stage 1 work | Stage 2 R4 |
| Weigh one spool-on purge (3 L against the 2.54 L spool volume) | Stage 2 C3 (purge rule), R3 (pressure check, F207) |
| Field, with the operator guide | ☑ §4 |
| Timed sample list test | ☑ done in use: the Bois Chamblard overnight series and both LéXPLORE series ran on the schedule |
| Early fix: manifold shifts (HW9) | Stage 2 B2 (P0) |
| Blocking: out of service after 6 Nov (F257) | Stage 2 Phase 0 (R1–R3) |

**Running the machine today** (until the stage 2 P0 steps are done): the [operator tutorial](../field/operator-tutorial.md) holds the field limits. The main ones:
- Someone stays at the 24 V switch for every sample, and cuts it before any reset or flash (F215, D86).
- Use `trusted_ml`, not the plain total (F150, F254).
- Always run samples through the session tool (F210).
- Check the spool, the prefilter and each filter's expiry by hand before a run (F246, F248, F253).
