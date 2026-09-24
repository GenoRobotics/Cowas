# Tests pending hardware

Tests that can't run yet because a piece of hardware is missing, not installed or not trusted. Every step that adds such a test lists it here, with the exact commands, so it can be run the day the hardware arrives (ROADMAP §5, §6 rule 7). When a test has passed, move its row to "Done" with the date and the `test_logs/` file.

## Pending

| Test | Blocked by | Exact commands | Pass criteria |
|---|---|---|---|
| Pressure sensor reads ambient, then a pressure cap trip | **No pressure sensor installed** (F50): needs an ABP on SPI D8 (HW4) + FW3 | Written by FW3 (its prompt: the cap-trip test with an outlet clamp, low pump power, CAP temporarily at 1.0 bar) | `pressure` shows P_in ≈ 0 ± 0.05 bar at rest, no SPI error; the cap trips at 1.0 ± 0.1 bar |
| Every pumping test (`pump`, `sample_cycle*`, `multi_sample*`, `calibrate_flow`, the purge/sample commands) under supervision of the cap | same (F50): the cap reads a floating A2 today | after FW3: as listed in docs/COMMANDS.md | the cap is live (see above) before any closed-outlet or unattended run |

## Done

| Test | Date | Result / log file |
|---|---|---|
| | | |
