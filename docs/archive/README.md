# Archive

Superseded documents and removed code, kept for the record. Nothing here is current or compiled.

## Documents

| File | Superseded by | Why kept |
|---|---|---|
| [roadmap-stage2-prompts-2026-10-23.md](roadmap-stage2-prompts-2026-10-23.md) | [stage 2](../roadmap/stage2-pilots.md) §7 | The step prompts of stage 2 as they were on 2026-10-23. The useful ideas are in the step cards |
| [roadmap-stage3-prompts-2026-10-23.md](roadmap-stage3-prompts-2026-10-23.md) | [stage 3](../roadmap/stage3-full-system.md) | The step prompts of stage 3 as they were on 2026-10-23. The useful ideas are in the step cards |
| [roadmap-stage2-2026-11-12.md](roadmap-stage2-2026-11-12.md) | [stage 2](../roadmap/stage2-pilots.md) (reorganised 2026-11-12) | Stage 2 as it was after the field work, before the reorganisation into a software lane and a hardware lane. Holds the old step cards (FW4–FW9, HO2–HO5, HW0–HW10, PW1 full text, VA1–VA4) and the old §7b list. The mapping old → new is in stage 2 §10 |
| [roadmap-stage3-2026-11-12.md](roadmap-stage3-2026-11-12.md) | [stage 2](../roadmap/stage2-pilots.md) (the upgrades) and [stage 3](../roadmap/stage3-full-system.md) (the V2 design) | Stage 3 with its P2-1 … P2-19 steps, before the split by "upgrade of the current machine" or "needs a new design". The mapping is in stage 3 §5 |
| [roadmap-stage2-details-2026-10-23.md](roadmap-stage2-details-2026-10-23.md) | [stage 2](../roadmap/stage2-pilots.md) | The full detailed sections of stage 2 before it was made concise (power table, findings F1–F36, step table, bench checks, parts and pins) |
| [roadmap-v1.md](roadmap-v1.md) | [stage 2](../roadmap/stage2-pilots.md) and [stage 3](../roadmap/stage3-full-system.md) | The first full plan (2026-09). Its links to files that were never committed are dead on purpose |
| [roadmap-stage1-2026-11-05.md](roadmap-stage1-2026-11-05.md) | [stage 1 roadmap](../roadmap/stage1-FINISHED-prototype.md) (rewritten 2026-11-12 as the record of the work done) | The last "to do" version of stage 1: its "Left to do" list, the timed-list field test and the field limits |
| [roadmap-stage1-2026-10-15.md](roadmap-stage1-2026-10-15.md) | [stage 1 roadmap](../roadmap/stage1-FINISHED-prototype.md) | The long prompts and checklists of the old stage 1 |
| [roadmap-stage1-2026-10-23.md](roadmap-stage1-2026-10-23.md) | [stage 1 roadmap](../roadmap/stage1-FINISHED-prototype.md) | The full stage 1 plan of 2026-10-23, before it was cut to the operator test |
| [memo-prehistory.md](memo-prehistory.md) | The log, "Prehistory" section | The original memo on the hardware and firmware before FW0 |
| [renames-2026-10-21.md](renames-2026-10-21.md) | The current file names | Following an old link or a memory |
| [sensor-notes-2026-09.md](sensor-notes-2026-09.md) | The order list in [stage 2 §11.1](../roadmap/stage2-pilots.md) | Condensed purchase notes: pressure sensors, flowmeters |

## Reports (`reports/`)

| File | Superseded by | Why kept |
|---|---|---|
| [field-report-2026-11.md](reports/field-report-2026-11.md) (+ docx) | [robustness update report](../reports/robustness-update-report-2026-11.md) §4 | The field report of 2026-11-11: problems per run (its appendix B), time per step, the manual pairing table |
| [revision-report-2026-11.md](reports/revision-report-2026-11.md) (+ docx) | [robustness update report](../reports/robustness-update-report-2026-11.md) §2–3 | The revision report of 2026-11-11; the docx holds the user's edits of 2026-11-12, the model for the new report. Its unit-test suite table (appendix C) is not in the new report |

## Field plans (`field-plans/`)

The plans written before the 2026 field work, kept verbatim as the user supplied them. They are context only: the field logs are the record of what happened on site.

| File | Superseded by | Why kept |
|---|---|---|
| [bois-chamblard-field-protocol-2026-10.md](field-plans/bois-chamblard-field-protocol-2026-10.md) | [Bois Chamblard field log](../logs/field-bois-chamblard-2026-10.md) §5 (plan versus site) | The October plan for the Bois Chamblard campaign (air and water). The air sampling is out of scope for the CoWaS work |
| [lexplore-first-step-project-description.md](field-plans/lexplore-first-step-project-description.md) | [Lexplore field log](../logs/field-lexplore-geneva-2026-11-03.md) §32 and the [field report](reports/field-report-2026-11.md) | The initial project description of the L'eXPLORE study. Its design changed a lot on site (depths, time points, sample count) |
| [lexplore-field-protocol-2026-11.md](field-plans/lexplore-field-protocol-2026-11.md) | [Lexplore field log](../logs/field-lexplore-geneva-2026-11-03.md) §32 | The field protocol for the 2 to 6 November L'eXPLORE expeditions. Its own caveat: "not exactly accurate but still useful" |

## Old step IDs (roadmap v1 → stage 2 and 3)

Used when an old note cites a step ID from roadmap v1.

| v1 | Now | Note |
|---|---|---|
| S0.0 | FW2 | Plus endurance test and `TEST\|` logging |
| S0.1 | FW0 | The command ledger became [commands.md](../software/commands.md) |
| S0.2 | FW4 | Plus purge volume, flow factor, air-push logging |
| S0.3 | FW3 | Moved up: on the critical path |
| S1.1 | Stage 3 P2-1 | Restructure postponed |
| S1.2 | FW5 | |
| S1.3 + S1.4 | FW6 | Lean modes |
| S2.1 | Stage 3 P2-2 | Calibrations stay in `Settings.h` |
| S2.2 | FW1 | Moved to week 1 to unblock the host |
| S2.3 | FW7 | Plus boot into host mode |
| S3.1–S3.5 | Stage 3 P2-3 to P2-7 | FW8 keeps a minimal clog end condition |
| S3.6 | FW9 (water temperature) and stage 3 P2-8 | |
| H1 | HO1 | |
| H2 + H3 | HO2 | FAIRe/MIxS and merge → stage 3 P2-12 |
| H4 + H5 | HO3 | Commissioning workflow → stage 3 P2-13 |
| H6, H7 | Stage 3 P2-9, P2-10 | CLI over SSH for the pilots |
| H8 | HO5 (lite) and stage 3 P2-11 | |
| H9 | HO4 | |
| H10 | HO5 | |
| I1 | VA1 | |
| I2 | FW7 | |
| I3 | Stage 3 P2-14 | |
| I4 | Stage 3 P2-18 | |
| HW0–HW10 | HW0–HW9 | Reorganised. Power and field fluidics added |

## Code

[code/](code/README.md): removed firmware code, sorted into `reusable/` (may be useful again), `superseded/` (replaced), and `snapshots/` (whole files before a clean-up). The index in that folder says what each item was and why it was removed.

## Tests

Removed tests are in `code/superseded/tests/`.
