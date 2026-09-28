"""Tests for tools/analyze_la3.py on a synthetic LA3 dataset (known answers)."""

import csv
import json
from pathlib import Path

import pytest

import analyze_la3 as la3

HEADER = ["filter_id", "group", "stage", "time", "mass_g", "temp_c", "notes"]

# Paired design, group A: dry 8.00 g, residual water 0.40 g, shield 1.50 g
# (liquid0 = 1.90 g), then a steady loss of 0.019 g/day = 1 %/day over 7 days.
# Group F: full cycle only. Filter H01: handling check, 0.01 g per cycle.
T0 = "2026-10-12 10:00"


def synthetic_rows():
    rows = []
    for i, dry in enumerate((8.00, 8.10, 7.90), start=1):
        fid = f"A0{i}"
        rows += [
            [fid, "A-today", "dry", "2026-10-12 08:00", dry],
            [fid, "A-today", "water", "2026-10-12 09:50", dry + 0.40],
            [fid, "A-today", "shield", T0, dry + 1.90],
            [fid, "A-today", "hold", "2026-10-13 10:00", dry + 1.90 - 0.019 * 1],
            [fid, "A-today", "hold", "2026-10-15 10:00", dry + 1.90 - 0.019 * 3],
            [fid, "A-today", "hold", "2026-10-19 10:00", dry + 1.90 - 0.019 * 7],
        ]
    rows += [
        ["F01", "F-warm", "dry", "2026-10-20 08:00", 8.00],
        ["F01", "F-warm", "full", "2026-10-20 10:00", 9.95],
    ]
    return [r + ["", ""] for r in rows]


def write_csv(path: Path, rows, header=HEADER):
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(header)
        w.writerows(rows)
    return path


@pytest.fixture
def masses(tmp_path):
    return write_csv(tmp_path / "masses.csv", synthetic_rows())


def run(masses, th=None, **overrides):
    thresholds = la3.load_thresholds(None, [f"{k}={v}" for k, v in overrides.items()])
    if th:
        thresholds.update(th)
    return la3.analyse(la3.read_rows(masses), thresholds)


def by_id(results):
    return {r.filter_id: r for r in results}


def test_volumes_and_ratio(masses):
    results, h = run(masses)
    a1 = by_id(results)["A01"]
    assert a1.residual_ml == pytest.approx(0.40)
    assert a1.shield_ml == pytest.approx(1.50)
    assert a1.ratio == pytest.approx(3.75)
    assert a1.water_plus_shield_ml == pytest.approx(1.90)
    assert h is None


def test_retention_rate(masses):
    a1 = by_id(run(masses)[0])["A01"]
    assert a1.hold_days == pytest.approx([1, 3, 7])
    assert a1.retained_pct == pytest.approx([99, 97, 93])
    assert a1.loss_pct_per_day == pytest.approx(1.0)
    assert a1.loss_ml_per_day == pytest.approx(0.019)


def test_full_cycle_group(masses):
    f1 = by_id(run(masses)[0])["F01"]
    assert f1.water_plus_shield_ml == pytest.approx(1.95)
    assert f1.residual_ml is None and f1.shield_ml is None
    assert f1.verdict == la3.NA  # nothing to check against


def test_density_changes_shield_volume(masses):
    a1 = by_id(run(masses, shield_density_g_ml=1.25)[0])["A01"]
    assert a1.shield_ml == pytest.approx(1.20)
    assert a1.residual_ml == pytest.approx(0.40)  # water density unchanged


def test_all_open_without_thresholds(masses):
    results, _ = run(masses)
    a1 = by_id(results)["A01"]
    assert set(a1.checks.values()) == {la3.OPEN}
    assert a1.verdict == la3.OPEN


def test_pass_and_fail_boundaries(masses):
    ok = dict(shield_min_ml=1.5, residual_max_ml=0.4, ratio_min=3.75, retention_loss_max_pct_per_day=1.0)
    a1 = by_id(run(masses, **ok)[0])["A01"]
    assert a1.verdict == la3.PASS, a1.checks  # exactly at every limit = PASS

    a1 = by_id(run(masses, **{**ok, "shield_min_ml": 1.51})[0])["A01"]
    assert a1.checks["shield_min"] == la3.FAIL
    a1 = by_id(run(masses, **{**ok, "residual_max_ml": 0.39})[0])["A01"]
    assert a1.checks["residual_max"] == la3.FAIL
    a1 = by_id(run(masses, **{**ok, "ratio_min": 3.8})[0])["A01"]
    assert a1.checks["ratio_min"] == la3.FAIL
    a1 = by_id(run(masses, **{**ok, "retention_loss_max_pct_per_day": 0.9})[0])["A01"]
    assert a1.checks["retention"] == la3.FAIL
    assert a1.verdict == la3.FAIL


def test_delivered_fraction(masses):
    a1 = by_id(run(masses, shield_target_ml=2.0)[0])["A01"]
    assert a1.delivered_pct == pytest.approx(75.0)


def test_handling_loss_is_added_back(tmp_path):
    rows = synthetic_rows() + [
        ["H01", "H-handling", "dry", "2026-10-12 08:00", 8.0, "", ""],
        ["H01", "H-handling", "handling", "2026-10-12 11:00", 9.90, "", ""],
        ["H01", "H-handling", "handling", "2026-10-12 11:05", 9.89, "", ""],
        ["H01", "H-handling", "handling", "2026-10-12 11:10", 9.88, "", ""],
    ]
    results, h = run(write_csv(tmp_path / "m.csv", rows))
    assert h == pytest.approx(0.01)
    a1 = by_id(results)["A01"]
    # each hold weighing k gets k x 0.01 g back: day 7 = 1.90 - 0.133 + 0.03
    assert a1.retained_pct[-1] == pytest.approx(100 * (1.90 - 0.133 + 0.03) / 1.90)


def test_zero_residual_gives_infinite_ratio(tmp_path):
    rows = [
        ["Z1", "g", "dry", "2026-10-12 08:00", 8.0, "", ""],
        ["Z1", "g", "water", "2026-10-12 09:00", 8.0, "", ""],
        ["Z1", "g", "shield", "2026-10-12 10:00", 9.5, "", ""],
    ]
    z = by_id(run(write_csv(tmp_path / "m.csv", rows), ratio_min=1)[0])["Z1"]
    assert z.ratio == float("inf")
    assert z.checks["ratio_min"] == la3.PASS


@pytest.mark.parametrize(
    "bad_row, message",
    [
        (["A01", "A-today", "dry", "2026-10-12 08:00", 8.0], "appears 2 times"),
        (["A01", "B-long", "hold", "2026-10-20 10:00", 9.0], "several groups"),
        (["X1", "g", "weird", "2026-10-12 08:00", 8.0], "unknown stage"),
        (["X1", "g", "dry", "2026-10-12 08:00", "abc"], "mass_g must be a number"),
        (["X1", "g", "dry", "2026-10-12 08:00", -1], "mass_g must be a number"),
        (["X1", "g", "hold", "", 8.0], "needs a time"),
        (["X1", "g", "dry", "12/10/2026", 8.0], "bad time"),
        (["X2", "g", "water", "2026-10-12 08:00", 8.0], "no 'dry' weighing"),
        (["A01", "A-today", "hold", "2026-10-12 09:00", 9.0], "not after the start"),
    ],
)
def test_bad_input_is_refused(tmp_path, bad_row, message):
    rows = synthetic_rows() + [bad_row + ["", ""]]
    with pytest.raises(la3.InputError, match=message):
        run(write_csv(tmp_path / "m.csv", rows))


def test_missing_column(tmp_path):
    path = write_csv(tmp_path / "m.csv", [["A", "g", "dry", 8.0]], header=["filter_id", "group", "stage", "mass_g"])
    with pytest.raises(la3.InputError, match="missing columns: time"):
        la3.read_rows(path)


def test_thresholds_file_and_overrides(tmp_path):
    repo_file = Path(__file__).resolve().parents[1] / "la3_thresholds.json"
    th = la3.load_thresholds(repo_file, [])
    assert set(th) == set(la3.THRESHOLD_KEYS)
    assert all(v is None for v in th.values())  # nothing decided yet: keep it that way until LA1

    f = tmp_path / "t.json"
    f.write_text(json.dumps({"shield_min_ml": 1.2, "_note": "x"}))
    th = la3.load_thresholds(f, ["ratio_min=1", "shield_min_ml=null"])
    assert th["ratio_min"] == 1.0 and th["shield_min_ml"] is None
    with pytest.raises(la3.InputError):
        la3.load_thresholds(None, ["nonsense=1"])
    with pytest.raises(la3.InputError):
        la3.load_thresholds(None, ["shield_density_g_ml=0"])


def test_main_exit_codes_and_report(masses, tmp_path, capsys):
    assert la3.main([str(masses)]) == 0
    out = capsys.readouterr().out
    assert "OVERALL: OPEN" in out
    assert "PLACEHOLDER" in out  # the default density is flagged

    out_csv = tmp_path / "res.csv"
    assert la3.main([str(masses), "--set", "shield_min_ml=2", "--csv", str(out_csv)]) == 1
    assert "OVERALL: FAIL" in capsys.readouterr().out
    with out_csv.open() as f:
        rows = list(csv.DictReader(f))
    assert {r["filter_id"] for r in rows} == {"A01", "A02", "A03", "F01"}

    bad = write_csv(tmp_path / "bad.csv", [["X", "g", "nope", "", 1, "", ""]])
    assert la3.main([str(bad)]) == 2


def test_template_parses():
    template = Path(__file__).resolve().parents[1] / "la3_masses_template.csv"
    rows = la3.read_rows(template)
    results, _ = la3.analyse(rows, la3.load_thresholds(None, []))
    assert results
