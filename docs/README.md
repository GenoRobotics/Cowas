# CoWaS documentation

CoWaS is an autonomous water sampler for environmental DNA. A pump draws lake water through a Sterivex filter on a rotating manifold, then a preservative (DNA/RNA Shield) is added. The software runs on an Arduino Due.

**Where we are (2026-11-12):** stage 1 and the field campaigns are done (Bois Chamblard, LéXPLORE = Pilot A). The machine is being repaired after the incident of 6 Nov (F257). Next: [stage 2](roadmap/stage2-pilots.md) §1, the upgrades of the current machine up to an unattended pilot. The V2 machine is [stage 3](roadmap/stage3-full-system.md).

## Start here

1. [Project rules and build](../CLAUDE.md): what the machine must never do, and how to build and test.
2. [Roadmap, stage 1](roadmap/stage1-FINISHED-prototype.md): where the project stands, what is done, what is left.
3. [Operator guide](field/operator-tutorial.md): how a session runs.
4. [Code guide](software/code-guide.md): how the firmware is organised.
5. [For Felix](for-felix.md): where things are and what is left for the calibrations.

## Reports (November 2026)

[reports/](reports/README.md): the field report (Bois Chamblard and LéXPLORE), the revision report (what changed since August 2026, and why), the sample list, and a link to the operator tutorial.

## Folders

| Folder | What it holds | Start with |
|---|---|---|
| [roadmap/](roadmap/) | The plan: stage 1 (done), stage 2 (upgrades of the current machine, up to Pilot B), stage 3 (the V2 machine) | [stage1-FINISHED-prototype.md](roadmap/stage1-FINISHED-prototype.md) |
| [hardware/](hardware/) | Pinout, power, the electrical audit | [pinout.md](hardware/pinout.md) |
| [software/](software/) | Firmware reference: console commands, the code, the host protocol, the session log tool | [code-guide.md](software/code-guide.md) |
| [lab/](lab/) | The filters after sampling: the printed SOP and the shield protocol | [README.md](lab/README.md) |
| [field/](field/) | The operator guide for a session on the water | [operator-tutorial.md](field/operator-tutorial.md) |
| [archive/](archive/README.md) | Superseded documents and removed code. Only to trace an old decision | [README.md](archive/README.md) |
| [logs/](logs/) | The project log (chronological), the findings, the decisions, and the transcripts of bench runs | [LOG.md](logs/LOG.md), [findings.md](logs/findings.md), [decisions.md](logs/decisions.md) |

## Conventions

- Findings are `F<number>` (in [findings.md](logs/findings.md)) and decisions are `D<number>` (in [decisions.md](logs/decisions.md)). The chronological log is [LOG.md](logs/LOG.md).
- "ROADMAP §N" in older notes means the stage 2 roadmap.
- Placeholders are marked `PLACEHOLDER` in the code and printed on every test line.
