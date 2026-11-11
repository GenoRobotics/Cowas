# Manual sampling protocol (LéXPLORE, 2026)

Hand sampling for comparison with the CoWaS autosampler. Each manual sample is paired with the automatic sample of the same depth and time point ([LéXPLORE log §33](../logs/field-lexplore-geneva-2026-11-03.md), sheet [manual-sampling-lexplore-2026-11.csv](../logs/manual-sampling-lexplore-2026-11.csv)). Part of the [LéXPLORE field protocol](lexplore-field-protocol.md).

**Labels.** `E1-T6-20M-R1`: E1 is the first 24 h run, T6 the time point, 20M the depth (20 m; 05M 5 m; 01M 1 m), R1 the primary sample, R2 the replicate. Both methods were run in duplicate at all three depths, at one time point per run: E1 T6 and E2 T1.

**Time points.** T1 16:00, T2 20:00, T3 00:00 (midnight), T4 04:00, T5 08:00, T6 12:00 (noon), T7 16:00. Manual samples were taken at E1 T6 and T7, and at E2 T1 and T2. There was no manual sample at 08:00. The campaign has 18 manual samples and one manual blank (`E2_manual_blank`, 5 Nov, 15:45), which is kept with the manual samples and labelled as a blank.

## Materials

- Niskin bottle and electric winch (the platform's own equipment)
- Bucket, sterilised before use
- Manual pump: Vampire Sampler (Bürkle, Huber lab)
- Syringe, for the ethanol flush
- 70 % ethanol
- Sterivex filter, 0.22 µm PVDF, with caps (the same type as the CoWaS)
- DNA Shield, 1 mL per filter
- Labels, labelled bags, and the paper sheet (sample name, time, depth)
- Gloves

## Steps

1. Take the water sample with the Niskin bottle and the winch, at the desired depth.
2. Sterilise the bucket, then flush it with 1 L of the sample water.
3. Rinse the pump quickly with ethanol, then pump some air out.
4. Pump 1 L of sample water from the Niskin through the pump, **with no filter fitted**. This purges the pump and the line.
5. Fit the filter.
6. Pump another 1 L of sample water through the filter, then pump some air.
7. Unscrew the filter, keeping it horizontal.
8. Pump ethanol through the syringe, getting all the excess out.
9. Put 1 mL of DNA Shield through the filter.
10. Put on the caps.
11. Label the filter and place it in a labelled bag.

**Filtered volume: 1 L** (step 6). Step 4 is a purge with no filter, so 2 L of water are pumped in total.

## What differs from the CoWaS

- **Volume:** the manual filter gets 1 L, the same target as the CoWaS. But the CoWaS volume is read from its flowmeter, which under-reads at this flow (F254): the real CoWaS volume can be about 20 % larger than recorded.
- **DNA Shield:** the manual filter gets 1 mL, measured by hand. The CoWaS adds its shield with a fixed-time pump (190 s in each full run; the volume is not measured).
- **Timing:** the manual sample is taken at the time on the sheet. The CoWaS sample is taken during its filtration window (filter start to end, in the log).
