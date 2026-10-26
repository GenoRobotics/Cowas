# For Felix: what is left

Guides: [operator tutorial](field/operator-tutorial.md) · [code map](software/code-guide.md) · [commands](software/commands.md) · [stage 1](roadmap/stage1-FINISHED-prototype.md)

## Decide these first

1. **DNA shield time.** In `step_DNA_shield()` (`src/Step_functions.cpp`) the micro pump runs the full `FILL_STERIVEX_TIME` (250 s). The `/10` test value (25 s, F4) was removed in `a4f7809`. Decide: keep 250 s, or bring `/10` back until you have the real value.
2. **`abort_sample` without purge** (`b4f605e`). The `step_purge()` call in `abort_sample()` is commented out. Decide if an abort without purge is safe for the eDNA rules (ROADMAP §3.7).
   - Not needed: delete the commented block and log the change as a behaviour change.
   - Needed: restore the call.

## Then

3. **DNA shield dose.** Calibrate with `shield_calibrate` ([protocol](lab/shield-protocol.md)). Put your value in `DNA_SHIELD_FILL_MS` in `step_DNA_shield()`.
4. **Air push after the shield.** `PUMP_SHIELD_TIME` (300 ms, placeholder) and `PUMP_SHIELD_POWER` (13 %) in `include/Settings.h`. The whole push takes about 4.8 s with the ramp.
5. **Air drying of the filter.** `EMPTY_STX_AIR_HOLD_MS` (40 s, placeholder) in `include/Settings.h`. The purge drying, `EMPTY_AIR_HOLD_MS` (20 s), is not on your list.
6. **`cal_DNA` pressure cap** (F202). Its pump run does not check the cap. Add the check, or remove the routine.
7. **Tubing.** Clean it before the acquisition campaign.

Values are in `include/Settings.h`, except the shield dose (task 3).

## To finish

1. `git pull origin stage1-field`
2. Make the changes. Run `pio test -e native` (176 must pass) and `pio run -e due` (must build).
3. Add one line per decision to `docs/logs/LOG.md`.
4. Commit, then `git push origin stage1-field`. Tell Quentin when it is pushed.
