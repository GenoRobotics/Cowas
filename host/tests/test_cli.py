"""The `cowas` CLI with --sim, and the platform-import rule of ROADMAP HO1."""

from __future__ import annotations

import json
import re
from pathlib import Path

import pytest

from cowas.cli import EXIT_FAILED, EXIT_OK, EXIT_USAGE, main

SRC = Path(__file__).resolve().parents[1] / "src" / "cowas"


def run(capsys, tmp_path, *argv: str) -> tuple[int, str]:
    code = main([*argv, "--sim", "--data-dir", str(tmp_path)])
    return code, capsys.readouterr().out


def test_hello(capsys, tmp_path):
    code, out = run(capsys, tmp_path, "hello")
    assert code == EXIT_OK and "cowas-V1" in out


def test_json_output(capsys, tmp_path):
    code, out = run(capsys, tmp_path, "status", "--json")
    assert code == EXIT_OK and json.loads(out)["mode"] == "IDLE"


def test_global_options_before_the_command(capsys, tmp_path):
    code = main(["--sim", "--data-dir", str(tmp_path), "info"])
    assert code == EXIT_OK and "fill" in capsys.readouterr().out


def test_run_cycle_saves_the_result(capsys, tmp_path):
    code, out = run(
        capsys,
        tmp_path,
        "run-cycle",
        "--slot",
        "3",
        "--volume",
        "500",
        "--max-duration",
        "900",
        "--sample-id",
        "T-1",
    )
    assert code == EXIT_OK and "COMPLETED" in out
    saved = list((tmp_path / "sim" / "results").glob("*.json"))
    assert len(saved) == 1
    payload = json.loads(saved[0].read_text(encoding="utf-8"))
    assert payload["sim"] is True
    assert payload["response"]["data"]["sample_id"] == "T-1"
    assert not (tmp_path / "results").exists()  # simulated runs never mix with real ones


def test_run_cycle_no_volume_cap_and_skip(capsys, tmp_path):
    code, out = run(
        capsys,
        tmp_path,
        "run-cycle",
        "--slot",
        "2",
        "--no-volume-cap",
        "--max-duration",
        "120",
        "--skip",
        "shield,empty",
    )
    assert code == EXIT_OK and "duration_reached" in out


def test_host_refuses_bad_arguments(capsys, tmp_path):
    code, out = run(capsys, tmp_path, "run-cycle", "--slot", "0", "--volume", "500")
    assert code == EXIT_USAGE and "nothing sent" in out
    code, out = run(
        capsys, tmp_path, "run-cycle", "--slot", "2", "--volume", "5", "--skip", "rinse"
    )
    assert code == EXIT_USAGE


def test_volume_or_no_cap_is_required(tmp_path):
    with pytest.raises(SystemExit):
        main(["run-cycle", "--slot", "3", "--sim", "--data-dir", str(tmp_path)])


def test_wrong_mode_after_boot(capsys, tmp_path):
    code, out = run(capsys, tmp_path, "rotate", "3", "--sim-boot")
    assert code == EXIT_FAILED and "wrong_mode" in out


def test_recover_needs_confirmation(capsys, tmp_path, monkeypatch):
    monkeypatch.setattr("sys.stdin.isatty", lambda: False)
    code, out = run(capsys, tmp_path, "recover", "--sim-boot")
    assert code == EXIT_FAILED and "actuator_home" in out
    code, out = run(capsys, tmp_path, "recover", "--sim-boot", "--yes")
    assert code == EXIT_OK and out.strip().endswith("IDLE")


def test_stop_and_clear_fault(capsys, tmp_path):
    code, out = run(capsys, tmp_path, "stop")
    assert code == EXIT_OK and "clear-fault" in out
    code, out = run(capsys, tmp_path, "clear-fault")  # a fresh sim is IDLE: refused
    assert code == EXIT_FAILED and "wrong_mode" in out


def test_motion_commands(capsys, tmp_path):
    for argv in (("rotate", "5"), ("unlock",), ("lock",)):
        code, out = run(capsys, tmp_path, *argv)
        assert code == EXIT_OK, out


def test_monitor_and_logs(capsys, tmp_path):
    code, out = run(capsys, tmp_path, "monitor", "--duration", "30", "--sim-speed", "1e9")
    assert code == EXIT_OK
    assert re.search(r"\btel\b.*purge", out) and "evt" in out and "log" in out
    code, out = run(capsys, tmp_path, "logs", "-n", "3")
    assert code == EXIT_OK
    assert len(out.strip().splitlines()) == 3
    assert re.match(r"\d{4}-\d\d-\d\dT", out)


def test_ports_sim(capsys, tmp_path):
    code, out = run(capsys, tmp_path, "ports")
    assert code == EXIT_OK and "sim" in out


def test_no_platform_specific_imports_outside_adapters():
    """ROADMAP HO1: no PyQt, no ZMQ, no Pi-specific imports outside cowas/adapters/."""
    banned = re.compile(
        r"^\s*(from|import)\s+(PyQt\w*|zmq|RPi|gpiozero|smbus\w*|systemd)\b", re.MULTILINE
    )
    for path in SRC.rglob("*.py"):
        if "adapters" in path.parts:
            continue
        assert not banned.search(path.read_text(encoding="utf-8")), path
