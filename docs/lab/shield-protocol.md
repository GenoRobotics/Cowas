# Shield protocol: dose, residual water, retention

**Status (2026-10-23):** Part A (calibration of the micro pump) can run now. Part C (weighing campaign) can start: the pressure cap works. **No measurement yet.**

**Questions it answers:**
1. How much Shield goes into a Sterivex? → Part A (the fill time).
2. How much lake water is still in the filter when the Shield arrives? It dilutes the Shield (F28). → Part C.
3. How much liquid does a filter lose while it waits in the machine? (F29). → Part C, holds.

**Tools:** `shield_calibrate` (console), `tools/analyze_la3.py` (analysis), `tools/la3_thresholds.json` (thresholds, `null` until decided), `tools/la3_masses_template.csv` (weighing sheet). How the dose is produced in the code: [software/code-guide.md §4](../software/code-guide.md).

---

## 1. Equipment

- Scale, 0.01 g, ≥ 200 g, level. Check it with a reference mass at the start of each session.
- 13 Sterivex (12 for the plan, 1 spare). Each filter keeps its numbered pair of luer caps: they are part of its dry mass.
- DNA/RNA Shield, ~60 mL (15 for Part A, 20 for filters, the rest for primes). Record the lot.
- Tubes, pipette (1 mL), thermometers or loggers (one at the manifold, one in the warm box), a warm box at 30 °C for group F.

---

## 2. Part A: micro pump calibration (now)

The micro pump is timed. Dose = rate × time. The Shield has its own tube to the manifold: it must stay full between samples (A4).

**A1. Density (5 min).** Pipette 1 mL of Shield into a tared tube, weigh, repeat five times in the same tube. Density = slope of mass against volume (1.00–1.10 g/mL expected). Set it in `tools/la3_thresholds.json`. Until then the code uses 1.00.

**A2. Rate and fill time: `shield_calibrate` (about 1 h).**
1. Connect the bottle as in production. Take the Shield tube off the manifold and hold its end over an empty tube on the scale. Keep the bottle half full.
2. The command asks: density (0 = 1.00), target volume (0 = 1.5 mL), three durations (0 = 10, 20, 40 s). Choose durations so the longest run gives at least 1 g.
3. Line empty? Answer `y` to measure the first drop (A3).
4. Prime: press a key when the Shield drips steadily **without bubbles**.
5. Nine runs, three per duration, interleaved. For each: tare the tube, press a key, touch the tube end to the wall to collect the hanging drop, type the mass.

The command prints `TEST|shield_calibrate|…` lines and `DNA_SHIELD_FILL_MS`. **Pass:** each duration's CV ≤ 5 %, fit R² ≥ 0.995, |offset| < 0.1 mL. Otherwise: bubbles (prime longer), a drop not collected, or a drifting rate (rerun; try a 3-minute run).

To recompute for another target: `fill_ms = 1000 × (target − offset) / rate`.

**A3. Line volume, for priming (inside A2).** Empty the line, answer `y` to "line empty", and press a key at the first drop. The command gives the time to fill the line and the suggested prime time (1.25 × that time). Cross-check with the tubing: volume = π × (ID/2)² × length.

**A4. Drain-back (overnight).** Prime, reconnect to the manifold, wait at least 12 h, then run one dose into a tared tube. **Pass:** at least 95 % of the expected dose.

---

## 3. Part C: weighing campaign

| Group | Filters | Cycle | Weighings | Hold |
|---|---|---|---|---|
| A | 3 | Production, today's air push | dry → water → shield → hold | In the machine |
| B | 3 | Production, a longer emptying (set `EMPTY_AIR_HOLD_MS` and `EMPTY_MAX_TIME_MS` in `Settings.h`) | dry → water → shield → hold | In the machine |
| F | 3 | Production, no weighing between stages | dry → full → hold | Warm box, 30 °C |
| S | 2 | As F | dry → full → hold | Next to the machine |
| H | 1 | As F | dry → full → 4 handling weighings | None |

**Why:**
- **A and B:** each filter is weighed after the water and again after the Shield. The Shield volume is measured on the same filter.
- **F and S:** filters kept on the bench from water to Shield. If their totals match A, the paired weighing did not bias the result.
- **Holds in the machine:** the real rest state. Twice a day, rotate to an empty slot and back, as a sample does.
- **H:** each hold weighing means removing and refitting the filter, which can lose a drop. H measures that loss.

**Weighing one filter:** cap the outlet first, unscrew, cap the inlet at once. Wipe, weigh when stable (≤ 10 s), note mass, time and room temperature. Refit: uncap the inlet, screw in, uncap the outlet.

**Day 0.** For each A and B filter: dry weighing, load, `full_cycle <slot> purge=y shield=n empty=y volume=2000` (water only), water weighing, refit, then the Shield (`full_cycle` with `shield=y`, or the total minus the water weighing). The Shield weighing is **t = 0** of its hold. F, S and H: `full_cycle <slot> purge=y shield=y empty=y volume=2000` with no weighing in between.

**Holds:** weigh at about 24 h, 72 h and 168 h after t = 0. Record the min and max temperature and, before each weighing, what the outlet looks like.

**CSV:** one row per weighing: `filter_id,group,stage,time,mass_g,temp_c,notes`. Stages: `dry`, `water`, `shield`, `full`, `hold`, `handling`. Save as `docs/logs/test-logs/<date>_LA3_masses.csv`.

---

## 4. Analysis

```
python tools/analyze_la3.py docs/logs/test-logs/<date>_LA3_masses.csv --thresholds tools/la3_thresholds.json --csv docs/logs/test-logs/<date>_LA3_results.csv
```

Per filter: residual water = (water − dry) / ρ_water; Shield = (shield − water) / ρ_Shield; ratio = Shield / residual water; retention = the kept fraction of the liquid at each hold, and the loss per day. Each check prints PASS, FAIL or OPEN (no threshold yet). Exit code 1 on any FAIL.

The scale cannot tell Shield from water, so retention measures the loss of liquid.

---

## 5. Decisions

**The dose.** Use `fill_ms` from A2. Check on filters: the mean Shield of group A within ±10 % of the target. If not, correct once (`fill_ms × target / measured mean`) and check on three more filters.

**The air push.** Keep the current push unless a longer one lowers the residual water by at least 0.1 mL and 20 %. Air does not cross a wet membrane (F28): a stronger push drains the housing only. If the ratio stays below the minimum, use a larger dose.

**Caps or check valves on the outlets (F29).** With the O-ring locked, liquid can only leave through the outlet, and it evaporates slowly. If a filter loses too much over the pilot's length: Pilot A, cap the outlet after each cycle (the operator is there); Pilot B, luer check valves (cracking pressure ≤ 0.1 bar) on each outlet, then repeat the warm hold.

**Priming.** The shield line must be full before the first dose. Prime at the purge slot (slot 0), never a sample slot, for the time from A3 (× 1.25). Prime before each deployment and after any bottle change.

---

## 6. Results

| Part | Result | Pass? |
|---|---|---|
| Density | | 1.00–1.10 |
| Rate (mL/s), offset (mL), R² | | R² ≥ 0.995, \|offset\| < 0.1 mL |
| CV per duration | | ≤ 5 % |
| `fill_ms` for the target | | |
| `line_ml`, prime time | | |
| Drain-back: delivered / expected | | ≥ 95 % |

| Group | Residual water (mL) | Shield (mL) | Ratio | Kept at day 7 (%) | Loss (%/day) |
|---|---|---|---|---|---|
| A | | | | | |
| B | | | | | |
| F | | | | | |
| S | | | | | |

**Decisions:** dose `DNA_SHIELD_FILL_MS` = · air push = · outlet caps = · prime time = .

---

## 7. Open items

| Item | Blocks | Where |
|---|---|---|
| Thresholds B1–B5 and the hold temperature | Every PASS/FAIL (printed as OPEN) | §8 below; `tools/la3_thresholds.json` |
| Shield accepted in the extraction housing (A2) | The Shield-in-housing design | §8 |
| Shield density | The mL conversion | `SHIELD_DENSITY_G_PER_ML` |
| Part A not run | The dose and the prime | §2 |
| Console command for the Shield stage alone | The paired design (step 4 of day 0) | Not built |

---

## 8. Values to set (with Felix)

These replace the question list of the former lab meeting (dropped 2026-10-23). The current value is the one the code or the draft uses.

| ID | Value | Used by | Current value | Status |
|---|---|---|---|---|
| A2 | Does the extraction kit accept a housing filled with Shield? | The design | Assumed yes | Check before Pilot A (blocking) |
| A3 | Sterivex model: pore size and membrane | Clogging, water retained (F28), the dose | 0.22 µm PES (e.g. SVGP01050) | Proposal |
| B1 | Target Shield volume per filter (mL) | `DNA_SHIELD_FILL_MS` | 1.5 mL (`SHIELD_TARGET_ML`) | To set (blocking for the dose) |
| B2 | Minimum acceptable Shield volume (mL) | Pass/fail | none | To set |
| B3 | Maximum residual water, or minimum Shield:water ratio | Pass/fail | ratio 1:1 proposed | To set (blocking for the dose) |
| B5 | Acceptable loss while waiting in the machine (% per day) | Pass/fail | none | To set |
| C1 | Target filtered volume per Sterivex | `STX_SAMPLE_MILLILITERS` | 1000 mL (field setting) | To set |
| D1 | Time in the machine and temperature range | Warm hold (30 °C), storage limit | 30 °C, up to ~2 days (Pilot A), ~14 days (Pilot B) | To set (blocking for Pilot B) |
| E1 | `sample_id` format | Labels, file names | `<device>-D<deployment>-<slot>-<UTC start>` | Proposal |
| F2 | Equipment blank volume | Packing list | 2 L (plus the purge) | Proposal |
| L1 | Bleach for the wetted parts | Filter SOP §1, §4 | 10 % household bleach (~0.5 % hypochlorite), 10 min, DNA-free rinse | Proposal (blocking for the workshop) |
