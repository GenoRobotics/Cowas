# Archive

Superseded documents and removed code, kept for the record. Nothing here is current or compiled.

## Documents

| File | Superseded by | Why kept |
|---|---|---|
| [roadmap-stage2-prompts-2026-10-23.md](roadmap-stage2-prompts-2026-10-23.md) | [stage 2](../roadmap/stage2-pilots.md) §7 | The step prompts of stage 2 as they were on 2026-10-23. The useful ideas are in the step cards |
| [roadmap-stage3-prompts-2026-10-23.md](roadmap-stage3-prompts-2026-10-23.md) | [stage 3](../roadmap/stage3-full-system.md) | The step prompts of stage 3 as they were on 2026-10-23. The useful ideas are in the step cards |
| [roadmap-stage2-details-2026-10-23.md](roadmap-stage2-details-2026-10-23.md) | [stage 2](../roadmap/stage2-pilots.md) | The full detailed sections of stage 2 before it was made concise (power table, findings F1–F36, step table, bench checks, parts and pins) |
| [roadmap-v1.md](roadmap-v1.md) | [stage 2](../roadmap/stage2-pilots.md) and [stage 3](../roadmap/stage3-full-system.md) | The first full plan (2026-09). Its links to files that were never committed are dead on purpose |
| [roadmap-stage1-2026-10-15.md](roadmap-stage1-2026-10-15.md) | [stage 1 roadmap](../roadmap/stage1-FINISHED-prototype.md) | The long prompts and checklists of the old stage 1 |
| [roadmap-stage1-2026-10-23.md](roadmap-stage1-2026-10-23.md) | [stage 1 roadmap](../roadmap/stage1-FINISHED-prototype.md) | The full stage 1 plan of 2026-10-23, before it was cut to the operator test |
| [memo-prehistory.md](memo-prehistory.md) | The log, "Prehistory" section | The original memo on the hardware and firmware before FW0 |
| [renames-2026-10-21.md](renames-2026-10-21.md) | The current file names | Following an old link or a memory |
| [sensor-notes-2026-09.md](sensor-notes-2026-09.md) | The order list in [stage 2 §11.1](../roadmap/stage2-pilots.md) | Condensed purchase notes: pressure sensors, flowmeters |

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
