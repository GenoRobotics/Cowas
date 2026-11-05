"""The sample loading order and the sample availability of one session (the `slots.json` next to the session log).

The operator types `run_sample` and the tool picks the next sample in LOADING_ORDER that is still available. The
loading order is the firmware's FILL_ORDER (include/Settings.h): the tool reads it from there, so there is one order.
Pure logic: no port, no prompts (session_log.py asks the questions).
"""

import json
import os
import re
import threading

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SETTINGS_H = os.path.join(REPO_ROOT, "include", "Settings.h")


def read_fill_order(path=SETTINGS_H):
    """FILL_ORDER from Settings.h, as a tuple of sample slots. It must be each sample 1..n once (n = its length)."""
    with open(path, encoding="utf-8") as f:
        text = f.read()
    found = re.search(r"FILL_ORDER\[\d+\]\s*=\s*\{([^}]*)\}", text)
    if not found:
        raise ValueError(f"{path}: FILL_ORDER not found")
    order = tuple(int(x) for x in found.group(1).split(","))
    if sorted(order) != list(range(1, len(order) + 1)):
        raise ValueError(
            f"{path}: FILL_ORDER is not each sample 1 to {len(order)} once"
        )
    return order


# Slot numbers are the sample numbers written on the device. Slot 0 is the purge and is never a sample.
LOADING_ORDER = read_fill_order()

AVAILABLE = "available"
USED = "used"
UNAVAILABLE = "unavailable"  # not loaded in this session (the session start said how many are loaded)

STATE_NAME = "slots.json"


def parse_slot_list(text):
    """The samples typed at the start of a session: '1,2,3', '1 2 3', '2-5,9' or none/ENTER (empty set).
    Returns a set of samples. Raises ValueError with the reason (a number that is not a sample, or a bad word)."""
    text = text.strip().lower()
    if text in ("", "none", "no", "n"):
        return set()
    chosen = set()
    for token in text.replace(",", " ").split():
        if "-" in token:
            first, _, last = token.partition("-")
            if not (first.isdigit() and last.isdigit()) or int(first) > int(last):
                raise ValueError(f"not a range: {token!r} (use 2-5)")
            numbers = range(int(first), int(last) + 1)
        elif token.isdigit():
            numbers = [int(token)]
        else:
            raise ValueError(f"not a sample number: {token!r}")
        for n in numbers:
            if n not in LOADING_ORDER:
                raise ValueError(f"slot {n} is not a sample (1 to {len(LOADING_ORDER)})")
            chosen.add(n)
    return chosen


class SlotBook:
    """The status of each sample in one session. `path` is written on every change."""

    def __init__(self, path, slots, loaded):
        self.path = path
        self.slots = dict(slots)  # slot (int) -> AVAILABLE / USED / UNAVAILABLE
        self.loaded = loaded
        self._lock = (
            threading.Lock()
        )  # the reader thread marks samples used; the operator's lines read them

    @classmethod
    def new(cls, path, loaded):
        """All samples available except the ones after the first `loaded` in LOADING_ORDER."""
        if not 0 <= loaded <= len(LOADING_ORDER):
            raise ValueError(f"loaded must be 0 to {len(LOADING_ORDER)}, not {loaded}")
        slots = {
            s: (AVAILABLE if i < loaded else UNAVAILABLE)
            for i, s in enumerate(LOADING_ORDER)
        }
        book = cls(path, slots, loaded)
        book.save()
        return book

    @classmethod
    def load(cls, path):
        """The book saved in `path`, or None if there is none yet."""
        if not os.path.exists(path):
            return None
        with open(path, encoding="utf-8") as f:
            data = json.load(f)
        slots = {int(k): v for k, v in data["slots"].items()}
        if set(slots) != set(LOADING_ORDER):
            raise ValueError(
                f"{path}: the slots are not the samples 1 to 14 of the loading order"
            )
        return cls(path, slots, data["loaded"])

    def save(self):
        """Writes the book to a temporary file, then replaces the old one, so a cut write leaves the old state."""
        with self._lock:
            data = {
                "loading_order": list(LOADING_ORDER),
                "loaded": self.loaded,
                "slots": {str(s): self.slots[s] for s in LOADING_ORDER},
            }
            tmp = self.path + ".tmp"
            with open(tmp, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=2)
                f.write("\n")
            os.replace(tmp, self.path)

    def status(self, slot):
        """AVAILABLE, USED or UNAVAILABLE, or None for a slot that is not a sample."""
        with self._lock:
            return self.slots.get(slot)

    def next_available(self, exclude=()):
        """The first sample in LOADING_ORDER that is available and not in `exclude`, or None if none is left.
        `exclude`: the slots a schedule row forces (2026-11-03): the automatic order must not take them."""
        with self._lock:
            for s in LOADING_ORDER:
                if self.slots[s] == AVAILABLE and s not in exclude:
                    return s
        return None

    def set_available(self, available):
        """The start question (2026-11-03): `available` is the set of samples that can be run now. Each of them becomes
        AVAILABLE (also a used one, when the operator says it is available again). A sample that was available and is not
        listed becomes USED. A sample not loaded stays not loaded."""
        with self._lock:
            for s in LOADING_ORDER:
                if s in available:
                    self.slots[s] = AVAILABLE
                elif self.slots[s] == AVAILABLE:
                    self.slots[s] = USED
        self.save()

    def mark_used(self, slot):
        """A run_sample has started on `slot`. Slots that are not samples are ignored."""
        if slot not in self.slots:
            return
        with self._lock:
            self.slots[slot] = USED
        self.save()

    def release(self, slot):
        """A run stopped before its filter was loaded (the dive or the purge): the filter is unused, so the slot is available
        again. A slot that is not used is left as it is."""
        with self._lock:
            if self.slots.get(slot) == USED:
                self.slots[slot] = AVAILABLE
        self.save()

    def erase(self, slot):
        """Makes `slot` available again (the operator forces it). The sample records already saved are not touched."""
        with self._lock:
            self.slots[slot] = AVAILABLE
        self.save()

    def summary(self):
        """One line for the screen: the loading order with each status."""
        with self._lock:
            parts = []
            for s in LOADING_ORDER:
                mark = {AVAILABLE: "", USED: " (used)", UNAVAILABLE: " (not loaded)"}[
                    self.slots[s]
                ]
                parts.append(f"{s}{mark}")
        return ", ".join(parts)
