# Tests pending hardware

Tests that can't run yet because a piece of hardware is missing, not installed or not trusted. Every step that adds such a test lists it here, with the exact commands, so it can be run the day the hardware arrives (ROADMAP §5, §6 rule 7). When a test has passed, move its row to "Done" with the date and the `test_logs/` file.

## Pending

| Test | Blocked by | Exact commands | Pass criteria |
|---|---|---|---|
| **Is the spare 150 psi ABP (ABPDANV150PGSA3) alive?** Do this first: it decides whether FW3 can be tested before the 060 order arrives | Not done yet: out of time on 2026-09-24 (F50) | **1. Find pin 1** (datasheet Table 8: 6 pins, rows `1 2 3` / `6 5 4`; SPI = 1 GND, 2 Vsupply 3.3 V, 3 SS, 4 NC, 5 MISO, 6 SCLK; **not reverse-polarity protected**). Sensor unplugged, multimeter in diode test: the pin that reads OL against all 5 others in both directions is **pin 4 (NC)**; **pin 1 is diagonally opposite it**. Cross-check: red on 1, black on 2 → a diode drop. If no pin is fully open, stop. **2. Wire:** 1 → Due GND, 2 → Due **3.3V** (never the 5 V pin of the SPI header), 3 → D8, 5 → MISO and 6 → SCK on the 6-pin SPI header; or use the shield's old P1 connector after checking it (3.3 V, D8). **3. Test** (current firmware, no flash): `pio device monitor`, `pressure_sensors`, read the `Trustability` line for 30 s, blow gently into the port, any key stops | Alive: steady ~0.0–0.1 bar at rest, a clear rise when blowing, no or occasional `invalid SPI response` (occasional = F27 stale bits, fixed in FW3). Dead or wiring: the error on every read + 0.00 (recheck 3.3 V at pin 2 and MISO first). Warm sensor = shorted: unplug. Record the result in docs/FIRMWARE_LOG.md (F50) |
| Pressure sensor reads ambient, then a pressure cap trip | **No pressure sensor installed** (F50): needs an ABP on SPI D8 (HW4) + FW3 | Written by FW3 (its prompt: the cap-trip test with an outlet clamp, low pump power, CAP temporarily at 1.0 bar) | `pressure` shows P_in ≈ 0 ± 0.05 bar at rest, no SPI error; the cap trips at 1.0 ± 0.1 bar |
| Every pumping test (`pump`, `sample_cycle*`, `multi_sample*`, `calibrate_flow`, the purge/sample commands) under supervision of the cap | same (F50): the cap reads a floating A2 today | after FW3: as listed in docs/COMMANDS.md | the cap is live (see above) before any closed-outlet or unattended run |

## Done

| Test | Date | Result / log file |
|---|---|---|
| | | |
