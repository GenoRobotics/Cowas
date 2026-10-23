# Filter SOP: Sterivex filters in CoWaS

How to load, unload, clean and label the filters, and how to run blanks. Print it: one copy per deployment, filled in by hand. Each step has a box (☐), the record to write, and **Who** (initials).

Status: draft, 2026-10-23. The values in it are the current working values (open ones: [shield-protocol §8](shield-protocol.md#8-values-to-set-with-felix)). Change a value here when the practice changes, and log it.

---

## 0. Rules

- **Gloves** (nitrile, powder-free) for every contact with a filter, cap, manifold or wetted part. Change them after touching anything else, between blanks and samples, and when wet.
- **Hands out of the manifold while the machine can move.** Before reaching in: no sample running, and the **24 V switch off**. Switch it on only when your hands are out.
- **Never pump against a closed outlet.** A cap left on an outlet is a closed outlet. Check it twice (§2).
- **The O-ring stays locked when idle.** Do not turn the rotor by hand. Counter-hold the manifold when you screw or unscrew a filter. If the rotor moved, note it and run `actuator_home` before the next sample.
- **Bleach:** eye protection and ventilation. **Never mix bleach with the DNA/RNA Shield or with lysis buffers** (some contain guanidine salts, which release toxic gas with bleach).
- **Slot 0 is the purge outlet and never holds a filter.** Sample slots are 1–14. Mark the slot numbers on the rotor.
- **Times:** local time with the zone, e.g. `14:32 CEST`.

---

## 1. Clean the manifold (the day before or the day of loading)

| ☐ | Action | Record | Who |
|---|---|---|---|
| ☐ | Gloves on. 24 V off. Wipe the bench with bleach, 10 min, then DNA-free water | time | |
| ☐ | Flush the wetted path (§4.2) | volumes, contact time | |
| ☐ | Wipe the manifold faces (O-ring seat, each slot's luer, purge outlet) with bleach, 10 min, then DNA-free water twice. Dry | contact time | |
| ☐ | Check the O-ring: no cut, flattening or debris | OK / notes | |
| ☐ | Filter lot and expiry in the header; packages intact | lot, expiry | |

---

## 2. Load

Load as late as possible before the deployment: the time in the machine counts against the storage limit.

- Load the slots the plan lists, in order. Leave the others empty.
- The **inlet** (female luer-lock end) screws onto the slot's luer. The **outlet** (male end) stays free. Check the printed flow direction on each lot.

| ☐ | Action | Record | Who |
|---|---|---|---|
| ☐ | 24 V off. O-ring locked. Gloves fresh | | |
| ☐ | For each slot, one at a time: open the package, remove the **inlet** cap, screw the inlet on. Keep the **outlet** cap on | slot table | |
| ☐ | Tighten finger-tight, then about ¼ turn, counter-holding the manifold. A gentle pull: the filter must not move | slot table | |
| ☐ | When all are on: remove **every outlet cap**, last. Count caps removed = filters loaded | ___ = ___ | |
| ☐ | Second person checks: no cap on any outlet, slot numbers match the plan, nothing on slot 0 | | |
| ☐ | Close the machine. 24 V on | time | |
| ☐ | Record the loaded slots and the filter lot | | |

Slot table: slot · inlet on · tight · outlet cap off · checked · planned role (one row per loaded slot).

Leak check during the first sample: only the active slot's **outlet** may drip. A drip at a **thread**: stop, tighten, note the slot.

---

## 3. Unload

As soon as possible after the last sample. Have the labels, new caps, zip bags, a cool box with ice packs and the chain-of-custody form ready before you open the machine.

**Order:** equipment blanks first (start, then end), then the samples, then any unused filters. Blanks first, so gloves that touched a lake sample never touch a blank.

| ☐ | Action | Record | Who |
|---|---|---|---|
| ☐ | Console idle. 24 V off. O-ring stays locked. Gloves fresh | time | |
| ☐ | For each filter: **(a)** cap the **outlet** while it is still on the manifold; **(b)** counter-hold, unscrew, keep it upright (inlet up); **(c)** cap the **inlet** at once. New caps only | tick | |
| ☐ | Label: the label's slot matches the slot the filter came from. Label on the housing body, not over a luer. A second label on the zip bag | tick | |
| ☐ | Record the retrieval time of each filter on the form | tick | |
| ☐ | One filter per zip bag. Into the cool box with ice packs, in the dark, upright | time box closed | |
| ☐ | A filter with a fault, a leak, a crack or no visible Shield **is still delivered**: note it under "flags" | | |
| ☐ | Count: filters in the box + unused = filters loaded | ___ + ___ = ___ | |

---

## 4. Clean, and biosecurity

Lake Geneva has **quagga mussels**. Their larvae survive in a few drops of water. **Nothing wet leaves the site, and nothing that touched the lake enters another water body until it is clean and dry.**

### 4.1 At the site, before leaving

| ☐ | Action | Who |
|---|---|---|
| ☐ | Empty the lines with the intake out of the water: `full_cycle <slot> purge=y shield=n empty=y volume=50` for the slots used | |
| ☐ | Pull the intake hose and strainer. Remove mud, plants, shells and leave them at the site | |
| ☐ | Drain the hose, strainer, housing and tray **back into the same lake**. Hold the hose ends low until nothing drips | |
| ☐ | Wipe the machine and the hose dry. Bag the wet gear in a "lake" bag | |

### 4.2 In the workshop: decontaminate the wetted path

No filter in the manifold. A person stays at the machine for the whole flush. A tray under the manifold: empty slots spray. Never with a capped outlet.

Flush a slot with no filter fitted: `full_cycle <slot> purge=y shield=n empty=y volume=50`. It purges through the purge outlet, then pumps through the slot. Repeat for each slot used, changing the bucket between steps.

| ☐ | Action | Record | Who |
|---|---|---|---|
| ☐ | Gloves, eye protection, waste tray, a separate bleach-waste container | | |
| ☐ | Clean the strainer and the hose outside with a brush and water; rinse | | |
| ☐ | **Bleach:** intake in fresh bleach. Flush the purge and each slot used (~50 mL each). Bucket ≥ the total + 1 L | volume | |
| ☐ | **Contact time** with the lines full: 10 min, pump off | start, end | |
| ☐ | **Water rinse:** intake in clean tap water; flush the purge and each slot again. Bleach corrodes metal and would damage the next sample's DNA | volume | |
| ☐ | **DNA-free water rinse:** intake in DNA-free water; flush the purge and each slot (~50 mL each) | volume, bottle ID | |
| ☐ | **Dry:** empty the lines; leave the manifold ports, the purge outlet and both hose ends open to air | | |
| ☐ | Manifold faces: as §1 | | |
| ☐ | **No bleach on the shield line** (micro pump and tubing) | | |
| ☐ | Bleach waste to the drain with plenty of water, per the workshop rules. **Never into a lake or river** | | |

### 4.3 Before any other water body

- Everything that touched the lake (hose, strainer, pump, lines, manifold, tray, ropes, buckets, boots, the "lake" bag) is cleaned and **completely dry**.
- Dry for **at least 5 days** in dry air (a common rule for mussel larvae). Check the cantonal rules for anything stricter. Hot water (≥ 60 °C) also kills larvae, on parts rated for it.
- Write in the deployment notes: cleaned on, dry since, by whom.

---

## 5. Chain of custody

One form per cool box. It travels **with the filters**. The data file and the logs go by e-mail the same day.

- **Header:** deployment ID and site · cool box ID and ice packs · filter counts (machine, equipment blanks, field blank, manual, unused) · temperature logger (yes/no, ID) · destination.
- **One row per filter:** `sample_id` · slot (or MAN / FB) · role (`sample`, `equipment_blank`, `field_blank`, `manual_sample`, `unused`) · retrieved (date, time) · condition and flags · checked at the lab (initials).
- **One row per hand-over:** released by · received by · date, time · temperature in the box · box sealed (☐) · notes.
- **At the lab:** filter count matches · temperature on arrival · every label readable and matching a row · storage started.

---

## 6. Equipment blanks

An equipment blank runs the **normal cycle** with DNA-free water from a bottle instead of lake water. It shows whether the machine itself adds DNA. Two per deployment: **one at the start, before any lake water**, and **one at the end**. Never skip them.

**Water:** the purge (100 mL today) + the blank volume (proposal: 2 L) + ~0.5 L margin, so the tip never sucks air. About 3 L per blank, ~6 L for two, plus the rinses of §4.2.

| ☐ | Action | Record | Who |
|---|---|---|---|
| ☐ | Open a new, sealed DNA-free water bottle. Record its lot. Set it upright, near the machine | lot, time opened | |
| ☐ | **Start blank:** the hose has not touched the lake (clean since §4.2). **End blank:** pull the hose out, rinse the last ~30 cm with DNA-free water away from the bottle, wipe. No bleach on the tip | | |
| ☐ | Put the hose tip, with its strainer, into the bottle. Cover the neck around the hose | | |
| ☐ | Run the sample from the console: `run_sample <slot>` with the condition `equipment blank` | slot, time | |
| ☐ | Watch the cycle: the tip stays under water. If the bottle runs low, add water from another sealed bottle and note it | sample ID, end reason, volume | |
| ☐ | **Start blank:** move the tip to the lake (intake depth, strainer on). **End blank:** leave the hose out | time | |
| ☐ | Leftover blank water: discard. Never reuse an opened bottle | | |

Record the blank like any filter (§5), with the role `equipment_blank`.
