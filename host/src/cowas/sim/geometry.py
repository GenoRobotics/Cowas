"""Manifold geometries for the simulator, as GET_INFO data (§5.7).

`cowas_v1()` mirrors the firmware today (include/Settings.h, include/Manifold.h): 16 raw
positions, the one 4 steps from purge has no hole, so 15 slots, purge = 0, FILL_ORDER as in
Settings.h. Nothing else in the host assumes this: it always reads GET_INFO.
"""

from __future__ import annotations

import math
from collections.abc import Sequence

from cowas.protocol.models import GetInfoData, Limits, Position

V1_FILL_ORDER = (1, 2, 3, 14, 13, 12, 11, 10, 9, 8, 4, 5, 6, 7)
V1_PLACEHOLDERS = ("LINEAR_ACT_LOCKED_STEPS", "calibrationFactor_small", "DNA_SHIELD_FILL_MS")


def regular_geometry(
    raw_positions: int,
    omitted_raw: Sequence[int] = (),
    *,
    reference_angle_deg: float = 162.07,
    purge_slot: int = 0,
    fill_order: Sequence[int] | None = None,
    limits: Limits | None = None,
    placeholders: Sequence[str] = (),
) -> GetInfoData:
    """Evenly spaced raw positions (some without a hole); human slot i = the i-th raw position
    with a hole, counted from the reference. Clock labels as the firmware's `clocklist`."""
    raws = [r for r in range(raw_positions) if r not in set(omitted_raw)]
    pitch = 360.0 / raw_positions
    positions = [
        Position(
            slot=i,
            angle_deg=round((reference_angle_deg - pitch * r) % 360.0, 2),
            clock=math.floor(r * 60.0 / raw_positions + 30.0 + 0.5) % 60,
        )
        for i, r in enumerate(raws)
    ]
    samples = [s for s in range(len(raws)) if s != purge_slot]
    return GetInfoData(
        slot_count=len(raws),
        purge_slot=purge_slot,
        sample_slots=samples,
        fill_order=list(fill_order) if fill_order is not None else samples,
        raw_positions=raw_positions,
        clock_dir="cw",
        positions=positions,
        limits=limits or Limits(),
        placeholders=list(placeholders),
    )


def cowas_v1(limits: Limits | None = None) -> GetInfoData:
    return regular_geometry(
        16,
        (4,),
        fill_order=V1_FILL_ORDER,
        limits=limits,
        placeholders=V1_PLACEHOLDERS,
    )
