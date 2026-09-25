# Tests pending hardware

Tests that can't run yet because a piece of hardware is missing, not installed or not trusted. Every step that adds such a test lists it here, with the exact commands, so it can be run the day the hardware arrives (ROADMAP §5, §6 rule 7). When a test has passed, move its row to "Done" with the date and the `test_logs/` file.

## Pending

| Test | Blocked by | Exact commands | Pass criteria |
|---|---|---|---|
| **Is the spare 150 psi ABP (ABPDANV150PGSA3) alive?** Do this first: it decides whether FW3 can be tested before the 060 order arrives | Not done yet: out of time on 2026-09-24 (F50) | **1. Find pin 1** (datasheet Table 8: 6 pins, rows `1 2 3` / `6 5 4`; SPI = 1 GND, 2 Vsupply 3.3 V, 3 SS, 4 NC, 5 MISO, 6 SCLK; **not reverse-polarity protected**). Sensor unplugged, multimeter in diode test: the pin that reads OL against all 5 others in both directions is **pin 4 (NC)**; **pin 1 is diagonally opposite it**. Cross-check: red on 1, black on 2 → a diode drop. If no pin is fully open, stop. **2. Wire:** 1 → Due GND, 2 → Due **3.3V** (never the 5 V pin of the SPI header), 3 → D8, 5 → MISO and 6 → SCK on the 6-pin SPI header; or use the shield's old P1 connector after checking it (3.3 V, D8). **3. Test** (current firmware, no flash): `pio device monitor`, `pressure_sensors`, read the `Trustability` line for 30 s, blow gently into the port, any key stops | Alive: steady ~0.0–0.1 bar at rest, a clear rise when blowing, no or occasional `invalid SPI response` (occasional = F27 stale bits, fixed in FW3). Dead or wiring: the error on every read + 0.00 (recheck 3.3 V at pin 2 and MISO first). Warm sensor = shorted: unplug. Record the result in docs/FIRMWARE_LOG.md (F50) |
| Pressure sensor reads ambient, then a pressure cap trip | **No pressure sensor installed** (F50): needs an ABP on SPI D8 (HW4) + FW3 | Written by FW3 (its prompt: the cap-trip test with an outlet clamp, low pump power, CAP temporarily at 1.0 bar) | `pressure` shows P_in ≈ 0 ± 0.05 bar at rest, no SPI error; the cap trips at 1.0 ± 0.1 bar |
| Every pumping test (`pump`, `sample_cycle*`, `multi_sample*`, `calibrate_flow`, the purge/sample commands) under supervision of the cap | same (F50): the cap reads a floating A2 today | after FW3: as listed in docs/COMMANDS.md | the cap is live (see above) before any closed-outlet or unattended run |
| **SOP_FILTERS dry run** (LA2): two people follow docs/SOP_FILTERS.md §1–§3 and §6 on the bench, with a printed copy, 14 Sterivex (bench-test filters are fine) and tap water in place of the lake and of the DNA-free water | The lab's answers (LAB_INTERFACE §4), the cap (F50: FW3 + HW4) and, for the host steps, FW7 + HO2/HO3. Loading/unloading alone (§2, §3 without cycles) can run earlier | §1 (faces only), §2 with every check, one `RUN_CYCLE`-equivalent per filled slot (or `sample_cycle_full` on the console before FW7, supervised), §6 once, §3, §5 filled with dummy data, §4.2 once. Time each section | loading 14 filters ≤ 30 min and unloading ≤ 20 min; **0** outlet caps left on; **0** drips at a luer thread over the cycles; **0** label/slot mismatches; every box and "Who" field filled; each unclear step written down and fixed in the SOP |

## Done

| Test | Date | Result / log file |
|---|---|---|
| | | |
