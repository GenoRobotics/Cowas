/**
 * @file Tests_mech.cpp
 * @brief FW2 mechanical test suite: locking, sealing and rotation checks, and the
 * pumping tests prepared for (or run without) the pressure sensor. Every test prints one
 * TEST|<name>|<id>|<PASS/FAIL/SKIP>|<k=v;...> line per result and a summary table
 * (Test_report.cpp); tools/hw_test_log.py captures them to test_logs/.
 *
 * Common options: 'x' aborts at every pause; `auto` = no pauses (x still aborts between
 * steps); from=N (a slot number) count=M order=index|fill for the multi-slot tests
 * (FILL_ORDER by default). No slot count is hard-coded (NB_SLOT, FILL_ORDER).
 *
 * The procedures are the real ones (lock_oring, unlock_oring, goto_slot_locked,
 * rotateMotor, load_slot*, step_*); the pure decisions live in src/Core/.
 */
#include <Arduino.h>
#include "Tests_mech.h"
#include "Tests.h"
#include "Test_report.h"
#include "Test_openloop.h"
#include "Oring_lock.h"
#include "Manifold.h"
#include "Step_functions.h"
#include "Valve_2_2.h"
#include "Micro_pump.h"
#include "Pump.h"
#include "Flow_sensor.h"
#include "Settings.h"
#include "Core/test_args.h"
#include "Core/slot_iteration.h"
#include "Core/manifold_geometry.h"

extern Valve_2_2 valve_1;
extern Valve_2_2 valve_manifold;
extern Micro_Pump micro_pump;
extern Pump pump;
extern Flow_sensor flow_sensor_small;
extern Manifold manifold;

static const int FILL_ORDER_LEN = sizeof FILL_ORDER / sizeof FILL_ORDER[0];
static const int MAX_LIST = NB_SLOT + 1;
static const uint32_t DELAY_VALVES_MS = 1000; // like DELAY_ACTIONS in Step_functions.cpp

// ================================================================ small helpers

static bool abort_pending()
{
    char k = real_keypress_waiting();
    return k == 'x' || k == 'X';
}

// A pause between steps: waits for a key unless `auto`. True = the operator aborted.
static bool pause_or_abort(bool auto_mode, const String &what)
{
    if (auto_mode)
    {
        return abort_pending();
    }
    Serial.println(what + " - any key = continue, x = abort");
    return wait_for_keypress_or_abort();
}

// Asks until y, n or x. Returns 'y', 'n' or 'x'.
static char ask_ynx(const String &question)
{
    while (true)
    {
        Serial.println(question + " (y/n, x = abort)");
        char c = wait_for_keypress();
        if (c == 'Y' || c == 'N' || c == 'X')
        {
            c = c - 'A' + 'a';
        }
        if (c == 'y' || c == 'n' || c == 'x')
        {
            return c;
        }
    }
}

// Waits ms, polling for x. True = aborted.
static bool wait_or_abort(uint32_t ms)
{
    uint32_t start = millis();
    while (millis() - start < ms)
    {
        if (abort_pending())
        {
            return true;
        }
        delay(20);
    }
    return false;
}

static String f2(float v)
{
    return (v != v) ? String("nan") : String(v, 2);
}

static bool require_unlocked(const char *name)
{
    if (oring_lock_state() == core::LOCK_STATE_UNLOCKED)
    {
        return true;
    }
    Serial.println(String("REFUSED | ") + name + ": needs the O-ring UNLOCKED (state: " + oring_lock_state_name() + ")" +
                   (oring_lock_state() == core::LOCK_STATE_UNKNOWN ? " - run actuator_home" : " - run oring_unlock"));
    return false;
}

static bool require_known(const char *name)
{
    if (oring_lock_state() != core::LOCK_STATE_UNKNOWN)
    {
        return true;
    }
    Serial.println(String("REFUSED | ") + name + ": O-ring lock state is UNKNOWN - run actuator_home");
    return false;
}

// The slot in front of the O-ring, if the encoder says it is aligned. -1 (and a message) otherwise.
static int aligned_slot(const char *name, float *error_out)
{
    float error;
    bool ok;
    int slot = manifold_nearest_slot(&error, &ok);
    if (error_out)
    {
        *error_out = error;
    }
    core::HomeDecision d = core::may_home(ok, error, ACTUATOR_HOME_SLOT_TOLERANCE_DEG);
    if (d != core::HOME_ALLOWED)
    {
        Serial.println(String("REFUSED | ") + name + ": manifold not aligned to a slot (" + core::home_decision_name(d) +
                       ", nearest " + slot + ", error " + f2(error) + " deg)");
        return -1;
    }
    return slot;
}

// Rotates (O-ring must be UNLOCKED), waits for the coast, returns the encoder error.
static bool rotate_and_measure(int slot, float *error)
{
    *error = NAN;
    if (!rotateMotor(slot))
    {
        return false;
    }
    delay(TEST_ROTATE_SETTLE_MS);
    return manifold_slot_error(slot, error);
}

// ================================================================ argument handling

struct Cmd
{
    char name[24];
    core::Args args;
};

// Checks the tokens against what the command accepts; prints the usage line if not.
static bool check_args(const Cmd &c, const char *usage, const char *const *keys, int nkeys,
                       const char *const *flags, int nflags, int max_positionals)
{
    int bad = core::args_first_unknown(c.args, keys, nkeys, flags, nflags, max_positionals);
    if (bad >= 0)
    {
        Serial.println(String("Unknown or extra argument '") + (c.args.key[bad][0] ? c.args.key[bad] : c.args.value[bad]) + "'");
        Serial.println(String("Usage: ") + usage);
        return false;
    }
    return true;
}

static bool arg_long(const Cmd &c, const char *key, long *out, long lo, long hi)
{
    const char *v = core::args_value(c.args, key);
    if (!v)
    {
        return true; // keep the default
    }
    long n;
    if (!core::parse_long_strict(v, &n) || n < lo || n > hi)
    {
        Serial.println(String("Bad value ") + key + "=" + v + " (allowed " + lo + ".." + hi + ")");
        return false;
    }
    *out = n;
    return true;
}

static bool arg_float(const Cmd &c, const char *key, float *out, float lo, float hi)
{
    const char *v = core::args_value(c.args, key);
    if (!v)
    {
        return true;
    }
    float f;
    if (!core::parse_float_strict(v, &f) || f < lo || f > hi)
    {
        Serial.println(String("Bad value ") + key + "=" + v + " (allowed " + String(lo, 0) + ".." + String(hi, 0) + ")");
        return false;
    }
    *out = f;
    return true;
}

static bool arg_yes_no(const Cmd &c, const char *key, bool *out)
{
    const char *v = core::args_value(c.args, key);
    if (!v)
    {
        return true;
    }
    if (!core::parse_yes_no(v, out))
    {
        Serial.println(String("Bad value ") + key + "=" + v + " (y or n)");
        return false;
    }
    return true;
}

// The i-th positional as a number (if present). False + message if malformed/out of range.
static bool positional_long(const Cmd &c, int i, long *out, long lo, long hi, const char *what)
{
    int seen = 0;
    for (int t = 0; t < c.args.count; t++)
    {
        const char *v = c.args.value[t];
        if (c.args.key[t][0] || strcmp(v, "auto") == 0 || strcmp(v, "dry") == 0)
        {
            continue; // key=value or a flag
        }
        if (seen++ == i)
        {
            long n;
            if (!core::parse_long_strict(v, &n) || n < lo || n > hi)
            {
                Serial.println(String("Bad ") + what + " '" + v + "' (allowed " + lo + ".." + hi + ")");
                return false;
            }
            *out = n;
            return true;
        }
    }
    return true; // absent: keep the default
}

// from/count/order -> the slot list. Returns the count, or -1 after printing why.
static int select_slots(const Cmd &c, bool include_purge, int *out)
{
    core::SlotSelection base = core::default_selection();
    base.include_purge = include_purge;
    core::SlotSelection sel;
    const char *error = "";
    if (!core::selection_from_args(c.args, base, &sel, &error))
    {
        Serial.println(String("Bad slot selection: ") + error);
        return -1;
    }
    int n = core::build_slot_list(FILL_ORDER, FILL_ORDER_LEN, NB_SLOT, PURGE_SLOT, sel, out, MAX_LIST);
    if (n < 0)
    {
        Serial.println(String("Bad slot selection: ") + core::slot_list_error_name(n));
        return -1;
    }
    String list = "Slots (" + String(n) + "): ";
    for (int i = 0; i < n; i++)
    {
        list += String(out[i]) + (i + 1 < n ? ", " : "");
    }
    Serial.println(list);
    return n;
}

// ================================================================ sensorless phases

// Water through the CURRENT (locked) slot: V1 closed, Vman open, open-loop pump.
static OpenLoopResult water_phase_sensorless(long power, float ml, uint32_t max_ms)
{
    valve_1.set_close_way();
    valve_manifold.set_open_way();
    delay(DELAY_VALVES_MS);
    OpenLoopResult r = pump_open_loop_test(power, ml, max_ms, true);
    valve_manifold.set_close_way();
    return r;
}

// Air through the CURRENT (locked) slot: V1 open, Vman closed, time-only open-loop run.
static OpenLoopResult air_phase_sensorless(long power)
{
    valve_1.set_open_way();
    valve_manifold.set_close_way();
    delay(DELAY_VALVES_MS);
    OpenLoopResult r = pump_open_loop_test(power, 0, TEST_OPENLOOP_AIR_PUSH_MS, false);
    valve_1.set_close_way();
    return r;
}

static String phase_kv(const char *prefix, const OpenLoopResult &r)
{
    return String(prefix) + "_reason=" + core::openloop_stop_name(r.reason) + ";" + prefix + "_ml=" + String(r.volume_ml, 0) +
           ";" + prefix + "_s=" + String(r.elapsed_ms / 1000.0, 1);
}

// ================================================================ commands

static void cmd_help()
{
    Serial.println("=== Test menu commands (FW2) ===");
    Serial.println("O-ring lock state: LOCKED / UNLOCKED / UNKNOWN (UNKNOWN at boot). Rotation needs UNLOCKED.");
    Serial.println("Common options: x aborts at every pause; auto = no pauses; from=<slot> count=<n> order=index|fill");
    Serial.println("-- lock / rotation (FW2) --");
    Serial.println("  oring_state                   lock state, encoder angle, nearest slot, pressure-sensor flag");
    Serial.println("  actuator_home                 aligned to a slot (+-" + String(ACTUATOR_HOME_SLOT_TOLERANCE_DEG, 1) + " deg): overdrive lock, unlock -> UNLOCKED");
    Serial.println("  oring_lock | oring_unlock     lock / unlock at the current slot (state must be known)");
    Serial.println("  lock_cycle [n] [auto]         n lock/unlock cycles here (default 5), times each move");
    Serial.println("  lock_endurance [n] [auto]     n cycles (default 200), encoder every 10, temperature question every 50");
    Serial.println("  rotate_all [dry] [hold=s] [auto] [from= count= order=]  purge + FILL_ORDER: rotate, error, lock, hold, unlock");
    Serial.println("  encoder_repeat [slot] [n] [auto]  leave and come back n times (default 10), angle spread");
    Serial.println("  valve_check                   V1 and Vman on/off with y/n confirmation (V23 removed, F51)");
    Serial.println("-- pumping (sensorless while PRESSURE_SENSOR_AVAILABLE = false: type NOSENSOR) --");
    Serial.println("  leak_hold [mL|Ns] [power=%]   lock, water, 'leak?', air push, unlock (default " + String(TEST_LEAK_HOLD_DEFAULT_ML, 0) + " mL)");
    Serial.println("  multi_sample [volume=mL] [power=%] [auto] [from= count= order=]  load_slot per available slot");
    Serial.println("  shield_only [slot] [ms] [auto]  micro pump only (time-based), sensorless OK");
    Serial.println("  full_cycle [slot] purge=y/n shield=y/n empty=y/n volume=mL   real step functions (needs the sensor)");
    Serial.println("  full_all [purge= shield= empty= volume=] [auto] [from= count= order=]");
    Serial.println("-- existing commands --");
    Serial.println("  slotN, slot0, clockMM, clocklist, manifold, cal, zero, manifold_direction, encoder_manifold, enc_rot");
    Serial.println("  linear_actuator, calibrate_actuator (both make the lock state UNKNOWN), calibrate_all");
    Serial.println("  pump, sample_cycle, sample_cycle_full, multi_sample_full, calibrate_flow, cal_DNA (need the pressure sensor)");
    Serial.println("  valves, micro_pump, calibrate_dna_volume, pressure, pressure_sensors, calibrate_pressure2");
    Serial.println("  sample1m, demo, abort (need the pressure sensor), micro_switch, buttons_command, temp");
    Serial.println("  stop = leave the menu (panel + legacy Pi link)");
}

static void cmd_oring_state()
{
    float angle = NAN, error = NAN;
    bool enc = read_manifold_angle(&angle);
    int slot = enc ? manifold_nearest_slot(&error, &enc) : -1;
    Serial.println(String("O-ring lock state: ") + oring_lock_state_name());
    if (enc)
        Serial.println("Encoder: " + f2(angle) + " deg, nearest slot " + String(slot) + " (clock " +
                       String(clock_minutes_for_slot(slot)) + "), error " + f2(error) + " deg");
    else
        Serial.println("Encoder: no valid reading");
    Serial.println(String("PRESSURE_SENSOR_AVAILABLE = ") + (PRESSURE_SENSOR_AVAILABLE ? "true" : "false (pumping: sensorless tests only)"));
}

static void cmd_actuator_home()
{
    test_begin("actuator_home");
    int slot;
    float error;
    bool ok = actuator_home(&slot, &error);
    test_result("home", ok ? core::TEST_PASS : core::TEST_FAIL,
                "slot=" + String(slot) + ";err_deg=" + f2(error) + ";tol_deg=" + String(ACTUATOR_HOME_SLOT_TOLERANCE_DEG, 1) +
                    ";state=" + oring_lock_state_name());
    test_end();
}

static void cmd_lock_move(bool lock)
{
    if (!require_known(lock ? "oring_lock" : "oring_unlock"))
    {
        return;
    }
    if (lock && !is_oring_locked() && aligned_slot("oring_lock", nullptr) < 0)
    {
        return;
    }
    bool ok = lock ? lock_oring() : unlock_oring();
    Serial.println(String(ok ? "done" : "FAILED") + ", state " + oring_lock_state_name());
}

// Shared by lock_cycle and lock_endurance: one timed lock + unlock. False on a failure.
static bool one_lock_cycle(uint32_t *lock_ms, uint32_t *unlock_ms)
{
    uint32_t t0 = millis();
    if (!lock_oring())
    {
        return false;
    }
    uint32_t t1 = millis();
    if (!unlock_oring())
    {
        return false;
    }
    *lock_ms = t1 - t0;
    *unlock_ms = millis() - t1;
    return true;
}

static void cmd_lock_cycle(const Cmd &c)
{
    static const char *const flags[] = {"auto"};
    if (!check_args(c, "lock_cycle [n] [auto]", nullptr, 0, flags, 1, 1))
        return;
    long n = 5;
    if (!positional_long(c, 0, &n, 1, 1000, "cycle count"))
        return;
    bool auto_mode = core::args_has_flag(c.args, "auto");
    int slot;
    if (!require_unlocked("lock_cycle") || (slot = aligned_slot("lock_cycle", nullptr)) < 0)
        return;

    test_begin("lock_cycle");
    uint32_t lock_min = 0xFFFFFFFF, lock_max = 0, unlock_min = 0xFFFFFFFF, unlock_max = 0, lock_sum = 0, unlock_sum = 0;
    long done = 0;
    for (long i = 1; i <= n; i++)
    {
        uint32_t lms = 0, ums = 0;
        bool ok = one_lock_cycle(&lms, &ums);
        test_result(String(i).c_str(), ok ? core::TEST_PASS : core::TEST_FAIL,
                    "slot=" + String(slot) + ";lock_ms=" + lms + ";unlock_ms=" + ums + ";state=" + oring_lock_state_name());
        if (!ok)
            break;
        done++;
        lock_min = min(lock_min, lms), lock_max = max(lock_max, lms), lock_sum += lms;
        unlock_min = min(unlock_min, ums), unlock_max = max(unlock_max, ums), unlock_sum += ums;
        if (i < n && pause_or_abort(auto_mode, "Cycle " + String(i) + "/" + String(n) + " done"))
        {
            Serial.println("Aborted by operator.");
            break;
        }
    }
    String kv = "cycles=" + String(done);
    if (done > 0)
    {
        kv += ";lock_ms_min=" + String(lock_min) + ";lock_ms_max=" + lock_max + ";lock_ms_mean=" + (lock_sum / done) +
              ";unlock_ms_min=" + unlock_min + ";unlock_ms_max=" + unlock_max + ";unlock_ms_mean=" + (unlock_sum / done);
        Serial.println("Lock time (ms) min/mean/max: " + String(lock_min) + " / " + (lock_sum / done) + " / " + lock_max);
        Serial.println("Unlock time (ms) min/mean/max: " + String(unlock_min) + " / " + (unlock_sum / done) + " / " + unlock_max);
    }
    test_end(kv);
}

static void cmd_lock_endurance(const Cmd &c)
{
    static const char *const flags[] = {"auto"};
    if (!check_args(c, "lock_endurance [n] [auto]", nullptr, 0, flags, 1, 1))
        return;
    long n = 200;
    if (!positional_long(c, 0, &n, 1, 100000, "cycle count"))
        return;
    int slot;
    if (!require_unlocked("lock_endurance") || (slot = aligned_slot("lock_endurance", nullptr)) < 0)
        return;
    float start_angle;
    if (!read_manifold_angle(&start_angle))
    {
        Serial.println("REFUSED | lock_endurance: no encoder reading");
        return;
    }

    Serial.println("lock_endurance: " + String(n) + " cycles at slot " + String(slot) + " (~" + String(n * 14 / 60) +
                   " min). x aborts between cycles.");
    test_begin("lock_endurance");
    long done = 0;
    bool stop = false;
    for (long i = 1; i <= n && !stop; i++)
    {
        uint32_t lms, ums;
        if (!one_lock_cycle(&lms, &ums))
        {
            test_result(("c" + String(i)).c_str(), core::TEST_FAIL, "cycle=" + String(i) + ";state=" + oring_lock_state_name());
            break;
        }
        done++;
        if (i % 10 == 0 || i == n)
        {
            float angle;
            bool enc = read_manifold_angle(&angle);
            float drift = enc ? core::angle_diff_deg(angle, start_angle) : NAN;
            bool pass = enc && fabsf(drift) < TEST_ROTATE_MAX_ERROR_DEG;
            test_result(("c" + String(i)).c_str(), pass ? core::TEST_PASS : core::TEST_FAIL,
                        "cycle=" + String(i) + ";drift_deg=" + f2(drift) + ";lock_ms=" + lms + ";unlock_ms=" + ums);
        }
        if (i % 50 == 0)
        {
            char a = ask_ynx("Cycle " + String(i) + ": is the A4988 driver or the NEMA17 too hot to touch?");
            test_result(("t" + String(i)).c_str(), a == 'n' ? core::TEST_PASS : a == 'y' ? core::TEST_FAIL : core::TEST_SKIP,
                        "cycle=" + String(i) + ";too_hot=" + (a == 'y' ? "yes" : a == 'n' ? "no" : "aborted"));
            stop = (a != 'n');
        }
        if (!stop && abort_pending())
        {
            Serial.println("Aborted by operator.");
            stop = true;
        }
    }
    test_end("cycles=" + String(done) + ";slot=" + slot);
}

static void cmd_rotate_all(const Cmd &c)
{
    static const char *const keys[] = {"hold", "from", "count", "order"};
    static const char *const flags[] = {"dry", "auto"};
    if (!check_args(c, "rotate_all [dry] [hold=s] [auto] [from=N count=M order=index|fill]", keys, 4, flags, 2, 0))
        return;
    long hold_s = TEST_LOCK_HOLD_S;
    if (!arg_long(c, "hold", &hold_s, 0, 600))
        return;
    bool dry = core::args_has_flag(c.args, "dry");
    bool auto_mode = core::args_has_flag(c.args, "auto");
    int list[MAX_LIST];
    int n = select_slots(c, true, list);
    if (n < 0 || !require_unlocked("rotate_all"))
        return;

    test_begin(dry ? "rotate_all_dry" : "rotate_all");
    float worst = 0;
    for (int i = 0; i < n; i++)
    {
        int slot = list[i];
        float err;
        bool rotated = rotate_and_measure(slot, &err);
        String kv = "slot=" + String(slot) + ";clock=" + clock_minutes_for_slot(slot) + ";err_deg=" + f2(err);
        bool pass = rotated && fabsf(err) < TEST_ROTATE_MAX_ERROR_DEG;
        if (rotated && fabsf(err) > fabsf(worst))
            worst = err;

        if (rotated && !dry)
        {
            if (!(fabsf(err) <= ACTUATOR_HOME_SLOT_TOLERANCE_DEG))
            {
                kv += ";lock=skipped_not_aligned";
                pass = false;
            }
            else
            {
                uint32_t t0 = millis();
                bool locked = lock_oring();
                uint32_t t1 = millis();
                bool aborted = locked && wait_or_abort(hold_s * 1000UL);
                uint32_t t2 = millis();
                bool unlocked = locked && unlock_oring();
                float err_after = NAN;
                if (unlocked)
                    manifold_slot_error(slot, &err_after);
                kv += ";lock_ms=" + String(t1 - t0) + ";hold_s=" + hold_s + ";unlock_ms=" + (millis() - t2) +
                      ";err_after_unlock_deg=" + f2(err_after);
                pass = pass && locked && unlocked;
                if (aborted)
                {
                    test_result(String(slot).c_str(), core::TEST_SKIP, kv + ";aborted=1");
                    Serial.println("Aborted by operator.");
                    break;
                }
            }
        }
        test_result(String(slot).c_str(), pass ? core::TEST_PASS : core::TEST_FAIL, kv);
        if (!rotated || oring_lock_state() != core::LOCK_STATE_UNLOCKED)
        {
            Serial.println("Stopped: rotation failed or the O-ring is not unlocked.");
            break;
        }
        if (i + 1 < n && pause_or_abort(auto_mode, "Slot " + String(slot) + " done"))
        {
            Serial.println("Aborted by operator.");
            break;
        }
    }
    test_end("worst_err_deg=" + f2(worst) + ";limit_deg=" + String(TEST_ROTATE_MAX_ERROR_DEG, 2));
}

static void cmd_encoder_repeat(const Cmd &c)
{
    static const char *const flags[] = {"auto"};
    if (!check_args(c, "encoder_repeat [slot] [n] [auto]", nullptr, 0, flags, 1, 2))
        return;
    bool auto_mode = core::args_has_flag(c.args, "auto");
    long slot = -1, n = 10;
    if (!positional_long(c, 0, &slot, 0, NB_SLOT - 1, "slot") || !positional_long(c, 1, &n, 1, 100, "repeat count"))
        return;
    if (!require_unlocked("encoder_repeat"))
        return;
    if (slot < 0 && (slot = aligned_slot("encoder_repeat (give a slot)", nullptr)) < 0)
        return;
    if (NB_SLOT < 3)
    {
        Serial.println("REFUSED | encoder_repeat: needs at least 3 slots");
        return;
    }

    const int away = NB_SLOT / 2; // about half a turn, approached from both sides in turn
    test_begin("encoder_repeat");
    float lo = 0, hi = 0;
    int got = 0;
    bool all_ok = true;
    for (long i = 1; i <= n; i++)
    {
        int away_slot = (i % 2) ? (slot + away) % NB_SLOT : (slot - away + NB_SLOT) % NB_SLOT;
        float dummy, err;
        if (!rotate_and_measure(away_slot, &dummy) || !rotate_and_measure(slot, &err))
        {
            test_result(String(i).c_str(), core::TEST_FAIL, "slot=" + String(slot) + ";from=" + away_slot + ";rotation=failed");
            all_ok = false;
            break;
        }
        bool pass = fabsf(err) < TEST_ROTATE_MAX_ERROR_DEG;
        all_ok = all_ok && pass;
        test_result(String(i).c_str(), pass ? core::TEST_PASS : core::TEST_FAIL,
                    "slot=" + String(slot) + ";from=" + away_slot + ";err_deg=" + f2(err));
        lo = got ? min(lo, err) : err;
        hi = got ? max(hi, err) : err;
        got++;
        if (i < n && pause_or_abort(auto_mode, "Repeat " + String(i) + "/" + String(n) + " done"))
        {
            Serial.println("Aborted by operator.");
            break;
        }
    }
    float spread = got ? hi - lo : NAN;
    bool pass = got >= 2 && all_ok && spread < TEST_ENCODER_MAX_SPREAD_DEG;
    test_result("spread", got >= 2 ? (pass ? core::TEST_PASS : core::TEST_FAIL) : core::TEST_SKIP,
                "slot=" + String(slot) + ";repeats=" + got + ";spread_deg=" + f2(spread) + ";min_deg=" + f2(lo) +
                    ";max_deg=" + f2(hi) + ";limit_deg=" + String(TEST_ENCODER_MAX_SPREAD_DEG, 2));
    Serial.println("Arrival error spread: " + f2(spread) + " deg (min " + f2(lo) + ", max " + f2(hi) + ")");
    test_end();
}

static void cmd_valve_check(const Cmd &c)
{
    static const char *const flags[] = {"auto"}; // accepted, ignored: every step needs an answer
    if (!check_args(c, "valve_check", nullptr, 0, flags, 1, 0))
        return;
    Valve_2_2 *valves[] = {&valve_1, &valve_manifold};
    const char *names[] = {"V1", "Vman"};
    const char *what[] = {"V1 (air inlet, D44)", "Vman (deployment line, D46)"};
    test_begin("valve_check");
    Serial.println("V23 is physically removed (F51): not tested.");
    bool aborted = false;
    for (int v = 0; v < 2 && !aborted; v++)
    {
        valves[v]->set_open_way();
        char a_open = ask_ynx(String(what[v]) + " OPEN: did it click / let flow through?");
        valves[v]->set_close_way();
        char a_close = a_open == 'x' ? 'x' : ask_ynx(String(what[v]) + " CLOSED: did it click / stop the flow?");
        aborted = (a_open == 'x' || a_close == 'x');
        core::TestResult r = aborted ? core::TEST_SKIP : (a_open == 'y' && a_close == 'y') ? core::TEST_PASS : core::TEST_FAIL;
        test_result(names[v], r, String("open=") + a_open + ";close=" + a_close);
    }
    valve_1.set_close_way();
    valve_manifold.set_close_way();
    test_end();
}

static void cmd_leak_hold(const Cmd &c)
{
    static const char *const keys[] = {"power"};
    if (!check_args(c, "leak_hold [mL | Ns] [power=%]", keys, 1, nullptr, 0, 1))
        return;
    float ml = TEST_LEAK_HOLD_DEFAULT_ML;
    long seconds = 0;
    const char *amount = core::args_positional(c.args, 0);
    if (amount && !core::parse_ml_or_seconds(amount, &ml, &seconds))
    {
        Serial.println("Usage: leak_hold [mL | Ns], e.g. leak_hold 50 or leak_hold 20s");
        return;
    }
    long power = TEST_OPENLOOP_MAX_POWER;
    if (!arg_long(c, "power", &power, 1, 100))
        return;
    int slot;
    if (!require_unlocked("leak_hold") || (slot = aligned_slot("leak_hold", nullptr)) < 0)
        return;
    if (PRESSURE_SENSOR_AVAILABLE && seconds > 0)
    {
        Serial.println("REFUSED | leak_hold: the Ns form is sensorless only; give a volume in mL");
        return;
    }

    test_begin("leak_hold");
    String kv = "slot=" + String(slot) + ";mode=" + (PRESSURE_SENSOR_AVAILABLE ? "sensor" : "sensorless");
    OpenLoopSession session("leak_hold"); // asks NOSENSOR only when sensorless
    if (!PRESSURE_SENSOR_AVAILABLE && !session.armed())
    {
        test_result(String(slot).c_str(), core::TEST_SKIP, kv + ";reason=not_confirmed");
        test_end();
        return;
    }
    if (!lock_oring())
    {
        test_result(String(slot).c_str(), core::TEST_FAIL, kv + ";lock=failed");
        test_end();
        return;
    }

    bool water_ok = true;
    if (PRESSURE_SENSOR_AVAILABLE)
    {
        load_slot_water(ml);
        kv += ";water_ml=" + String(flow_sensor_small.get_totalFlowMilliL());
    }
    else
    {
        OpenLoopResult w = water_phase_sensorless(power, seconds > 0 ? 0 : ml, seconds > 0 ? seconds * 1000UL : TEST_OPENLOOP_WATER_MAX_MS);
        kv += ";" + phase_kv("water", w) + ";power=" + w.power;
        water_ok = w.ran && (seconds > 0 ? w.reason == core::OPENLOOP_MAX_TIME : w.reason == core::OPENLOOP_VOLUME);
    }

    char leak = ask_ynx("Any leak at the manifold, the O-ring or a joint (during pumping or now)?");
    if (leak == 'x')
    {
        valve_1.set_close_way();
        valve_manifold.set_close_way();
        test_result(String(slot).c_str(), core::TEST_SKIP, kv + ";leak=aborted");
        Serial.println("Aborted: no air push, O-ring left LOCKED (oring_unlock when safe).");
        test_end();
        return;
    }
    kv += String(";leak=") + (leak == 'y' ? "yes" : "no");
    if (leak == 'y')
    {
        Serial.println("Where? (type a short description, then ENTER)");
        String where = read_console_line();
        char buf[40];
        where.toCharArray(buf, sizeof buf);
        core::sanitize_kv_value(buf);
        kv += ";where=" + String(buf);
    }

    if (PRESSURE_SENSOR_AVAILABLE)
    {
        load_slot_air();
    }
    else
    {
        OpenLoopResult a = air_phase_sensorless(power);
        kv += ";" + phase_kv("air", a);
    }
    bool unlocked = unlock_oring();
    kv += String(";unlock=") + (unlocked ? "ok" : "failed");
    test_result(String(slot).c_str(), (water_ok && leak == 'n' && unlocked) ? core::TEST_PASS : core::TEST_FAIL, kv);
    test_end();
}

static void cmd_multi_sample(const Cmd &c)
{
    static const char *const keys[] = {"volume", "power", "from", "count", "order"};
    static const char *const flags[] = {"auto"};
    if (!check_args(c, "multi_sample [volume=mL] [power=%] [auto] [from=N count=M order=index|fill]", keys, 5, flags, 1, 0))
        return;
    float volume = TEST_MULTI_SAMPLE_VOLUME_ML;
    long power = TEST_OPENLOOP_MAX_POWER;
    if (!arg_float(c, "volume", &volume, 1, 5000) || !arg_long(c, "power", &power, 1, 100))
        return;
    bool auto_mode = core::args_has_flag(c.args, "auto");
    int list[MAX_LIST];
    int n = select_slots(c, false, list);
    if (n < 0 || !require_known("multi_sample"))
        return;

    Serial.println("=== Multi-sample loading test (dry run, " + String(volume, 0) + " mL/slot, " +
                   (PRESSURE_SENSOR_AVAILABLE ? "load_slot" : "SENSORLESS") + ") ===");
    Serial.println("Visits every available slot in the list. Send any key to start, or 'x' to abort now.");
    if (!auto_mode && wait_for_keypress_or_abort())
    {
        Serial.println("Aborted before start");
        return;
    }

    test_begin("multi_sample");
    OpenLoopSession session("multi_sample");
    if (!PRESSURE_SENSOR_AVAILABLE && !session.armed())
    {
        test_result("all", core::TEST_SKIP, "reason=not_confirmed");
        test_end();
        return;
    }

    int visited = 0;
    for (int i = 0; i < n; i++)
    {
        int slot = list[i];
        if (manifold.get_state(slot) != available)
        {
            test_result(String(slot).c_str(), core::TEST_SKIP, "slot=" + String(slot) + ";reason=slot_marked_used");
            continue;
        }
        Serial.println("--- Slot " + String(slot) + " (clock " + String(clock_minutes_for_slot(slot)) + " min), " +
                       String(i + 1) + "/" + String(n) + " ---");
        String kv = "slot=" + String(slot);
        bool pass;
        if (PRESSURE_SENSOR_AVAILABLE)
        {
            // the pre-FW2 sequence: unlock, rotate, load_slot (lock, water, air, unlock)
            bool moved = unlock_oring() && rotateMotor(slot);
            bool ok = moved && load_slot(volume);
            kv += String(";rotate=") + (moved ? "ok" : "failed") + ";water_ml=" + flow_sensor_small.get_totalFlowMilliL();
            pass = ok;
        }
        else
        {
            bool moved = goto_slot_locked(slot);
            kv += String(";move_lock=") + (moved ? "ok" : "failed");
            pass = moved;
            if (moved)
            {
                OpenLoopResult w = water_phase_sensorless(power, volume, TEST_OPENLOOP_WATER_MAX_MS);
                OpenLoopResult a = air_phase_sensorless(power);
                bool unlocked = unlock_oring();
                kv += ";" + phase_kv("water", w) + ";" + phase_kv("air", a) + ";unlock=" + (unlocked ? "ok" : "failed");
                pass = w.ran && w.reason == core::OPENLOOP_VOLUME && a.ran && unlocked;
            }
        }
        test_result(String(slot).c_str(), pass ? core::TEST_PASS : core::TEST_FAIL, kv);
        visited++;
        if (oring_lock_state() != core::LOCK_STATE_UNLOCKED)
        {
            Serial.println("Stopped: the O-ring is not unlocked (a move failed).");
            break;
        }
        if (i + 1 < n && pause_or_abort(auto_mode, "Inspect joints for leaks"))
        {
            Serial.println("Aborted by operator.");
            break;
        }
    }
    test_end("visited=" + String(visited));
}

// One full production cycle at `slot`; returns the TEST result.
static core::TestResult run_full_cycle(int slot, bool purge, bool shield, bool empty, float volume, String *kv)
{
    *kv = "slot=" + String(slot) + ";purge=" + (purge ? "y" : "n") + ";shield=" + (shield ? "y" : "n") +
          ";empty=" + (empty ? "y" : "n") + ";volume_ml=" + String(volume, 0);
    if (!require_pressure_sensor("full_cycle"))
    {
        *kv += ";reason=no_pressure_sensor";
        return core::TEST_SKIP;
    }
    bool ok = !purge || step_purge();
    const char *failed = ok ? "" : "purge";
    if (ok)
    {
        ok = step_sampling(slot, true, volume);
        *kv += ";sampled_ml=" + String(flow_sensor_small.get_totalFlowMilliL());
        failed = ok ? "" : "sampling";
    }
    if (ok && shield)
    {
        ok = step_DNA_shield(slot);
        failed = ok ? "" : "shield";
    }
    if (ok && empty)
    {
        ok = step_empty();
        failed = ok ? "" : "empty";
    }
    if (ok)
    {
        ok = unlock_oring(); // FW2 tests end unlocked (the locked rest state comes in FW4)
        failed = ok ? "" : "unlock";
    }
    if (!ok)
    {
        *kv += String(";failed_at=") + failed;
    }
    return ok ? core::TEST_PASS : core::TEST_FAIL;
}

static bool full_cycle_options(const Cmd &c, bool *purge, bool *shield, bool *empty, float *volume)
{
    *purge = *shield = *empty = true;
    *volume = STX_SAMPLE_MILLILITERS;
    return arg_yes_no(c, "purge", purge) && arg_yes_no(c, "shield", shield) && arg_yes_no(c, "empty", empty) &&
           arg_float(c, "volume", volume, 1, 5000);
}

static void cmd_full_cycle(const Cmd &c)
{
    static const char *const keys[] = {"purge", "shield", "empty", "volume"};
    if (!check_args(c, "full_cycle [slot] purge=y/n shield=y/n empty=y/n volume=mL", keys, 4, nullptr, 0, 1))
        return;
    bool purge, shield, empty;
    float volume;
    long slot = -1;
    if (!full_cycle_options(c, &purge, &shield, &empty, &volume) || !positional_long(c, 0, &slot, 0, NB_SLOT - 1, "slot"))
        return;
    if (slot == PURGE_SLOT)
    {
        Serial.println("REFUSED | full_cycle: slot " + String(PURGE_SLOT) + " is the purge slot");
        return;
    }
    test_begin("full_cycle");
    if (PRESSURE_SENSOR_AVAILABLE && (!require_known("full_cycle") ||
                                      (slot < 0 && (slot = aligned_slot("full_cycle (give a slot)", nullptr)) < 0)))
    {
        test_end();
        return;
    }
    String kv;
    core::TestResult r = run_full_cycle(slot, purge, shield, empty, volume, &kv);
    test_result(String(slot).c_str(), r, kv);
    test_end();
}

static void cmd_full_all(const Cmd &c)
{
    static const char *const keys[] = {"purge", "shield", "empty", "volume", "from", "count", "order"};
    static const char *const flags[] = {"auto"};
    if (!check_args(c, "full_all [purge= shield= empty= volume=] [auto] [from=N count=M order=index|fill]", keys, 7, flags, 1, 0))
        return;
    bool purge, shield, empty;
    float volume;
    if (!full_cycle_options(c, &purge, &shield, &empty, &volume))
        return;
    bool auto_mode = core::args_has_flag(c.args, "auto");
    int list[MAX_LIST];
    int n = select_slots(c, false, list);
    if (n < 0)
        return;
    test_begin("full_all");
    if (!PRESSURE_SENSOR_AVAILABLE || !require_known("full_all"))
    {
        test_result("all", core::TEST_SKIP, PRESSURE_SENSOR_AVAILABLE ? "reason=lock_state_unknown" : "reason=no_pressure_sensor");
        require_pressure_sensor("full_all");
        test_end();
        return;
    }
    for (int i = 0; i < n; i++)
    {
        String kv;
        core::TestResult r = run_full_cycle(list[i], purge, shield, empty, volume, &kv);
        test_result(String(list[i]).c_str(), r, kv);
        if (r != core::TEST_PASS)
        {
            Serial.println("Stopped after a failed cycle.");
            break;
        }
        if (i + 1 < n && pause_or_abort(auto_mode, "Slot " + String(list[i]) + " done"))
        {
            Serial.println("Aborted by operator.");
            break;
        }
    }
    test_end();
}

static void cmd_shield_only(const Cmd &c)
{
    static const char *const keys[] = {"ms"};
    static const char *const flags[] = {"auto"};
    if (!check_args(c, "shield_only [slot] [ms] [auto]", keys, 1, flags, 1, 2))
        return;
    long slot = -1;
    long ms = FILL_STERIVEX_TIME / 10; // the production value today (PLACEHOLDER, F4)
    if (!positional_long(c, 0, &slot, 0, NB_SLOT - 1, "slot") || !positional_long(c, 1, &ms, 100, FILL_STERIVEX_TIME, "ms") ||
        !arg_long(c, "ms", &ms, 100, FILL_STERIVEX_TIME))
        return;
    bool auto_mode = core::args_has_flag(c.args, "auto");
    if (!require_known("shield_only"))
        return;

    bool in_place = slot < 0;
    if (in_place && (slot = aligned_slot("shield_only (give a slot)", nullptr)) < 0)
        return;
    Serial.println("shield_only: slot " + String(slot) + ", micro pump " + String(ms) + " ms (time-based, no flow sensor on this line)");
    if (!auto_mode && wait_for_keypress_or_abort())
    {
        Serial.println("Aborted before start");
        return;
    }

    test_begin("shield_only");
    bool placed = in_place ? lock_oring() : goto_slot_locked(slot);
    if (!placed)
    {
        test_result(String(slot).c_str(), core::TEST_FAIL, "slot=" + String(slot) + ";move_lock=failed");
        test_end();
        return;
    }
    micro_pump.start();
    uint32_t start = millis();
    bool aborted = false;
    while (millis() - start < (uint32_t)ms)
    {
        if (real_keypress_waiting())
        {
            aborted = true;
            break;
        }
        delay(5);
    }
    micro_pump.stop();
    uint32_t ran = millis() - start;
    bool unlocked = unlock_oring();
    test_result(String(slot).c_str(), (!aborted && unlocked) ? core::TEST_PASS : aborted ? core::TEST_SKIP : core::TEST_FAIL,
                "slot=" + String(slot) + ";ms_asked=" + ms + ";ms_run=" + ran + ";stopped_by=" + (aborted ? "key" : "time") +
                    ";unlock=" + (unlocked ? "ok" : "failed"));
    test_end();
}

// ================================================================ dispatcher

bool mech_tests_dispatch(const String &line)
{
    Cmd c;
    core::ArgsError e = core::parse_args(line.c_str(), c.name, sizeof c.name, &c.args);
    const char *known[] = {"help", "oring_state", "actuator_home", "oring_lock", "oring_unlock", "lock_cycle",
                           "lock_endurance", "rotate_all", "encoder_repeat", "valve_check", "leak_hold",
                           "multi_sample", "full_cycle", "full_all", "shield_only"};
    bool mine = false;
    for (unsigned i = 0; i < sizeof known / sizeof known[0]; i++)
    {
        mine = mine || strcmp(c.name, known[i]) == 0;
    }
    if (!mine)
    {
        return false;
    }
    if (e != core::ARGS_OK)
    {
        Serial.println(String("Bad arguments: ") + core::args_error_name(e) + " - see `help`");
        return true;
    }

    if (!strcmp(c.name, "help"))
        cmd_help();
    else if (!strcmp(c.name, "oring_state"))
        cmd_oring_state();
    else if (!strcmp(c.name, "actuator_home"))
        cmd_actuator_home();
    else if (!strcmp(c.name, "oring_lock"))
        cmd_lock_move(true);
    else if (!strcmp(c.name, "oring_unlock"))
        cmd_lock_move(false);
    else if (!strcmp(c.name, "lock_cycle"))
        cmd_lock_cycle(c);
    else if (!strcmp(c.name, "lock_endurance"))
        cmd_lock_endurance(c);
    else if (!strcmp(c.name, "rotate_all"))
        cmd_rotate_all(c);
    else if (!strcmp(c.name, "encoder_repeat"))
        cmd_encoder_repeat(c);
    else if (!strcmp(c.name, "valve_check"))
        cmd_valve_check(c);
    else if (!strcmp(c.name, "leak_hold"))
        cmd_leak_hold(c);
    else if (!strcmp(c.name, "multi_sample"))
        cmd_multi_sample(c);
    else if (!strcmp(c.name, "full_cycle"))
        cmd_full_cycle(c);
    else if (!strcmp(c.name, "full_all"))
        cmd_full_all(c);
    else if (!strcmp(c.name, "shield_only"))
        cmd_shield_only(c);
    return true;
}
