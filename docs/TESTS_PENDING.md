# Tests pending hardware

Tests that can't run yet because a piece of hardware is missing, not installed or not trusted. Every step that adds such a test lists it here, with the exact commands, so it can be run the day the hardware arrives (ROADMAP §5, §6 rule 7). When a test has passed, move its row to "Done" with the date and the `test_logs/` file.

## Pending

| Test | Blocked by | Exact commands | Pass criteria |
|---|---|---|---|
| **Is the spare 150 psi ABP (ABPDANV150PGSA3) alive?** Do this first: it decides whether FW3 can be tested before the 060 order arrives | Not done yet: out of time on 2026-09-24 (F50) | **1. Find pin 1** (datasheet Table 8: 6 pins, rows `1 2 3` / `6 5 4`; SPI = 1 GND, 2 Vsupply 3.3 V, 3 SS, 4 NC, 5 MISO, 6 SCLK; **not reverse-polarity protected**). Sensor unplugged, multimeter in diode test: the pin that reads OL against all 5 others in both directions is **pin 4 (NC)**; **pin 1 is diagonally opposite it**. Cross-check: red on 1, black on 2 → a diode drop. If no pin is fully open, stop. **2. Wire:** 1 → Due GND, 2 → Due **3.3V** (never the 5 V pin of the SPI header), 3 → D8, 5 → MISO and 6 → SCK on the 6-pin SPI header; or use the shield's old P1 connector after checking it (3.3 V, D8). **3. Test** (current firmware, no flash): `pio device monitor`, `pressure_sensors`, read the `Trustability` line for 30 s, blow gently into the port, any key stops | Alive: steady ~0.0–0.1 bar at rest, a clear rise when blowing, no or occasional `invalid SPI response` (occasional = F27 stale bits, fixed in FW3). Dead or wiring: the error on every read + 0.00 (recheck 3.3 V at pin 2 and MISO first). Warm sensor = shorted: unplug. Record the result in docs/FIRMWARE_LOG.md (F50) |
| Pressure sensor reads ambient, then a pressure cap trip | **No pressure sensor installed** (F50): needs an ABP on SPI D8 (HW4) + FW3 | Written by FW3 (its prompt: the cap-trip test with an outlet clamp, low pump power, CAP temporarily at 1.0 bar) | `pressure` shows P_in ≈ 0 ± 0.05 bar at rest, no SPI error; the cap trips at 1.0 ± 0.1 bar |
| Every pumping test (`pump`, `sample_cycle*`, `multi_sample_full`, `calibrate_flow`, `cal_DNA`, the purge/sample commands) under supervision of the cap | same (F50). Since FW2 they **refuse** while `PRESSURE_SENSOR_AVAILABLE = false` (Settings.h) | after FW3: set `PRESSURE_SENSOR_AVAILABLE = true`, then as listed in docs/COMMANDS.md | the cap is live (see above) before any closed-outlet or unattended run |
| FW2 `leak_hold` in **sensor mode** (PID water phase + PID air push via `load_slot_water` / `load_slot_air`) | F50 + FW3; `PRESSURE_SENSOR_AVAILABLE = true` | `actuator_home` → `slot5` → `leak_hold 100` → answer the leak question | water phase ends on volume (≈ 100 mL on the flowmeter, factor still uncalibrated F6), no SAFETY line, `leak=no`, `TEST\|leak_hold\|5\|PASS` |
| FW2 `multi_sample` in **sensor mode** (the pre-FW2 `load_slot` sequence per slot) | F50 + FW3 | `actuator_home` → `multi_sample auto count=3` | 3 × `TEST\|multi_sample\|<slot>\|PASS`, no SAFETY line, no leak seen, O-ring UNLOCKED at the end |
| FW2 `full_cycle` (the real `step_purge` → `step_sampling` → `step_DNA_shield` → `step_empty`) | F50 + FW3 (refuses with `SKIP ... reason=no_pressure_sensor` today) | `actuator_home` → `slot1` → `full_cycle 1 volume=100` | `TEST\|full_cycle\|1\|PASS`, `sampled_ml` ≈ 100 (±20 % until HW3 calibrates the flowmeter), purge + empty through slot 0, O-ring UNLOCKED at the end, no SAFETY line |
| FW2 `full_all` over 3 slots | F50 + FW3 | `full_all count=3 volume=100` (pauses between slots; `auto` for none) | 3 × PASS; stops at the first FAIL |
| Production path after boot (FW2 guard): `sample1m` refuses while the lock is UNKNOWN, runs after `actuator_home` | F50 + FW3 | reset → `sample1m` (expect `REFUSED ... UNKNOWN`) → `actuator_home` → `sample1m` + START | first refused with nothing moving; second completes, O-ring UNLOCKED at the end |

## Done

| Test | Date | Result / log file |
|---|---|---|
| | | |
