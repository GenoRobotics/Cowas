# Sensor purchase notes (2026-09, condensed)

Condensed on 2026-10-21 from three HTML notes (pressure sensor, outlet flowmeter, sensor additions). The HTML files were deleted. Prices are September 2026, excluding shipping. The order status is in [stage 2 §11.1](../roadmap/stage2-pilots.md).

## 1. Pressure sensors

**Proposal:** 3 × Honeywell ABPDANV060PGSA3 (about 45 € each, about 135 € total). One after the filter, one to replace the upstream sensor, one spare. One model in both positions means one spare part.

**Why:** the same family as the installed ABPDANV150PGSA3 (same case, SPI bus, 3.3 V, code and tubing), but 0–4.1 bar instead of 0–10.3 bar.

**Requirements:**

| Requirement | Target | Reason |
|---|---|---|
| Range | ≥ 3.5 bar | Safety at 2.5 bar, adjustable to 3 bar, plus margin |
| Accuracy on ΔP (before − after filter) | ≤ ±0.15 bar | Clogging measure; the two errors add up |
| Fluid | Water and air | The cycle alternates filtering and air emptying |
| Logic | 3.3 V | The Due accepts no 5 V on its pins |
| Overpressure | Above the pump's maximum (closed valve) | Survive a blockage |
| Fitting | Fits our tubing; holds 2.5 bar | The installed sensor already popped out |

**Comparison:**

| | Installed 150 psi | Proposed 60 psi | Industrial MIP 6 bar |
|---|---|---|---|
| Range | 0–10.3 bar | 0–4.1 bar | 0–6 bar absolute (about 0–5 bar gauge) |
| Total error (0–50 °C) | ±1.5 % FS = ±0.16 bar | ±1.5 % FS = ±0.06 bar | ±1.0 % FS = ±0.06 bar |
| ΔP error (two sensors) | ±0.31 bar raw; about ±0.05 bar with the calibrations in 2b | ±0.12 bar raw; about ±0.02 bar with the calibrations | ±0.12 bar |
| Water and air | Yes (option V) | Yes (option V) | Yes (welded stainless) |
| Output | 3.3 V SPI | 3.3 V SPI | 5 V analogue 0.5–4.5 V (needs a divider) |
| Overpressure | 17 bar | 17 bar | 24 bar |
| Fitting | Grooved port on the board | Same as installed, plus a clamp | G1/4 stainless, IP67 |
| Integration | None: same code, bus, tubing | New fitting, divider, zero re-set (absolute) | |
| Price | about 45 € | about 45 € | about 85 € |

The 60 psi part meets the ΔP goal with no calibration. The 150 psi part meets it only with the calibrations in 2b. The 150 psi part stays as the fallback if the 60 psi supplier fails.

The MIP is only worth it if the grooved port keeps popping out even with a clamp.

**2b. Two software calibrations (recommended, no extra hardware):**
- Auto-zero before each sample, with the pump off and the lines at air. This removes the zero error and its temperature drift.
- Cross-calibration at commissioning: outlet closed, pump at 2 bar, both sensors must read the same. Correct the gain of one against the other.
- Resolution is not the limit (14 bit, about 0.3 mbar per step).

**3. Wiring:** the SPI bus is already there (MISO, SCK), shared with the manifold encoder. Each sensor adds one chip-select: D8 (before the filter, already wired) and D45 (free in the code, after the filter). No analogue pin used. About 3 mA per sensor.

**4. Check before ordering:**
- SPI cable length: above about 30 cm, use twisted or shielded cable and a lower SPI clock. This is also the likeliest cause of the "invalid SPI response" on the installed sensor. If the after-filter sensor must be far from the Due, the analogue MIP is better.
- D45 is reachable on the shield PCB.
- Pump maximum with the valve closed: must stay under 17 bar (very likely for our diaphragm pump).
- Exact part: ABPDANV060PGSA3 = DIP case, axial grooved port, liquid option with diagnostics, 0–60 psi gauge, SPI, 3.3 V. Not the …5 (5 V) or …AA… (analogue) versions. Off-distributor suppliers: ask for the certificate of origin (counterfeit risk).
- Order 3 clamps for the grooved ports.

**Sources:** Honeywell ABP datasheet (ranges, ±1.5 % total error, 250 psi overpressure, liquid option); newark.com (about $49, 060PGSA3); Honeywell MIP datasheet (MIPAG1XX006BAAAX, ±1.0 %, min load 2 kΩ, IP67); vikiwat.com (about 85 €).

## 2. Outlet flowmeter

**Proposal:** measure our real flow first (1 hour), then order from the result.

**Why measure first:** the installed MW-FS-2.0 measures only from 0.15 L/min. At the end of a clog, the flow probably falls below that. The sensor then under-reads or stops, exactly when it is needed. Ordering without the measurement risks buying again in weeks.

**Requirements:**

| Requirement | Target | Reason |
|---|---|---|
| Minimum flow | ≤ flow at end of clog (to measure) | Exact volume, leak detection to the end |
| Maximum flow | ≥ clean-filter and purge flow (to measure) | No saturation |
| Accuracy | ≤ ±5 % | Volume per sample (metadata); inlet/outlet comparison |
| Same model at inlet and outlet | Yes | Direct comparison (leaks, merged measures) |
| Signal | 3.3 V or open collector | Due accepts no 5 V |
| Fitting | 6 mm tube | Our tubing |

**Comparison:**

| | Installed MW-FS-2.0 | BIO-TECH FCH-m-POM-LC (low flow) | Sensirion SLF3S-4000B (high end) |
|---|---|---|---|
| Principle | Turbine | Turbine | Thermal, no moving parts |
| Range | 0.15–1.0 L/min | 0.015–1.0 L/min (1.0 mm nozzle) | 0–0.6 L/min |
| Accuracy | ±5 % | Not published (ask) | ±5 % up to 0.3 L/min, ±10 % above |
| Pulses | About 4,620 per L | About 10,000 per L | Digital I²C |
| Supply / signal | 2.5–24 V; output = supply voltage, so run it at 3.3 V | 5–24 V, open-collector NPN, needs a 3.3 V pull-up | 3.2–3.8 V, I²C 3.3 V |
| Max pressure | Not stated | 4 bar | 3 bar (burst 5.5) |
| Temperature | Up to 80 °C | −20 to 80 °C | +5 to 50 °C (cold water out of spec) |
| Unfiltered water | Yes | Yes | No: thin channel, after the filter only |
| Air (emptying) | Counts false volume (handled in software) | Same | No rotation; flags the air |
| Price | about 4 € | about 20–40 € (to confirm) | about 150 € |

**Decision rule (after the measurement):**
- Minimum measured ≥ 0.2 L/min: keep the MW-FS-2.0. Order 2 (outlet and spare), about 8 €. Best value, nothing changes.
- Minimum measured < 0.15 L/min (most likely with a clogging filter): BIO-TECH FCH-m-POM-LC with the 1.0 mm nozzle, 3 units (inlet, outlet, spare; same model in both places), about 60–120 €. Same 6 mm tube and wiring.
- Sensirion: only for lab-grade volume at the outlet, if the budget allows. After the filter only (water is clean there), never at the inlet. Limits: 3 bar, no water below 5 °C, fittings to adapt.

**Measure before ordering:**
1. Clean filter, pump at working pressure (2 bar): time the filling of a measuring cylinder → maximum flow.
2. Same during purge (no filter) → purge flow.
3. Muddy water until clogging: note the flow every 5 min → useful minimum flow.

**Wiring:**
- The inlet meter is on D13. The outlet meter goes on D12 (already planned in the code for a second meter) or D36. All Due pins accept the interrupts used for counting.
- Voltage (check now): the MW-FS-2.0 outputs a signal equal to its supply voltage. Powered at 5 V it puts 5 V on D13, which damages the Due over time. Power it at 3.3 V (it works from 2.5 V). The BIO-TECH open-collector output needs a pull-up to 3.3 V.
- Installation: at the outlet the meter must stay full of water. Mount it low, with the tube rising after it, in the direction of the arrow.

**Sources:** tinytronics.nl (MW-FS-2.0: range, 77 Hz per L/min, 2.5–24 V, ±5 %, about 4 €); btflowmeter.com and rapidonline.com (BIO-TECH FCH-m-POM-LC); Sensirion SLF3S-4000B datasheet; digikey.co.uk (price).

## 3. Sensor additions (summary of the reasoning)

- **Pressure after the filter:** today only the pressure before the filter is measured, so a clogged filter looks like a pinched tube or a closed valve. A sensor after the filter gives the drop across the filter. It tells when to stop a clogged sample, records how clogged each filter was (metadata), and shows a torn filter (no drop) or a blocked outlet. It sits on a tee after the filter, with a clamp on the barb (where the installed sensor came off).
- **Flow after the filter:** with an inlet and an outlet meter, we record the volume that went through (eDNA results are per litre), detect leaks (in but not out), and confirm clogging (flow falls while the drop rises). The model must match the inlet meter. Small turbine meters under-read at low flow, which is when a filter clogs.
