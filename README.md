# CoWaS

Autonomous eDNA water sampler: firmware for an Arduino Due, with its documentation.

**New here?** Read [docs/README.md](docs/README.md). It explains the project and gives the reading order.

## Quick start

```
pio run -e due                  # build the firmware
pio test -e native              # run the unit tests
pio device monitor              # open the console (9600 baud)
```

Flashing (`pio run -e due -t upload`) is done by hand when a change is ready. The operator's steps are in [docs/field/operator-tutorial.md](docs/field/operator-tutorial.md).

## Folders

| Folder | Content |
|---|---|
| `src/`, `include/` | Firmware: procedures, drivers, pure logic (`src/Core/`), console commands |
| `test/` | Unit tests of the pure logic |
| `tools/` | Operator session log, analysis scripts |
| `host/` | Python host package (for the future laptop side) |
| `protocol/` | Test vectors shared by the firmware and the host |
| `docs/` | Roadmaps, operator and lab documents, reference, log |
