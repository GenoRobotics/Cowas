"""pytest for tools/analyze_la4.py on synthetic data with known answers.

Run: python -m pytest tools/tests
"""

import math
import os
import sys

import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import analyze_la4 as la4  # noqa: E402

# Console `clocklist` (FW0 goldens): slot -> clock minute.
CLOCK = {0: 30, 1: 34, 2: 38, 3: 41, 4: 49, 5: 53, 6: 56, 7: 0, 8: 4, 9: 8,
         10: 11, 11: 15, 12: 19, 13: 23, 14: 26}

BLANK = 5.0
GAIN = 20000.0  # reading units per unit fraction (e.g. µS/cm for a 20 g/L NaCl stock)


def reading(fraction):
    return BLANK + GAIN * fraction


def std_rows(blanks=(4.9, 5.0, 5.1), fractions=(1e-4, 1e-3, 1e-2, 0.1, 1.0), gain=None):
    rows = [{"fraction": "0", "reading": str(b)} for b in blanks]
    for f in fractions:
        r = BLANK + (gain(f) if gain else GAIN * f)
        rows.append({"fraction": str(f), "reading": str(r)})
    return rows


def cal():
    return la4.Calibration.from_rows(std_rows())


def plug_exp_mean(v1, v2, v0, vm):
    """Mean of rel(V) = 1 for V < v0, exp(-(V - v0)/vm) after, over [v1, v2]."""
    def integral(a, b):  # of rel over [a, b]
        total = 0.0
        if a < v0:
            total += min(b, v0) - a
        lo = max(a, v0)
        if b > lo:
            total += vm * (math.exp(-(lo - v0) / vm) - math.exp(-(b - v0) / vm))
        return total
    return integral(v1, v2) / (v2 - v1)


def washout_rows(v0=80.0, vm=40.0, cup=25.0, n=40, model=None):
    rows = []
    v = 0.0
    for _ in range(n):
        m = model(v, v + cup) if model else plug_exp_mean(v, v + cup, v0, vm)
        rows.append({"volume_ml": str(cup), "reading": str(reading(m))})
        v += cup
    return rows


# ------------------------------------------------------------------- helpers


def test_hose_volume_matches_roadmap_example():
    # ROADMAP §3.7: 5 m of 6 mm ID = 141 mL.
    assert la4.hose_volume_ml(6.0, 5.0) == pytest.approx(141.37, abs=0.01)
    assert la4.hose_volume_ml(6.0, 0.0) == 0.0
    with pytest.raises(la4.La4Error):
        la4.hose_volume_ml(-1.0, 1.0)


def test_linear_fit_exact_line():
    a, b, r2 = la4.linear_fit([0, 1, 2, 3], [1, 3, 5, 7])
    assert (a, b, r2) == pytest.approx((1.0, 2.0, 1.0))


# --------------------------------------------------------------- calibration


def test_calibration_linear_and_limits():
    c = cal()
    assert c.blank_mean == pytest.approx(5.0)
    assert c.blank_sd == pytest.approx(0.1)
    assert c.fraction(reading(0.05)) == pytest.approx(0.05)
    assert c.fraction(reading(0.0)) == 0.0
    assert c.fraction(1.0) == 0.0  # below the blank -> 0, never negative
    assert c.lod == pytest.approx(3 * 0.1 / GAIN)
    assert c.loq == pytest.approx(10 * 0.1 / GAIN)


def test_calibration_nonlinear_exact_at_standards():
    gain = lambda f: 18000.0 * f**0.93  # conductivity bends at high concentration
    c = la4.Calibration.from_rows(std_rows(gain=gain))
    for f in (1e-4, 1e-3, 1e-2, 0.1, 1.0):
        assert c.fraction(BLANK + gain(f)) == pytest.approx(f, rel=1e-9)
    # between standards the interpolation stays within a few % of the true curve
    assert c.fraction(BLANK + gain(0.03)) == pytest.approx(0.03, rel=0.05)


def test_calibration_extrapolates_above_top_standard():
    c = la4.Calibration.from_rows(std_rows(fractions=(0.01, 0.1)))
    assert c.fraction(reading(0.2)) == pytest.approx(0.2)


@pytest.mark.parametrize(
    "rows, msg",
    [
        ([{"fraction": "0.1", "reading": "10"}, {"fraction": "1", "reading": "100"}], "blank"),
        ([{"fraction": "0", "reading": "1"}, {"fraction": "0", "reading": "1.1"},
          {"fraction": "1", "reading": "100"}], "2 non-zero"),
        ([{"fraction": "0", "reading": "1"}, {"fraction": "0.1", "reading": "10"},
          {"fraction": "1", "reading": "100"}], "noise"),
        ([{"fraction": "0", "reading": "1"}, {"fraction": "0", "reading": "1.1"},
          {"fraction": "0.1", "reading": "50"}, {"fraction": "1", "reading": "40"}], "increase"),
        ([{"fraction": "0", "reading": "1"}, {"fraction": "0", "reading": "x"}], "not a number"),
        ([{"fraction": "2", "reading": "1"}], "0..1"),
    ],
)
def test_calibration_rejects_bad_standards(rows, msg):
    with pytest.raises(la4.La4Error, match=msg):
        la4.Calibration.from_rows(rows)


def test_calibration_single_blank_with_given_sd():
    rows = [{"fraction": "0", "reading": "5"}] + std_rows(blanks=())
    c = la4.Calibration.from_rows(rows, blank_sd=0.2)
    assert c.blank_sd == 0.2


# ------------------------------------------------------------------- washout


def test_washout_recovers_plug_plus_mixer():
    v0, vm = 80.0, 40.0
    r = la4.analyze_washout("run", washout_rows(v0, vm), cal(), bench_hose_ml=50.0)
    assert r.fit is not None
    assert r.fit.vm == pytest.approx(vm, rel=0.01)
    assert r.fit.r2 > 0.999
    assert r.used_fit == "all"
    # line volume = area under the curve = v0 + vm
    assert r.line_volume_ml == pytest.approx(v0 + vm, rel=0.01)
    assert r.internal_dead_volume_ml == pytest.approx(v0 + vm - 50.0, rel=0.02)
    # instantaneous 0.1 %: v0 + vm ln(1000)
    expected = v0 + vm * math.log(1000)
    assert r.purge_needed_ml == pytest.approx(expected, rel=0.02)
    assert r.factor_needed == pytest.approx(expected / (v0 + vm), rel=0.02)
    # model-free: the fraction boundary right after the true crossing
    assert expected - 25 <= r.purge_measured_ml <= expected + 25
    # sample-averaged carry-over over 2 L is far below the instantaneous target
    assert r.sample_avg_at_purge == pytest.approx(vm / 2000 * 0.001, rel=0.05)
    assert not r.warnings


def test_washout_pure_mixer_needs_ln1000_volumes():
    # A perfectly mixed volume (no plug) needs ~6.9 volumes for 0.1 %: 3x is not enough.
    r = la4.analyze_washout("mixer", washout_rows(v0=0.0, vm=60.0, n=30), cal())
    assert r.factor_needed == pytest.approx(math.log(1000), rel=0.03)


def test_washout_mass_and_density():
    rows = [{"mass_g": str(25.0 * 1.01), "reading": x["reading"]} for x in washout_rows()]
    r = la4.analyze_washout("mass", rows, cal(), density=1.01)
    assert r.total_ml == pytest.approx(40 * 25.0)


def test_washout_tailing_is_flagged_and_purge_stays_conservative():
    # a fast mixer plus a slow dead leg holding 2 % of the tracer
    def instant(v):
        fast = 1.0 if v < 50 else math.exp(-(v - 50.0) / 20.0)
        return 0.98 * fast + 0.02 * math.exp(-v / 150.0)

    def model(a, b):
        return 0.98 * plug_exp_mean(a, b, 50.0, 20.0) + 0.02 * plug_exp_mean(a, b, 0.0, 150.0)

    lo, hi = 50.0, 5000.0  # true instantaneous 0.1 % crossing, by bisection
    for _ in range(100):
        mid = (lo + hi) / 2
        lo, hi = (mid, hi) if instant(mid) > 0.001 else (lo, mid)

    r = la4.analyze_washout("tail", washout_rows(model=model, n=60), cal())
    assert any("tailing" in w for w in r.warnings)
    assert r.tail_fit.vm == pytest.approx(150.0, rel=0.05)
    # the fast part alone would say ~188 mL; the slow leg pushes the real answer to ~450 mL
    assert r.purge_needed_ml >= lo
    assert r.purge_needed_ml < 1.1 * lo


def test_washout_uses_tail_fit_when_it_asks_for_more():
    # the fit window starts in the fast part; a long flat tail must win
    def model(a, b):
        return 0.9 * plug_exp_mean(a, b, 50.0, 15.0) + 0.1 * plug_exp_mean(a, b, 0.0, 100.0)

    r = la4.analyze_washout("tail2", washout_rows(model=model, cup=10.0, n=40), cal())
    assert r.used_fit == "tail"
    assert r.purge_needed_ml == pytest.approx(r.tail_fit.volume_for(0.001))


def test_washout_warns_when_line_not_full():
    rows = washout_rows(model=lambda a, b: 0.5 * plug_exp_mean(a, b, 80.0, 40.0))
    r = la4.analyze_washout("half", rows, cal())
    assert any("really full" in w for w in r.warnings)


def test_washout_warns_when_never_below_target():
    r = la4.analyze_washout("short", washout_rows(n=8), cal())
    assert r.purge_measured_ml is None
    assert any("never went below" in w for w in r.warnings)


def test_washout_warns_when_lod_above_target():
    noisy = la4.Calibration.from_rows(std_rows(blanks=(-5.0, 5.0, 15.0)))
    r = la4.analyze_washout("noisy", washout_rows(), noisy)
    assert any("LOD" in w for w in r.warnings)


def test_washout_no_fit_with_too_few_points():
    rows = washout_rows(v0=0.0, vm=5.0, cup=100.0, n=6)  # falls from 100 % to < LOQ at once
    r = la4.analyze_washout("coarse", rows, cal())
    assert r.fit is None and r.purge_needed_ml is None
    assert any("no exponential fit" in w for w in r.warnings)


@pytest.mark.parametrize(
    "rows, msg",
    [
        ([{"volume_ml": "25", "reading": "5"}] * 4, "at least 5"),
        ([{"reading": "5"}] * 6, "volume_ml or mass_g"),
        ([{"volume_ml": "0", "reading": "5"}] * 6, "> 0"),
    ],
)
def test_washout_rejects_bad_rows(rows, msg):
    with pytest.raises(la4.La4Error, match=msg):
        la4.analyze_washout("bad", rows, cal())


def test_summary_recommends_rounded_factor_and_internal():
    runs = [
        la4.analyze_washout(f"r{i}", washout_rows(80.0, vm), cal(), bench_hose_ml=50.0)
        for i, vm in enumerate((38.0, 40.0, 42.0))
    ]
    s = la4.summarize_washout(runs, la4.hose_volume_ml(6.0, 5.0))
    assert s.runs == 3 and not s.warnings
    assert s.internal_dead_volume_ml == pytest.approx(70.0, rel=0.03)
    assert s.recommended_internal_ml % 5 == 0
    assert s.recommended_internal_ml >= s.internal_dead_volume_ml
    assert s.recommended_factor % 0.5 == 0
    assert s.recommended_factor >= s.factor_max * la4.FACTOR_MARGIN
    assert s.field_purge_ml == pytest.approx(
        s.recommended_factor * (141.37 + s.recommended_internal_ml), rel=1e-3
    )


def test_summary_warns_on_one_run_and_spread():
    a = la4.analyze_washout("a", washout_rows(80.0, 40.0), cal(), bench_hose_ml=0.0)
    s = la4.summarize_washout([a], 141.0)
    assert any("3" in w for w in s.warnings)
    b = la4.analyze_washout("b", washout_rows(200.0, 80.0), cal(), bench_hose_ml=0.0)
    s2 = la4.summarize_washout([a, b, a], 141.0)
    assert any("disagree" in w for w in s2.warnings)


# ---------------------------------------------------------------- carry-over


def carry_rows(rels, eluate=None):
    rows = [{"kind": "outlet", "volume_ml": "50", "reading": str(reading(r))} for r in rels]
    if eluate is not None:
        rows.append({"kind": "eluate", "volume_ml": "10", "reading": str(reading(eluate))})
    return rows


def test_carryover_pass():
    r = la4.analyze_carryover(carry_rows([2e-4, 1e-5, 0, 0], eluate=0), cal())
    assert r.verdict == "PASS"
    assert r.first_fraction_rel == pytest.approx(2e-4)
    assert r.carryover == pytest.approx((2e-4 + 1e-5) * 50 / 200, rel=1e-6)


def test_carryover_fail_counts_the_eluate():
    # outlet alone is below 0.1 %, the filter eluate pushes it over
    r = la4.analyze_carryover(carry_rows([1e-4] * 4, eluate=0.02), cal())
    assert r.tracer_ml_eluate == pytest.approx(0.2)
    assert r.carryover == pytest.approx((4 * 50 * 1e-4 + 0.2) / 200)
    assert r.verdict == "FAIL"


def test_carryover_inconclusive_when_lod_too_high():
    noisy = la4.Calibration.from_rows(std_rows(blanks=(-15.0, 5.0, 25.0)))
    r = la4.analyze_carryover(carry_rows([0, 0, 0]), noisy)
    assert r.carryover == 0.0
    assert r.verdict == "INCONCLUSIVE"


def test_carryover_rejects_bad_rows():
    with pytest.raises(la4.La4Error, match="outlet or eluate"):
        la4.analyze_carryover([{"kind": "x", "volume_ml": "1", "reading": "5"}], cal())
    with pytest.raises(la4.La4Error, match="outlet fraction"):
        la4.analyze_carryover([{"kind": "eluate", "volume_ml": "1", "reading": "5"}], cal())


# ---------------------------------------------------------------- cross-talk


def xt_rows(source=5, values=None, controls=(), outlet_ml="100"):
    """One row per slot; values = {slot: fraction in the eluate} (default 0)."""
    values = values or {}
    rows = []
    for order, slot in enumerate(s for s in CLOCK if s != 0):
        role = "source" if slot == source else ("control" if slot in controls else "test")
        rel = values.get(slot, 0.0)
        rows.append({
            "slot": str(slot), "role": role, "clock_min": str(CLOCK[slot]),
            "cycle_order": str(order), "outlet_ml": outlet_ml, "outlet_reading": str(reading(0)),
            "eluate_ml": "10", "eluate_reading": str(reading(rel)),
        })
    return rows


def test_crosstalk_clean_passes_and_finds_neighbours():
    r = la4.analyze_crosstalk(xt_rows(), cal(), source_ml=2.0)
    assert r.verdict == "PASS"
    nbrs = sorted(int(x.slot) for x in r.rows if x.neighbour)
    assert nbrs == [4, 6]  # slot 5 sits at clock 53, between 4 (49) and 6 (56)
    assert r.rows[0].distance_deg <= r.rows[-1].distance_deg  # sorted by distance


def test_crosstalk_neighbours_wrap_around_the_dial():
    r = la4.analyze_crosstalk(xt_rows(source=7), cal(), source_ml=2.0)  # clock 00
    assert sorted(int(x.slot) for x in r.rows if x.neighbour) == [6, 8]


def test_crosstalk_detects_a_leaking_neighbour():
    # 1 % of the eluate in slot 6 = 0.1 mL of source in 10 mL -> 5 % of a 2 mL source
    r = la4.analyze_crosstalk(xt_rows(values={6: 0.01}), cal(), source_ml=2.0)
    row = next(x for x in r.rows if x.slot == "6")
    assert row.fraction_of_source == pytest.approx(0.05)
    assert row.verdict == "FAIL" and row.neighbour
    assert r.verdict == "FAIL"


def test_crosstalk_control_does_not_fail_but_warns():
    r = la4.analyze_crosstalk(xt_rows(values={10: 0.01}, controls=(10,)), cal(), source_ml=2.0)
    assert r.verdict == "PASS"
    assert any("control slot 10" in w for w in r.warnings)


def test_crosstalk_big_outlet_volume_cannot_prove_the_target():
    # A clean 500 mL cycle: 500 mL x LOD (1.5e-5) = 7.5 µL of source = 0.4 % of 2 mL.
    # The dilution hides a 0.1 % leak: the protocol keeps the cycles short (LA4 §4.3).
    r = la4.analyze_crosstalk(xt_rows(outlet_ml="500"), cal(), source_ml=2.0)
    assert r.verdict == "INCONCLUSIVE"


def test_crosstalk_inconclusive_with_poor_sensitivity():
    noisy = la4.Calibration.from_rows(std_rows(blanks=(-15.0, 5.0, 25.0)))
    r = la4.analyze_crosstalk(xt_rows(), noisy, source_ml=0.1)
    assert r.verdict == "INCONCLUSIVE"


def test_crosstalk_any_slot_count():
    # 24 positions, one per 2.5 clock minutes: neighbours are still the adjacent ones
    rows = [{"slot": "S", "role": "source", "clock_min": "0"}]
    for k in range(1, 24):
        rows.append({"slot": str(k), "role": "test", "clock_min": str(2.5 * k),
                     "eluate_ml": "10", "eluate_reading": str(reading(0))})
    r = la4.analyze_crosstalk(rows, cal(), source_ml=2.0)
    assert sorted(int(x.slot) for x in r.rows if x.neighbour) == [1, 23]


@pytest.mark.parametrize(
    "mutate, msg",
    [
        (lambda rows: [r for r in rows if r["role"] != "source"], "exactly one"),
        (lambda rows: [dict(r, eluate_reading="") for r in rows], "go together"),
        (lambda rows: [dict(r, role="bogus") if r["role"] == "test" else r for r in rows], "role"),
    ],
)
def test_crosstalk_rejects_bad_rows(mutate, msg):
    with pytest.raises(la4.La4Error, match=msg):
        la4.analyze_crosstalk(mutate(xt_rows()), cal(), source_ml=2.0)


def test_crosstalk_needs_source_volume():
    with pytest.raises(la4.La4Error, match="source-ml"):
        la4.analyze_crosstalk(xt_rows(), cal(), source_ml=0.0)


# ----------------------------------------------------------------------- CLI


def write_csv(path, rows):
    keys = list(rows[0].keys())
    with open(path, "w", encoding="utf-8") as fh:
        fh.write("# comment lines are ignored\n")
        fh.write(",".join(keys) + "\n")
        for r in rows:
            fh.write(",".join(r[k] for k in keys) + "\n")
    return str(path)


def test_cli_washout_end_to_end(tmp_path, capsys):
    std = write_csv(tmp_path / "std.csv", std_rows())
    runs = [write_csv(tmp_path / f"run{i}.csv", washout_rows()) for i in range(3)]
    rc = la4.main(["washout", std, *runs, "--bench-hose-id-mm", "6", "--bench-hose-length-m", "1"])
    out = capsys.readouterr().out
    assert rc == 0
    assert "PURGE_DEAD_VOLUME_FACTOR" in out and "INTERNAL_DEAD_VOLUME_ML" in out


def test_cli_json_and_errors(tmp_path, capsys):
    std = write_csv(tmp_path / "std.csv", std_rows())
    slots = write_csv(tmp_path / "slots.csv", xt_rows())
    assert la4.main(["crosstalk", std, slots, "--source-ml", "2", "--json"]) == 0
    assert '"verdict": "PASS"' in capsys.readouterr().out
    b = write_csv(tmp_path / "b.csv", carry_rows([1e-4, 0]))
    assert la4.main(["carryover", std, b]) == 0
    run = write_csv(tmp_path / "run.csv", washout_rows())
    assert la4.main(["washout", std, run]) == 2  # bench hose missing
    assert "bench-hose" in capsys.readouterr().err
    assert la4.main(["carryover", std, str(tmp_path / "missing.csv")]) == 2


# ------------------------------------------------------ dilution, passed_over


def test_dilution_gives_the_same_washout():
    plain = washout_rows()
    diluted = []
    for r in plain:
        rel = (float(r["reading"]) - BLANK) / GAIN
        if rel > 0.1:  # out of the meter's range: diluted 1:10 by mass before reading
            diluted.append({"volume_ml": r["volume_ml"], "reading": str(reading(rel / 10)),
                            "dilution": "10"})
        else:
            diluted.append(dict(r, dilution=""))
    a = la4.analyze_washout("plain", plain, cal())
    b = la4.analyze_washout("diluted", diluted, cal())
    assert b.line_volume_ml == pytest.approx(a.line_volume_ml, rel=1e-9)
    assert b.purge_needed_ml == pytest.approx(a.purge_needed_ml, rel=1e-9)


def test_dilution_scales_the_lod_in_the_upper_bound():
    rows = [{"kind": "outlet", "volume_ml": "100", "reading": str(reading(0)), "dilution": "100"}]
    r = la4.analyze_carryover(rows, cal())
    assert r.upper_bound == pytest.approx(cal().lod * 100)
    assert r.verdict == "INCONCLUSIVE"  # diluting a clean sample 1:100 throws the sensitivity away


def test_dilution_below_one_is_rejected():
    rows = [{"kind": "outlet", "volume_ml": "10", "reading": "5", "dilution": "0.5"}]
    with pytest.raises(la4.La4Error, match=">= 1"):
        la4.analyze_carryover(rows, cal())


def test_crosstalk_passed_over_column():
    rows = xt_rows()
    for r in rows:
        r["passed_over"] = "y" if r["slot"] in ("9", "10") else "n"
    res = la4.analyze_crosstalk(rows, cal(), source_ml=2.0)
    assert {x.slot for x in res.rows if x.passed_over} == {"9", "10"}
    rows[0]["passed_over"] = "maybe"
    with pytest.raises(la4.La4Error, match="passed_over"):
        la4.analyze_crosstalk(rows, cal(), source_ml=2.0)
