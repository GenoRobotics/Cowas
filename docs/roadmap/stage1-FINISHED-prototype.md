# CoWaS roadmap, stage 1: the prefield test

**Goal:** the operator runs samples, one command per sample, and the records come back.
**Status (2026-10-23):** the prefield tests have passed. Stage 1 is done apart from the items below. Done steps are in [LOG.md](../logs/LOG.md).

---

## Left to do

1. **Felix: clean the tubing** before his acquisition campaign.
2. **Felix: DNA shield dose and air timing** ([for-felix.md](../for-felix.md)).
3. **Commit** the stage 1 work.
4. **Field.** Operator guide: [field/operator-tutorial.md](../field/operator-tutorial.md).

---

## Limits to know in the field

- **Stop rules:** a `SAFETY` line, pressure above 3.6 bar, or a seal leak stops the run. Cut the 24 V switch, then send the log. Someone stays at the 24 V switch for every sample.
- **The vent never stops a sample.** If the line stays above 0.3 bar after the vent, the log says `NOT released` and the run goes on.
- **The flowmeter reads almost nothing at slow flow.** Use `trusted_ml`, not `sampled_ml`.
- **The manifold sometimes does not turn** (F203, open). Check the cable connector first. Details in stage 2 §7b.
- **Not in the record:** filter ID, weights and shield amount. These go on the paper sheet. The shield is timed, not measured.
- **The records need the session tool:** `tools\session_log.cmd` ([software/session-log.md](../software/session-log.md)).
