# SOP: Sterivex filters in CoWaS

Loading, unloading, cleaning, biosecurity, equipment blanks, chain of custody. **Print this file** and fill it in by hand, one copy per deployment. Every step has a box (☐), a place to write what was recorded, and a **Who** field (initials).

**Status:** draft (LA2, 2026-09-25). Written **before** the lab meeting (docs/LAB_INTERFACE.md §4 is still empty). Every value the lab has not confirmed yet is marked **⟨LAB X⟩**, where X is the question ID in LAB_INTERFACE.md, and shows our proposal. After the meeting, replace each marker with the decided value and delete this paragraph. Section 0.2 lists them all.

**Where the rules come from:** ROADMAP §3.7 (eDNA quality rules), §3.2 (rest state = O-ring locked), §9.1 (Pilot A plan); docs/PROTOCOL.md (commands, sample record); docs/LAB_INTERFACE.md (the lab's answers).

---

## 0. Before you start

### 0.1 Deployment header (fill in once)

| Field | Value |
|---|---|
| `deployment_id` | |
| Site, date | |
| Device | `cowas-V1` |
| Operators (name + initials) | |
| Sterivex model and lot | |
| DNA/RNA Shield lot | |
| DNA-free water: source, lot, bottle IDs | |
| Bleach: product, stock % (on the bottle), dilution date | |
| Firmware hash / host version (`cowas hello`) | |

### 0.2 Values still waiting for the lab

| Marker | What | Our proposal (used in this draft) |
|---|---|---|
| ⟨LAB L1⟩ | bleach concentration and contact time | 10 % household bleach (~0.5 % sodium hypochlorite from a ~5 % stock), **made fresh the same day**, 10 min contact |
| ⟨LAB L2⟩ | extra measures (UV, ethanol, lab coat, clean box) | gloves + a bleached work surface; no UV |
| ⟨LAB L3⟩ | biosecurity rule between water bodies | clean, drain, dry (§4.3) |
| ⟨LAB D2⟩ | transport | cool box + ice packs, dark, at the lab the same day |
| ⟨LAB D3⟩ | storage at the lab | lab's decision |
| ⟨LAB D4⟩ | orientation in transport | upright, inlet (manifold end) up |
| ⟨LAB D5⟩ | delivery address, hours, receiver | |
| ⟨LAB D6⟩ | loaded but unused filters | discarded, recorded on the form as `unused` |
| ⟨LAB D7⟩ | caps at unloading | new caps from a sealed bag (not the ones removed at loading) |
| ⟨LAB E1⟩ | `sample_id` format | `<device>-D<deployment>-<slot>-<UTC start>` (LAB_INTERFACE E1) |
| ⟨LAB E2⟩ | label type, where it goes | cryo label on the housing body, not over a luer end |
| ⟨LAB E3⟩ | who prints labels | printed by the host from the plan before the deployment, 2 per filter (housing + bag) |
| ⟨LAB F2⟩ | equipment-blank volume | the same volume as a sample (proposal 2 L, C1) |
| ⟨LAB F3⟩ | DNA-free water source | commercial molecular-grade water, sealed bottles |
| ⟨LAB F7⟩ | the hose tip for the end blank | rinse the outside with DNA-free water, no bleach (§6.2) |
| ⟨LAB I1–I3⟩ | chain-of-custody form, what travels with the filters | the form in §5; CSV, logs, photos by e-mail the same day |

### 0.3 General rules (read once, apply everywhere)

- **Gloves** (nitrile, powder-free) for every contact with a filter, a cap, the manifold or a wetted part. **Change them** after touching anything else (phone, pen, box, face), between blanks and samples, and whenever torn or wet with lake water.
- **Hands off while the machine can move.** Before putting hands in the manifold: no mission running (`cowas mission status` = paused or none), `cowas status` = `IDLE` (not `BUSY`), **then the E-stop pressed** (24 V off). Release it only when your hands are out.
- **Never pump against a capped outlet.** No pressure sensor is installed yet (F50): the pressure cap cannot trip. A cap left on a filter outlet = a closed outlet. §2 checks this twice.
- **Rest state = the O-ring locked** (ROADMAP §3.2). Leave it locked while loading and unloading, **never turn the rotor by hand**, and counter-hold the manifold when you screw or unscrew a filter. If the rotor moved: write it in the notes, and run `cowas recover` before the next cycle.
- **Bleach safety:** eye protection, ventilation. **Never mix bleach with DNA/RNA Shield or with lysis buffers**: check the Shield SDS. Some preservation and lysis buffers contain guanidine salts, which release toxic gases with bleach. Keep Shield waste and bleach waste in separate containers, and don't bleach the Shield line (§4.2).
- **Times:** write local time with the zone (e.g. `14:32 CEST`). The host stores UTC.
- **Slot numbers:** slot 0 is the purge outlet and **never holds a filter**. The sample slots are the ones `GET_INFO` lists (1–14 on CoWaS V1). The host fills them in `FILL_ORDER` (Settings.h: 1, 2, 3, 14, 13, 12, 11, 10, 9, 8, 4, 5, 6, 7). The slot numbers must be marked on the rotor (permanent marker, one-time job).
- **Photos** go to the deployment folder (`data/pilots/<pilot>_<date>/photos/`, ROADMAP §9.1), named `<deployment_id>_<what>.jpg`.

---

## 1. Before loading: clean the manifold and the wetted parts

In the workshop, the day of loading or the day before. The machine has **no filters** in it. Skip 1.4–1.6 if §4.2 was done after the last deployment **and** the machine has been kept closed and dry since; write "done on <date>, §4.2" instead.

| ☐ | # | Action | Record | Who |
|---|---|---|---|---|
| ☐ | 1.1 | Clean surface: wipe the bench with bleach solution ⟨LAB L1⟩, 10 min, then wipe with DNA-free water. Lay out: gloves, lint-free wipes, the filters (still sealed), new caps (sealed bag), labels, zip bags, waste bags. | time | |
| ☐ | 1.2 | Make the bleach solution fresh ⟨LAB L1⟩: stock % × dilution; label the container with the time. | stock %, dilution, time | |
| ☐ | 1.3 | Gloves on. Machine safe (§0.3): IDLE, no mission, E-stop pressed. | | |
| ☐ | 1.4 | **Wetted path:** flush bleach solution through the purge outlet and through every sample slot, as in §4.2 steps 4.2.3–4.2.8 (bleach, contact time, rinse, DNA-free rinse, air dry). | volumes, contact time | |
| ☐ | 1.5 | **Manifold contact faces** (the faces the O-ring seals on, each slot's luer, the purge outlet): wipe with a lint-free wipe wet with bleach ⟨LAB L1⟩, leave wet for the contact time (10 min), wipe with a wipe wet with DNA-free water, **twice**, then dry with a clean wipe. Bleach must not stay on metal or on the O-ring. | contact time | |
| ☐ | 1.6 | Inspect the O-ring (visible part): no cut, no flattening, no debris. | OK / notes | |
| ☐ | 1.7 | Filter lot recorded in the header (0.1); every package intact and in date. | lot, expiry | |
| ☐ | 1.8 | Change gloves. | | |

---

## 2. Loading

Load **as late as possible** before the deployment: the time in the machine counts against the storage limit ⟨LAB D1⟩.

**Which slots:**
- Pilot A (ROADMAP §9.1) uses all 14 sample slots: 1 start equipment blank + 12 samples + 1 end equipment blank.
- If the plan needs fewer filters, load the **first N slots of `FILL_ORDER`** (the host allocates in that order), and leave the others empty.
- `cowas mission validate` (HO3) must pass with the slots you load: it checks that there are enough READY slots.

**Orientation:** the Sterivex **inlet** (the female luer-lock end) screws onto the slot's luer on the manifold; the **outlet** (the male luer end) stays free, and the water leaves there. Check on the first filter of each lot that the flow direction printed on the housing or the package points away from the manifold.

| ☐ | # | Action | Record | Who |
|---|---|---|---|---|
| ☐ | 2.1 | Machine safe (§0.3): IDLE, no mission, **E-stop pressed**. O-ring locked (rest state). Gloves fresh. | | |
| ☐ | 2.2 | List the slots to load, in `FILL_ORDER`. | slots: | |
| ☐ | 2.3 | For each slot, **one filter at a time**: open the package at the slot → remove the **inlet** cap → screw the inlet onto the slot's luer. Don't touch the luer tips. Keep the outlet cap **on** for now. | tick in the slot table below | |
| ☐ | 2.4 | **Tightness:** finger-tight only, no tool: stop at contact, then ~¼ turn. Counter-hold the manifold. Too tight cracks the housing. Then a gentle pull and a gentle twist: the filter must not move. | tick below | |
| ☐ | 2.5 | When every filter is on: remove **every outlet cap**, the last thing before closing the machine. Count: outlet caps removed = filters loaded. Removed caps go to waste ⟨LAB D7⟩. | caps counted: ___ = filters: ___ | |
| ☐ | 2.6 | **Second person checks:** no cap left on any outlet (a capped outlet = pumping into a closed outlet, F50), every slot number matches the list, nothing loaded on slot 0. | | |
| ☐ | 2.7 | **Photo** of the loaded manifold, slot numbers readable, all outlets visible: `<deployment_id>_loaded.jpg`. | file name | |
| ☐ | 2.8 | Close the machine. Release the E-stop. | time | |
| ☐ | 2.9 | Mark the slots READY in the host: `cowas slots load <slots> --filter-lot <lot>` (HO2; see note). Check that `cowas slots` shows exactly those slots READY. | command output OK | |
| ☐ | 2.10 | Loading time (for the time-in-machine limit ⟨LAB D1⟩). | date, time | |

**Slot table (tick each column):**

| Slot | Inlet on (2.3) | Tight (2.4) | Outlet cap off (2.5) | Checked (2.6) | Planned role (from the plan) |
|---|---|---|---|---|---|
| 1 | ☐ | ☐ | ☐ | ☐ | |
| 2 | ☐ | ☐ | ☐ | ☐ | |
| 3 | ☐ | ☐ | ☐ | ☐ | |
| 14 | ☐ | ☐ | ☐ | ☐ | |
| 13 | ☐ | ☐ | ☐ | ☐ | |
| 12 | ☐ | ☐ | ☐ | ☐ | |
| 11 | ☐ | ☐ | ☐ | ☐ | |
| 10 | ☐ | ☐ | ☐ | ☐ | |
| 9 | ☐ | ☐ | ☐ | ☐ | |
| 8 | ☐ | ☐ | ☐ | ☐ | |
| 4 | ☐ | ☐ | ☐ | ☐ | |
| 5 | ☐ | ☐ | ☐ | ☐ | |
| 6 | ☐ | ☐ | ☐ | ☐ | |
| 7 | ☐ | ☐ | ☐ | ☐ | |

(Rows in `FILL_ORDER`. Another machine: copy its `fill_order` from `GET_INFO`.)

**Leak check:** watch the luer threads during the first cycle (the start equipment blank, §6). Only the active slot's **outlet** may drip; any drip at a **thread** → STOP, tighten, note the slot.

> **Note (host commands):** `cowas slots load` is the HO2 reload flow (not built yet). Until HO2 exists, record the loaded slots and the lot on this sheet only. Before FW7, there is no host at all: bench tests run from the console and this sheet is the only record.

---

## 3. Unloading (retrieval)

As soon as possible after the last cycle (time in the machine ⟨LAB D1⟩). The printed labels, new caps, zip bags, the cool box with ice packs and the chain-of-custody form (§5) are ready **before** you open the machine.

**Order:**
1. The **equipment blanks** first (start blank, then end blank), with fresh gloves.
2. Change gloves. Then the **samples**, in `FILL_ORDER`.
3. Then any **loaded but unused** filters ⟨LAB D6⟩.

Blanks first, so gloves that touched a lake sample never touch a blank.

| ☐ | # | Action | Record | Who |
|---|---|---|---|---|
| ☐ | 3.1 | Host: the mission is finished or paused; `cowas status` = IDLE. Print or copy the list of filters: slot, `sample_id`, role, `end_reason`, warnings. | | |
| ☐ | 3.2 | **Photo** of the manifold before touching anything: `<deployment_id>_before_unload.jpg`. | file name | |
| ☐ | 3.3 | Machine safe (§0.3): **E-stop pressed**. O-ring stays locked. Fresh gloves. | time | |
| ☐ | 3.4 | For each filter, in the order above: **(a)** cap the **outlet** while the filter is still on the manifold; **(b)** counter-hold the manifold and unscrew the filter, keeping it upright (inlet up) ⟨LAB D4⟩; **(c)** cap the **inlet** at once. New caps only ⟨LAB D7⟩. | tick in the table below | |
| ☐ | 3.5 | **Label** from the printed labels ⟨LAB E2, E3⟩: the label's slot must match the slot the filter came from, and its `sample_id` must match the host's list. On the housing body, not over a luer end. The second label on the zip bag. | tick below | |
| ☐ | 3.6 | **Retrieval time** of each filter on the form (§5, column "Retrieved"). Enter it in the host too (`cowas slots unload`, HO2; see note). | tick below | |
| ☐ | 3.7 | Zip bag, one filter per bag, into the **cool box with ice packs, in the dark** ⟨LAB D2⟩, upright. Put a layer (cardboard, a towel) between the filters and the ice packs so nothing freezes. | time the box was closed | |
| ☐ | 3.8 | A filter with a warning, a fault, a leak, a crack or no visible Shield **is still delivered**: write it in the form's "Condition / flags" column. Never discard a filter silently (ROADMAP §6 rule 5). | | |
| ☐ | 3.9 | **Photo** of the empty manifold (`<deployment_id>_unloaded.jpg`) and of all the labelled filters laid out before bagging (`<deployment_id>_filters.jpg`). | file names | |
| ☐ | 3.10 | Count: filters in the cool box + unused = filters loaded (§2.2). | ___ + ___ = ___ | |
| ☐ | 3.11 | Close the cool box; start the chain of custody (§5). Then clean (§4). | time | |

**Unloading table:**

| Slot | Role | `sample_id` (last digits) | Outlet cap (a) | Inlet cap (c) | Label OK (3.5) | Retrieved (time) | In cool box |
|---|---|---|---|---|---|---|---|
| | equipment_blank (start) | | ☐ | ☐ | ☐ | | ☐ |
| | equipment_blank (end) | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |
| | | | ☐ | ☐ | ☐ | | ☐ |

> **Note (host commands):** `cowas slots unload` (enter the retrieval time per filter, `retrieval_time_utc` in the export) is part of HO2 (added to its prompt with this SOP). Until then, the form is the record; copy the times into the host when it exists.

---

## 4. Cleaning after a deployment, and biosecurity

Lake Geneva has **quagga mussels**. Their larvae are invisible and survive in a few drops of water. **Nothing wet leaves the site, and nothing that touched the lake goes into another water body before it is clean and dry** ⟨LAB L3⟩.

### 4.1 At the site, before leaving

| ☐ | # | Action | Record | Who |
|---|---|---|---|---|
| ☐ | 4.1.1 | Run the host's empty stage (`EMPTY`) so the air pushes the water out of the lines through the purge outlet. | | |
| ☐ | 4.1.2 | Pull the intake hose and the strainer out. Inspect: remove mud, plants, shells, anything attached, and **leave them at the site** (in the lake or a bin, never in the car). | notes | |
| ☐ | 4.1.3 | Drain the hose, the strainer, the housing, the waste tray, buckets and ropes **at the site**, back into the same lake. Hold the hose ends low until nothing drips. | | |
| ☐ | 4.1.4 | Wipe the outside of the machine and the hose with dry cloths. Bag the wet gear separately (a marked "lake" bag). | | |
| ☐ | 4.1.5 | Gloves off into the waste bag; wash or sanitize hands. | | |

### 4.2 In the workshop: decontaminate the wetted path

No filters in the manifold. A person stays at the machine for the whole flush (no pressure sensor yet, F50): all outlets open, low pump power, never with a capped outlet.

"Flush a slot" means: the host rotates to that slot and pumps a small volume through it with no filter on (v1: `ROTATE` + `SAMPLE` with no filter, or `PURGE` for slot 0; see the note below). Put a tray under the manifold: the empty slots spray.

| ☐ | # | Action | Record | Who |
|---|---|---|---|---|
| ☐ | 4.2.1 | Gloves, eye protection. Waste tray under the manifold, a separate container for bleach waste. | | |
| ☐ | 4.2.2 | Clean the intake strainer and the outside of the hose with a brush and water; rinse. | | |
| ☐ | 4.2.3 | **Bleach flush:** intake hose in a bucket of fresh bleach solution ⟨LAB L1⟩. `PURGE` (fills the intake, pump and sensors), then flush every sample slot with ~50 mL each (proposal; LA4 may change it). Bucket volume ≥ purge + 50 mL × slots + 1 L margin. | volume used | |
| ☐ | 4.2.4 | **Contact time** with the lines full: 10 min ⟨LAB L1⟩. Pump off. | start, end | |
| ☐ | 4.2.5 | **Water rinse:** hose into clean tap water; `PURGE` ×2, then every slot again. This removes the bleach (it corrodes metal and would destroy the DNA of the next samples). | volume used | |
| ☐ | 4.2.6 | **DNA-free water rinse:** hose into DNA-free water; `PURGE`, then every slot (~50 mL each). | volume, bottle ID | |
| ☐ | 4.2.7 | **Dry:** `EMPTY` (air push), then flush each slot with air. Leave the manifold ports, the purge outlet and both hose ends open to air-dry. | | |
| ☐ | 4.2.8 | Manifold contact faces: as §1.5 (bleach wipe, contact time, DNA-free water wipe ×2, dry). | | |
| ☐ | 4.2.9 | **Not with bleach:** the Shield line (micro pump D48 and its tubing). Rinse it with DNA-free water only if the lab or LA3 asks; otherwise leave it as LA3/`PRIME_SHIELD` decides. Never let bleach reach the Shield bottle. | | |
| ☐ | 4.2.10 | Bleach waste: to the drain with plenty of water, per the workshop's rules. **Never into a lake or a river.** | | |

> **Note (commands):** the host has no one-shot "clean" command yet; HO2 adds `cowas clean` (added to its prompt with this SOP): `PURGE`, then `SAMPLE` through each slot with no filter, then `EMPTY`, pausing for the bucket changes. Until FW7 + HO2, only a person who knows the console runs this flush, following docs/COMMANDS.md and its cautions (rotations ignore the O-ring lock before FW4: F5, F40).

### 4.3 Before any other water body

| ☐ | # | Action | Record | Who |
|---|---|---|---|---|
| ☐ | 4.3.1 | Every part that touched the lake (hose, strainer, pump, lines, manifold, tray, ropes, buckets, boots, the "lake" bag) is **cleaned (4.1, 4.2) and completely dry**. | | |
| ☐ | 4.3.2 | Drying time ≥ **5 days** in dry air (proposal; a common rule for mussel larvae) ⟨LAB L3⟩. Check the local cantonal guidance for anything stricter. Hot water (≥ 60 °C) also kills larvae, but only on parts rated for it (not the pump, the sensors or the O-ring). | dry since: | |
| ☐ | 4.3.3 | Written in the deployment notes: cleaned on, dry since, by. | | |

---

## 5. Chain-of-custody form

One form per cool box. It travels **with the filters**. The CSV export, the device logs and the photos go by e-mail the same day ⟨LAB I3⟩. If the lab has its own form ⟨LAB I1⟩, use theirs and keep this one as a checklist.

**Header**

| Field | Value |
|---|---|
| `deployment_id` / site | |
| Cool box ID, number of ice packs | |
| Number of filters in the box | machine: ___ blanks: ___ manual: ___ field blank: ___ unused: ___ |
| Temperature logger in the box (yes/no, ID) | |
| Destination (lab, address, receiver) ⟨LAB D5⟩ | |

**Filters** (one row per filter; also the manual comparison samples and the field blank, VA2)

| # | `sample_id` | Slot (or MAN / FB) | Role | Retrieved (date, time) | Condition / flags (caps on, Shield visible, crack, warning code) | Checked at lab (initials) |
|---|---|---|---|---|---|---|
| 1 | | | | | | |
| 2 | | | | | | |
| 3 | | | | | | |
| 4 | | | | | | |
| 5 | | | | | | |
| 6 | | | | | | |
| 7 | | | | | | |
| 8 | | | | | | |
| 9 | | | | | | |
| 10 | | | | | | |
| 11 | | | | | | |
| 12 | | | | | | |
| 13 | | | | | | |
| 14 | | | | | | |
| 15 | | | | | | |
| 16 | | | | | | |
| 17 | | | | | | |
| 18 | | | | | | |
| 19 | | | | | | |
| 20 | | | | | | |
| 21 | | | | | | |
| 22 | | | | | | |
| 23 | | | | | | |
| 24 | | | | | | |

Roles: `sample`, `equipment_blank`, `field_blank`, `manual_sample`, `unused` (the last three are proposals, LAB_INTERFACE E1/H1).

**Transfers** (a row each time the box changes hands)

| # | Released by (name, signature) | Received by (name, signature) | Date, time | Temperature in the box (°C) or logger reading | Box sealed / intact? | Notes |
|---|---|---|---|---|---|---|
| 1 | | | | | ☐ | |
| 2 | | | | | ☐ | |
| 3 | | | | | ☐ | |

**Reception at the lab** ⟨LAB I2⟩

| ☐ | Check | Result | Who |
|---|---|---|---|
| ☐ | Filter count matches the header | | |
| ☐ | Temperature on arrival | ___ °C | |
| ☐ | Every label readable and matching a row | | |
| ☐ | Storage started (where, temperature) ⟨LAB D3⟩ | | |
| ☐ | E-mail with the CSV, logs and photos received | | |

---

## 6. Equipment-blank procedure

An equipment blank runs the **normal cycle** (`RUN_CYCLE`: purge, sample, shield, empty) with DNA-free water from a bottle instead of lake water. It tells the lab whether the machine itself adds DNA. Two per deployment (ROADMAP §3.7 rule 4): **one at the start, before any lake water**, and **one at the end**. They cannot be skipped, even when replicates are cut (LAB_INTERFACE K2).

### 6.1 How much DNA-free water

Per blank: the purge volume (≥ 3 × the dead volume: the machine's `purge_ml`, e.g. ~0.5 L for a 5 m hose, ROADMAP §3.7 rule 1) + the blank volume ⟨LAB F2⟩ (proposal: the sample volume, 2 L) + ~0.5 L margin so the tip never sucks air ≈ **3 L per blank**. Two blanks ≈ **6 L**, plus the rinses of §4.2.6 and the field blank. (ROADMAP §9.1 still says "≥ 3 L": too low if blanks are 2 L; see LAB_INTERFACE F2.)

### 6.2 Steps

| ☐ | # | Action | Record | Who |
|---|---|---|---|---|
| ☐ | 6.1 | The mission reaches the blank event and **waits for the operator** (HO3; see note). `cowas status` = IDLE. | time | |
| ☐ | 6.2 | Fresh gloves. Open a **new, sealed** DNA-free water bottle (or a lab container) ⟨LAB F3⟩. Record its lot or ID. Set it upright and stable, near the machine, below the pump if possible. | lot / bottle ID, time opened | |
| ☐ | 6.3 | **Start blank:** the hose has not touched the lake yet (clean since §4.2). **End blank:** pull the hose out of the lake, and rinse the outside of the last ~30 cm with DNA-free water from a squeeze bottle, away from the blank bottle, then wipe it with a clean wipe. No bleach on the tip just before a blank ⟨LAB F7⟩. | | |
| ☐ | 6.4 | Put the hose tip (with its strainer, as for the samples) into the bottle, near the bottom. Cover the bottle neck around the hose (foil or a clean wipe). | | |
| ☐ | 6.5 | Confirm in the host: the event runs with **role `equipment_blank`** on its slot, the **purge first** (the purge uses blank water: that is why 6.1 counts it), then the normal cycle. | slot | |
| ☐ | 6.6 | Watch the whole cycle. The tip must stay under water: if the bottle runs low, add water from another sealed bottle (record it); never let it suck air mid-sample. Watch the luer threads for drips (§2, leak check). | end_reason, volume_ml | |
| ☐ | 6.7 | **Start blank:** move the hose tip to the lake (intake depth, strainer on). **End blank:** leave the hose out; go to §3. | time | |
| ☐ | 6.8 | Confirm in the host that the mission may continue (or ends, after the end blank). | | |
| ☐ | 6.9 | Leftover blank water: discard; never reuse an opened bottle for another blank. | | |

**Record:** the blank's `sample_id`, slot, `end_reason` and `volume_ml` go on the form (§5) like any filter. A blank with a fault or a low volume is still delivered, flagged.

> **Note (host):** HO3's mission runner must **pause before and after each `equipment_blank` event** for the hose move, and resume on the operator's confirmation (added to the HO3 prompt with this SOP). Until HO3 exists, run the blank by hand: `cowas run-cycle --slot <n> --volume <mL>` (HO1), and record its role on this sheet.

---

## 7. Sign-off

| ☐ | Section | Done (date, time) | Who |
|---|---|---|---|
| ☐ | 1. Cleaned before loading | | |
| ☐ | 2. Loaded, photo, slots READY | | |
| ☐ | 6. Start equipment blank | | |
| ☐ | 6. End equipment blank | | |
| ☐ | 3. Unloaded, photos, cool box | | |
| ☐ | 5. Chain of custody started | | |
| ☐ | 4. Cleaned, dry since ___ | | |
