# Lab documents

Everything about the filters once they leave the water: how they are handled, how the shield is dosed, and what the data contains.

| Document | For | What it covers |
|---|---|---|
| [filter-sop.md](filter-sop.md) | Operators, on site and in the workshop | Printed checklist: cleaning, loading, unloading, blanks, chain of custody |
| [shield-protocol.md](shield-protocol.md) | Quentin and Felix | How the shield dose is calibrated and checked; the values to set (§8) |

## What CoWaS does with a filter

CoWaS is an autonomous water sampler. A pump pushes lake water through a Sterivex filter held in a rotating manifold (14 sample slots and a purge outlet). After filtration, air pushes out the water in the housing, and DNA/RNA Shield is injected. The filter stays in the machine until someone retrieves it.

For each filter:
1. **Purge:** lake water through the purge outlet, not a filter, flushes the hose (100 mL for now).
2. **Filtration:** lake water through the Sterivex, up to the target volume, or until it clogs, or a time limit. The machine records the volume, the times, the pressure and the reason it stopped.
3. **Air push:** air pushes out the water in the housing. A wet membrane keeps some water (measured in the shield protocol).
4. **Shield:** a timed dose of DNA/RNA Shield, a few minutes after filtration.
5. **Rest:** the filter waits in its slot, sealed, until retrieval.

**Blanks:** at the start and the end of each deployment, the same cycle runs on DNA-free water (equipment blanks). A field blank and manual comparison samples are filtered by hand on site.

**No filter is discarded silently.** A filter with a fault is delivered, flagged.

**Pilots:** Pilot A (supervised, 1–2 days, a pier or boat on Lake Geneva, fixed intake depth), then Pilot B (unattended, 1–2 weeks, 14 filters). See [stage 2](../roadmap/stage2-pilots.md).

## What the data contains

Per filter (one row per filter in the export; the source is the machine, the operator or the paper sheet):

| Field | Source | Unit / format |
|---|---|---|
| Sample name, condition, notes | Operator (`run_sample` asks them). Start and finish time: the machine (PC clock), not typed | Text |
| Site, date, experiment, operator | Operator (`session` asks them, once a day) | Text |
| Slot | Machine | 1–14 |
| Filtered volume: total and confident | Machine (flowmeter) | mL |
| End reason | Machine (`volume`, `clogged`, `stalled`, `pressure`, `timeout`, `stop_key`) | Text |
| Pressure, mean and max | Machine | bar (gauge) |
| Purge volume | Machine | mL |
| Shield dose (time) | Machine (timed) | s |
| Shield dose (mass) | Paper sheet, by weighing | g |
| Filter ID, weights, retrieval time, notes | Paper sheet | Text |

The machine writes one JSON file per pump run; the session log keeps the answers and the console output with PC times. Details: [operator tutorial](../field/operator-tutorial.md), section 4.

## Status

Draft (2026-10-23). The values still to set (with Felix) are in [shield-protocol §8](shield-protocol.md#8-values-to-set-with-felix). Once they are set, the SOP and the shield protocol are the reference.
