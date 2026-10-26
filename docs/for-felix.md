# For Felix: where things are and what is left

**Operator guide:** [field/operator-tutorial.md](field/operator-tutorial.md) · **Code map:** [software/code-guide.md](software/code-guide.md) · **Commands:** [software/commands.md](software/commands.md) · **Log tool:** [software/session-log.md](software/session-log.md) · **Stage 1:** [roadmap/stage1-FINISHED-prototype.md](roadmap/stage1-FINISHED-prototype.md)

**Your tasks**

1. **DNA shield dose.** Calibrate with `shield_calibrate` (procedure: [lab/shield-protocol.md](lab/shield-protocol.md)). Then put the dose time in the production run: in `step_DNA_shield()` (`src/Step_functions.cpp`), the micro pump runs `FILL_STERIVEX_TIME` (250 s) with no `/10` since commit `a4f7809`. Replace it with your value, `DNA_SHIELD_FILL_MS`. Until then, decide whether 250 s is the field value or whether the `/10` (25 s, test shortcut, F4) must come back.
2. **Air push after the shield.** `PUMP_SHIELD_TIME` in `include/Settings.h` (300 ms, placeholder), with `PUMP_SHIELD_POWER` (13 %). The whole push takes about 4.8 s with the ramp.
3. **Air drying of the filter.** `EMPTY_STX_AIR_HOLD_MS` in `include/Settings.h` (40 s, placeholder). The purge drying, `EMPTY_AIR_HOLD_MS` (20 s), is not in your list.
4. **`cal_DNA` cap check** (F202): its pump run does not check the pressure cap. Add the check, or remove the routine.
5. **Tubing:** clean it before your acquisition campaign.
6. **`abort_sample` without purge** (commit `b4f605e`). In `abort_sample()` (`src/Step_functions.cpp`) the `step_purge()` call is commented out ("Not installed anymore"). Decide whether an abort without purge is safe for the eDNA quality rules (ROADMAP §3.7: purge volume, blanks, recorded `end_reason`). If the purge is not needed, delete the commented block and list the change as a behaviour change in the commit message and in `docs/logs/LOG.md` (ROADMAP §6, rule 1). If it is needed, restore the call.

**Before you commit:** `git pull origin stage1-field`, make the changes, run `pio test -e native` (176 tests must pass) and `pio run -e due` (must build), add a line to `docs/logs/LOG.md` for each decision, then `git push origin stage1-field` and tell Quentin.

Every value above is in `include/Settings.h` except the shield dose line. Search for the name to find it.
