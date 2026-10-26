import datetime as dt
import re
import sys
from pathlib import Path

import pytest

sys.path.insert(
    0, str(Path(__file__).resolve().parents[1])
)  # tools/ is a folder of scripts, not a package

import sample_slots as ss
import session_log as sl

TODAY = dt.date(2026, 10, 26)
SETTINGS = Path(__file__).resolve().parents[2] / "include" / "Settings.h"


def firmware_fill_order():
    text = SETTINGS.read_text(encoding="utf-8")
    body = re.search(r"FILL_ORDER\[14\]\s*=\s*\{([^}]*)\}", text).group(1)
    return [int(x) for x in body.split(",")]


def test_loading_order_is_the_samples_1_to_14_once_each():
    assert sorted(ss.LOADING_ORDER) == list(range(1, 15))


def test_loading_order_is_the_firmware_fill_order():
    # one order for the firmware (rotate_all, run_sample) and the tool: the tool reads FILL_ORDER from Settings.h
    assert ss.LOADING_ORDER == tuple(firmware_fill_order())
    assert ss.LOADING_ORDER == (1, 2, 3, 14, 13, 12, 11, 10, 9, 8, 4, 5, 6, 7)


def test_read_fill_order_refuses_a_broken_order(tmp_path):
    bad = tmp_path / "Settings.h"
    bad.write_text("const uint8_t FILL_ORDER[3] = {1, 1, 2};\n", encoding="utf-8")
    with pytest.raises(ValueError):
        ss.read_fill_order(str(bad))


def test_new_book_marks_the_first_loaded_as_available(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 5)
    assert [book.status(s) for s in ss.LOADING_ORDER[:5]] == [ss.AVAILABLE] * 5
    assert [book.status(s) for s in ss.LOADING_ORDER[5:]] == [ss.UNAVAILABLE] * 9


def test_new_book_with_all_loaded_and_none(tmp_path):
    all_loaded = ss.SlotBook.new(str(tmp_path / "a.json"), 14)
    assert all(all_loaded.status(s) == ss.AVAILABLE for s in range(1, 15))
    none = ss.SlotBook.new(str(tmp_path / "b.json"), 0)
    assert none.next_available() is None


@pytest.mark.parametrize("loaded", [-1, 15])
def test_new_book_refuses_a_count_out_of_range(tmp_path, loaded):
    with pytest.raises(ValueError):
        ss.SlotBook.new(str(tmp_path / "slots.json"), loaded)


def test_book_round_trips_through_the_file(tmp_path):
    path = str(tmp_path / "slots.json")
    book = ss.SlotBook.new(path, 14)
    book.mark_used(3)
    loaded = ss.SlotBook.load(path)
    assert loaded.status(3) == ss.USED
    assert loaded.status(1) == ss.AVAILABLE
    assert loaded.loaded == 14


def test_load_returns_none_when_no_file(tmp_path):
    assert ss.SlotBook.load(str(tmp_path / "missing.json")) is None


def test_next_available_follows_the_loading_order_and_skips_used(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 14)
    picked = []
    for _ in range(14):
        slot = book.next_available()
        picked.append(slot)
        book.mark_used(slot)
    assert picked == list(ss.LOADING_ORDER)
    assert book.next_available() is None


def test_mark_used_ignores_the_purge_and_slots_out_of_range(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 14)
    book.mark_used(0)
    book.mark_used(15)
    assert book.status(0) is None
    assert book.status(15) is None


def test_erase_makes_a_slot_available_again(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 14)
    book.mark_used(9)
    book.erase(9)
    assert book.status(9) == ss.AVAILABLE
    assert book.next_available() == 1


def answers(*items):
    """A `read` that returns the items in turn and records the prompts."""
    queue = list(items)
    prompts = []

    def read(prompt):
        prompts.append(prompt)
        return queue.pop(0)

    return read, prompts


def collect():
    lines = []
    return lines.append, lines


# --- the start of a folder: how many are loaded


def test_ask_loaded_confirms_the_count_once_sorted():
    read, _ = answers("5", "y")
    write, lines = collect()
    assert sl.ask_loaded(read, write) == 5
    assert any("1, 2, 3, 14, 13" in line for line in lines)


def test_ask_loaded_asks_again_when_not_confirmed_and_when_out_of_range():
    read, _ = answers("20", "3", "n", "3", "y")
    write, lines = collect()
    assert sl.ask_loaded(read, write) == 3
    assert any("Type a number from 0 to 14" in line for line in lines)
    assert any("Not confirmed" in line for line in lines)


# --- routing of a typed run_sample


def routed(line, book, *items):
    read, prompts = answers(*items)
    write, lines = collect()
    return sl.route_run_sample(line, book, read, write), lines, prompts


def test_bare_run_sample_sends_the_next_available_slot(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 14)
    book.mark_used(1)
    out, lines, _ = routed("run_sample", book)
    assert out == "run_sample 2"
    assert any("next sample in the loading order: 2" in line for line in lines)


def test_bare_run_sample_keeps_its_extra_arguments(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 14)
    out, _, _ = routed("run_sample volume=100", book)
    assert out == "run_sample 1 volume=100"


def test_bare_run_sample_refuses_when_none_is_left(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 1)
    book.mark_used(1)
    out, lines, _ = routed("run_sample", book)
    assert out is None
    assert any("REFUSED" in line for line in lines)


def test_run_sample_with_an_available_slot_is_sent_without_a_question(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 14)
    out, _, prompts = routed("run_sample 7 volume=50", book)
    assert out == "run_sample 7 volume=50"
    assert prompts == []


def test_run_sample_of_a_used_slot_asks_to_erase_and_yes_erases(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 14)
    book.mark_used(3)
    out, _, prompts = routed("run_sample 3", book, "y")
    assert out == "run_sample 3"
    assert len(prompts) == 1 and "used" in prompts[0]
    assert book.status(3) == ss.AVAILABLE


def test_run_sample_of_a_used_slot_no_sends_nothing(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 14)
    book.mark_used(3)
    out, _, _ = routed("run_sample 3", book, "")
    assert out is None
    assert book.status(3) == ss.USED


def test_run_sample_of_an_unloaded_slot_asks_to_erase(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 2)
    out, _, prompts = routed("run_sample 6", book, "n")
    assert out is None
    assert "unavailable" in prompts[0]


def test_run_sample_of_the_purge_or_a_wrong_slot_is_refused(tmp_path):
    book = ss.SlotBook.new(str(tmp_path / "slots.json"), 14)
    for line in ("run_sample 0", "run_sample 15"):
        out, lines, _ = routed(line, book)
        assert out is None
        assert any("not a sample" in line for line in lines)


# --- the device tells the sample is used (META line of run_sample)


def test_the_meta_line_of_a_run_sample_marks_its_sample_used(tmp_path):
    session = sl.Session(str(tmp_path), "t", today=TODAY)
    book = ss.SlotBook.new(str(tmp_path / "day-slots.json"), 14)
    sl.on_device_line(session, "META|sample;slot=4;name=A;condition=c;notes=", book)
    assert book.status(4) == ss.USED
    session.close()


def test_other_device_lines_do_not_change_the_book(tmp_path):
    session = sl.Session(str(tmp_path), "t", today=TODAY)
    book = ss.SlotBook.new(str(tmp_path / "day-slots.json"), 14)
    sl.on_device_line(session, "META|purge;slot=0", book)
    sl.on_device_line(session, "TEST|run_sample|4|PASS|x=1", book)
    assert book.status(4) == ss.AVAILABLE
    session.close()


def test_the_log_hello_is_a_command_the_firmware_knows():
    # the tool sends PC_LOG_HELLO at its start; the Due must know that name (Tests_mech.cpp dispatcher)
    firmware = (
        Path(__file__).resolve().parents[2] / "src" / "Tests_mech.cpp"
    ).read_text(encoding="utf-8")
    assert f'"{sl.PC_LOG_HELLO}"' in firmware


def test_a_run_refused_after_its_name_frees_the_slot_again(tmp_path, capsys):
    session = sl.Session(str(tmp_path), "t", today=TODAY)
    book = ss.SlotBook.new(str(tmp_path / "day-slots.json"), 14)
    sl.on_device_line(session, "META|sample;slot=4;name=A;condition=c;notes=", book)
    sl.on_device_line(
        session, "=== run_sample ===", book
    )  # test_begin prints this after the name
    assert book.status(4) == ss.USED and session.run_start is not None
    # O-ring state unknown: the device refuses after the name was taken, so no pump run took place
    sl.on_device_line(
        session,
        "REFUSED | run_sample: O-ring lock state is UNKNOWN - run actuator_home",
        book,
    )
    assert book.status(4) == ss.AVAILABLE
    assert session.run_start is None and session.open_slot is None
    assert "available again" in capsys.readouterr().out
    session.close()


def test_a_refusal_before_any_name_frees_nothing(tmp_path):
    session = sl.Session(str(tmp_path), "t", today=TODAY)
    book = ss.SlotBook.new(str(tmp_path / "day-slots.json"), 14)
    sl.on_device_line(
        session, "REFUSED | run_sample: no session yet - send `session` first", book
    )
    assert book.status(1) == ss.AVAILABLE and session.open_slot is None
    session.close()


def test_a_finished_run_clears_the_open_slot_so_a_later_refusal_frees_nothing(tmp_path):
    session = sl.Session(str(tmp_path), "t", today=TODAY)
    book = ss.SlotBook.new(str(tmp_path / "day-slots.json"), 14)
    sl.on_device_line(session, "META|sample;slot=2;name=B;condition=c;notes=", book)
    sl.on_device_line(session, "TEST|run_sample|2|PASS|x=1", book)
    assert session.open_slot is None
    sl.on_device_line(
        session, "REFUSED | run_sample: O-ring lock state is UNKNOWN", book
    )
    assert book.status(2) == ss.USED  # the run had ended: the sample stays used
    session.close()
