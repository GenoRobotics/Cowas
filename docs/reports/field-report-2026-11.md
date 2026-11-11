# Field report: Bois Chamblard and LéXPLORE (26 Oct – 6 Nov 2026)

## Summary

- **Aim:** test the auto sampler, with its revised firmware ([revision report](revision-report-2026-11.md)), on real water and in long unattended runs, at two sites.
- **Bois Chamblard (river and lake, 26–29 Oct):** 14 samples, all taken. The scheduled overnight series ran on its own with no failure.
- **LéXPLORE (Lake Geneva, 3–6 Nov):** two 24 h series, a sample every 4 h at 1, 5 and 20 m. **67 of 69 planned samples collected:** 48 filters from the auto sampler (46 samples, 2 blanks) and 19 manual ones for comparison (18 samples, 1 blank).
- **Two samples lost, neither to the sampling program:** one to a wrong slot list typed by the operator, one to a power incident on the platform.
- **What worked:** no leak at the manifold seal, pressure always under the limit (highest 3.44 bar, limit 3.6 bar), clogged filters handled, failed tries recovered on their own.
- **What failed:** the spool (first field use, fixed on site), prototype hardware, and set-up errors.

## 1. The two field works

| | Bois Chamblard | LéXPLORE |
|---|---|---|
| Dates | 26–29 Oct | 3–6 Nov |
| Water | River, then lake | Lake Geneva, 1, 5 and 20 m |
| Depth spool | Not fitted | Fitted (up to 40 m of tube) |
| Purge before each sample | 0.5 L | 3 L |
| How samples started | 8 by the schedule, 6 by the operator | By the schedule (two 24 h series, every 4 h) |
| Controls | 2 blanks | 2 blanks from the auto sampler, 1 manual blank |
| Comparison | Air eDNA sampling (Bois Chamblard project) | 18 manual samples at the same depth and time |

- **Manual samples:** Niskin bottle on the platform winch; 1 L pumped through the filter with a portable peristaltic pump (Vampire Sampler, Bürkle, Huber lab); DNA Shield added by hand.
- The Bois Chamblard project also took 3 samples and a blank at LéXPLORE on 3 Nov, all completed.

## 2. Results

- **Bois Chamblard:** river water filled the filters to the 1 L target; lake water clogged them earlier and those samples stopped on the clog rule with less water. One sample was stopped early by the operator.
- **LéXPLORE:** 46 valid samples and 2 blanks. 16 failed tries in 8 problems, all solved on site (5 spool software, 2 hardware, 1 set-up); after the spool fixes of 3 Nov evening, no failed try used a filter (appendix B).
- **Time per sample:** about 20 min at Bois Chamblard, about 29 min at LéXPLORE (the dive and the 3 L purge add about 10 min) (appendix C).
- **Manual comparison:** each manual sample was taken within −4 to +15 min of the auto sampler's sample, 4.9 min apart on average (appendix D). DNA results are for the lab.
- **Seal and pressure:** no manifold leak over the campaign, and the pressure stayed under the limit in all 155 recorded runs.
- **Unattended operation:** an overnight river series and two 24 h lake series ran on the schedule. Clogged lake filters ended cleanly with a valid sample, failed tries before the filter were retried on their own, and every run has a record and a full log.

## 3. Field feedback

**Spool: first field use, made robust on site**
- On the first day the spool lost its position several times (a false zero, a roll-up timeout, a list that kept going).
- Fixed during the campaign: switch presses are checked against the encoder, the origin is only accepted near zero, the roll-up runs twice, the prime retries, the schedule stops after a failed roll-up. From 3 Nov 22:49 the spool lost no more samples.

**Prototype hardware (repaired on site, to redesign)**
- The pressure sensor came out of its tubing and its short disturbed the manifold encoder. Dried and refitted. Next: bond it, add strain relief, keep it dry.
- The wooden spool parts got wet, swelled and blocked the drum; a loose screw, a slack belt and a cracked printed guide followed. Repaired on site. Next: water-resistant materials and metal fixings.
- The frame can misalign, and then the manifold leaks; the manifold also shifted by three slots twice on 3 Nov (encoder coupling). Next: a stiffer frame and a locked coupling.

**Set-up and handling**
- A wrong slot list was typed at a start (5-6 instead of 4-5), so E1_T5_01M_R1 ran through an empty slot and was lost. The list is now read back before it runs.
- A plastic spacer was fitted instead of the 20 µm prefilter, so the first tries found no flow. A prefilter check is now in the protocol.
- The last filter available was past its expiry date (E1_T7_20M_R1); the sample is flagged. A bent DNA Shield tube leaked at a joint and was fixed.

**Power incident (LéXPLORE, 6 Nov, under investigation)**
- While the manifold was turning after the last 5 m sample, the log stopped and the computer lost the connection. The manifold kept turning until it was tangled in its tubing and cables. The platform's generator started at about the same time. The controller no longer accepts new firmware.
- **Main hypothesis (in progress):** a power drop on the platform stopped the controller while the motor driver stayed on.
- The 20 m and 5 m samples were saved (DNA Shield of the 5 m added by hand); E2_T7_01M_R1 was not taken. Nothing currently stops the motors if the controller itself fails.

**Measurement limits (the samples are valid, the figures need work)**
- **Volume:** the flowmeter reads low at low flow (1250 mL real for 1001 mL counted; 800 mL for 549 mL), so the real volumes are higher than recorded.
- **DNA Shield:** dosed by time, not measured, so the dose varies with the filter's clogging.
- **Pump pauses:** on clogged filters the pump pauses at the pressure limit many times (up to 113 in one sample). Safe, but slow and hard to read in the log.

## 4. Next steps

1. Repair the machine; add a stop that does not depend on the controller, and a safe restart after any power loss.
2. Water-resistant spool parts, a bonded pressure sensor, a stiffer frame with a locked encoder coupling.
3. Flowmeter correction at field flow; a measured DNA Shield dose.
4. Records: mark E1_T5_01M_R1 as not valid; flag E1_T7_20M_R1 (expired filter); correct the label of the E2 T3 1 m sample (typed E1_T3_01M_R1); note the hand-added DNA Shield of E2_T7_05M_R1; ethanol-clean the three slots used last.

## Appendix A. Samples collected

Every LéXPLORE sample is listed in the [sample list](lexplore-sample-list-2026-11.xlsx).

| Series | Planned | Collected | Missing |
|---|---|---|---|
| LéXPLORE E1 (3–4 Nov): 7 time points × 3 depths, replicates at T6 | 24 | 23 | E1_T5_01M_R1 (wrong slot list) |
| LéXPLORE E2 (5–6 Nov): 7 time points × 3 depths, replicates at T1 | 24 | 23 | E2_T7_01M_R1 (power incident) |
| LéXPLORE blanks from the machine (4 and 6 Nov) | 2 | 2 | – |
| LéXPLORE manual samples and manual blank | 19 | 19 | – |
| Bois Chamblard river (27–28 Oct, 1 blank) | 10 | 10 | – |
| Bois Chamblard lake (29 Oct, 1 blank) | 4 | 4 | – |
| Bois Chamblard project at LéXPLORE (3 Nov, 1 blank) | 4 | 4 | – |

## Appendix B. Problems seen in the field

**B1. Failed tries at LéXPLORE** (a problem is one failure, or a run of failures from the same cause, until it was solved)

| When | Cause | Failed tries | How it ended |
|---|---|---|---|
| 3 Nov 16:45–17:35 | Spool lost its position; the schedule went on | 3 | Both samples run again by the operator |
| 3 Nov 20:15 | Prime found no flow | 1 | Restarted, passed |
| 3 Nov 21:22–22:10 | Spool lost its position again (false top-switch press) | 2 | Passed after the fix |
| 4 Nov 04:19–04:34 | Pressure sensor out of its tubing; its short disturbed the encoder | 2 | Sensor dried and refitted, both passed |
| 4 Nov 08:52 | Wrong spool zero from a top-switch press | 1 | Retried by the schedule, passed |
| 5 Nov 16:06–16:59 | Plastic spacer instead of the prefilter; then a drum stall | 5 | Prefilter fitted, passed |
| 5 Nov 19:20 | Wrong spool zero from a top-switch press | 1 | Retried by the schedule, passed |
| 5 Nov 19:53 | Spool drum stalled (wet wood) | 1 | Repaired, passed |

**B2. Other problems at LéXPLORE** (no failed run, or counted elsewhere)

| When | What | Effect | Status |
|---|---|---|---|
| 3 Nov | Manifold plate not square and the manifold shifted by 3 slots, twice | Slot positions recalibrated on site | Open: stiffer frame, locked encoder coupling |
| 3 Nov 13:00 | USB link to the computer dropped during a test run | Session tool ended, reopened 2 min later | Cause unknown |
| 3 Nov | The pump relay can close at a reset or a flash | Known hazard; the pump supply is cut before any reset | Open: pull-down resistor, safe first lines at start-up |
| 3 Nov | The schedule went on after a failed roll-up | Next sample started from an unknown position | Fixed: the schedule stops |
| 3–4 Nov | Spool tube pulled sideways by the current; friction near the up switch | Depth less accurate than the tube length | Open |
| 4 Nov night | Pressure sensor failed and the software did not notice for about 3.5 h | The dead sensor went unnoticed for hours | Sensor refitted; open: detect a flat reading |
| 4 Nov 05:45 | Console froze after the sensor and encoder faults | Reset | Cause unknown |
| 4 Nov 08:02 | Small leak at a DNA Shield tube joint (bent tube) | None on the sample | Fixed on site |
| 4 Nov 08:00 | Wrong slot list typed at the start (5-6 instead of 4-5) | E1_T5_01M_R1 ran through an empty slot and was lost | Fixed: the list is read back |
| 4 Nov 16:15 | Last filter available was past its expiry date (E1_T7_20M_R1) | Sample flagged | Check added to the protocol |
| 4 Nov 17:50 | A manual spool move did not move for 3.8 s | Stopped by the operator | Open |
| 5 Nov | Wooden board of the up switch swelled and jammed it | Drum stalled and the switch stuck | Repaired; replace the wood |
| 5 Nov evening | Wooden-frame screw loosened, drive belt slack, printed guide cracked | Drum could not move | Repaired on site; replace the parts |
| 6 Nov 08:00 and 12:58 | Roll-up reported "home" with the tube still out (false zero) | Checked: later dives and rises had normal distances | Two-pass roll-up added; open: reject a stop far from zero |
| 6 Nov | MilliQ blank reached the pressure margin | Not a fault: the tip left the water at the end of the run | Blank valid |
| 6 Nov 16:55 | Manifold turned until tangled; computer lost the controller | E2_T7_01M_R1 not taken | Under investigation |
| All runs | Pump pauses at the pressure margin, up to 113 times in one sample | Slow, and hard to read in the log | Open: tune the control |
| All runs | Flowmeter reads low; DNA Shield dose not measured | Real volume above the recorded one; dose varies | Open |

**B3. Bois Chamblard**

| When | What | Effect | Status |
|---|---|---|---|
| 27 Oct | First sample started 1 h late (air sampler not ready) | None | Write planned and real times on the sheet |
| 27 Oct 13:09 | `actuator_home` reported FAIL: the manifold was 3.7° from a slot (limit 3.0°) | The sample still ran | Run `actuator_home` before the schedule and check the error |
| 27 Oct 13:22 | A sample ended early with `s` (784 mL) | Partial sample | Use `x` to abort, `s` only to end a pump step |
| 27 Oct 17:15 | River blank stopped on the clog rule at 414 mL | Valid blank, small volume | Cause not recorded |
| 27 Oct 20:40 | Four dummy rows needed to mark slots as used | Rows started and stopped at once | Fixed: the start question lists the free slots |
| 27 Oct 20:39 | Session date typed one day ahead | The 20:45 record carries the wrong date | Check the date against the clock |
| 29 Oct | Lake water clogged all three filters | Samples under 1 L | Expected: lake water, clog rule |

## Appendix C. Time per step (medians of the passed runs)

| Step | LéXPLORE (spool) | Bois Chamblard (no spool) |
|---|---|---|
| Dive | 4.4 min | – |
| Purge | 7.2 min (3 L) | 1.0 min (0.5 L) |
| Filtration and emptying | 13.6 min | 14.8 min |
| DNA Shield | 4.0 min | 4.2 min |
| Whole sample | 28.7 min | 20.1 min |

## Appendix D. Manual and machine samples at LéXPLORE

Offset = time of the manual sample minus the start of the machine's filtration.

| Sample | Manual | Machine | Offset (min) |
|---|---|---|---|
| E1-T6-20M-R1 | 13:20 | 13:16–13:28 | +3.5 |
| E1-T6-20M-R2 | 13:48 | 13:47–13:58 | +0.8 |
| E1-T6-05M-R1 | 14:15 | 14:18–14:27 | −3.6 |
| E1-T6-05M-R2 | 15:00 | 14:46–14:58 | +13.2 |
| E1-T6-01M-R1 | 15:24 | 15:18–15:29 | +5.9 |
| E1-T6-01M-R2 | 15:47 | 15:48–15:58 | −1.6 |
| E1-T7-20M-R1 | 16:13 | 16:15–16:28 | −2.7 |
| E1-T7-05M-R1 | 16:47 | 16:47–16:59 | −0.6 |
| E1-T7-01M-R1 | 17:15 | 17:19–17:30 | −4.4 |
| E2-T1-20M-R1 | 17:21 | 17:20–17:31 | +0.7 |
| E2-T1-20M-R2 | 18:00 | 17:50–18:06 | +9.7 |
| E2-T1-05M-R1 | 18:27 | 18:25–18:33 | +1.0 |
| E2-T1-05M-R2 | 18:52 | 18:52–19:08 | −0.5 |
| E2-T1-01M-R1 | 19:28 | 19:32–19:45 | −4.3 |
| E2-T1-01M-R2 | 20:17 | 20:14–20:25 | +2.4 |
| E2-T2-20M-R1 | 20:50 | 20:42–20:56 | +7.0 |
| E2-T2-05M-R1 | 21:30 | 21:18–21:30 | +11.3 |
| E2-T2-01M-R1 | 22:08 | 21:53–22:05 | +14.7 |

The manual blank was taken on 5 Nov at 15:45.
