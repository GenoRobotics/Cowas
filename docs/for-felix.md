# For Felix: where things are and what is left

**Operator guide:** [field/operator-tutorial.md](field/operator-tutorial.md) · **Code map:** [software/code-guide.md](software/code-guide.md) · **Commands:** [software/commands.md](software/commands.md) · **Log tool:** [software/session-log.md](software/session-log.md) · **Stage 1:** [roadmap/stage1-FINISHED-prototype.md](roadmap/stage1-FINISHED-prototype.md)

**Your tasks**

1. **DNA shield dose.** Calibrate with `shield_calibrate` (procedure: [lab/shield-protocol.md](lab/shield-protocol.md)). Then put the dose time in the production run: in `step_DNA_shield()` (`src/Step_functions.cpp`), replace `FILL_STERIVEX_TIME / 10.` (the test shortcut, 25 s) with your value, `DNA_SHIELD_FILL_MS`.
2. **Air push after the shield.** `PUMP_SHIELD_TIME` in `include/Settings.h` (300 ms, placeholder), with `PUMP_SHIELD_POWER` (13 %). The whole push takes about 4.8 s with the ramp.
3. **Air drying of the filter.** `EMPTY_STX_AIR_HOLD_MS` in `include/Settings.h` (40 s, placeholder). The purge drying, `EMPTY_AIR_HOLD_MS` (20 s), is not in your list.
4. **`cal_DNA` cap check** (F202): its pump run does not check the pressure cap. Add the check, or remove the routine.
5. **Tubing:** clean it before your acquisition campaign.

Every value above is in `include/Settings.h` except the shield dose line. Search for the name to find it.
