# LA3: Shield volume, residual water, shield retention

**Status (2026-09-25):** protocol and tools ready; **no measurement done yet.** Part A can run now. Parts C–D wait for FW4 (and FW3 + HW4 for pumping), and every pass/fail threshold waits for the lab (LA1). §7 lists what is still open and where each item is tracked.

**Goal:** the numbers we need to trust each filter:
- how much Shield the machine really puts into a Sterivex;
- how much lake water is still in the filter when the Shield arrives (it dilutes the Shield, F28);
- how much liquid a filter loses while it waits in the machine for days (F29).

**Tools:** the console command `shield_calibrate` (Part A), `tools/analyze_la3.py` (Part D), the thresholds file `tools/la3_thresholds.json`, and the CSV template `tools/la3_masses_template.csv`.

---

## 0. When each part can run

| Part | What | Needs | Can run |
|---|---|---|---|
| A | Shield micro pump: density, mL/s, line volume, drain-back | micro pump + a 0.01 g scale; **no main pump, no valve, no rotation** | **now** (current firmware + this branch) |
| B | Set `DNA_SHIELD_FILL_MS`, choose the air-push variants | Part A; FW4 items 7 and 8 | after FW4 |
| C | Weighing campaign: residual water, shield volume, retention | FW4 (auto-unlock, rest locked, 2 L, emptying params); **FW3 + HW4** (a working pressure cap: pumping into a Sterivex is pumping against a closing outlet, CLAUDE.md "Known hazards"); FW2's `full_cycle` | after FW3 + HW4 + FW4 |
| D | Analysis | the Part C CSV | any time (with synthetic data, now) |
| E | Decisions | A–D + the lab thresholds (LAB_INTERFACE §4) | after LA1 + C |

**Safety for Parts C and E:** until FW3 + HW4 are done, the pressure cap can't trip (F50). Don't pump into filters before that. Once they're done, stay at the machine for every cycle and keep the outlet side of the manifold visible.

---

## 1. Equipment and consumables

| Item | Quantity | Notes |
|---|---|---|
| Scale, 0.01 g readability, ≥ 200 g | 1 | Level it, keep it away from drafts. Check it with a calibration mass (or a coin weighed once as a reference) at the start of each session; note the reading in the session log (§6). |
| Sterivex (the model from LAB_INTERFACE A3) | **13** | 12 in the plan (§3.1) + 1 spare. Record the lot. |
| Luer caps, numbered | 2 per filter (26) | Male and female luer caps. **Each filter always uses the same numbered pair**: they are part of its dry mass. |
| DNA/RNA Shield (the lab's product) | ~60 mL | ~15 mL for Part A, ~20 mL for the filters, the rest for primes and repeats. Record the lot. |
| 1.5 or 2 mL tubes with lids | ~15 | Part A |
| 1 mL pipette (P1000) + tips | 1 | density check (A1) |
| Lint-free tissues, gloves, marker, timer | | wipe the outside of each filter before weighing |
| Thermometer, min/max, or a logger | 2 | one near the manifold, one in the warm box |
| Warm box at 30 °C (incubator, or an insulated box with a heating pad and a thermostat) | 1 | the warm hold (§3.4). 30 °C is our worst-case guess for a closed box in the sun (LAB_INTERFACE D1 is still open). |
| Clean water for the cycles | ~20 L | tap water is fine here (no DNA work). Use the same water for every cycle. |

---

## 2. Part A: the Shield micro pump (runs now)

The micro pump is time-based: there is no flowmeter on the Shield line, so the dose is `rate × time`. The Shield has **its own tube to the manifold** (user, 2026-09-25): whatever that tube holds must be filled before the first dose (the prime, A3) and must stay filled between samples (A4).

### A1. Density of the Shield (5 min)

1. Tare a closed tube on the scale.
2. With the P1000, pipette 1.000 mL of Shield into it, close it, weigh it. Repeat 5 times into the same tube (tare once, read the total after each mL).
3. Density = the slope of mass vs mL (or total mass / 5 mL). Pipette error is ~1 %, so expect 1.00–1.10 g/mL.
4. Write it in §6 and into `tools/la3_thresholds.json` (`shield_density_g_ml`). Until then everything uses **1.00 (PLACEHOLDER)**.

### A2. Flow rate: `shield_calibrate` (about 1 h)

Setup: connect the Shield bottle as in production. **Take the Shield tube off the manifold** and hold its end over a tube on the scale. Keep the bottle at its normal height and about half full (the level changes the rate a little).

On the console (`pio device monitor`), type `shield_calibrate`:

1. Density (0 = 1.00) and target volume per filter (0 = 1.5 mL; LAB_INTERFACE B1, PLACEHOLDER: the rate doesn't depend on it, only the printed fill time does).
2. Three durations (0 = 10, 20, 40 s; `-1` at the third = only two). **Choose them so the longest run gives ≥ 1 g** (1 % resolution on a 0.01 g scale) and so they bracket the expected production dose. If you have no idea of the rate, accept the defaults, look at the first 40 s run, and restart with better durations if it gave < 1 g.
3. If the line is empty, answer `y` to measure the time to the first drop (A3), otherwise `n`.
4. Prime: the pump runs until you press a key (max 120 s). Stop when the Shield drips steadily **without bubbles**.
5. Nine runs (3 per duration, interleaved: 10, 20, 40, 10, 20, 40, ...). For each run, put an empty tube on the scale, tare, press a key, wait, then **touch the tube end to the tube wall** to collect the hanging drop and type the mass in g.

Output (`TEST|` lines, copy them into §6):
```
TEST|shield_calibrate|d10000|PASS|n=3;mean_ml=...;sd_ml=...;cv_pct=...;max_cv_pct=5.0
TEST|shield_calibrate|fit|PASS|verdict=PASS;rate_ml_s=...;offset_ml=...;r2=...;density_g_ml=...;runs=9;durations=3
TEST|shield_calibrate|fill|PASS|target_ml=1.500;fill_ms=...
TEST|shield_calibrate|line_fill|PASS|t_first_drop_ms=...;line_ml=...;prime_ms_suggested=...
```

**Pass criteria:** every duration's CV ≤ 5 % (`SHIELD_CAL_MAX_CV_PCT`), the line fit R² ≥ 0.995 (`SHIELD_CAL_MIN_R2`). The offset is the volume at t = 0 (spin-up loss if negative, drip if positive); it should be small next to the dose (|offset| < 0.1 mL). If it fails:
- CV too high → bubbles in the line (prime longer), a hanging drop not collected, or the tube end moving;
- R² too low → the rate drifts (the pump heats up, or the bottle vents badly): rerun, and try one 3-minute run to see whether the rate changes over time.

The console then prints `DNA_SHIELD_FILL_MS = <ms>` for the target. **To recompute it for another target without rerunning:** `fill_ms = 1000 × (target_mL − offset_mL) / rate_mL_per_s`.

### A3. Line volume, for `PRIME_SHIELD` (10 min, can be done inside A2)

Empty the Shield line (disconnect the bottle and let the pump run the line dry, or start with a new dry tube). Reconnect the bottle, run `shield_calibrate`, answer `y` to "line empty", and press a key at the **first drop** at the tube end. The result gives:
- `t_first_drop_ms` = the time to fill the empty line;
- `line_ml = rate × t` = the line volume (bottle intake + pump + tube);
- `prime_ms_suggested = t × 1.25` (`SHIELD_PRIME_MARGIN`).

Cross-check with the tubing: `volume = π × (ID/2)² × length` for each piece (e.g. 1 m of 1.6 mm ID = 2.0 mL: **as much as a whole dose**, which is why a first sample without a prime gets almost no Shield). Write the tube IDs and lengths in §6.

### A4. Does the line stay full between samples? (drain-back; overnight)

In production the line sits for hours between two doses. If it drains back into the bottle (siphon, a pump that doesn't close), the next dose is short.

1. After a prime, reconnect the tube **to the manifold** as in production (the manifold at the purge slot, O-ring locked).
2. Wait the longest planned interval between samples (at least overnight, ≥ 12 h).
3. Take the tube off the manifold and run **one production dose** (`fill_ms` from A2) into a tared tube. Weigh it.
4. Compare with the calibrated dose: `delivered / expected`.

**Pass:** ≥ 95 % of the calibrated dose. If it's lower, the line drains back: see §5.4.

---

## 3. Part C: weighing campaign (after FW3 + HW4 + FW4)

### 3.1 Plan: groups and filters

| Group | Filters | Cycle | Weighings | Hold |
|---|---|---|---|---|
| **A-today** | 3 | today's air push (FW4 defaults: 0.4 bar PID target, ends at P < 0.8 bar) | dry → water → shield → hold | in the machine, room temperature |
| **B-long** | 3 | a longer air push: `EMPTY_STX_MIN_MS = EMPTY_STX_MAX_MS = 60 s` at today's setpoint | dry → water → shield → hold | in the machine, room temperature |
| *B-high* (optional) | 3 | 60 s at a higher setpoint, e.g. 2.0 bar = `pressure_target()`: below the 2.5 bar cap and well below the membrane bubble point (≥ 3.4 bar estimated for 0.22 µm PES, F28: check the datasheet of your lot) | dry → water → shield | none |
| **F-warm** | 3 | the full production cycle, no weighing in between, the chosen settings | dry → full → hold | off-machine surrogate at 30 °C |
| **S-room** | 2 | same as F | dry → full → hold | off-machine surrogate at room temperature, next to the machine |
| **H-handling** | 1 | same as F | dry → full → handling ×4 | none |

12 filters (15 with B-high). *B-high* needs a setpoint parameter for the emptying phase that FW4 does not add (FW4 keeps the 0.4 bar setpoint and only adds `EMPTY_STX_MIN_MS`/`EMPTY_STX_MAX_MS`): it's in the follow-up list (§7). Skip it if that parameter doesn't exist yet.

**Why this design:**
- **Paired weighings (A, B):** each filter is weighed after the water and again after the Shield, so the Shield volume is measured on the same filter, without subtracting a group mean. The cost is one extra handling (removal and reinstall) and a longer end-of-filtration-to-Shield time (~5 min), which doesn't change the volumes.
- **F and S (the production check):** filters that were not taken out between the water and the Shield. If `water + shield` of F/S matches A (same settings), the paired design did not bias anything.
- **Holds in the machine (A, B):** the real rest state: O-ring locked (§3.2), the filters in their slots. To be like Pilot B, **twice a day** rotate to an empty slot and back (`slotN`, which unlocks, rotates and re-locks from FW4): each sample in the field unlocks the O-ring once.
- **The warm hold is off the machine** (a surrogate): the machine can't be at 30 °C and at room temperature at the same time, and a second 7-day hold would cost a week. A filter in a locked manifold has its inlet closed by the rotor and its outlet open. The surrogate copies that: **inlet capped, outlet open, same orientation as in the manifold**, in the warm box. S-room (same surrogate, at room temperature next to the machine) is compared with A to check that the surrogate behaves like the machine. If S-room loses much less than A, the machine's unlocks matter, and the warm result is optimistic.
- **H (handling):** each hold weighing means taking the filter out and putting it back, which may lose a drop. H measures that loss per handling, and the analysis adds it back to the hold weighings.

### 3.2 How to weigh a filter (every weighing)

1. Gloves. Filter in the manifold: **cap the outlet first** (its numbered cap), unscrew the filter from the manifold, cap the inlet at once. Keep it in the orientation it had in the manifold.
2. Wipe the outside with a lint-free tissue (no liquid on the housing or the caps).
3. Weigh it with its two caps. Wait until the reading is stable (≤ 10 s). Write the mass, the time, the room temperature.
4. To put it back: uncap the inlet, screw it in, then uncap the outlet. Keep the caps with the filter.

The **dry** weighing is the new filter with its two caps, out of its bag, before anything.

### 3.3 Day 0: cycles and weighings (A, B, then F, S, H)

For each A and B filter, one at a time:
1. `dry` weighing (§3.2), then load it into its slot (LA2 SOP; record the slot).
2. **Cycle without the Shield:** FW2's `full_cycle <slot> shield=n volume=2000` (2 L = the FW4 production volume; use the lab's C1 volume once decided). With the B settings for the B group. Record the `end_reason`, the volume, the emptying-phase log lines (FW4 item 8: pressure and duration of each emptying phase).
3. `water` weighing, straight after the cycle. Put it back.
4. **Shield stage alone**, as production runs it (micro pump for `DNA_SHIELD_FILL_MS`, then the air push of the Shield): protocol v1 `DNA_SHIELD <slot>` (FW7). On the console there is no such command yet: FW2's `shield_only` runs the micro pump **without** the air push, so it is not the production stage. See §7: a `sample=n` option to `full_cycle` or a `shield_stage <slot>` command is needed (follow-up PR).
5. `shield` weighing. Put it back. The time of this weighing is **t = 0** of its hold.

For F, S and H: `dry` → load → `full_cycle <slot> volume=2000` (everything: purge, water, Shield, empty) → `full` weighing (t = 0).
- F: inlet capped, outlet uncapped, into the warm box.
- S: same, on the bench next to the machine.
- H: four `handling` weighings in a row, 5 min apart, each a full remove → weigh → reinstall (§3.2). Then discard.

### 3.4 Holds: day 1, 3, 7

- A and B: in the machine, O-ring locked, two rotations a day (§3.1). Weigh them (`hold`) at ~24 h, ~72 h and ~168 h after their t = 0, and put them back each time.
- F and S: weigh them (`hold`) at the same times. Put them back in their box with the inlet capped and the outlet open.
- Record the min/max temperature of each hold at each weighing (the `temp_c` column: the mean, and the min/max in `notes`).
- **Note where the liquid goes:** before each weighing, look at the outlet: a drop, a wet spot on the manifold, dried Shield crystals? Write it in `notes`. A tissue under the outlets of S and F, weighed at each weighing, separates drainage from evaporation (optional).

### 3.5 The CSV

One row per weighing, in `tools/la3_masses_template.csv` format (delete the example rows):

```
filter_id,group,stage,time,mass_g,temp_c,notes
A01,A-today,dry,2026-10-12 08:00,8.12,21.0,caps C1+C2; slot 3
A01,A-today,water,2026-10-12 09:50,8.53,21.0,end_reason=volume_reached; air push 5.1 s
...
```

Stages: `dry`, `water`, `shield`, `full`, `hold`, `handling`. Times in local time, `YYYY-MM-DD HH:MM`. Keep the CSV under `test_logs/` (e.g. `test_logs/2026-10-12_LA3_masses.csv`).

---

## 4. Part D: analysis

```sh
python tools/analyze_la3.py test_logs/<date>_LA3_masses.csv --thresholds tools/la3_thresholds.json --csv test_logs/<date>_LA3_results.csv
```

Per filter:
- `residual water (mL) = (water − dry) / ρ_water`
- `shield (mL) = (shield − water) / ρ_shield`
- `ratio = shield / residual water` (the lab's B3 rule may be "≥ 1:1")
- `water + shield (mL) = (shield or full − dry) / ρ_shield`
- retention: `% kept(t) = (m(t) + k × handling loss − dry) / (m(t=0) − dry)` at each hold weighing k; `loss %/day` = the slope of a straight line through (0, 100 %) and the hold points; `loss mL/day` = the same in mL.

Per group: mean ± SD (n). Each check prints **PASS**, **FAIL**, or **OPEN** (the lab hasn't set that threshold yet); a filter fails if any check fails. Exit code 1 if anything fails.

Limits of the method: the scale can't tell Shield from water, so the retention is a loss of **liquid** (Shield + residual water). If the lab needs the Shield concentration, the retention loss is assumed to be the mixture.

---

## 5. Part E: decisions and recommendations

The rules below turn the results into settings. The numbers they need are still missing (§7).

### 5.1 `DNA_SHIELD_FILL_MS`

1. From A2: `fill_ms = 1000 × (B1 target − offset) / rate`.
2. Check it on the filters: the mean `shield` of group A should be within ±10 % of the target. If not, correct it once: `fill_ms × target / measured mean`. Then check it again on 3 filters (it is the dose that reaches the filter that counts, not the pump output: some Shield may stay in the manifold passage, or pass through the membrane and out of the outlet during the air push).
3. FW4 item 7 writes it into `Settings.h` (until then: `FILL_STERIVEX_TIME / 10.` = 25 s, of unknown volume, F4).

**Recommendation now:** none: the rate is unknown. Run A2 first; it takes an hour and needs no other step.

### 5.2 The air push

- Keep today's settings (A) unless B lowers the mean residual water by **≥ 0.1 mL and ≥ 20 %** (well above the 0.01 g resolution and the filter-to-filter spread), with no leak and no filter damage.
- Physics (F28): air can't cross the wet membrane below its bubble point, so a longer or stronger push only drains more of the housing; the water in the membrane stays. Expect a small gain.
- If even the best variant leaves the ratio below the lab's minimum (B3), **the fix is a larger Shield dose, not a longer push**: tell the lab the measured residual water and let them choose the dose.

### 5.3 Caps or check valves on the filter outlets (the question from F29 and FW1)

With the O-ring locked at rest, the rotor closes each filter's inlet. The liquid can then only leave through the outlet, and only if air can get in (it can't through the capped-by-rotor inlet or through the wet membrane). So we expect small losses: evaporation through the small outlet opening, and "breathing" when the temperature changes (an air bubble trapped in the housing grows by ~3 % per 10 °C and pushes out that much liquid). Each unlock (one per sample) briefly vents the inlets.

**Decision rule** (per pilot, using the warm hold F as the worst case):

| Result | Pilot A (2 days, supervised) | Pilot B (14 days, unattended) |
|---|---|---|
| After the pilot's duration (extrapolated from `loss %/day`), every filter still meets B2 (min Shield) and B3 (ratio) | no caps | no caps |
| It doesn't | the operator caps each outlet after its cycle (cheap: the caps are on the packing list) | caps are impossible unattended: **luer check valves** (duckbill, cracking pressure ≤ 0.1 bar) on every outlet, then repeat the warm hold with them |

**Recommendation now:** plan for caps in Pilot A (the operator is there anyway, and a capped outlet can't drip or evaporate), and order ~16 luer check valves as a hedge for Pilot B. They are cheap, and a check valve's cracking pressure adds little back-pressure during filtration. The LA3 result then decides whether they get fitted. Also watch S-room vs A: if the machine loses much more than the surrogate, the unlocks are the cause, and the check valves matter even more.

### 5.4 `PRIME_SHIELD`: how much and where (the question deferred from FW1)

- **Where: the purge slot (slot 0).** It is the only port that never holds a filter; its outlet goes to waste, and the next purge flushes the manifold passage. Never a sample slot: the Shield would end up in a filter. This matches the FW1 draft (PROTOCOL.md `PRIME_SHIELD`: rotate to purge, lock, micro pump).
- **How much:** fill the line from the bottle to the manifold, plus a margin: `PRIME_SHIELD_MS = t_first_drop × 1.25` from A3 (`prime_ms_suggested`), i.e. `line_ml × 1.25` of Shield. The margin covers the manifold passage and bubbles.
- **When:** before each deployment, after any bottle change, and after anything that emptied the line.
- **If A4 fails (the line drains back between samples):** first choice, a check valve at the tube end on the manifold side (it keeps the line full and stops lake water from backing into the Shield line). Second choice, a short re-prime into the purge slot before every Shield dose, of the drained volume measured in A4 (costs Shield every sample; FW7 would have to call PRIME_SHIELD inside RUN_CYCLE).
- Until A3 runs: `PRIME_SHIELD_MS` stays PLACEHOLDER (PROTOCOL.md §14, FW7 item 3).

---

## 6. Results (fill in)

**Session log**

| Date | Who | Part | Scale check (reference mass, g) | Room °C | Firmware (commit) | Notes |
|---|---|---|---|---|---|---|
| | | | | | | |

**Part A**

| Value | Result | Pass? |
|---|---|---|
| Shield product and lot | | |
| Density (g/mL), A1 | | 1.00–1.10 expected |
| Durations (ms) | | |
| Rate (mL/s), offset (mL), R² | | R² ≥ 0.995, \|offset\| < 0.1 mL |
| CV per duration (%) | | each ≤ 5 % |
| `fill_ms` for the target (mL) | | |
| Tubes: ID (mm) × length (m) of each piece; computed volume (mL) | | |
| `t_first_drop_ms`, `line_ml`, `prime_ms_suggested` | | line_ml ≈ computed volume ± 20 % |
| Drain-back (A4): interval (h), delivered / expected | | ≥ 95 % |

**Parts C–D** (from `analyze_la3.py`)

| Group | Residual water (mL) | Shield (mL) | Ratio | Water + Shield (mL) | Kept at day 7 (%) | Loss (%/day) | Verdict |
|---|---|---|---|---|---|---|---|
| A-today | | | | | | | |
| B-long | | | | | | | |
| F-warm | | | | | | | |
| S-room | | | | | | | |
| H: handling loss (g per handling) | | | | | | | |

**Decisions (Part E):** `DNA_SHIELD_FILL_MS` = ___ · air push = ___ · caps Pilot A = ___ · check valves Pilot B = ___ · `PRIME_SHIELD_MS` = ___ (destination: purge slot) · drain-back check valve on the Shield line = ___

---

## 7. Still open, and where each item is tracked

| Open item | Blocks | Tracked in |
|---|---|---|
| Target Shield volume (B1), min Shield (B2), max residual water or min ratio (B3), retention limit (B5), time/temperature in the machine (D1, the 30 °C choice) | every PASS/FAIL (they print OPEN), `fill_ms` | LAB_INTERFACE §4 (rows point to LA3); `tools/la3_thresholds.json` (all `null`) |
| Does the extraction kit accept the Shield in the housing (A2)? | the whole Shield-in-housing design | LAB_INTERFACE A2 (blocking) |
| Shield density | the mL conversion (1.00 PLACEHOLDER until A1) | `SHIELD_DENSITY_G_PER_ML` in Settings.h; `la3_thresholds.json` |
| Part A not run yet | `DNA_SHIELD_FILL_MS` (FW4 item 7), `PRIME_SHIELD_MS` (FW7 item 3) | docs/TESTS_PENDING.md |
| **User decision pending (asked 2026-09-25, unanswered):** approve the follow-up PR with the two code items below | Part C step 4, B-high | docs/FIRMWARE_LOG.md LA3 entry |
| A console command that runs **the Shield stage alone** (micro pump + air push), for step 4 of §3.3 | Part C paired design | follow-up PR (ask first); FW2 may add `sample=n` to `full_cycle` |
| An emptying-phase **setpoint** parameter (e.g. `EMPTY_STX_SETPOINT_BAR`, default 0.4) | the optional B-high variant | follow-up PR (ask first) |
| Part C/D not run (needs FW3 + HW4 + FW4) | §5 decisions, VA1 | docs/TESTS_PENDING.md; ROADMAP §7 LA3 row |
| The 7-day in-machine hold can double as HW7's "rest locked for 7 days" check (compression set): same week, same locked manifold; run `leak_hold` at the end | saves a week | ROADMAP HW7 row |
