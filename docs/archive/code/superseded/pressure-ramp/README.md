# Pressure ramp: soft limit (superseded)

`ramp_soft_limit()` capped the pressure setpoint of a ramp a little below the cap (`cap − overshoot`), so a ramp could not overshoot. Its test is `test_soft_limit()`.

**Removed:** from the working tree before 2026-10-23 (the file was already modified when this session started; the reason is not in the log). The last commit still has it (`d3e2717`, whose message says "remove soft cap"). Recovered here from that commit.

**Replaced by:** the cap check on every pass (`Pump::enforce_pressure_safety`) and the pause at 3.35 bar, which stop the pump instead of shaping the setpoint.

**Reuse:** the setpoint shaping is the idea behind the layered pressure controller of stage 3 (P2-6). Read `ramp_soft_limit.cpp.txt` and `test_soft_limit.cpp.txt` there.
