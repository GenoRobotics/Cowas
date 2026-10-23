# Weighing checkpoints in `fill_test` (parked, removed from the working tree 2026-10-23)

**What it did:** `fill_test checkpoint=N` paused the pump every N mL, waited for the operator to type the weighed grams, and restarted the pump. It printed `CHECKPOINT|n=…;flow_ml=…;weight_g=…;diff_ml=…`, and it used a `FilterWatch` resume (not re-arming the stall windows).

**Why it was removed:** the first use (cal1, 2026-10-23) worked up to the restart. The restart after the first checkpoint climbed past the pressure cap (3.38 → 3.64 bar), because the pump restarted at 15 % on a line with no flow. The operator then weighed by hand instead. The cause is the start rate, which the operation pump now ramps (see `superseded/` notes in LOG F196).

**Why it is kept:** the flow-versus-weight comparison it automated is the calibration of the flowmeter (F6, F150). A future calibration tool can reuse this logic, once the restart is fixed.

**Files here:**
- `read_checkpoint_grams.cpp.txt`: the console reader for the typed grams (returns 0 = grams, 1 = x, 2 = timeout).
- `checkpoint_run_loop_block.txt`: the block inside the `fill_test` run loop that did the pause and the restart.
- The `checkpoint=` argument, its help text and the state variables were removed with it. The full file before removal is in `docs/archive/code/snapshots/2026-10-23-before-removal/Tests_mech.cpp`.
