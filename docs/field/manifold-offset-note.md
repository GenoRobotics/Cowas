# Manifold overshoot: recalibrate

Needs: serial console, O-ring unlocked (after reset: `actuator_home`).

1. `rotate_all dry order=fill auto`
2. Average the `err_deg` of the CW moves → `C`, and of the CCW moves → `K`.
3. Set `angle_offset_pos = 1.2 + C` in `src/Hardware/Manifold.cpp` (line 17).
   Set `angle_offset_neg = 1.2 + K` in `include/Manifold.h` (line 63).
4. Build (`pio run -e due`), upload (ask first).
5. Re-run step 1. Pass: every `|err_deg|` < 1.0. If not, go to step 2.

Keep the old value in a comment when you change a number.
