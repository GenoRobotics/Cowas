"""Loading protocol/vectors.json and its matching rule (docs/software/protocol.md §13).

Used by the tests (and usable by bench tools): every key of an expected object must be in the
actual one with an equal value; "$num", "$str", "$bool", "$any" match any value of that
type ("$any" includes null); arrays must have the same length and match element by element.
"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

WILDCARDS = ("$num", "$str", "$bool", "$any")


def find_vectors_file(start: Path | None = None) -> Path:
    """protocol/vectors.json in this checkout (searched upwards from `start` or this file)."""
    here = (start or Path(__file__)).resolve()
    for parent in [here, *here.parents]:
        candidate = parent / "protocol" / "vectors.json"
        if candidate.is_file():
            return candidate
    raise FileNotFoundError("protocol/vectors.json not found above " + str(here))


def load_vectors(path: Path | None = None) -> dict[str, Any]:
    with open(path or find_vectors_file(), encoding="utf-8") as fh:
        return json.load(fh)


def request_line(vector: dict[str, Any]) -> str:
    """The host line of a request vector: `line`, or `line_gen` expanded to `length` bytes."""
    if "line" in vector:
        return vector["line"]
    gen = vector["line_gen"]
    prefix, fill, suffix = gen["prefix"], gen["fill"], gen.get("suffix", "")
    n_fill = gen["length"] - len(prefix.encode()) - len(suffix.encode())
    body = (fill * (n_fill // len(fill) + 1))[:n_fill]
    line = prefix + body + suffix
    assert len(line.encode()) == gen["length"]
    return line


def merged_state(defaults: dict[str, Any], state: dict[str, Any] | None) -> dict[str, Any]:
    """defaults.state updated with a vector's state; `caps` and `limits` merge key by key."""
    out = json.loads(json.dumps(defaults))
    for key, value in (state or {}).items():
        if key in ("caps", "limits") and isinstance(value, dict):
            out[key] = {**out.get(key, {}), **value}
        else:
            out[key] = value
    return out


def _mismatch(expected: Any, actual: Any, path: str) -> str | None:
    if isinstance(expected, str) and expected in WILDCARDS:
        ok = {
            "$num": isinstance(actual, (int, float)) and not isinstance(actual, bool),
            "$str": isinstance(actual, str),
            "$bool": isinstance(actual, bool),
            "$any": True,
        }[expected]
        return None if ok else f"{path}: expected {expected}, got {actual!r}"
    if isinstance(expected, dict):
        if not isinstance(actual, dict):
            return f"{path}: expected an object, got {actual!r}"
        for key, sub in expected.items():
            if key not in actual:
                return f"{path}.{key}: missing"
            problem = _mismatch(sub, actual[key], f"{path}.{key}")
            if problem:
                return problem
        return None
    if isinstance(expected, list):
        if not isinstance(actual, list) or len(actual) != len(expected):
            return f"{path}: expected {len(expected)} items, got {actual!r}"
        for i, (e, a) in enumerate(zip(expected, actual, strict=True)):
            problem = _mismatch(e, a, f"{path}[{i}]")
            if problem:
                return problem
        return None
    # bool is not a number and a number is not a bool (1 == True in Python).
    if isinstance(expected, bool) != isinstance(actual, bool) or expected != actual:
        return f"{path}: expected {expected!r}, got {actual!r}"
    return None


def mismatch(expected: Any, actual: Any) -> str | None:
    """None when `actual` matches `expected`, else a short description of the first difference."""
    return _mismatch(expected, actual, "$")


def matches(expected: Any, actual: Any) -> bool:
    return mismatch(expected, actual) is None
