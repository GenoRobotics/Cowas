# CoWaS ↔ lab interface

**Status:** draft template (LA1, 2026-09-25). Fill in the **Answer** column during the meeting, then copy the values that later steps need into §4 ("Decisions").

| | |
|---|---|
| Lab | `<lab name>` |
| Contact | `<name, e-mail, phone>` |
| Meeting date | `<YYYY-MM-DD>` |
| Attendees (CoWaS) | `<names>` |
| Attendees (lab) | `<names>` |

**Before the meeting:** send the lab the one-page summary in §6 ("Sterivex handling in CoWaS"). The questions below assume they have read it.

**How to use this file:**
- Every question has an ID (e.g. `B2`). The SOPs, the firmware settings and the host export cite these IDs, so don't renumber them. Add new questions at the end of a section.
- "What we know / propose" is our starting position. The lab may overrule any of it.
- "Feeds" is the step or setting that uses the answer (ROADMAP §7). An answer marked **blocking** is needed before that step can start.
- Write "don't know" rather than leaving a cell empty. Then add an action to §5.

---

## 1. Context in five lines

CoWaS is an autonomous water sampler. A pump pushes lake water through Sterivex filters held in a rotating manifold (14 sample slots + 1 purge slot). After filtration, the machine pushes air through the filter and then injects DNA/RNA Shield. The filters stay in the machine until someone retrieves them.
- **Pilot A** (week 8, about 2026-11): supervised, 1–2 days, from a pier or a boat on Lake Geneva, fixed intake depth. It includes equipment blanks, a field blank and manual comparison samples. The filters go to the lab.
- **Pilot B** (weeks 12–16): unattended, 1–2 weeks, 14 filters.
- The question Pilot A answers: **does the machine give the same eDNA result as a person filtering by hand, with clean blanks?**

---

## 2. Questions

### A. Filters and extraction

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| A1 | Which extraction kit will you use for the Sterivex filters? | e.g. a Sterivex-specific kit (Qiagen DNeasy PowerWater Sterivex) or a kit made for Shield-preserved samples (Zymo). No preference on our side. | | LA2, LA3 |
| A2 | Does that kit accept a Sterivex filled with DNA/RNA Shield directly, or does the protocol need a change (e.g. drain the shield first, open the housing, add the shield to the lysis buffer)? | We fill the housing with Shield in the field; we don't open the housing. | | LA3 (**blocking**) |
| A3 | Which Sterivex model? Pore size (0.22 or 0.45 µm), membrane (PES or PVDF)? | Our design assumes 0.22 µm PES (e.g. Sterivex-GP, SVGP01050). The pore size decides how fast the filter clogs and how much water stays in it (F28). | | HW0 #15, LA3, FW4 |
| A4 | Do you supply the filters and the Shield, or do we buy them? Lot numbers to record? | We need ≥ 40 Sterivex for bench tests + the pilot, and ≥ 14 × the target shield volume + ~100 mL for calibration (ROADMAP §11.1 #15). | | HW0 #15 |
| A5 | Do you also want to extract RNA (eRNA), or DNA only? | DNA/RNA Shield preserves both. RNA may change the storage rules (D-section). | | D1–D4 |

### B. DNA/RNA Shield and residual water

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| B1 | Target Shield volume per Sterivex (mL)? | Usually ~1–2 mL for a Sterivex (ROADMAP §3.7 rule 3). The machine doses by time; we calibrate mL/s in LA3. | | LA3, FW4 `DNA_SHIELD_FILL_MS` (**blocking**) |
| B2 | Minimum acceptable Shield volume (mL)? Below it, the filter is flagged. | | | LA3 pass/fail |
| B3 | Maximum acceptable residual water in the filter before the Shield goes in (mL), or the minimum Shield:water ratio? | The air push cannot dry a wet membrane (F28), so some lake water stays in the housing. We will weigh it (LA3). A common rule for liquid samples is at least 1:1 Shield:sample; is that the right rule here? | | LA3 pass/fail (**blocking**) |
| B4 | Maximum time from the end of filtration to the Shield, before the sample is compromised? | The machine records it per filter (`t_end_to_shield_s`); today it is ~2 min. | | FW8 warning threshold |
| B5 | If a filter loses Shield while it waits in the machine (drains or evaporates), what loss is acceptable (e.g. % per day)? | Retention is measured in LA3 (1, 3, 7 days). | | LA3 pass/fail |

### C. Filtered volume

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| C1 | Target filtered volume per filter? | 2 L proposed. It depends on turbidity: in autumn, Lake Geneva surface water may clog a 0.22 µm Sterivex before 2 L. | | FW4 `volume_ml`, HO3 |
| C2 | Minimum usable volume? Below it, is the filter still extracted, or discarded? | The machine stops early when the filter clogs (`end_reason = clogged`) or on a timeout, and records the real volume. | | FW8, HO2 flag |
| C3 | Should manual comparison samples match the machine volume exactly, or the same "until clogged" rule? | Proposal: same target volume as the machine, recorded. | | VA2 |
| C4 | Accuracy needed on the volume (± %)? | The flowmeter is not calibrated yet (F6); HW3 targets < 3 % spread, and VA1 cross-checks by weighing. | | HW3, VA1 |

### D. Storage and transport

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| D1 | How long may a filter stay in the machine after the Shield, and in which temperature range? | Pilot A: up to ~2 days. Pilot B: up to ~14 days. Inside a closed box on a pier, the temperature may reach `<25–30>` °C in the sun. The Shield maker claims room-temperature stability; is that enough for you? | | LA3 retention test temps, VA2, VA4 (**blocking** for Pilot B) |
| D2 | Transport after retrieval: temperature (cool box + ice packs? dry ice?), light, maximum time until the lab? | Proposal: capped both ends, labelled, zip bag per filter, cool box with ice packs, dark, in the lab the same day. | | LA2 |
| D3 | Storage at the lab until extraction: temperature, maximum time? | | | LA2 |
| D4 | Filter orientation during transport (upright, flat)? Must the Shield stay inside, or can it drain into the bag? | | | LA2 |
| D5 | Delivery: address, opening hours, who receives the filters, weekends? | | | LA2, VA2 |
| D6 | Filters loaded but not used in a deployment (they sat in the machine, open at the outlet): discard them, or send them as extra handling blanks? | Proposal: discard, recorded as `unused` on the chain-of-custody form. | | LA2 |
| D7 | Caps at retrieval: new caps from a sealed bag, or the original caps kept since loading? | Proposal: new caps (the originals sat open in the workshop). | | LA2, HW0 #15 |

### E. Labels and sample IDs

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| E1 | `sample_id` format? Does your LIMS impose one? | Proposal (≤ 40 characters, protocol limit): `<device>-D<deployment>-<slot>-<UTC start>`, e.g. `CW01-D03-03-20261015T0930`. Blanks and manual samples get the same pattern with a role code, e.g. `CW01-D03-EB1-...`, `CW01-D03-MAN2-...`, `CW01-D03-FB-...`. | | HO2 (**blocking**), LA2 |
| E2 | Label type: freezer- and ethanol-proof cryo labels? Size (a Sterivex housing is small)? Barcode or QR code? | We have not bought labels yet (HW0 #15). | | HW0 #15, LA2 |
| E3 | Who prints the labels, and when (before loading, or at retrieval)? | Proposal: the host prints the labels from the plan before the deployment; the slot number is on the label. | | HO2, LA2 |

### F. Blanks and negative controls

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| F1 | Equipment blanks per deployment: how many, when? | Proposal (ROADMAP §3.7 rule 4): one at the start (before any lake water) and one at the end. DNA-free water pumped from a bottle through the whole machine, with the normal cycle. | | VA2 plan, HO3 |
| F2 | Volume of an equipment blank: the same as a sample (2 L), or less? | Each blank also needs the purge first (≥ 3 × the dead volume, ~0.5 L for a 5 m hose). At 2 L per blank: ~5 L of DNA-free water for 2 blanks. **ROADMAP §9.1 says "≥ 3 L": too low if blanks are 2 L.** | | VA2 packing list, LA2 |
| F3 | Source of the DNA-free water (commercial molecular-grade, or lab-made)? Who supplies it, in which containers? | | | LA2, VA2 |
| F4 | Field blank: how many, and how (DNA-free water through a manual Sterivex on site, same Shield)? | Proposal: 1 per deployment, at any time. | | VA2, LA2 |
| F5 | Extraction blanks and PCR negatives: your standard? Do you need anything from us for them? | We assume the lab handles these. | | — |
| F6 | What counts as a "clean" blank (a read threshold, a Ct cut-off)? What do you do with a sample when its blank is not clean? | | | K1 |
| F7 | End equipment blank: the intake hose tip has been in the lake. How should we treat its outside before it goes into the blank bottle (rinse with DNA-free water? bleach, which could destroy DNA and hide contamination?) | Proposal (SOP_FILTERS §6): rinse the outside of the last ~30 cm with DNA-free water away from the bottle, wipe, no bleach. | | LA2 |

### G. Positive controls

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| G1 | Positive control for Pilot A: none, or a mock community / a spike added at the lab? | Proposal: none in the machine for the pilots (automated controls are P2). The lab's own PCR positives only. | | VA2 |
| G2 | Later: which positive control would you want the machine to run (a spike of a species absent from the lake, a synthetic DNA)? | P2; we ask now so the fluidics can plan for it. | | ROADMAP_LATER |
| G3 | Could you run a qPCR carry-over test for us: a DNA spike (a species absent from the lake) through one slot, then qPCR on the next filter? | Optional part of LA4 (carry-over between samples). A dye test comes first. | | LA4 |

### H. Metadata

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| H1 | Go through the column table in §3: which columns do you need, under which names, units and formats? | We export one CSV row per filter. | | HO2 (**blocking**) |
| H2 | Do you follow a metadata standard (FAIRe, MIxS, your own template)? Can you send it? | A full mapping is P2; for the pilots we only need your column names. | | HO2, P2 |
| H3 | File format and delivery: CSV (separator, decimal mark, encoding), Excel, your LIMS import? | Proposal: CSV, comma, `.` decimal, UTF-8, times in UTC ISO 8601. | | HO2 |

### I. Chain of custody

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| I1 | Do you have a chain-of-custody form? Can we use it? | If not, we draft one in LA2: one line per filter, `sample_id`, retrieval time, who handed over, who received, temperature on arrival, condition (cap on, Shield visible, damage), signatures. | | LA2 |
| I2 | Who hands over, who receives, and what is checked at reception? | | | LA2 |
| I3 | What travels with the filters: a paper form, the CSV, the field log, photos of the manifold? | Proposal: the paper form with the filters; the CSV, logs and photos by e-mail the same day. | | LA2, VA2 |

### J. Markers and assays

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| J1 | Which markers or assays (e.g. 12S fish, COI invertebrates, 16S bacteria, species-specific qPCR such as quagga mussel)? | | | C1 |
| J2 | Does the marker choice change the target volume, the pore size or the replicate count? | | | C1, A3, K2 |
| J3 | PCR inhibitors (humic acids, turbidity): anything we should record in the field (turbidity, water colour, temperature)? | We can record water temperature (a thermometer in Pilot A) and notes. | | HO2 columns, FW9 |

### K. Pilot A plan and success criteria

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| K1 | What does a successful Pilot A mean for you? | Proposal: blanks clean (F6); machine and manual replicates of the same timepoint detect comparable taxa (define "comparable": e.g. same species list above a read threshold, or overlap ≥ x %). ROADMAP §9.1 adds, on our side: ≥ 12 of 14 cycles without a fault, no safety event. | | VA2 (**blocking**) |
| K2 | Sampling plan (ROADMAP §9.1): 1 equipment blank + 4 timepoints × 3 machine replicates + 1 equipment blank = 14 slots; plus 2–3 manual comparison timepoints × 1–3 replicates and 1 manual field blank. Change anything? | Replicates can be cut if time is short; blanks cannot. | | VA2, HO3 |
| K3 | How many filters can you extract and sequence for Pilot A (the manual ones included), and the turnaround time for results? | Up to ~14 machine + ~9 manual + 1 field blank ≈ 24 filters. | | VA2 |
| K4 | Does someone from the lab come to the site, or to the retrieval? | | | VA2 |
| K5 | Pilot B: how many samples per day (1 or 2), and does anything change in D-section for 14 days in the machine? | | | VA4 |

### L. Cleaning and biosecurity

| ID | Question | What we know / propose | Answer | Feeds |
|---|---|---|---|---|
| L1 | Decontamination of the wetted parts and the manifold faces between deployments: bleach concentration and contact time, then DNA-free water rinse? | Proposal: 10 % bleach (~0.5 % sodium hypochlorite from a 5 % stock), 10 min, DNA-free water rinse, dry. Bleach must not stay in contact with the metal parts: rinse well. | | LA2 (**blocking**) |
| L2 | Anything else the lab wants: UV, ethanol, a separate clean box for loading, lab coats? | Loading happens in a workshop, not a clean room. | | LA2 |
| L3 | Biosecurity (quagga mussels in Lake Geneva): does the lab have a rule for equipment moving between water bodies? | We clean, rinse and dry every wetted part before any other water body (ROADMAP §3.7 rule 8). | | LA2 |

---

## 3. Metadata columns (one row per filter)

Start: ROADMAP §3.6 and the machine's sample record (docs/PROTOCOL.md §6). "Source": **M** = measured by the machine, **H** = host software, **O** = entered by the operator. The last three columns are for the lab.

| Our column (proposed) | Unit / format | Source | Meaning | Needed? (Y/N) | Lab's column name | Lab's unit / format |
|---|---|---|---|---|---|---|
| `sample_id` | text ≤ 40 (E1) | H | unique filter ID, on the label | | | |
| `deployment_id` | text | H | one deployment = one trip to a site | | | |
| `device_id` | text (`cowas-V1`) | M | which machine | | | |
| `slot` | int 1..14 (0 = purge, never a filter) | M | manifold position | | | |
| `role` | `sample`, `equipment_blank`, `field_blank`, `manual_sample` | H/O | what the filter is | | | |
| `replicate` | int 1..n | H | replicate index within a timepoint | | | |
| `planned_time_utc` | ISO 8601 UTC | H | scheduled start | | | |
| `start_time_utc`, `end_time_utc` | ISO 8601 UTC | H | real filtration start / end | | | |
| `site` | text | O | site name | | | |
| `lat`, `lon` | decimal degrees, WGS84 | O | entered by hand | | | |
| `depth_m` | m | O | fixed intake depth | | | |
| `volume_ml` | mL | M | filtered volume (flowmeter) | | | |
| `duration_s` | s | M | filtration time | | | |
| `end_reason` | `volume_reached`, `clogged`, `duration_reached`, `timeout`, `pressure_cap`, `fault`, `stopped` | M | why filtration ended (§3.7 rule 2) | | | |
| `p_in_max_bar`, `p_in_mean_bar` | bar (gauge) | M | pressure upstream of the filter | | | |
| `flow_mean_ml_min`, `flow_min_ml_min` | mL/min | M | flow during filtration (a falling flow = clogging) | | | |
| `purge_ml` | mL | M | lake water flushed before this filter | | | |
| `shield_ms` / `shield_ml_est` | ms / mL (from the LA3 calibration) | M | Shield injected | | | |
| `t_end_to_shield_s` | s | M | end of filtration → Shield | | | |
| `retrieval_time_utc` | ISO 8601 UTC | O | filter taken out of the machine (§3.7 rule 6) | | | |
| `water_temp_mean_degC` | °C | M/O | when a probe is installed; a thermometer reading in Pilot A | | | |
| `faults`, `warnings` | codes, `;`-separated | M | anything that went wrong; a flagged filter is still delivered | | | |
| `filter_lot`, `shield_lot` | text | O | consumable lots | | | |
| `operator` | text | O | who loaded / retrieved | | | |
| `fw_version`, `host_version` | text | M/H | software versions | | | |
| `notes` | free text | O | weather, leaks, anything unusual | | | |
| *(lab adds)* | | | | | | |

---

## 4. Decisions (fill in after the meeting)

The values later steps read. Copy them from the answers above; write "open" if not decided.

| Value | Decided | From | Used by |
|---|---|---|---|
| Extraction kit, and "Shield in the housing" OK? | | A1, A2 | LA2, LA3 |
| Sterivex model / pore size | | A3 | HW0, FW4 |
| Target Shield volume (mL) | | B1 | LA3 → `DNA_SHIELD_FILL_MS` (FW4) |
| Minimum Shield volume (mL) | | B2 | LA3 |
| Maximum residual water (mL) or minimum Shield:water ratio | | B3 | LA3 |
| Shield retention limit (% per day) | | B5 | LA3 |
| Target filtered volume (mL) | | C1 | FW4, HO3 |
| Minimum usable volume (mL) | | C2 | FW8, HO2 |
| Maximum time in the machine / temperature range | | D1 | LA3, VA2, VA4 |
| Transport and storage conditions | | D2–D4 | LA2 |
| `sample_id` format | | E1 | HO2, LA2 |
| Label type and printing | | E2, E3 | HW0, LA2 |
| Blanks per deployment, blank volume, water source | | F1–F4 | VA2, LA2, HO3 |
| Clean-blank criterion | | F6 | VA2 |
| Positive control for Pilot A | | G1 | VA2 |
| qPCR spike test for LA4 (yes/no) | | G3 | LA4 |
| Metadata columns | | H1–H3, §3 | HO2 |
| Chain-of-custody form | | I1–I3 | LA2 |
| Markers | | J1, J2 | C1 |
| Pilot A success criteria | | K1 | VA2 |
| Pilot A sampling plan | | K2, K3 | VA2, HO3 |
| Bleach % and contact time | | L1 | LA2 |
| Unused filters, caps at retrieval, hose tip for the end blank | | D6, D7, F7 | LA2 |

---

## 5. Actions

| # | Action | Who | Due | Done |
|---|---|---|---|---|
| 1 | Send §6 to the lab before the meeting | | | ☐ |
| 2 | | | | ☐ |

---

## 6. Sterivex handling in CoWaS (one page, for the lab)

*What happens to each filter, from loading to your bench. Machine details are left out; ask us for any of them.*

**The machine.** A pump draws lake water through a hose with a strainer at a fixed depth. The water passes a flowmeter and a pressure sensor, then a rotating manifold that holds 14 Sterivex filters and one purge outlet. A rubber O-ring seals the manifold against the selected filter. A small second pump adds DNA/RNA Shield from a bottle.

**Loading (in the workshop, before the deployment).** Wearing gloves, we clean the manifold faces and the wetted parts (bleach, then DNA-free water rinse; your answer to L1), and screw 14 new Sterivex into the manifold by their luer inlets. We record the filter lot, take a photo, and the software marks the 14 slots as ready.

**For each filter, the machine runs one cycle:**
1. **Purge.** Lake water is pumped through the purge outlet, *not* through a filter: at least 3 × the volume of the hose and pipes (e.g. ~0.5 L for a 5 m hose). This flushes out old water and what is left of the previous sample.
2. **Filtration.** The manifold turns to the filter and seals it. Lake water flows through the Sterivex until the target volume (e.g. 2 L), or until the filter clogs, or a time limit. The machine records the volume, the time and why it stopped.
3. **Air push.** Air pushes out the water that is still in the hose and the housing. It cannot pass through a wet membrane, so **some lake water stays in the filter** (we are measuring how much).
4. **Shield.** DNA/RNA Shield is pumped into the filter (the volume you set in B1), a few minutes after filtration ends. The delay is recorded for each filter.
5. **Rest.** Between cycles the manifold stays sealed, parked on the purge outlet. The used filters wait in their slots until retrieval; the outlet end of each Sterivex is open (caps are being evaluated).

**Blanks.** At the start and at the end of each deployment, the same cycle runs with DNA-free water from a bottle instead of lake water (equipment blanks). Someone also filters DNA-free water by hand on site (field blank). In Pilot A, a person also filters lake water by hand at the same place and time (manual comparison samples).

**Retrieval.** With gloves, the operator unscrews each filter, caps both ends, sticks the printed label on, bags it, and puts it in a cool box with ice packs, in the dark. The retrieval time goes on the chain-of-custody form. The filters reach you with the form; the data file (one row per filter, §3) follows by e-mail.

**What you get per filter:** `sample_id`, role (sample, blank, manual), filtered volume, filtration time, why filtration stopped, pressure and flow, the Shield dose and its delay, faults and warnings, and the times from sampling to retrieval. **A filter with a problem is still delivered, flagged**; we never discard data silently.

**What we have not measured yet (results before Pilot A):**

| Risk | How we measure it | When |
|---|---|---|
| Residual lake water diluting the Shield | weighing filters before and after each stage | LA3 |
| Shield draining or evaporating from filters waiting in the machine for days | weighing after 1, 3, 7 days | LA3 |
| Carry-over: part of the previous sample ending up in the next filter | a dye through the machine, measured at the outlet (optionally a DNA spike + your qPCR, G3) | LA4 |
| Cross-talk between neighbouring filters in the manifold | a dye in one filter, the neighbours checked after the manifold turns | LA4 |
| Filtered volume accuracy | the flowmeter calibrated by weighing the outlet water | HW3, VA1 |

**What we need from you:** the answers in §2, above all the Shield volume (B1, B3), the target volume (C1), the storage limits (D1), the `sample_id` and label format (E), the blank plan (F) and what success means for you (K1).
