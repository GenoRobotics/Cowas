#!/usr/bin/env python3
"""LA4 analysis: washout curve, carry-over into a filter, slot cross-talk.

Protocol and CSV formats: not written yet (LA4 protocol). Standard library only, so it runs on
any laptop with Python >= 3.8 (no numpy).

Every reading (conductivity, absorbance, photometry value...) is first turned into a
*relative concentration* ``rel`` = the fraction of the tracer stock (1.0 = pure stock,
0.001 = 0.1 %), using a calibration series made from the same stock:

    standards.csv   fraction,reading      (fraction 0 = a blank; >= 2 blanks)

Subcommands:

    washout   standards.csv run1.csv [run2.csv ...]   test a: dead volume + purge factor
    carryover standards.csv slotB.csv                 test b: carry-over into the next filter
    crosstalk standards.csv slots.csv                 test c: cross-talk between slots

Run ``python tools/analyze_la4.py <subcommand> -h`` for the options.
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import sys
from dataclasses import asdict, dataclass, field
from typing import Dict, List, Optional, Sequence, Tuple

DEFAULT_TARGET = 0.001  # 0.1 % carry-over (ROADMAP LA4)
DEFAULT_FIT_MAX = 0.2  # fit the exponential only below 20 % of the stock (past the plug front)
TAIL_WARN_RATIO = 1.2  # tail decay constant this much longer than the full fit -> use the tail
FACTOR_MARGIN = 1.2  # safety margin on the measured purge factor
# PLACEHOLDER field hose: the ROADMAP §3.7 example (5 m of 6 mm ID) until HW6 records the real one.
DEFAULT_FIELD_HOSE_ID_MM = 6.0
DEFAULT_FIELD_HOSE_LENGTH_M = 5.0


class La4Error(ValueError):
    """Bad input data (reported to the user without a traceback)."""


# --------------------------------------------------------------------------- helpers


def hose_volume_ml(inner_diameter_mm: float, length_m: float) -> float:
    """Inner volume of a hose: pi * (ID/2)^2 * L, in mL (1 mL = 1 cm^3)."""
    if inner_diameter_mm < 0 or length_m < 0:
        raise La4Error("hose ID and length must be >= 0")
    radius_cm = inner_diameter_mm / 20.0
    return math.pi * radius_cm**2 * length_m * 100.0


def _float(row: Dict[str, str], key: str, where: str, required: bool = True) -> Optional[float]:
    raw = (row.get(key) or "").strip()
    if raw == "":
        if required:
            raise La4Error(f"{where}: missing value for '{key}'")
        return None
    try:
        return float(raw)
    except ValueError:
        raise La4Error(f"{where}: '{key}' = '{raw}' is not a number") from None


def read_csv(path: str) -> List[Dict[str, str]]:
    """Rows as dicts; lines starting with '#' and empty lines are ignored."""
    with open(path, newline="", encoding="utf-8-sig") as handle:
        lines = [ln for ln in handle if ln.strip() and not ln.lstrip().startswith("#")]
    rows = list(csv.DictReader(lines))
    if not rows:
        raise La4Error(f"{path}: no data rows")
    return [{(k or "").strip(): (v or "") for k, v in row.items()} for row in rows]


def linear_fit(xs: Sequence[float], ys: Sequence[float]) -> Tuple[float, float, float]:
    """Least squares y = a + b x. Returns (a, b, r2)."""
    n = len(xs)
    if n < 2:
        raise La4Error("a fit needs at least 2 points")
    mx = sum(xs) / n
    my = sum(ys) / n
    sxx = sum((x - mx) ** 2 for x in xs)
    if sxx == 0:
        raise La4Error("a fit needs at least 2 different x values")
    sxy = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
    b = sxy / sxx
    a = my - b * mx
    ss_tot = sum((y - my) ** 2 for y in ys)
    ss_res = sum((y - (a + b * x)) ** 2 for x, y in zip(xs, ys))
    r2 = 1.0 - ss_res / ss_tot if ss_tot > 0 else 1.0
    return a, b, r2


# ----------------------------------------------------------------------- calibration


@dataclass
class Calibration:
    """Reading -> fraction of stock, by piecewise-linear interpolation through the standards.

    Piecewise-linear (not one straight line) because conductivity is not quite linear over
    four decades of concentration; the standards carry the curve, no model is assumed.
    """

    blank_mean: float
    blank_sd: float
    points: List[Tuple[float, float]]  # (net reading, fraction), increasing, starts at (0, 0)
    lod: float = 0.0  # fraction at 3 x blank SD
    loq: float = 0.0  # fraction at 10 x blank SD

    @classmethod
    def from_rows(cls, rows: List[Dict[str, str]], blank_sd: Optional[float] = None) -> Calibration:
        blanks: List[float] = []
        standards: List[Tuple[float, float]] = []
        for i, row in enumerate(rows, start=2):
            fraction = _float(row, "fraction", f"standards line {i}")
            reading = _float(row, "reading", f"standards line {i}")
            assert fraction is not None and reading is not None
            if fraction < 0 or fraction > 1:
                raise La4Error(f"standards line {i}: fraction must be in 0..1 (got {fraction})")
            if fraction == 0:
                blanks.append(reading)
            else:
                standards.append((fraction, reading))
        if not blanks:
            raise La4Error("standards: at least one blank (fraction 0) is needed")
        if len(standards) < 2:
            raise La4Error("standards: at least 2 non-zero standards are needed")
        mean = sum(blanks) / len(blanks)
        if blank_sd is None:
            if len(blanks) < 2:
                raise La4Error("standards: >= 2 blanks are needed to know the noise (or --blank-sd)")
            blank_sd = math.sqrt(sum((b - mean) ** 2 for b in blanks) / (len(blanks) - 1))
        points = [(0.0, 0.0)]
        for fraction, reading in sorted(standards):
            net = reading - mean
            if net <= points[-1][0]:
                raise La4Error(
                    f"standards: the reading must increase with the fraction "
                    f"(fraction {fraction} reads {reading}); redo the standard or the blank"
                )
            points.append((net, fraction))
        cal = cls(blank_mean=mean, blank_sd=blank_sd, points=points)
        cal.lod = cal.net_to_fraction(3.0 * blank_sd)
        cal.loq = cal.net_to_fraction(10.0 * blank_sd)
        return cal

    def net_to_fraction(self, net: float) -> float:
        pts = self.points
        if net <= 0:
            return 0.0
        for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
            if net <= x1:
                return y0 + (y1 - y0) * (net - x0) / (x1 - x0)
        (x0, y0), (x1, y1) = pts[-2], pts[-1]  # above the top standard: extend the last segment
        return y0 + (y1 - y0) * (net - x0) / (x1 - x0)

    def fraction(self, reading: float) -> float:
        """Fraction of stock for a raw reading (never negative)."""
        return self.net_to_fraction(reading - self.blank_mean)

    @property
    def top(self) -> float:
        return self.points[-1][1]


def _flag(cal: Calibration, rel: float) -> str:
    if rel < cal.lod:
        return "<LOD"
    if rel < cal.loq:
        return "<LOQ"
    if rel > cal.top * 1.05:
        return ">top std"
    return ""


def _dilution(row: Dict[str, str], key: str, where: str) -> float:
    """Optional dilution factor (e.g. 10 when a cup was diluted 1:10 before reading)."""
    d = _float(row, key, where, required=False)
    if d is None:
        return 1.0
    if d < 1.0:
        raise La4Error(f"{where}: {key} must be >= 1 (got {d})")
    return d


def measure(cal: Calibration, reading: float, dilution: float = 1.0) -> Tuple[float, float, str]:
    """(rel in the original liquid, its LOD, flag). The flag is judged on the diluted reading."""
    rel_read = cal.fraction(reading)
    return rel_read * dilution, cal.lod * dilution, _flag(cal, rel_read)


# --------------------------------------------------------------------------- washout


@dataclass
class ExpFit:
    """ln(rel) = a - V / vm  (vm = decay volume, mL)."""

    a: float
    vm: float
    r2: float
    n: int
    v_from: float
    v_to: float

    def rel_at(self, volume: float) -> float:
        return math.exp(self.a - volume / self.vm)

    def volume_for(self, rel: float) -> float:
        """Cumulative volume at which the outlet falls to ``rel`` of the stock."""
        return self.vm * (self.a - math.log(rel))


@dataclass
class WashoutResult:
    name: str
    n_fractions: int
    total_ml: float
    line_volume_ml: float  # area under rel(V): the volume that held tracer
    tail_ml: float  # part of line_volume_ml extrapolated beyond the last fraction
    fit: Optional[ExpFit]
    tail_fit: Optional[ExpFit]
    used_fit: str  # "all" or "tail"
    purge_needed_ml: Optional[float]  # instantaneous outlet <= target
    purge_measured_ml: Optional[float]  # model-free: last fraction end where rel <= target for good
    factor_needed: Optional[float]  # purge_needed / line volume
    internal_dead_volume_ml: Optional[float]
    sample_avg_at_purge: Optional[float]  # sample-averaged carry-over with purge = purge_needed
    warnings: List[str] = field(default_factory=list)
    fractions: List[Dict[str, object]] = field(default_factory=list)


def _fit_exp(points: List[Tuple[float, float]]) -> ExpFit:
    xs = [v for v, _ in points]
    ys = [math.log(r) for _, r in points]
    a, b, r2 = linear_fit(xs, ys)
    if b >= 0:
        raise La4Error("the washout does not decay (the slope of ln(rel) is >= 0)")
    return ExpFit(a=a, vm=-1.0 / b, r2=r2, n=len(points), v_from=min(xs), v_to=max(xs))


def analyze_washout(
    name: str,
    rows: List[Dict[str, str]],
    cal: Calibration,
    target: float = DEFAULT_TARGET,
    fit_max: float = DEFAULT_FIT_MAX,
    bench_hose_ml: float = 0.0,
    sample_ml: float = 2000.0,
    density: float = 1.0,
) -> WashoutResult:
    """Test a. Each row = one collected fraction, in collection order.

    Columns: ``volume_ml`` or ``mass_g`` (net water mass; volume = mass / density), ``reading``,
    optional ``dilution`` (10 = the cup was diluted 1:10 before reading, for out-of-range cups).
    """
    warnings: List[str] = []
    fractions: List[Dict[str, object]] = []
    v_end = 0.0
    rels: List[float] = []
    mids: List[float] = []
    vols: List[float] = []
    for i, row in enumerate(rows, start=2):
        where = f"{name} line {i}"
        vol = _float(row, "volume_ml", where, required=False)
        if vol is None:
            mass = _float(row, "mass_g", where, required=False)
            if mass is None:
                raise La4Error(f"{where}: give volume_ml or mass_g")
            vol = mass / density
        if vol <= 0:
            raise La4Error(f"{where}: the fraction volume must be > 0")
        reading = _float(row, "reading", where)
        assert reading is not None
        rel, _, flag = measure(cal, reading, _dilution(row, "dilution", where))
        mid = v_end + vol / 2.0
        v_end += vol
        rels.append(rel)
        mids.append(mid)
        vols.append(vol)
        fractions.append(
            {"i": len(fractions) + 1, "v_end_ml": v_end, "rel": rel, "flag": flag}
        )
    if len(rels) < 5:
        raise La4Error(f"{name}: a washout needs at least 5 fractions")

    # Fit window: after the peak, below fit_max, and quantifiable (>= LOQ).
    peak = max(range(len(rels)), key=lambda k: rels[k])
    if rels[peak] < 0.8:
        warnings.append(
            f"the first fractions peak at {rels[peak]:.1%} of the stock, not ~100 %: "
            "was the line really full of tracer? (the line volume is then underestimated)"
        )
    window = [
        (mids[k], rels[k])
        for k in range(peak, len(rels))
        if cal.loq <= rels[k] <= fit_max
    ]
    fit: Optional[ExpFit] = None
    tail_fit: Optional[ExpFit] = None
    used = "all"
    if len(window) < 3:
        warnings.append(
            f"only {len(window)} fractions between the LOQ ({cal.loq:.2e}) and {fit_max:.0%}: "
            "no exponential fit (use smaller fractions or a stronger stock)"
        )
    else:
        fit = _fit_exp(window)
        if len(window) >= 6:
            half = len(window) // 2
            head_fit = _fit_exp(window[:half])
            tail_fit = _fit_exp(window[half:])
            if tail_fit.vm > TAIL_WARN_RATIO * head_fit.vm:
                warnings.append(
                    f"tailing: the late half decays over {tail_fit.vm:.0f} mL vs "
                    f"{head_fit.vm:.0f} mL for the early half (a slow dead leg or wall release)"
                )
            # Keep whichever fit asks for the larger purge: the conservative one.
            if tail_fit.volume_for(target) > 1.01 * fit.volume_for(target):
                used = "tail"
        if fit.r2 < 0.95 and used == "all":
            warnings.append(f"poor exponential fit (R2 = {fit.r2:.3f} < 0.95)")
    chosen = tail_fit if used == "tail" else fit

    # Line volume = area under rel(V) (exact for a line fully filled at the stock
    # concentration), plus the exponential tail beyond the last fraction.
    area = sum(r * v for r, v in zip(rels, vols))
    tail_ml = 0.0
    if chosen is not None:
        tail_ml = chosen.vm * chosen.rel_at(v_end)
    elif rels[-1] > cal.loq:
        warnings.append("the last fraction is still above the LOQ and there is no fit: "
                        "the line volume is underestimated (collect more)")
    line_volume = area + tail_ml

    # Model-free purge: the end of the last fraction still above the target (every later
    # fraction is at or below it).
    purge_measured: Optional[float] = None
    if rels[-1] <= target:
        k = len(rels) - 1
        while k >= 0 and rels[k] <= target:
            k -= 1
        purge_measured = 0.0 if k < 0 else float(fractions[k]["v_end_ml"])  # type: ignore[arg-type]
    else:
        warnings.append(f"the outlet never went below the target {target:.2%}: collect more")
    if cal.lod > target:
        warnings.append(
            f"the method's LOD ({cal.lod:.3%}) is above the target ({target:.3%}): "
            "a stronger stock or a more sensitive instrument is needed to prove it"
        )

    purge_needed = sample_avg = factor = None
    if chosen is not None:
        purge_needed = max(0.0, chosen.volume_for(target))
        factor = purge_needed / line_volume if line_volume > 0 else None
        if sample_ml > 0:
            sample_avg = (
                chosen.vm / sample_ml
                * chosen.rel_at(purge_needed)
                * (1.0 - math.exp(-sample_ml / chosen.vm))
            )
    internal = line_volume - bench_hose_ml if bench_hose_ml >= 0 else None
    if internal is not None and internal < 0:
        warnings.append("the line volume is smaller than the bench hose alone: check the hose data")

    return WashoutResult(
        name=name,
        n_fractions=len(rels),
        total_ml=v_end,
        line_volume_ml=line_volume,
        tail_ml=tail_ml,
        fit=fit,
        tail_fit=tail_fit,
        used_fit=used,
        purge_needed_ml=purge_needed,
        purge_measured_ml=purge_measured,
        factor_needed=factor,
        internal_dead_volume_ml=internal,
        sample_avg_at_purge=sample_avg,
        warnings=warnings,
        fractions=fractions,
    )


@dataclass
class WashoutSummary:
    runs: int
    internal_dead_volume_ml: Optional[float]  # mean over runs
    internal_sd_ml: Optional[float]
    factor_max: Optional[float]
    recommended_factor: Optional[float]  # factor_max x margin, rounded up to 0.5
    recommended_internal_ml: Optional[float]  # mean rounded up to 5 mL
    field_hose_ml: float
    field_purge_ml: Optional[float]
    warnings: List[str] = field(default_factory=list)


def summarize_washout(
    results: List[WashoutResult],
    field_hose_ml: float,
) -> WashoutSummary:
    warnings: List[str] = []
    internals = [r.internal_dead_volume_ml for r in results if r.internal_dead_volume_ml is not None]
    factors = [r.factor_needed for r in results if r.factor_needed is not None]
    mean = sd = None
    if internals:
        mean = sum(internals) / len(internals)
        if len(internals) > 1:
            sd = math.sqrt(sum((x - mean) ** 2 for x in internals) / (len(internals) - 1))
            if mean > 0 and sd / mean > 0.2:
                warnings.append(f"the runs disagree: internal volume spread {sd / mean:.0%} > 20 %")
    if len(results) < 3:
        warnings.append(f"{len(results)} run(s): the protocol asks for 3")
    fmax = max(factors) if factors else None
    rec_factor = math.ceil(fmax * FACTOR_MARGIN * 2.0) / 2.0 if fmax is not None else None
    rec_internal = math.ceil(max(mean, 0.0) / 5.0) * 5.0 if mean is not None else None
    field_purge = None
    if rec_factor is not None and rec_internal is not None:
        field_purge = rec_factor * (field_hose_ml + rec_internal)
    return WashoutSummary(
        runs=len(results),
        internal_dead_volume_ml=mean,
        internal_sd_ml=sd,
        factor_max=fmax,
        recommended_factor=rec_factor,
        recommended_internal_ml=rec_internal,
        field_hose_ml=field_hose_ml,
        field_purge_ml=field_purge,
        warnings=warnings,
    )


# ------------------------------------------------------------------------- carry-over


@dataclass
class CarryoverResult:
    outlet_ml: float
    first_fraction_rel: float
    tracer_ml_outlet: float  # mL of stock found in the outlet water
    tracer_ml_eluate: float  # mL of stock found in the filter eluate(s)
    carryover: float  # (outlet + eluate tracer) / outlet volume
    upper_bound: float  # the same with every <LOD value counted at the LOD
    verdict: str  # PASS / FAIL / INCONCLUSIVE
    warnings: List[str] = field(default_factory=list)
    fractions: List[Dict[str, object]] = field(default_factory=list)


def analyze_carryover(
    rows: List[Dict[str, str]], cal: Calibration, target: float = DEFAULT_TARGET
) -> CarryoverResult:
    """Test b. Rows in collection order: ``kind`` (outlet|eluate), ``volume_ml``, ``reading``,
    optional ``dilution``."""
    warnings: List[str] = []
    fractions: List[Dict[str, object]] = []
    outlet_ml = tr_out = tr_el = ub = 0.0
    first: Optional[float] = None
    for i, row in enumerate(rows, start=2):
        where = f"slot B line {i}"
        kind = (row.get("kind") or "outlet").strip().lower()
        if kind not in ("outlet", "eluate"):
            raise La4Error(f"{where}: kind must be outlet or eluate (got '{kind}')")
        vol = _float(row, "volume_ml", where)
        reading = _float(row, "reading", where)
        assert vol is not None and reading is not None
        if vol <= 0:
            raise La4Error(f"{where}: volume_ml must be > 0")
        rel, lod, flag = measure(cal, reading, _dilution(row, "dilution", where))
        ub += max(rel, lod) * vol
        if kind == "outlet":
            outlet_ml += vol
            tr_out += rel * vol
            if first is None:
                first = rel
        else:
            tr_el += rel * vol
        fractions.append({"kind": kind, "volume_ml": vol, "rel": rel, "flag": flag})
    if outlet_ml <= 0 or first is None:
        raise La4Error("slot B: at least one outlet fraction is needed")
    co = (tr_out + tr_el) / outlet_ml
    upper = ub / outlet_ml
    if co >= target:
        verdict = "FAIL"
    elif upper >= target:
        verdict = "INCONCLUSIVE"
        warnings.append("below the LOD, but the LOD is too high to prove the target")
    else:
        verdict = "PASS"
    return CarryoverResult(
        outlet_ml=outlet_ml,
        first_fraction_rel=first,
        tracer_ml_outlet=tr_out,
        tracer_ml_eluate=tr_el,
        carryover=co,
        upper_bound=upper,
        verdict=verdict,
        warnings=warnings,
        fractions=fractions,
    )


# -------------------------------------------------------------------------- cross-talk


@dataclass
class CrosstalkRow:
    slot: str
    role: str
    clock_min: Optional[float]
    distance_deg: Optional[float]
    neighbour: bool
    cycle_order: Optional[int]
    passed_over: Optional[bool]  # the rotor crossed the source slot before this cycle
    tracer_ml: float  # mL of source liquid found in this slot (outlet + eluate)
    fraction_of_source: float
    upper_bound: float
    flags: str
    verdict: str


@dataclass
class CrosstalkResult:
    source_slot: str
    source_ml: float
    neighbour_deg: float
    rows: List[CrosstalkRow]
    verdict: str
    warnings: List[str] = field(default_factory=list)


def _clock_distance_deg(a: float, b: float) -> float:
    d = abs(a - b) % 60.0
    return min(d, 60.0 - d) * 6.0


def analyze_crosstalk(
    rows: List[Dict[str, str]],
    cal: Calibration,
    source_ml: float,
    target: float = DEFAULT_TARGET,
    neighbour_deg: Optional[float] = None,
) -> CrosstalkResult:
    """Test c. One row per slot.

    Columns: ``slot``, ``role`` (source|test|control), ``clock_min`` (from the console's
    ``clocklist``), optional ``cycle_order``, and for each measured liquid a volume + reading:
    ``outlet_ml``/``outlet_reading`` and ``eluate_ml``/``eluate_reading`` (either may be empty),
    optional ``outlet_dilution``/``eluate_dilution``, and ``passed_over`` (y/n: the rotor crossed
    the source slot since the previous cycle).
    ``fraction_of_source`` = the source liquid found in the slot / the source liquid put in A.
    """
    if source_ml <= 0:
        raise La4Error("--source-ml (the tracer volume put into the source slot) must be > 0")
    warnings: List[str] = []
    sources = [r for r in rows if (r.get("role") or "").strip().lower() == "source"]
    if len(sources) != 1:
        raise La4Error(f"cross-talk: exactly one row must have role=source (found {len(sources)})")
    src = sources[0]
    src_clock = _float(src, "clock_min", "source row", required=False)

    clocks = [
        c for c in (_float(r, "clock_min", "clock", required=False) for r in rows) if c is not None
    ]
    if neighbour_deg is None:
        dists = sorted(
            {_clock_distance_deg(a, b) for a in clocks for b in clocks if a != b}
        )
        # 1.5 x the smallest spacing in the file: the adjacent positions, whatever the slot count.
        neighbour_deg = 1.5 * dists[0] if dists else 0.0

    out: List[CrosstalkRow] = []
    worst = "PASS"
    order = {"PASS": 0, "INCONCLUSIVE": 1, "FAIL": 2}
    for i, row in enumerate(rows, start=2):
        where = f"cross-talk line {i}"
        slot = (row.get("slot") or "").strip()
        role = (row.get("role") or "test").strip().lower()
        if role not in ("source", "test", "control"):
            raise La4Error(f"{where}: role must be source, test or control")
        if role == "source":
            continue
        clock = _float(row, "clock_min", where, required=False)
        dist = (
            _clock_distance_deg(clock, src_clock)
            if clock is not None and src_clock is not None
            else None
        )
        co = _float(row, "cycle_order", where, required=False)
        po_raw = (row.get("passed_over") or "").strip().lower()
        if po_raw not in ("", "y", "n", "yes", "no"):
            raise La4Error(f"{where}: passed_over must be y or n")
        passed_over = None if po_raw == "" else po_raw.startswith("y")
        tracer = ub = 0.0
        flags: List[str] = []
        measured = False
        for part in ("outlet", "eluate"):
            vol = _float(row, f"{part}_ml", where, required=False)
            reading = _float(row, f"{part}_reading", where, required=False)
            if vol is None and reading is None:
                continue
            if vol is None or reading is None:
                raise La4Error(f"{where}: {part}_ml and {part}_reading go together")
            measured = True
            rel, lod, f = measure(cal, reading, _dilution(row, f"{part}_dilution", where))
            tracer += rel * vol
            ub += max(rel, lod) * vol
            if f:
                flags.append(f"{part} {f}")
        if not measured:
            raise La4Error(f"{where}: slot {slot} has no measurement")
        frac = tracer / source_ml
        upper = ub / source_ml
        if frac >= target:
            verdict = "FAIL"
        elif upper >= target:
            verdict = "INCONCLUSIVE"
        else:
            verdict = "PASS"
        if role == "control":
            if frac >= target:
                warnings.append(f"control slot {slot} shows tracer: background or handling contamination")
            verdict = "control"
        elif order[verdict] > order[worst]:
            worst = verdict
        out.append(
            CrosstalkRow(
                slot=slot,
                role=role,
                clock_min=clock,
                distance_deg=dist,
                neighbour=dist is not None and 0 < dist <= neighbour_deg,
                cycle_order=int(co) if co is not None else None,
                passed_over=passed_over,
                tracer_ml=tracer,
                fraction_of_source=frac,
                upper_bound=upper,
                flags=", ".join(flags),
                verdict=verdict,
            )
        )
    if not any(r.role == "test" for r in out):
        raise La4Error("cross-talk: no test slot")
    if not any(r.neighbour for r in out):
        warnings.append("no neighbour of the source slot was measured (check clock_min)")
    out.sort(key=lambda r: (r.distance_deg is None, r.distance_deg or 0.0, r.slot))
    return CrosstalkResult(
        source_slot=(src.get("slot") or "").strip(),
        source_ml=source_ml,
        neighbour_deg=neighbour_deg,
        rows=out,
        verdict=worst,
        warnings=warnings,
    )


# ------------------------------------------------------------------------------ report


def _pct(x: Optional[float], digits: int = 3) -> str:
    return "n/a" if x is None else f"{x * 100:.{digits}f} %"


def _ml(x: Optional[float]) -> str:
    return "n/a" if x is None else f"{x:.0f} mL"


def _print_cal(cal: Calibration) -> None:
    print(
        f"Calibration: blank {cal.blank_mean:.4g} +/- {cal.blank_sd:.2g} (SD), "
        f"LOD {_pct(cal.lod, 4)}, LOQ {_pct(cal.loq, 4)} of the stock, top standard {_pct(cal.top, 1)}"
    )


def _print_warnings(prefix: str, warnings: List[str]) -> None:
    for w in warnings:
        print(f"  WARNING {prefix}: {w}")


def report_washout(results: List[WashoutResult], summary: WashoutSummary, target: float) -> None:
    for r in results:
        print(f"\n== {r.name}: {r.n_fractions} fractions, {r.total_ml:.0f} mL collected")
        for f in r.fractions:
            print(f"  {f['i']:>3}  V={f['v_end_ml']:>7.1f} mL  rel={f['rel']:.3e}  {f['flag']}")
        fit = r.tail_fit if r.used_fit == "tail" else r.fit
        if fit is not None:
            print(
                f"  fit ({r.used_fit}): rel = exp({fit.a:.3f} - V/{fit.vm:.1f} mL), "
                f"R2={fit.r2:.4f}, n={fit.n}, {fit.v_from:.0f}-{fit.v_to:.0f} mL"
            )
        print(f"  line volume (area)      : {_ml(r.line_volume_ml)} (tail {r.tail_ml:.1f} mL)")
        print(f"  internal dead volume    : {_ml(r.internal_dead_volume_ml)}")
        factor = "" if r.factor_needed is None else f"  -> factor {r.factor_needed:.2f} x line volume"
        print(f"  purge for {_pct(target, 2)} (fit)  : {_ml(r.purge_needed_ml)}{factor}")
        print(f"  purge (measured)        : {_ml(r.purge_measured_ml)}")
        print(f"  sample-averaged carry-over at that purge: {_pct(r.sample_avg_at_purge, 5)}")
        _print_warnings(r.name, r.warnings)
    s = summary
    print(f"\n== Summary over {s.runs} run(s)")
    sd = f" +/- {s.internal_sd_ml:.1f}" if s.internal_sd_ml is not None else ""
    print(f"  INTERNAL_DEAD_VOLUME_ML  : mean {_ml(s.internal_dead_volume_ml)}{sd} -> {_ml(s.recommended_internal_ml)}")
    fmax = "n/a" if s.factor_max is None else f"{s.factor_max:.2f}"
    print(f"  factor needed (max)      : {fmax}")
    print(f"  PURGE_DEAD_VOLUME_FACTOR : {s.recommended_factor} (max x {FACTOR_MARGIN}, rounded up to 0.5)")
    print(f"  field purge              : {_ml(s.field_purge_ml)} "
          f"(= factor x ({s.field_hose_ml:.0f} mL hose + internal))")
    _print_warnings("summary", s.warnings)


def report_carryover(r: CarryoverResult, target: float) -> None:
    print("\n== Carry-over into slot B")
    for f in r.fractions:
        print(f"  {f['kind']:<7} {f['volume_ml']:>7.1f} mL  rel={f['rel']:.3e}  {f['flag']}")
    print(f"  first outlet fraction   : {_pct(r.first_fraction_rel, 4)} of the stock")
    print(f"  tracer found            : outlet {r.tracer_ml_outlet:.3f} mL + eluate {r.tracer_ml_eluate:.3f} mL of stock")
    print(f"  carry-over              : {_pct(r.carryover, 4)} (upper bound {_pct(r.upper_bound, 4)})")
    print(f"  VERDICT (< {_pct(target, 2)}) : {r.verdict}")
    _print_warnings("slot B", r.warnings)


def report_crosstalk(r: CrosstalkResult, target: float) -> None:
    print(f"\n== Cross-talk from slot {r.source_slot} ({r.source_ml:g} mL of tracer); "
          f"neighbours = within {r.neighbour_deg:.0f} deg")
    print(f"  {'slot':>4} {'role':<8} {'dist':>6} {'nbr':>3} {'order':>5} {'over':>4} {'of source':>12} {'upper':>12}  verdict  flags")
    for row in r.rows:
        dist = "n/a" if row.distance_deg is None else f"{row.distance_deg:.0f}d"
        order = "" if row.cycle_order is None else str(row.cycle_order)
        over = "" if row.passed_over is None else ("yes" if row.passed_over else "no")
        print(
            f"  {row.slot:>4} {row.role:<8} {dist:>6} {'yes' if row.neighbour else '':>3} {order:>5} {over:>4} "
            f"{_pct(row.fraction_of_source, 4):>12} {_pct(row.upper_bound, 4):>12}  {row.verdict:<8} {row.flags}"
        )
    print(f"  VERDICT (every test slot < {_pct(target, 2)} of the source): {r.verdict}")
    _print_warnings("cross-talk", r.warnings)


def _jsonable(obj: object) -> object:
    if hasattr(obj, "__dataclass_fields__"):
        return asdict(obj)  # type: ignore[call-overload]
    if isinstance(obj, list):
        return [_jsonable(o) for o in obj]
    return obj


# --------------------------------------------------------------------------------- CLI


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)

    def common(sp: argparse.ArgumentParser) -> None:
        sp.add_argument("standards", help="calibration CSV: fraction,reading (fraction 0 = blank)")
        sp.add_argument("--blank-sd", type=float, help="blank noise (SD, reading units) if < 2 blanks")
        sp.add_argument("--target", type=float, default=DEFAULT_TARGET,
                        help="carry-over target as a fraction (default 0.001 = 0.1 %%)")
        sp.add_argument("--json", action="store_true", help="print JSON instead of tables")

    w = sub.add_parser("washout", help="test a: dead volume and purge factor")
    common(w)
    w.add_argument("runs", nargs="+", help="one CSV per run: volume_ml|mass_g,reading[,dilution]")
    w.add_argument("--bench-hose-ml", type=float, help="inner volume of the bench hose used (mL)")
    w.add_argument("--bench-hose-id-mm", type=float, help="or its ID (mm) ...")
    w.add_argument("--bench-hose-length-m", type=float, help="... and length (m)")
    w.add_argument("--field-hose-id-mm", type=float, default=DEFAULT_FIELD_HOSE_ID_MM,
                   help="field intake hose ID, HW6 (default 6 mm, PLACEHOLDER)")
    w.add_argument("--field-hose-length-m", type=float, default=DEFAULT_FIELD_HOSE_LENGTH_M,
                   help="field intake hose length, HW6 (default 5 m, PLACEHOLDER)")
    w.add_argument("--fit-max", type=float, default=DEFAULT_FIT_MAX,
                   help="fit only fractions below this fraction of the stock (default 0.2)")
    w.add_argument("--sample-ml", type=float, default=2000.0, help="sample volume (default 2000)")
    w.add_argument("--density", type=float, default=1.0, help="g/mL for mass_g (default 1.0)")

    c = sub.add_parser("carryover", help="test b: carry-over into the next filter")
    common(c)
    c.add_argument("slot_b", help="CSV: kind(outlet|eluate),volume_ml,reading[,dilution]")

    x = sub.add_parser("crosstalk", help="test c: cross-talk between slots")
    common(x)
    x.add_argument("slots", help="CSV: slot,role,clock_min,cycle_order,passed_over,outlet_ml,outlet_reading,"
                                "eluate_ml,eluate_reading (+ optional outlet_dilution, eluate_dilution)")
    x.add_argument("--source-ml", type=float, required=True, help="tracer volume put into the source slot (mL)")
    x.add_argument("--neighbour-deg", type=float,
                   help="max angle for a neighbour (default 1.5 x the smallest spacing in the file)")
    return p


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)
    try:
        cal = Calibration.from_rows(read_csv(args.standards), args.blank_sd)
        if args.cmd == "washout":
            if args.bench_hose_ml is not None:
                bench = args.bench_hose_ml
            elif args.bench_hose_id_mm is not None and args.bench_hose_length_m is not None:
                bench = hose_volume_ml(args.bench_hose_id_mm, args.bench_hose_length_m)
            else:
                raise La4Error("give --bench-hose-ml, or --bench-hose-id-mm and --bench-hose-length-m "
                               "(0 if the machine inlet was fed directly)")
            results = [
                analyze_washout(path, read_csv(path), cal, args.target, args.fit_max, bench,
                                args.sample_ml, args.density)
                for path in args.runs
            ]
            summary = summarize_washout(
                results, hose_volume_ml(args.field_hose_id_mm, args.field_hose_length_m)
            )
            if args.json:
                print(json.dumps({"calibration": _jsonable(cal), "runs": _jsonable(results),
                                  "summary": _jsonable(summary)}, indent=2))
            else:
                _print_cal(cal)
                report_washout(results, summary, args.target)
        elif args.cmd == "carryover":
            res = analyze_carryover(read_csv(args.slot_b), cal, args.target)
            if args.json:
                print(json.dumps({"calibration": _jsonable(cal), "result": _jsonable(res)}, indent=2))
            else:
                _print_cal(cal)
                report_carryover(res, args.target)
        else:
            res2 = analyze_crosstalk(read_csv(args.slots), cal, args.source_ml, args.target,
                                     args.neighbour_deg)
            if args.json:
                print(json.dumps({"calibration": _jsonable(cal), "result": _jsonable(res2)}, indent=2))
            else:
                _print_cal(cal)
                report_crosstalk(res2, args.target)
    except (La4Error, OSError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
