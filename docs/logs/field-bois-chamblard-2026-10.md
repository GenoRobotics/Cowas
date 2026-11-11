# Field log: Bois Chamblard, river and lake (26–29 October 2026)

Field test of the CoWaS (without the deployment module) at Bois Chamblard, in the air/water project (operator `FS` in the logs), mostly by hand and with the schedule for part of the river series. The project also took air samples; they are out of scope here, and only the CoWaS, its faults and their solutions are recorded. The sampler ran **without the spool** (`spool=absent`, 500 mL purge). Written 2026-11-06 from the run folders in `docs/logs/test-logs/` (`2026-10-26`, `2026-10-27`, `2026-10-29`) and checked against the field protocol, which is context only: where the two differ, the logs are the record (section 5). The plan is kept in [archive/field-plans/](../archive/field-plans/bois-chamblard-field-protocol-2026-10.md).

## 1. Summary

| Day | Site | Folder | Samples | Result |
|---|---|---|---|---|
| 26 Oct | BM EPFL (bench test) | `2026-10-26/01_field-BM_test-26_10_2026` | 1 purge + 1 test sample | clogged at 800 mL (745 trusted) |
| 27 Oct | River, day 1 | `2026-10-27/01_field-BoisChamblard-27_10_2026` | 3 samples + 1 blank, 2 of them by schedule | all ran (one ended by `s`) |
| 27–28 Oct | River, night and day 2 | `2026-10-27/03_field-BoisChamblard_attempt2-28_10_2026` | 6 by schedule (3.5 to 4 h apart) | 6 of 6 PASS, 1001 mL each |
| 29 Oct | Lake | `2026-10-29/01_field-BC-Lake-29_10_2026` | 3 samples + 1 blank, by hand | all PASS |
| 3 Nov | LéXPLORE platform (air/water runs, folder `03`) | `2026-11-03/03_felix-lexplore-20261103` | 3 samples and 1 blank, by hand | all PASS; 1 test stopped by `x` |

Totals for the air/water project: 19 CoWaS runs (14 at Bois Chamblard, 5 at LéXPLORE on 3 November). 17 PASS, 1 SKIP, 1 stopped by `x`. Recorded: all with the 500 mL purge and the 1000 mL target. The 24 h river series ran unattended by schedule with no failure of a real row.

## 2. Samples

River (`Bois Chamblard River`, experiment "Air/Water"), trusted volume in brackets:

| Time | Slot | Name | Mode | End | mL (trusted) | Note |
|---|---|---|---|---|---|---|
| 27 Oct 09:36 | 1 | BC-R-D1-1 | by hand | volume | 1001 (718) | started 1 h late; the air sampler was not yet running (note) |
| 27 Oct 13:10 | 2 | BC-R-D1-2 | schedule | skipped (`s`) | 784 (620) | pump step skipped by the operator at 13:22 |
| 27 Oct 16:40 | 3 | BC-R-D1-3 | schedule | volume | 1001 (872) | |
| 27 Oct 17:15 | 14 | BC-R-BLANK-W1 | by hand | clogged | 414 (287) | blank, milliQ (note); ended early, cause not recorded |
| 27 Oct 20:45 | 13 | BC-R-N1-1 | schedule | volume | 1001 (771) | |
| 28 Oct 00:20 | 12 | BC-R-N1-2 | schedule | volume | 1001 (829) | |
| 28 Oct 04:10 | 11 | BC-R-N1-3 | schedule | volume | 1001 (781) | |
| 28 Oct 08:30 | 10 | BC-R-D2-1 | schedule | volume | 1001 (887) | |
| 28 Oct 12:20 | 9 | BC-R-D2-2 | schedule | volume | 1001 (818) | |
| 28 Oct 16:10 | 8 | BC-R-D2-3 | schedule | volume | 1001 (826) | |

Lake (`Bois Chamblard Lake`, experiment "Air/water BC Lake"), by hand, slots 4 to 7:

| Time | Slot | Name | End | mL (trusted) | Pauses |
|---|---|---|---|---|---|
| 29 Oct 11:55 | 4 | BC-L-W1 | clogged | 854 (668) | 89 |
| 29 Oct 12:16 | 5 | BC-L-W2 | clogged | 912 (715) | 85 |
| 29 Oct 12:38 | 6 | BC-L-W3 | clogged | 885 (744) | 74 |
| 29 Oct 12:59 | 7 | BC-L-BLANK-W2 | volume | 1001 (970) | 36 |

Each sample took 18 to 22 min from `run_sample` to the `TEST` line (for example 13:10 to 13:28 on 27 Oct: purge 61 s, sample with emptying 783 s, shield 238 s, unlock 7 s). The earlier tutorial said 5 to 8 min; that was wrong for a full sample and has been corrected.

Observations:
- **River water filled the filter** (all eight river samples that ran to the end reached 1000 mL, trusted 718 to 887 mL), **lake water clogged it** (three of three, 854 to 912 mL). The lake blank (milliQ) reached 1000 mL with a trusted volume of 970 mL, close to the plain total, which is what a blank should look like.
- The river blank of 27 Oct stopped `clogged` at 414 mL. The note says milliQ; the cause (filter, tubing, or the intake leaving the water) is not recorded. The same pattern on a milliQ blank was later explained at Lexplore by the end of the water (field log §28), so check the volume of milliQ first.
- The lock state was UNLOCKED to LOCKED on every purge and sample, and every record has the flag `placeholder:ABP_FULL_SCALE_PSI` (the pressure scale is still unresolved, F126).

## 3. Events and lessons for the operator

1. **Late start (27 Oct 09:36).** The first sample started 1 h after the plan. The reason was outside the machine (air sampler); the note is in the record. Lesson: write planned and real start times on the sheet.
2. **`actuator_home` between `schedule start` and the first row (27 Oct 13:09).** It reported FAIL, with the manifold 3.69° from slot 0 (tolerance 3.0°), 30 s before the 13:10 row. The lock cycle ran, and the sample ran and ended normally. Lesson: run `actuator_home` before `schedule start`, and check the error is inside ±3°. A FAIL here means the manifold is not where the table says, so look before the next rotation.
3. **A sample ended by `s` (27 Oct 13:22).** `s` skips the pump step; the run goes on with the shield and finishes as `SKIP`, with the volume so far (784 mL). Use `x` to abort a sample, `s` only for a pump step that has to end.
4. **Dummy rows to mark used slots (27 Oct 20:40 to 20:43).** The second tool run (folder `03`) could only say how many samples were loaded (the first N in the loading order), so the slots already used in the morning (1, 2, 3, 14) could not be declared used. The list file held four rows `fix1` to `fix4` (condition `fix`, times 20:40 to 20:43, slots 1, 2, 3, 14) that were started at once and stopped with `s` and `x` within a minute each (four `FAIL` rows, empty purges `auto-9` to `auto-11`). This is a reading of the logs, not a note from Felix. Since then the tool asks which samples are AVAILABLE, so no dummy rows are needed. The four records are not samples: exclude `fix1` to `fix4` from the analysis.
5. **Session date typed one day ahead (27 Oct 20:39).** The date answer was `2026-10-28` for a run started on 27 Oct, so the 20:45 sample carries 28 Oct in its record (its time stamps are right). Lesson: the date in `session` is free text; check it against the clock.
6. **Reusing a used slot (29 Oct).** `run_sample 4` asked to erase slot 4 (marked used in the previous folder) and was answered `y`. The prompt works as intended, but nothing links the earlier sample to the slot afterwards (F229).
7. **Pauses at the cap.** 74 to 89 pauses per lake sample, 36 for the blank, in line with the Lexplore samples (F249).
8. **Vent check.** Every `Vent:` line read `released` (residual pressure 0.01 to 0.09 bar after the wait). The `NOT released` warning never appeared.

## 4. What this adds to the operator tutorial

Applied on 2026-11-06 ([operator-tutorial.md](../field/operator-tutorial.md)): the time per sample without the spool (about 20 min); `actuator_home` before `schedule start` and the ±3° check; `s` versus `x`; the date check at `session`; the AVAILABLE question at the start of every run.

## 5. Plan versus what happened (CoWaS part)

The protocol is a plan; this is what the logs show.

| Item | Protocol | On site |
|---|---|---|
| Water samples per 8 h block | 3 (8:30, 12:00, 15:30; night 20:30, 0:00, 3:30) | River: 09:36 (by hand, 1 h late), 13:10, 16:40; night 20:45, 00:20, 04:10; day 2 08:30, 12:20, 16:10. Gaps of 3.5 to 4 h, set by the schedule file |
| Time per CoWaS sample | about 1 h | 18 to 22 min |
| Purge | 2 to 3 L for blanks (6 L was too much milliQ) | 500 mL for every sample and blank (no spool) |
| Blank | day 2, 1 L through a Sterivex after a 2 to 3 L purge, 4 L of milliQ | River blank on 27 Oct 17:15, lake blank on 29 Oct 12:59, both with the 500 mL purge. The river blank stopped at 414 mL |
| Lake (spatial) | 3 samples back to back 11:00 to 14:00, intake moved a few metres between them | 3 samples 11:55, 12:16, 12:38, then the blank. Whether the intake was moved is not in the logs |
| Cleaning before the trip | Manual: straight tubes in place of the holders, 0.5 L slowly (pump 15 to 20 %), 100 mL per slot, ethanol through the micro pump, 10 min soak of the adapters | Done before the field (not logged). `ethanol_purge` replaced these steps by one command afterwards |
| Prefilter | 100 µm | Not recorded in the logs. Lexplore used a 20 µm prefilter for the bacterial work, changed at every ethanol purge (user, 2026-11-06) |
| Check at the end | Each Sterivex nearly full of DNA Shield (about 2.2 mL) | Not in the logs; added to the operator tutorial |

## 6. Air/water runs at LéXPLORE (3 November)

The air/water project also ran these on the LéXPLORE platform on 3 November, by hand. They are not part of the DIEL study (E1/E2) (folder `2026-11-03/03_felix-lexplore-20261103`). Spool installed, 3 L purge, 1000 mL target.

| Time | Slot | Name | Kind | End | Result | mL sampled (trusted) |
|---|---|---|---|---|---|---|
| 13:36 | 2 | LEX-W1 | sample | clogged | PASS | 152 (86) |
| 13:53 | 1 | LEX-W2 | sample, test | stopped by `x` | FAIL | 128 (124) |
| 14:17 | 3 | LEX-W2-02 | sample | clogged | PASS | 82 (27) |
| 14:49 | 14 | LEX-W3 | sample | clogged | PASS | 882 (784) |
| 15:11 | 13 | LEX-BLANK-3 | blank | clogged | PASS | 113 (90) |

The `x` at 13:52 stopped `LEX-W2` on purpose: it was a test run.

## 7. Open points

- The river blank of 27 Oct (clogged at 414 mL): check the paper sheet for milliQ volume and intake position.
- The paper sheet and the project's own notes are needed to confirm the first sample's start time and the filter IDs.
- Exclude `fix1` to `fix4` and `auto-8` to `auto-11` (stopped or empty purges) from the sample analysis.
- The first sample of 27 Oct (`BC-R-D1-1`, slot 1) is missing from `samples.csv` of its folder (the table starts with `auto-1` of 13:11); its `.json` record and the tests table (PASS, 1000 mL) exist. Check the record before filing.
