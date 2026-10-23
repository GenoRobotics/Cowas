#!/usr/bin/env python3
"""LA3 analysis: residual water, shield volume, shield/water ratio and retention.

Reads the weighing log of docs/lab/shield-protocol.md (one CSV row per weighing) and prints,
per filter and per group, the computed volumes with PASS / FAIL / OPEN against the
lab's thresholds. OPEN = the lab hasn't set that threshold yet (docs/lab/shield-protocol.md §7).

    python tools/analyze_la3.py masses.csv
    python tools/analyze_la3.py masses.csv --thresholds tools/la3_thresholds.json \
        --set shield_min_ml=1.2 --csv results.csv

CSV columns (header required, extra columns ignored):
    filter_id  unique per filter (e.g. S01)
    group      condition (e.g. A-today, B-long, F-warm, H-handling)
    stage      dry | water | shield | full | hold | handling
    time       ISO 8601 local time, e.g. 2026-10-12 14:05 (needed for shield/full/hold/handling)
    mass_g     scale reading of the capped filter, g
    temp_c, notes   optional, copied through

Stages (docs/lab/shield-protocol.md Part C):
    dry       the dry filter with its two caps, before anything
    water     after a cycle WITHOUT the shield (water + air push) -> residual water
    shield    after the shield stage on the same filter -> shield volume
    full      after a full cycle, not weighed in between -> water + shield only
    hold      retention weighings (any number), timed from the shield/full weighing
    handling  consecutive remove/weigh/reinstall cycles on one filter -> loss per handling

Exit code: 0 = no FAIL, 1 = at least one FAIL, 2 = bad input.
Standard library only (Python >= 3.8).
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import statistics
import sys
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path

STAGES = ("dry", "water", "shield", "full", "hold", "handling")
SINGLE_STAGES = ("dry", "water", "shield", "full")

# Threshold keys. None = not decided (OPEN). Source = docs/lab/shield-protocol.md §7.
THRESHOLD_KEYS = {
    "shield_target_ml": "B1",  # info: deviation of the delivered dose
    "shield_min_ml": "B2",
    "residual_max_ml": "B3",
    "ratio_min": "B3",  # shield / residual water
    "retention_loss_max_pct_per_day": "B5",
    "shield_density_g_ml": "LA3 protocol A1",
    "water_density_g_ml": "-",
}
DEFAULT_DENSITY = 1.0

PASS, FAIL, OPEN, NA = "PASS", "FAIL", "OPEN", "n/a"
# Masses differ by float noise (8.40 - 8.00 = 0.4000000000000004); the scale reads 0.01 g,
# so a value within EPS of a limit counts as at the limit.
EPS = 1e-9


class InputError(Exception):
    """The CSV or the thresholds can't be analysed; the message lists every problem."""


@dataclass
class Row:
    line: int
    filter_id: str
    group: str
    stage: str
    time: datetime | None
    mass_g: float


@dataclass
class FilterResult:
    filter_id: str
    group: str
    residual_ml: float | None = None
    shield_ml: float | None = None
    ratio: float | None = None
    water_plus_shield_ml: float | None = None
    delivered_pct: float | None = None  # shield_ml / shield_target_ml
    hold_days: list[float] = field(default_factory=list)
    retained_pct: list[float] = field(default_factory=list)
    loss_pct_per_day: float | None = None
    loss_ml_per_day: float | None = None
    handling_loss_g: float | None = None  # this filter's own handling check
    checks: dict[str, str] = field(default_factory=dict)

    @property
    def verdict(self) -> str:
        values = self.checks.values()
        if FAIL in values:
            return FAIL
        if OPEN in values:
            return OPEN
        if PASS in values:
            return PASS
        return NA


# ---------------------------------------------------------------- reading


def parse_time(text: str) -> datetime | None:
    text = text.strip()
    if not text:
        return None
    return datetime.fromisoformat(text.replace("Z", "+00:00"))


def read_rows(path: Path) -> list[Row]:
    errors: list[str] = []
    rows: list[Row] = []
    with path.open(newline="", encoding="utf-8-sig") as f:
        reader = csv.DictReader(f)
        missing = {"filter_id", "group", "stage", "mass_g", "time"} - set(reader.fieldnames or [])
        if missing:
            raise InputError(f"{path}: missing columns: {', '.join(sorted(missing))}")
        for i, rec in enumerate(reader, start=2):
            fid = (rec["filter_id"] or "").strip()
            stage = (rec["stage"] or "").strip().lower()
            if not fid and not stage and not (rec["mass_g"] or "").strip():
                continue  # blank line
            if not fid:
                errors.append(f"line {i}: empty filter_id")
            if stage not in STAGES:
                errors.append(f"line {i}: unknown stage '{stage}' (expected {', '.join(STAGES)})")
            try:
                mass = float((rec["mass_g"] or "").strip().replace(",", "."))
                if not math.isfinite(mass) or mass <= 0:
                    raise ValueError
            except ValueError:
                errors.append(f"line {i}: mass_g must be a number > 0, got '{rec['mass_g']}'")
                mass = math.nan
            try:
                t = parse_time(rec["time"] or "")
            except ValueError:
                errors.append(f"line {i}: bad time '{rec['time']}' (use e.g. 2026-10-12 14:05)")
                t = None
            if t is None and stage in ("shield", "full", "hold", "handling"):
                errors.append(f"line {i}: stage '{stage}' needs a time")
            rows.append(Row(i, fid, (rec["group"] or "").strip(), stage, t, mass))
    if errors:
        raise InputError("\n".join(errors))
    if not rows:
        raise InputError(f"{path}: no data rows")
    return rows


def load_thresholds(path: Path | None, overrides: list[str]) -> dict[str, float | None]:
    th: dict[str, float | None] = {k: None for k in THRESHOLD_KEYS}
    if path is not None:
        data = json.loads(path.read_text(encoding="utf-8"))
        for k, v in data.items():
            if k.startswith("_"):
                continue
            if k not in THRESHOLD_KEYS:
                raise InputError(f"{path}: unknown threshold '{k}'")
            th[k] = None if v is None else float(v)
    for item in overrides:
        k, sep, v = item.partition("=")
        k = k.strip()
        if not sep or k not in THRESHOLD_KEYS:
            raise InputError(f"--set {item}: expected key=value with key in {', '.join(THRESHOLD_KEYS)}")
        th[k] = None if v.strip().lower() in ("", "none", "null") else float(v)
    for k in ("shield_density_g_ml", "water_density_g_ml"):
        if th[k] is not None and not th[k] > 0:
            raise InputError(f"{k} must be > 0")
    return th


# ---------------------------------------------------------------- analysis


def slope(xs: list[float], ys: list[float]) -> float | None:
    """Least-squares slope, or None with < 2 distinct x."""
    if len(set(xs)) < 2:
        return None
    mx, my = statistics.fmean(xs), statistics.fmean(ys)
    sxx = sum((x - mx) ** 2 for x in xs)
    return sum((x - mx) * (y - my) for x, y in zip(xs, ys)) / sxx


def check_min(value: float | None, limit: float | None) -> str:
    if value is None:
        return NA
    if limit is None:
        return OPEN
    return PASS if value >= limit - EPS else FAIL


def check_max(value: float | None, limit: float | None) -> str:
    if value is None:
        return NA
    if limit is None:
        return OPEN
    return PASS if value <= limit + EPS else FAIL


def handling_loss(rows: list[Row]) -> float | None:
    """Mean mass lost per remove/reinstall cycle, from consecutive 'handling' weighings."""
    ordered = sorted((r for r in rows if r.stage == "handling"), key=lambda r: r.time)
    if len(ordered) < 2:
        return None
    return (ordered[0].mass_g - ordered[-1].mass_g) / (len(ordered) - 1)


def analyse(rows: list[Row], th: dict[str, float | None]) -> tuple[list[FilterResult], float | None]:
    rho_s = th["shield_density_g_ml"] or DEFAULT_DENSITY
    rho_w = th["water_density_g_ml"] or DEFAULT_DENSITY

    by_filter: dict[str, list[Row]] = {}
    for r in rows:
        by_filter.setdefault(r.filter_id, []).append(r)

    errors: list[str] = []
    for fid, frows in by_filter.items():
        groups = {r.group for r in frows}
        if len(groups) > 1:
            errors.append(f"filter {fid}: in several groups {sorted(groups)}")
        for st in SINGLE_STAGES:
            lines = [r.line for r in frows if r.stage == st]
            if len(lines) > 1:
                errors.append(f"filter {fid}: stage '{st}' appears {len(lines)} times (lines {lines})")
        stages = {r.stage for r in frows}
        if "dry" not in stages and stages - {"handling"}:
            errors.append(f"filter {fid}: no 'dry' weighing")
        if "shield" in stages and "full" in stages:
            errors.append(f"filter {fid}: has both 'shield' and 'full' (one design per filter)")
        if "shield" in stages and "water" not in stages:
            errors.append(f"filter {fid}: 'shield' without 'water' (the paired design needs both)")
        if "hold" in stages and not stages & {"shield", "full"}:
            errors.append(f"filter {fid}: 'hold' weighings without a 'shield' or 'full' start")
    if errors:
        raise InputError("\n".join(errors))

    per_filter_handling = {fid: handling_loss(fr) for fid, fr in by_filter.items()}
    known = [h for h in per_filter_handling.values() if h is not None]
    h_loss = statistics.fmean(known) if known else None

    results: list[FilterResult] = []
    for fid, frows in by_filter.items():
        st = {r.stage: r for r in frows if r.stage in SINGLE_STAGES}
        res = FilterResult(fid, frows[0].group, handling_loss_g=per_filter_handling[fid])
        dry = st.get("dry")
        if dry and "water" in st:
            res.residual_ml = (st["water"].mass_g - dry.mass_g) / rho_w
        if "shield" in st:
            res.shield_ml = (st["shield"].mass_g - st["water"].mass_g) / rho_s
        if res.shield_ml is not None and res.residual_ml is not None:
            res.ratio = res.shield_ml / res.residual_ml if res.residual_ml > 0 else math.inf
        start = st.get("shield") or st.get("full")
        if start and dry:
            res.water_plus_shield_ml = (start.mass_g - dry.mass_g) / rho_s
        if res.shield_ml is not None and th["shield_target_ml"]:
            res.delivered_pct = 100.0 * res.shield_ml / th["shield_target_ml"]

        holds = sorted((r for r in frows if r.stage == "hold"), key=lambda r: r.time)
        if holds and start and dry:
            liquid0 = start.mass_g - dry.mass_g
            if liquid0 <= 0:
                raise InputError(f"filter {fid}: no liquid at the start of the hold ({liquid0:.2f} g)")
            days, pct = [0.0], [100.0]
            for k, r in enumerate(holds, start=1):
                d = (r.time - start.time).total_seconds() / 86400.0
                if d <= 0:
                    raise InputError(f"line {r.line}: hold weighing is not after the start weighing")
                # each hold weighing costs one remove/reinstall cycle: add the handling loss back
                m = r.mass_g + (k * h_loss if h_loss is not None else 0.0)
                days.append(d)
                pct.append(100.0 * (m - dry.mass_g) / liquid0)
            res.hold_days = days[1:]
            res.retained_pct = pct[1:]
            s = slope(days, pct)
            if s is not None:
                res.loss_pct_per_day = -s
                res.loss_ml_per_day = -s / 100.0 * liquid0 / rho_s

        res.checks["shield_min"] = check_min(res.shield_ml, th["shield_min_ml"])
        res.checks["residual_max"] = check_max(res.residual_ml, th["residual_max_ml"])
        res.checks["ratio_min"] = check_min(res.ratio, th["ratio_min"])
        res.checks["retention"] = check_max(res.loss_pct_per_day, th["retention_loss_max_pct_per_day"])
        results.append(res)
    return results, h_loss


# ---------------------------------------------------------------- report


def fmt(v: float | None, nd: int = 2) -> str:
    if v is None:
        return "-"
    if math.isinf(v):
        return "inf"
    return f"{v:.{nd}f}"


def mean_sd(values: list[float]) -> str:
    vals = [v for v in values if v is not None and math.isfinite(v)]
    if not vals:
        return "-"
    if len(vals) == 1:
        return f"{vals[0]:.3f} (n=1)"
    return f"{statistics.fmean(vals):.3f} +/- {statistics.stdev(vals):.3f} (n={len(vals)})"


def report(results: list[FilterResult], th: dict[str, float | None], h_loss: float | None) -> str:
    out: list[str] = []
    out.append("LA3 analysis (docs/lab/shield-protocol.md)")
    if th["shield_density_g_ml"] is None:
        out.append(f"  shield_density_g_ml = {DEFAULT_DENSITY} (default, PLACEHOLDER: measure it, LA3 protocol A1)")
    else:
        out.append(f"  shield_density_g_ml = {th['shield_density_g_ml']}")
    out.append(f"  water_density_g_ml = {th['water_density_g_ml'] or DEFAULT_DENSITY}")
    for key, src in THRESHOLD_KEYS.items():
        if key.endswith("density_g_ml"):
            continue
        val = th[key]
        out.append(f"  {key} = {val if val is not None else 'OPEN (not decided: decision ' + src + ')'}")
    if h_loss is not None:
        out.append(f"  handling loss = {h_loss:.3f} g per remove/reinstall (added back to hold weighings)")
    else:
        out.append("  handling loss = not measured (hold weighings not corrected)")
    out.append("")

    head = (f"{'filter':<8} {'group':<12} {'resid mL':>8} {'shield mL':>9} {'ratio':>6} "
            f"{'w+s mL':>7} {'last d':>6} {'kept %':>6} {'loss %/d':>8}  checks")
    out.append(head)
    out.append("-" * len(head))
    for r in sorted(results, key=lambda r: (r.group, r.filter_id)):
        checks = " ".join(f"{k}={v}" for k, v in r.checks.items() if v != NA) or "-"
        out.append(
            f"{r.filter_id:<8} {r.group:<12} {fmt(r.residual_ml):>8} {fmt(r.shield_ml):>9} "
            f"{fmt(r.ratio):>6} {fmt(r.water_plus_shield_ml):>7} "
            f"{fmt(r.hold_days[-1] if r.hold_days else None, 1):>6} "
            f"{fmt(r.retained_pct[-1] if r.retained_pct else None, 1):>6} "
            f"{fmt(r.loss_pct_per_day):>8}  {r.verdict}: {checks}"
        )
    out.append("")

    out.append("Per group: mean +/- SD (n)")
    groups: dict[str, list[FilterResult]] = {}
    for r in results:
        groups.setdefault(r.group, []).append(r)
    for g in sorted(groups):
        rs = groups[g]
        verdicts = [r.verdict for r in rs]
        gv = FAIL if FAIL in verdicts else OPEN if OPEN in verdicts else PASS if PASS in verdicts else NA
        out.append(f"[{g}] {gv}  ({len(rs)} filters)")
        out.append(f"  residual water  mL : {mean_sd([r.residual_ml for r in rs])}")
        out.append(f"  shield          mL : {mean_sd([r.shield_ml for r in rs])}")
        out.append(f"  shield / water     : {mean_sd([r.ratio for r in rs])}")
        out.append(f"  water + shield  mL : {mean_sd([r.water_plus_shield_ml for r in rs])}")
        if th["shield_target_ml"]:
            out.append(f"  delivered % target : {mean_sd([r.delivered_pct for r in rs])}")
        out.append(f"  loss       %/day   : {mean_sd([r.loss_pct_per_day for r in rs])}")
        out.append(f"  loss       mL/day  : {mean_sd([r.loss_ml_per_day for r in rs])}")
    out.append("")
    overall = [r.verdict for r in results]
    summary = FAIL if FAIL in overall else OPEN if OPEN in overall else PASS if PASS in overall else NA
    out.append(f"OVERALL: {summary}" + ("  (OPEN = a lab threshold is still missing)" if summary == OPEN else ""))
    return "\n".join(out)


def write_csv(path: Path, results: list[FilterResult]) -> None:
    cols = ["filter_id", "group", "residual_ml", "shield_ml", "ratio", "water_plus_shield_ml",
            "delivered_pct", "last_hold_days", "last_retained_pct", "loss_pct_per_day",
            "loss_ml_per_day", "verdict"]
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(cols)
        for r in results:
            w.writerow([
                r.filter_id, r.group, r.residual_ml, r.shield_ml, r.ratio, r.water_plus_shield_ml,
                r.delivered_pct, r.hold_days[-1] if r.hold_days else None,
                r.retained_pct[-1] if r.retained_pct else None, r.loss_pct_per_day,
                r.loss_ml_per_day, r.verdict,
            ])


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("masses", type=Path, help="CSV of weighings")
    p.add_argument("--thresholds", type=Path, help="JSON thresholds (tools/la3_thresholds.json)")
    p.add_argument("--set", action="append", default=[], metavar="KEY=VALUE",
                   help="override one threshold (repeatable)")
    p.add_argument("--csv", type=Path, help="write the per-filter results to this CSV")
    args = p.parse_args(argv)
    try:
        th = load_thresholds(args.thresholds, args.set)
        results, h_loss = analyse(read_rows(args.masses), th)
    except (InputError, OSError, json.JSONDecodeError, ValueError) as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 2
    print(report(results, th, h_loss))
    if args.csv:
        write_csv(args.csv, results)
    return 1 if any(r.verdict == FAIL for r in results) else 0


if __name__ == "__main__":
    sys.exit(main())
