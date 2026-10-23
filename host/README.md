# cowas: the CoWaS host (HO1)

Python 3.11+, cross-platform (Windows laptop for Pilot A, Raspberry Pi later). It talks to the Arduino Due with protocol v1: the contract is [software/protocol.md](../docs/software/protocol.md), and the shared test vectors are in [protocol/vectors.json](../protocol/vectors.json).

**Status:** the firmware side of the protocol comes with FW7. Until then, use `--sim` (a simulated Due). A Due running today's firmware speaks the legacy text protocol and will not answer.

## Install

```sh
cd host
python -m venv .venv
.venv/Scripts/pip install -e ".[dev]"      # Windows  (Linux/Pi: .venv/bin/pip)
```

On the laptop the PlatformIO Python works as the base: `%USERPROFILE%\.platformio\python3\python.exe -m venv .venv`. If pip fails with `CERTIFICATE_VERIFY_FAILED`, set `SSL_CERT_FILE` / `PIP_CERT` to a PEM bundle that includes the Windows root store (see CLAUDE.md).

## Use

```sh
cowas ports                              # serial ports; the Due's Native USB port is auto-detected (2341:003E)
cowas hello | status | info
cowas stop                               # safe state, FAULT(USER_STOP)
cowas clear-fault                        # FAULT -> RECOVERY_REQUIRED
cowas recover [--yes]                    # asks before anything moves
cowas rotate 3 | lock | unlock
cowas run-cycle --slot 3 --volume 2000 [--max-duration 1800] [--purge-ml 424] [--skip shield,empty] [--sample-id ID]
cowas run-cycle --slot 3 --no-volume-cap --max-duration 1800
cowas monitor [--duration 60]            # live telemetry, events and device logs
cowas logs [-n 50] [--follow]            # the Due's log lines
```

Every command takes `--sim` (plus `--sim-boot` to start like after a power-on, `--sim-speed N` to slow down to N× real time), `--json`, `-v/-vv/-vvv`, `--port`, `--data-dir`.
Ctrl-C during a long command sends STOP, then fetches the partial result.

Files go under `./data` (or `$COWAS_DATA_DIR`), or `data/sim/` with `--sim`:
- `logs/device.log`: the Due's own log lines, timestamped in UTC on reception, rotated;
- `logs/host.log`: this program; `logs/wire.log`: every protocol line sent and received;
- `results/*.json`: every RUN_CYCLE result, with its request (HO2 replaces this with the sample store).

## Safety rules built in

- **Never 1200 baud** on the Due's ports (it erases the flash): refused in `SerialTransport`.
- The **Programming port** (2341:003D, resets the Due when opened) is refused unless `--bench` (single-cable bench mode).
- Request ids are unique **per host session** (`h<random>-<n>`): the Due's dedupe table outlives a host restart (F70).
- A retry always reuses the **same id** (§7), so a command never runs twice. After a **reboot** (new `boot_id`), pending ids are never resent: the command fails with `DeviceRebooted`, and the slot is SUSPECT.
- The host checks arguments with the same rules as the Due, and the limits from `GET_INFO`, before it sends anything. It never assumes a slot count.

## Layout

| Module | What |
|---|---|
| `cowas.protocol` | constants (`spec`), pydantic models, codec, the Due's request checks (`validate`), vector matching |
| `cowas.transport` | `SerialTransport` (pyserial, VID/PID), line routing (`{` → protocol, the rest and `{"t":"log"}` → device log), `DeviceLog`, `MemoryTransport` |
| `cowas.client` | `CowasClient`: ids, acks, timeouts (§10), same-id resend, heartbeat PING, reconnect, reboot detection, subscriptions |
| `cowas.sim` | `FakeController` (protocol v1 in virtual time) + injectable faults, `SimTransport` |
| `cowas.cli` | the `cowas` command |
| `cowas.adapters` | the only place for Pi-specific code (empty until HO4) |

The client is single-threaded: it pumps the transport while it waits and measures every timeout on the transport's clock, so it runs in real time on the serial port and in virtual time on the simulator. A 30-minute cycle takes about 0.1 s in the tests.

## Tests

```sh
ruff check . && ruff format --check .
pytest                                   # from host/, or `pytest host/` from the repo root
```
