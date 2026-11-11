# CoWaS field protocol (as run at LéXPLORE, Nov 2026)

Reusable for a new site: change the values in §1. Day-to-day commands: [operator tutorial](operator-tutorial.md). Manual steps: [manual-sampling-protocol.md](manual-sampling-protocol.md). Results: [field report](field-report-draft-2026-11.md). The pre-trip plan is in [archive/field-plans/](../archive/field-plans/lexplore-field-protocol-2026-11.md).

## 1. Design (set before the trip)

| Parameter | LéXPLORE 2026 |
|---|---|
| Aim | Compare autosampler with manual sampling over 24 h runs |
| Runs | E1 3–4 Nov, E2 5–6 Nov |
| Depths | 1 m (upper stratum), 5 m (chlorophyll differs, light −50 %), 20 m (below thermocline), from site profiles |
| Time points | Every 4 h from 16:00; ~85 min per 3-depth group |
| Replicates | R1/R2, one time point per run |
| Manual samples | Paired with automatic ones at chosen time points (18 here) |
| Blanks | CoWaS milliQ blank per run, one manual blank |
| Sample ID | `E<run>_T<time point>_<depth>M_R<replicate>`; blanks `E1-B1-00M-R1`, `E2-Blank` |

## 2. Kit

**CoWaS:** spool (drum, level-wind, top switch checked by hand) · 24 V supply with a switch (someone stays at it) · laptop on AC with `tools/session_log.cmd` and the COM port known · USB cable locked at both ends · tape measure · spare tubing (Sirflex for shield joints) · 0.22 µm Sterivex filters with caps, one per slot · 20 µm prefilter and holder, one set per cleaning.

**Liquids:** DNA Shield 2.2 mL per CoWaS filter (timed pump: check tank level each run), 1 mL per manual filter · 70 % ethanol 0.1 L per slot + 3 L for the spool · milliQ 4–5 L per CoWaS blank.

**Platform / manual:** Niskin bottle and winch (book ahead) · bucket, syringe, manual pump (Vampire, Huber lab) · labels, zip bags, marker, paper sheet, gloves · waste containers (ethanol, filtrate) · food, water, sleeping arrangements.

## 3. Before each run

1. Weather, access, people on site → paper sheet. Check site profiles.
2. **Prefilter first in the inlet, nothing between it and the filters** (a spacer in its place cost 4 runs). Check each filter's expiry date as it is loaded (F253).
3. Ethanol-clean the tubing if the last cleaning was missed.
4. Load filters; write each ID and slot on the sheet.
5. Start the session tool: answer the AVAILABLE question with the loaded slots, then `session` (spool `y`, 40 m).
6. `actuator_home` (UNLOCKED), `spool_height 90` (measured), `spool_status` (origin PRESSED).
7. Write planned times and filter IDs on the sheet. Schedule file: full dates past midnight.

## 4. Automatic sampling

- Load the schedule (one row per depth and time point, spool `on`, depth in m). `schedule start`, read the warnings, `y`.
- Each sample: dive to 40 m with water fill → prime → rise to depth → 3 L purge → sample → DNA Shield → roll up and empty.
- A run that fails before its filter is loaded retries on the same filter (up to twice). The list stops on `x`, a failed roll-up, or a refusal: fix it, then `schedule start <row>`.
- Check at each time point: `schedule`, `spool_status` (origin PRESSED when up), `Spool move:` lines (rise ends at depth + 90 cm). During a sample, only `s` or `x` may be typed.

## 5. Manual sampling and blanks

- **Manual:** Niskin at the sheet times, 1 L after a 1 L purge with no filter, 1 mL shield by hand. Write the clock time.
- **CoWaS blank:** by hand, `run_sample <slot>` with no `depth=`, condition `blank`. Intake tip in milliQ until the water runs out; the spool does not move.
- **Manual blank:** taken 5 Nov; water and steps to be confirmed and added to the manual protocol.

## 6. Filters and end of day

- Each filter: ID, sample name, time, slot on the sheet. When removed, each should be nearly full of shield (~2.2 mL). Cap both ends, bag, label, store at room temperature.
- End of day: `python tools/run_report.py <run folder>` → spool rolled up (else `spool_home`) → ethanol-clean with all filters out → new prefilter → `:quit` → copy the run folder (`session.log`, `tests.csv`, `samples/`, `schedule.csv`) and the sheet to the engineer.

## 7. Known limits

- **Volume** is read from the flowmeter, which under-reads at low flow (~20 % at 60–80 mL/min; F254). Real volume can exceed the record.
- **DNA Shield** is timed (190 s), not measured; the real dose varies with clogging (F255).
- **Spool** can lose its position. The procedure re-checks zero and stops before pumping if wrong.
- **Pressure sensor** coming loose stops the list, but a flat reading while pumping is not yet detected (F234).
