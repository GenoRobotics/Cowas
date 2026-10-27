/**
 * @file Tests_mech.cpp
 * @brief FW2 mechanical test suite: locking, sealing and rotation checks, and the
 * pumping tests prepared for (or run without) the pressure sensor. Every test prints one
 * TEST|<name>|<id>|<PASS/FAIL/SKIP>|<k=v;...> line per result and a summary table
 * (Test_report.cpp); tools/hw_test_log.py captures them to docs/logs/test-logs/.
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
#include "Run_keys.h"
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
#include "Core/pressure_ramp.h"
#include "Core/fill_stop.h"
#include "Core/pid_metrics.h"

extern Valve_2_2 valve_1;
extern Valve_2_2 valve_manifold;
extern Micro_Pump micro_pump;
extern Pump pump;
extern Flow_sensor flow_sensor_small;
extern Manifold manifold;
extern float read_p_in(); // main.cpp - the ABP, backs the cap and the PID loops

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

// The test needs the O-ring UNLOCKED: it unlocks it itself when it is locked (target model, see
// oring_go_to). Refuses only when the state is UNKNOWN (run actuator_home).
static bool require_unlocked(const char *name)
{
    if (oring_go_to(core::LOCK_STATE_UNLOCKED))
    {
        return true;
    }
    Serial.println(String("REFUSED | ") + name + ": O-ring not unlocked - see the message above");
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
    Serial.println("  actuator_home                 aligned to a slot (+-" + String(ACTUATOR_HOME_SLOT_TOLERANCE_DEG, 1) + " deg): overdrive lock, unlock -> UNLOCKED. x aborts a move: state UNKNOWN");
    Serial.println("  oring_lock | oring_unlock     lock / unlock at the current slot (state must be known)");
    Serial.println("  lock_cycle [n] [auto]         n lock/unlock cycles here (default 5), times each move");
    Serial.println("  lock_endurance [n] [auto]     n cycles (default 200), encoder every 10, temperature question every 50");
    Serial.println("  rotate_all [dry] [hold=s] [auto] [from= count= order=]  purge + FILL_ORDER: rotate, error, lock, hold, unlock");
    Serial.println("  encoder_repeat [slot] [n] [auto]  leave and come back n times (default 10), angle spread");
    Serial.println("  valve_check                   V1 and Vman on/off with y/n confirmation (V23 removed, F51)");
    Serial.println("-- pressure sensor (S1-1: ABP on SPI, backs the cap and the PID loops) --");
    Serial.println("  pressure                      one reading (calibrated + datasheet conversion)");
    Serial.println("  abp_raw                       20 raw SPI frames: status bits and counts");
    Serial.println("  pressure_zero                 pump off, open to air: prints PRESSURE_ZERO_OFFSET_BAR");
    Serial.println("  calibrate_pressure            syringe + manometer on a tee, no pump: prints PRESSURE_GAIN and the zero offset");
    Serial.println("-- pumping (sensorless while PRESSURE_SENSOR_AVAILABLE = false: type NOSENSOR) --");
    Serial.println("  leak_hold [mL|Ns] [power=%]   lock, water, 'leak?', air push, unlock (default " + String(TEST_LEAK_HOLD_DEFAULT_ML, 0) + " mL)");
    Serial.println("  pressure_ramp [target=bar] [rate=bar/s, 0=step] [hold=s] [decay=s] [maxpower=%] [start=%] [slew=%] [kp= ki= kd=] [filter=s] [label=] [quick]  PID by pressure: seal check and tuning");
    Serial.println("  fill_test [volume=mL] [target=bar] [maxpower=%] [timeout=s] [stall=s] [start=%] [slew=%] [kp= ki= kd=] [filter=s] [label=] [quick]  PID; pump a volume (default 1000 mL) through the filter of the current slot; pauses at the limit");
    Serial.println("  multi_sample [volume=mL] [power=%] [auto] [from= count= order=]  load_slot per available slot");
    Serial.println("  shield_only [slot] [ms] [auto]  micro pump only (time-based), sensorless OK");
    Serial.println("  prime_shield  DNA-shield pump runs until a key says the line is primed (x aborts)");
    Serial.println("  full_cycle [slot] purge=y/n shield=y/n empty=y/n volume=mL   real step functions (needs the sensor)");
    Serial.println("  full_all [purge= shield= empty= volume=] [auto] [from= count= order=]");
    Serial.println("  session                       ask site, date, experiment, operator (once per day; written to the log)");
    Serial.println("  run_sample [slot] [volume=mL] ask sample name, condition, notes; then the whole sample. Needs `session` first");
    Serial.println("  log_on                        the PC session log is running (sent by tools/session_log.py): run_sample saves its record");
    Serial.println("-- existing commands --");
    Serial.println("  slotN, slot0, clockMM, clocklist, manifold, cal, zero, manifold_direction, encoder_manifold, enc_rot");
    Serial.println("  linear_actuator, calibrate_actuator (both make the lock state UNKNOWN), calibrate_all");
    Serial.println("  pump, sample_cycle, sample_cycle_full, multi_sample_full, calibrate_flow, cal_DNA (need the pressure sensor)");
    Serial.println("  ethanol_purge                prime through purge, flush each sample slot to 50 mL, then purge 50 mL");
    Serial.println("  valves, micro_pump, calibrate_dna_volume, shield_calibrate (LA3)");
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
    bool aborted = false;
    bool ok = actuator_home(&slot, &error, &aborted);
    // the lock can run off-slot (D36), but the test passes only when the rotor is aligned
    bool aligned = ok && fabsf(error) <= ACTUATOR_HOME_SLOT_TOLERANCE_DEG;
    test_result("home", aligned ? core::TEST_PASS : core::TEST_FAIL,
                "slot=" + String(slot) + ";err_deg=" + f2(error) + ";tol_deg=" + String(ACTUATOR_HOME_SLOT_TOLERANCE_DEG, 1) +
                    ";state=" + oring_lock_state_name() + String(aborted ? ";reason=aborted" : ""));
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
        // rotateMotor() locks the O-ring again on return (rest state, Manifold.cpp), so unlock before every move
        if (oring_lock_state() != core::LOCK_STATE_UNLOCKED && !unlock_oring())
        {
            Serial.println("Stopped: could not unlock the O-ring before the move.");
            break;
        }
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
        if (!rotated)
        {
            Serial.println("Stopped: rotation failed.");
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
    if ((slot = aligned_slot("leak_hold", nullptr)) < 0) // the O-ring is locked below by target (no refusal)
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

// S1-1b: the pump regulated BY PRESSURE, not by a fixed power. The controller follows a setpoint that rises
// slowly to a target, holds it, then (optionally) the pump stops and the decay is read. Two uses: a seal check
// (the manifold seal leaks droplets around 3 bar, F144: keep the target at 2.5 or lower), and the tuning of the
// controller: the same run on the same slot with other kp= ki= kd= slew= start= filter=, compared by the numbers
// it prints (ovs, rise, rip, rev, pw..., tools/compare_tune.py).
//
// Water only: the valves are set as in load_slot_water (V1 closed, Vman open).
// Safety: the cap is checked on every pass (about 2 ms); there is no soft stop, the hard cap is the only one.
// It refuses to start if the line is not empty (P > PRESSURE_RAMP_MAX_START_BAR), so two runs are comparable.
// The controller is MiniPID (PRESSURE_RAMP_PID_KP/KI/KD from the model of the bench, F146; slew 3 % per 100 ms, start 15 %, raw pressure
// unless filter= is given). One change from the first version: the output goes through Pump::set_power_live
// (12-bit, never blocks) instead of Pump::set_power, whose blocking 4 s ramp on a step of 10 points made the
// loop blind (F139). 1..9 % still become the 10 % floor, 0 is "pump idle".
static void cmd_pressure_ramp(const Cmd &c)
{
    static const char *const keys[] = {"target", "rate", "hold", "decay", "maxpower", "start", "slew", "kp", "ki", "kd", "filter", "label"};
    static const char *const flags[] = {"quick"};
    const char *usage = "pressure_ramp [target=bar] [rate=bar/s, 0=step] [hold=s] [decay=s] [maxpower=%] [start=%] [slew=%/update, 0=off] "
                        "[kp= ki= kd=] [filter=s, 0=raw pressure] [label=text] [quick]";
    if (!check_args(c, usage, keys, 12, flags, 1, 0))
        return;
    if (!require_pressure_sensor("pressure_ramp"))
        return;
    float cap = pump.get_pressure_safety_cap();
    if (pump.get_pressure_safety_fn() == nullptr || !(cap > 0))
    {
        Serial.println("REFUSED | pressure_ramp: no pressure cap is registered on the pump");
        return;
    }
    float target = PRESSURE_RAMP_DEFAULT_TARGET_BAR;
    float rate = PRESSURE_RAMP_DEFAULT_RATE_BAR_S;
    float kp = PRESSURE_RAMP_PID_KP, ki = PRESSURE_RAMP_PID_KI, kd = PRESSURE_RAMP_PID_KD;
    float filter_s = PRESSURE_RAMP_DEFAULT_FILTER_S;
    long hold_s = PRESSURE_RAMP_DEFAULT_HOLD_S;
    long decay_s = PRESSURE_RAMP_DEFAULT_DECAY_S;
    long max_power = PRESSURE_RAMP_MAX_POWER;
    long start_power = PRESSURE_RAMP_START_POWER;
    long slew = PRESSURE_RAMP_MAX_STEP_PERCENT;
    if (!arg_float(c, "target", &target, 0, 10) || !arg_float(c, "rate", &rate, 0, 10) ||
        !arg_float(c, "kp", &kp, 0, 2000) || !arg_float(c, "ki", &ki, 0, 2000) || !arg_float(c, "kd", &kd, 0, 2000) ||
        !arg_float(c, "filter", &filter_s, 0, 10) ||
        !arg_long(c, "hold", &hold_s, 0, 600) || !arg_long(c, "decay", &decay_s, 0, 600) ||
        !arg_long(c, "maxpower", &max_power, 10, 100) || !arg_long(c, "start", &start_power, 0, 100) ||
        !arg_long(c, "slew", &slew, 0, 100))
        return;
    if (start_power > max_power)
    {
        Serial.println("Bad value start=" + String(start_power) + ": it must not be above maxpower=" + String(max_power));
        return;
    }
    const bool step = (rate == 0.0f); // operation-like: the setpoint is the target from the first update
    const bool quick = core::args_has_flag(c.args, "quick");
    char label[16] = "";
    const char *label_arg = core::args_value(c.args, "label");
    if (label_arg)
    {
        strncpy(label, label_arg, sizeof label - 1);
        label[sizeof label - 1] = 0;
        core::sanitize_kv_value(label);
    }
    core::RampPlanError plan = core::ramp_plan_check(target, cap, PRESSURE_RAMP_CAP_MARGIN_BAR, step ? PRESSURE_RAMP_MIN_RATE_BAR_S : rate,
                                                     PRESSURE_RAMP_MIN_RATE_BAR_S, PRESSURE_RAMP_MAX_RATE_BAR_S);
    if (plan != core::RAMP_PLAN_OK)
    {
        Serial.println(String("REFUSED | pressure_ramp: ") + core::ramp_plan_error_name(plan) + " (target " + String(target, 2) +
                       " bar must be at most " + String(cap - PRESSURE_RAMP_CAP_MARGIN_BAR, 2) + " (cap " + String(cap, 2) +
                       " minus " + String(PRESSURE_RAMP_CAP_MARGIN_BAR, 2) + "), rate 0 (step) or " + String(PRESSURE_RAMP_MIN_RATE_BAR_S, 2) +
                       ".." + String(PRESSURE_RAMP_MAX_RATE_BAR_S, 2) + " bar/s)");
        return;
    }
    float p_before = read_p_in();
    if (p_before > PRESSURE_RAMP_MAX_START_BAR || p_before < PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR)
    {
        Serial.println("REFUSED | pressure_ramp: the line is not empty or the sensor is not trusted (P = " + String(p_before, 2) +
                       " bar, allowed up to " + String(PRESSURE_RAMP_MAX_START_BAR, 2) + "). Let the pressure out first");
        return;
    }
    int slot;
    if ((slot = aligned_slot("pressure_ramp", nullptr)) < 0)
        return;

    uint32_t ramp_ms = step ? 0 : core::ramp_duration_ms(0.0f, target, rate);
    uint32_t hold_ms = (uint32_t)hold_s * 1000UL;
    Serial.println("=== pressure_ramp: slot " + String(slot) + (label[0] ? String(", label ") + label : String("")) + " ===");
    Serial.println("PID " + String(kp, 1) + " / " + String(ki, 2) + " / " + String(kd, 1) + ", output 0.." + String(max_power) + " %, start " +
                   String(start_power) + " %, steps " + (slew ? String(slew) + " % per update" : String("not limited")) + ", pressure " +
                   (filter_s > 0 ? String("filtered over ") + String(filter_s, 1) + " s" : String("raw")));
    Serial.println("Setpoint " + (step ? String("STEP to ") : String("0 -> ")) + String(target, 2) + " bar" +
                   (step ? String("") : String(" at ") + String(rate, 3) + " bar/s (" + String(ramp_ms / 1000.0, 0) + " s)") + ", hold " +
                   String(hold_s) + " s" + (decay_s ? ", then pump off and decay " + String(decay_s) + " s" : String("")) + ".");
    Serial.println("Hard cap (safety limit): " + String(cap, 2) + " bar. Someone at the 24 V switch.");
    Serial.println("Any key stops the pump. Send any key to start, or 'x' to abort now.");
    if (wait_for_keypress_or_abort())
    {
        Serial.println("Aborted before start");
        return;
    }

    test_begin("pressure_ramp");
    String kv = "slot=" + String(slot) + ";label=" + label;
    if (!lock_oring())
    {
        test_result(String(slot).c_str(), core::TEST_FAIL, kv + ";lock=failed");
        test_end();
        return;
    }
    valve_1.set_close_way();
    valve_manifold.set_open_way();
    delay(DELAY_VALVES_MS);

    MiniPID pid(kp, ki, kd);
    pid.setOutputLimits(0, max_power);
    float power_cmd = (float)start_power;
    pump.start_at((uint8_t)start_power); // at once, no 4 s soft start: the PID is in control from the first instant

    const char *reason = pump.is_running() ? nullptr : "start_refused";
    float start_bar = read_p_in();
    if (start_bar < 0.0f)
    {
        start_bar = 0.0f;
    }
    float p_filt = start_bar;
    core::PidRunStats st;
    core::pid_stats_begin(&st, target, 0.9f, PRESSURE_RAMP_REVERSAL_PERCENT, (float)max_power);
    float p_max = start_bar;
    uint32_t t_pmax = 0, hold_start = 0, last_update = 0, last_print = 0;
    bool in_hold = false;
    uint32_t t0 = millis();
    Serial.println("  t(s)  phase  setpoint  P(bar)  power(%)");
    while (reason == nullptr)
    {
        uint32_t now = millis();
        if (pump.enforce_pressure_safety()) // reads the sensor on every pass; stops the pump itself
        {
            reason = "cap";
            break;
        }
        float p = read_p_in();
        if (p > p_max)
        {
            p_max = p;
            t_pmax = now - t0;
        }
        if (abort_pending())
        {
            pump.stop();
            reason = "aborted";
            break;
        }
        uint32_t t = now - t0;
        if (!in_hold && t >= ramp_ms)
        {
            in_hold = true;
            hold_start = now;
        }
        if (in_hold && now - hold_start >= hold_ms)
        {
            break; // completed: the pump is stopped below
        }
        float sp = step ? target : core::ramp_setpoint(start_bar, target, rate, t);
        if (now - last_update >= PRESSURE_RAMP_UPDATE_MS)
        {
            float dt_s = (now - last_update) / 1000.0f;
            last_update = now;
            p_filt = core::lowpass_step(p_filt, p, dt_s, filter_s); // filter_s = 0: the raw pressure
            pid.setSetpoint(sp);
            float wanted = (float)pid.getOutput(p_filt);
            power_cmd = slew ? core::slew_limit(power_cmd, wanted, (float)slew) : wanted;
            pump.set_power_live(power_cmd); // fine (12-bit) and never blocking; 0 = pump idle, 1..9 become the 10 % floor
            core::pid_stats_add(&st, t, p, power_cmd, in_hold && (now - hold_start) >= hold_ms / 2);
        }
        if (now - last_print >= 1000)
        {
            last_print = now;
            Serial.println("  " + String(t / 1000.0, 0) + "  " + (in_hold ? "HOLD" : "RAMP") + "  " + String(sp, 2) + "  " +
                           String(p, 3) + "  " + String(power_cmd, 0));
        }
        delay(2);
    }

    float p_stop = read_p_in();
    bool completed = (reason == nullptr);
    if (completed)
    {
        pump.stop();
        reason = "completed";
    }
    float p_end = p_stop;
    if (completed && decay_s > 0)
    {
        Serial.println("Pump off. Pressure decay for " + String(decay_s) + " s (any key stops the wait):");
        uint32_t d0 = millis();
        uint32_t last = 0;
        while (millis() - d0 < (uint32_t)decay_s * 1000UL)
        {
            if (abort_pending())
            {
                break;
            }
            if (millis() - last >= 1000)
            {
                last = millis();
                p_end = read_p_in();
                Serial.println("  decay " + String((millis() - d0) / 1000.0, 0) + " s  P=" + String(p_end, 3) + " bar");
            }
            delay(20);
        }
    }
    valve_1.set_close_way();
    valve_manifold.set_close_way();

    bool has_late = core::pid_stats_has_late(&st);
    float err = core::pid_stats_mean_error(&st);
    // reached = the target was held, AND the run was calm (the g3 run said PASS with a 2.5 bar ripple)
    bool reached = completed && has_late && err >= -0.1f && core::pid_stats_overshoot(&st) <= PRESSURE_RAMP_PASS_MAX_OVERSHOOT_BAR &&
                   core::pid_stats_ripple(&st) <= PRESSURE_RAMP_PASS_MAX_RIPPLE_BAR;
    // The numbers to compare between runs (tools/compare_tune.py): ovs = overshoot over the target (bar),
    // rise = s to 90 % of the target, err = mean P - target and rip = max - min of P in the second half of
    // the hold, pw / pwp2p = mean and swing of the power there, rev = power reversals (chatter),
    // sat = % of the time at max power
    kv += ";end=" + String(reason) + ";tgt=" + String(target, 2) + ";kp=" + String(kp, 1) + ";ki=" + String(ki, 2) + ";kd=" + String(kd, 1) +
          ";slew=" + String(slew) + ";filt=" + String(filter_s, 1) + ";st=" + String(start_power) + ";mp=" + String(max_power) + ";pmax=" + String(p_max, 2) +
          ";tpmax=" + String(t_pmax / 1000.0, 1) + ";ovs=" + String(core::pid_stats_overshoot(&st), 2) + ";rise=" + String(st.rise_s, 1) +
          ";err=" + String(err, 2) + ";rip=" + String(core::pid_stats_ripple(&st), 2) + ";pw=" + String(core::pid_stats_power_mean(&st), 0) +
          ";pwp2p=" + String(core::pid_stats_power_p2p(&st), 0) + ";rev=" + String(st.reversals) + ";sat=" + String(core::pid_stats_saturated_pct(&st), 0) +
          ";reached=" + (reached ? "yes" : "no");
    if (decay_s > 0 && completed)
    {
        kv += ";pstop=" + String(p_stop, 2) + ";decay=" + String(p_stop - p_end, 3) + ";decay_s=" + String(decay_s);
    }
    Serial.println(String("pressure_ramp ended: ") + reason + ", P max " + String(p_max, 3) + " bar at " + String(t_pmax / 1000.0, 1) + " s");

    if (!completed)
    {
        test_result(String(slot).c_str(), core::TEST_FAIL, kv);
        Serial.println("The O-ring is left LOCKED (oring_unlock when the pressure is gone).");
        test_end();
        return;
    }
    char leak = 'n';
    if (!quick)
    {
        leak = ask_ynx("Any leak at the manifold, the O-ring or a joint (during the ramp, the hold or the decay)?");
        if (leak == 'x')
        {
            test_result(String(slot).c_str(), core::TEST_SKIP, kv + ";leak=aborted");
            Serial.println("Aborted: O-ring left LOCKED (oring_unlock when safe).");
            test_end();
            return;
        }
        kv += String(";leak=") + (leak == 'y' ? "yes" : "no");
        if (leak == 'y')
        {
            Serial.println("Where? (type a short description, then ENTER)");
            String where = read_console_line();
            char buf[24];
            where.toCharArray(buf, sizeof buf);
            core::sanitize_kv_value(buf);
            kv += ";where=" + String(buf);
        }
    }
    test_result(String(slot).c_str(), (reached && leak == 'n') ? core::TEST_PASS : core::TEST_FAIL, kv);
    Serial.println("O-ring left LOCKED (oring_unlock when the pressure is gone).");
    test_end();
}

// fill_test (2026-10-15): pump a volume (default 1 L) of water through the filter of the CURRENT slot, regulated by
// pressure with the same PID as pressure_ramp (kp= ki= kd= slew= start= filter= to tune). At the pressure limit (cap - 0.4 bar, on the RAW reading) the pump PAUSES
// (output 0, relay on, the water keeps draining through the filter) and resumes when the filtered pressure has fallen
// by 0.5 bar, at 85 % of the power it had: it does not stop the test. The test ends on the volume, a timeout, a stall
// (clogged: almost no flow for stall= seconds of PUMPING time), a pressure that stays up while paused (the filter holds it),
// an abort or the hard cap. The end= reason is in the TEST line (2026-10-19, src/Core/fill_stop.h).
// The flowmeter reads almost nothing at slow flow (F150): compare the collected water with trusted_ml, the fast-flow part.
static void cmd_fill_test(const Cmd &c)
{
    static const char *const keys[] = {"volume", "target", "maxpower", "timeout", "stall", "start", "slew", "kp", "ki", "kd", "filter", "label"};
    static const char *const flags[] = {"quick"};
    const char *usage = "fill_test [volume=mL] [target=bar] [maxpower=%] [timeout=s] [stall=s, 0=off] [start=%] [slew=%/update, 0=off] "
                        "[kp= ki= kd=] [filter=s, 0=raw pressure] [label=text] [quick]";
    if (!check_args(c, usage, keys, 13, flags, 1, 0))
        return;
    if (!require_pressure_sensor("fill_test"))
        return;
    float cap = pump.get_pressure_safety_cap();
    if (pump.get_pressure_safety_fn() == nullptr || !(cap > 0))
    {
        Serial.println("REFUSED | fill_test: no pressure cap is registered on the pump");
        return;
    }
    float volume = FILL_DEFAULT_ML, target = -1.0f; // -1 = not given: the highest target (FILL_MAX_TARGET_BAR, from the cap)
    float kp = PRESSURE_RAMP_PID_KP, ki = PRESSURE_RAMP_PID_KI, kd = PRESSURE_RAMP_PID_KD;
    float filter_s = PRESSURE_RAMP_DEFAULT_FILTER_S;
    long max_power = PRESSURE_RAMP_MAX_POWER, timeout_s = FILL_DEFAULT_TIMEOUT_S, stall_s = FILL_DEFAULT_STALL_S;
    long start_power = PRESSURE_RAMP_START_POWER, slew = PRESSURE_RAMP_MAX_STEP_PERCENT;
    if (!arg_float(c, "volume", &volume, 10, 5000) || !arg_float(c, "target", &target, 0.3, 10) ||
        !arg_float(c, "kp", &kp, 0, 2000) || !arg_float(c, "ki", &ki, 0, 2000) || !arg_float(c, "kd", &kd, 0, 2000) ||
        !arg_float(c, "filter", &filter_s, 0, 10) || !arg_long(c, "maxpower", &max_power, 10, 100) ||
        !arg_long(c, "start", &start_power, 0, 100) || !arg_long(c, "slew", &slew, 0, 100) ||
        !arg_long(c, "timeout", &timeout_s, 10, 7200) || !arg_long(c, "stall", &stall_s, 0, 600))
        return;
    if (start_power > max_power)
    {
        Serial.println("Bad value start=" + String(start_power) + ": it must not be above maxpower=" + String(max_power));
        return;
    }
    const float pause_bar = cap - FILL_PAUSE_BELOW_CAP_BAR;
    const float resume_bar = pause_bar - FILL_RESUME_BELOW_PAUSE_BAR;
    if (target < 0.0f)
    {
        target = resume_bar; // the default is the highest allowed target
    }
    if (target > resume_bar)
    {
        Serial.println("REFUSED | fill_test: target " + String(target, 2) + " bar must be at most " + String(resume_bar, 2) +
                       " (the pump pauses at " + String(pause_bar, 2) + " bar and resumes at " + String(resume_bar, 2) + ")");
        return;
    }
    const bool quick = core::args_has_flag(c.args, "quick");
    char label[16] = "";
    const char *label_arg = core::args_value(c.args, "label");
    if (label_arg)
    {
        strncpy(label, label_arg, sizeof label - 1);
        label[sizeof label - 1] = 0;
        core::sanitize_kv_value(label);
    }
    float p_before = read_p_in();
    if (p_before > PRESSURE_RAMP_MAX_START_BAR || p_before < PRESSURE_SENSOR_MIN_PLAUSIBLE_BAR)
    {
        Serial.println("REFUSED | fill_test: the line is not empty or the sensor is not trusted (P = " + String(p_before, 2) +
                       " bar, allowed up to " + String(PRESSURE_RAMP_MAX_START_BAR, 2) + "). Let the pressure out first");
        return;
    }
    int slot;
    if ((slot = aligned_slot("fill_test", nullptr)) < 0)
        return;

    Serial.println("=== fill_test: slot " + String(slot) + (label[0] ? String(", label ") + label : String("")) + " ===");
    Serial.println("Pump " + String(volume, 0) + " mL (flowmeter, calibrated; the fast-flow part is trusted_ml) at " + String(target, 2) + " bar. PID " + String(kp, 1) + " / " +
                   String(ki, 2) + " / " + String(kd, 1) + ", output 0.." + String(max_power) + " %, start " + String(start_power) + " %, steps " +
                   (slew ? String(slew) + " % per update" : String("not limited")) + ", pressure " +
                   (filter_s > 0 ? String("filtered over ") + String(filter_s, 1) + " s." : String("raw.")));
    Serial.println("Pauses at " + String(pause_bar, 2) + " bar (raw), resumes under " + String(resume_bar, 2) + " bar. Timeout " + String(timeout_s) +
                   " s, stall " + (stall_s ? String(stall_s) + " s" : String("off")) + ". Hard cap " + String(cap, 2) + " bar. Someone at the 24 V switch.");
    Serial.println("Any key stops the pump. Send any key to start, or 'x' to abort now.");
    if (wait_for_keypress_or_abort())
    {
        Serial.println("Aborted before start");
        return;
    }

    test_begin("fill_test");
    String kv = "slot=" + String(slot) + ";label=" + label;
    if (!lock_oring())
    {
        test_result(String(slot).c_str(), core::TEST_FAIL, kv + ";lock=failed");
        test_end();
        return;
    }
    valve_1.set_close_way();
    valve_manifold.set_open_way();
    delay(DELAY_VALVES_MS);

    flow_sensor_small.reset_values();
    flow_sensor_small.activate();

    MiniPID pid(kp, ki, kd);
    pid.setOutputLimits(0, max_power);
    pump.start_at((uint8_t)start_power); // at once, no 4 s soft start: the PID is in control from the first instant

    const char *reason = pump.is_running() ? nullptr : "start_refused";
    float p_filt = p_before > 0.0f ? p_before : 0.0f;
    float p_max = p_filt, power_cmd = (float)start_power, resume_power = (float)start_power, ml = 0.0f;
    bool paused = false;
    int n_pauses = 0;
    uint32_t pause_start = 0, t_paused = 0, last_update = 0, last_print = 0;
    uint32_t t0 = millis();
    FilterWatch watch; // the stop criteria, shared with the operation sampling (src/Hardware/Filter_watch.cpp)
    watch.begin((float)stall_s, FILL_CLOG_FRACTION, (float)FILL_PRESSURE_STUCK_S, true);
    watch.arm(t0, 0.0f); // the stall window starts with the run (2026-10-19: pauses no longer restart it, F149)
    core::TrustedVolume trusted; // the confident flowmeter volume of this run (F150, 2026-10-23)
    trusted.reset(0.0f, t0);
    Serial.println("  t(s)  state  ptarget  P(bar)  Pfilt  power(%)  mL_total  L/min");
    while (reason == nullptr)
    {
        uint32_t now = millis();
        uint32_t t = now - t0;
        if (pump.enforce_pressure_safety()) // reads the sensor on every pass; stops the pump itself
        {
            reason = "cap";
            break;
        }
        float p = read_p_in();
        if (p > p_max)
        {
            p_max = p;
        }
        if (abort_pending())
        {
            pump.stop();
            reason = "aborted";
            break;
        }
        if (!paused && p >= pause_bar)
        {
            paused = true;
            n_pauses++;
            pause_start = now;
            resume_power = power_cmd * FILL_BACKOFF_FACTOR;
            if (resume_power < (float)PUMP_MINIMUM_POWER_PERCENT)
            {
                resume_power = (float)PUMP_MINIMUM_POWER_PERCENT;
            }
            pump.set_power_live(0.0f);
            Serial.println("  PAUSE at " + String(p, 2) + " bar (" + String(n_pauses) + "): waiting for " + String(resume_bar, 2) + " bar");
        }
        if (now - last_update >= PRESSURE_RAMP_UPDATE_MS)
        {
            float dt_s = (now - last_update) / 1000.0f;
            last_update = now;
            flow_sensor_small.update();
            ml = (float)flow_sensor_small.get_totalFlowMilliL();
            trusted.update(ml, now, FLOW_TRUST_MIN_ML_PER_MIN, FLOW_TRUST_WINDOW_MS); // the confident volume (F150, 2026-10-23)
            p_filt = core::lowpass_step(p_filt, p, dt_s, filter_s); // filter_s = 0: the raw pressure
            if (paused)
            {
                if (p_filt <= resume_bar)
                {
                    paused = false;
                    t_paused += now - pause_start;
                    pid.reset(); // the integral of before the pause is no longer true
                    power_cmd = resume_power;
                    pump.set_power_live(power_cmd);
                }
            }
            else
            {
                float sp = core::ramp_setpoint(0.0f, target, PRESSURE_RAMP_DEFAULT_RATE_BAR_S, t);
                pid.setSetpoint(sp);
                float wanted = (float)pid.getOutput(p_filt);
                power_cmd = slew ? core::slew_limit(power_cmd, wanted, (float)slew) : wanted;
                pump.set_power_live(power_cmd);
            }
        }
        if (ml >= volume)
        {
            reason = "volume";
            break;
        }
        if (t >= (uint32_t)timeout_s * 1000UL)
        {
            reason = "timeout";
            break;
        }
        // stall (pumping time only) and pressure (paused, not coming back down): see src/Core/fill_stop.h
        const core::FillStop watch_reason = watch.step(now, paused, ml, p, resume_bar);
        if (watch_reason != core::FILL_GOING_ON)
        {
            reason = core::fill_stop_name(watch_reason);
            break;
        }
        if (now - last_print >= 2000)
        {
            last_print = now;
            float sp_show = core::ramp_setpoint(0.0f, target, PRESSURE_RAMP_DEFAULT_RATE_BAR_S, t);
            Serial.println("  " + String(t / 1000.0, 0) + "  " + (paused ? "PAUSE" : "FILL ") + "  " + String(sp_show, 2) + "  " + String(p, 3) + "  " +
                           String(p_filt, 2) + "  " + String(paused ? 0.0f : power_cmd, 1) + "  " + String(ml, 0) + "  " +
                           String(flow_sensor_small.get_flowRate(), 3));
        }
        delay(2);
    }
    if (paused)
    {
        t_paused += millis() - pause_start;
    }
    trusted.close(ml, millis(), FLOW_TRUST_MIN_ML_PER_MIN); // the last, partial window
    pump.stop();
    uint32_t t_total = millis() - t0;
    delay(1000);
    float p_end = read_p_in();
    valve_1.set_close_way();
    valve_manifold.set_close_way();

    bool done = (strcmp(reason, "volume") == 0);
    kv += ";end=" + String(reason) + ";vol_ml=" + String(ml, 0) + ";trusted_ml=" + String(trusted.trusted_ml(), 0) + ";want_ml=" + String(volume, 0) + ";t_s=" + String(t_total / 1000.0, 0) +
          ";tgt=" + String(target, 2) + ";kp=" + String(kp, 1) + ";ki=" + String(ki, 2) + ";kd=" + String(kd, 1) + ";slew=" + String(slew) +
          ";filt=" + String(filter_s, 1) + ";pmax=" + String(p_max, 2) +
          ";pauses=" + String(n_pauses) + ";paused_s=" + String(t_paused / 1000.0, 0) + ";clog_warn=" + String(watch.clog_warnings()) +";pw_end=" + String(power_cmd, 1) +
          ";ml_min=" + String(t_total ? ml * 60000.0f / (float)t_total : 0.0f, 0) + ";p_end=" + String(p_end, 2);
    Serial.println(String("fill_test ended: ") + reason + ", " + String(ml, 0) + " mL (flowmeter) in " + String(t_total / 1000.0, 0) + " s, P max " +
                   String(p_max, 2) + " bar, " + String(n_pauses) + " pause(s)");
    if (!quick)
    {
        Serial.println("Compare the water that came out with " + String(ml, 0) + " mL.");
    }
    test_result(String(slot).c_str(), done ? core::TEST_PASS : core::TEST_FAIL, kv);
    Serial.println("O-ring left LOCKED (oring_unlock when the pressure is gone).");
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

// A duration in seconds with one decimal, for the STEP TIMES line and the TEST line.
static String secs(uint32_t ms)
{
    return String(ms / 1000.0, 1);
}

// One full operation cycle at `slot`; returns the TEST result.
static core::TestResult run_full_cycle(int slot, bool purge, bool shield, bool empty, float volume, String *kv)
{
    const String given = *kv; // F198 (2026-10-23): the caller's fields (run_sample: name, condition, site ...) are kept on the TEST line
    *kv = "slot=" + String(slot) + ";purge=" + (purge ? "y" : "n") + ";shield=" + (shield ? "y" : "n") +
          ";empty=" + (empty ? "y" : "n") + ";volume_ml=" + String(volume, 0) +
          ";placeholders=purge_ml,shield_time,flow_factor"; // S1-C: set by hand, not measured (roadmap S1-C item 3)
    // S1-C item 3: the purge volume and whether the spool is on the machine (answered in `session`)
    *kv += ";purge_ml=" + String(core::purge_volume_ml(spool_installed_session, PURGE_NO_SPOOL_ML, PURGE_SPOOL_ML), 0) +
           ";spool=" + (spool_installed_session ? "installed" : "absent");
    if (given.length() > 0)
    {
        *kv += ";" + given;
    }
    if (!require_pressure_sensor("full_cycle"))
    {
        *kv += ";reason=no_pressure_sensor";
        return core::TEST_SKIP;
    }
    // step times (user, 2026-10-23): each step is timed and printed, and goes on the TEST line as t_*_s
    const uint32_t t_begin = millis();
    bool ok = !purge || step_purge();
    const uint32_t t_purge = millis() - t_begin;
    uint32_t t_sample = 0, t_shield = 0, t_unlock = 0;
    const char *failed = ok ? "" : "purge";
    bool skipped = false; // s typed during a step: that step ended early and is not counted as a PASS (S1-C item 1)
    if (ok && purge && strcmp(last_purge_end, "skipped") == 0)
    {
        skipped = true;
    }
    if (ok)
    {
        const uint32_t t_mark = millis();
        ok = step_sampling(slot, true, volume);
        t_sample = millis() - t_mark;
        // a clog is one reason with two criteria (2026-10-19): sample_end=clogged, and rule= says which one stopped it
        const bool clog = strcmp(last_sample_end, "stalled") == 0 || strcmp(last_sample_end, "clogged") == 0;
        *kv += ";sampled_ml=" + String(flow_sensor_small.get_totalFlowMilliL()) + ";trusted_ml=" + String(last_sample_trusted_ml, 0) +
               ";sample_end=" + (clog ? "clogged" : last_sample_end);
        if (clog)
        {
            *kv += ";rule=" + String(last_sample_end);
        }
        if (strcmp(last_sample_end, "skipped") == 0)
        {
            skipped = true;
        }
        failed = ok ? "" : "sampling";
    }
    if (ok && shield)
    {
        const uint32_t t_mark = millis();
        ok = step_DNA_shield(slot);
        t_shield = millis() - t_mark;
        failed = ok ? "" : "shield";
    }
    if (empty)
    {
        // the deployment-line emptying is no longer part of the cycle (user, 2026-10-19): the flag is kept on the TEST line, nothing runs
        *kv += ";empty_step=removed";
    }
    if (ok)
    {
        const uint32_t t_mark = millis();
        ok = unlock_oring(); // FW2 tests end unlocked (the locked rest state comes in FW4)
        t_unlock = millis() - t_mark;
        failed = ok ? "" : "unlock";
    }
    if (run_abort_latched())
    {
        *kv += ";aborted=x";
    }
    *kv += ";t_purge_s=" + secs(t_purge) + ";t_sample_s=" + secs(t_sample) + ";t_shield_s=" + secs(t_shield) +
           ";t_unlock_s=" + secs(t_unlock) + ";t_total_s=" + secs(millis() - t_begin);
    Serial.println("STEP TIMES | purge " + secs(t_purge) + " s | sample (with its emptying) " + secs(t_sample) + " s | shield " +
                   secs(t_shield) + " s | unlock " + secs(t_unlock) + " s | total " + secs(millis() - t_begin) + " s");
    if (!ok)
    {
        *kv += String(";failed_at=") + failed;
        return core::TEST_FAIL;
    }
    if (skipped)
    {
        *kv += ";skipped=y";
        return core::TEST_SKIP;
    }
    return core::TEST_PASS;
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

// prime_shield (2026-10-19, user): the DNA-shield peristaltic pump is preloaded by hand. It runs until a key says the line is
// primed (liquid dripping steadily, no bubbles). The same key loop as shield_calibrate's priming (Tests.cpp). x aborts.
static void cmd_prime_shield(const Cmd &c)
{
    (void)c;
    Serial.println("prime_shield: the DNA-shield pump runs. Press any key when the line drips steadily with no bubbles (x aborts; stops by itself after " +
                   String(SHIELD_CAL_PRIME_MAX_MS / 1000) + " s).");
    micro_pump.start(); // refuses (message) if the O-ring is not locked
    const uint32_t t0 = millis();
    char key = 0;
    while (millis() - t0 < SHIELD_CAL_PRIME_MAX_MS)
    {
        key = real_keypress_waiting();
        if (key)
        {
            break;
        }
        delay(5);
    }
    micro_pump.stop();
    const String secs = String((millis() - t0) / 1000);
    if (key == 0)
    {
        Serial.println("prime_shield: stopped by time after " + secs + " s - NOT confirmed primed");
    }
    else if (key == 'x' || key == 'X')
    {
        Serial.println("prime_shield: aborted with x after " + secs + " s");
    }
    else
    {
        Serial.println("prime_shield: primed - pump stopped after " + secs + " s");
    }
}

static void cmd_shield_only(const Cmd &c)
{
    static const char *const keys[] = {"ms"};
    static const char *const flags[] = {"auto"};
    if (!check_args(c, "shield_only [slot] [ms] [auto]", keys, 1, flags, 1, 2))
        return;
    long slot = -1;
    long ms = FILL_STERIVEX_TIME / 10; // the operation value today (PLACEHOLDER, F4)
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

// ================================================================ field metadata (2026-10-23, user)
// `session` asks the site, date, experiment and operator once for the day. `run_sample <slot>` asks the sample name,
// the condition and the notes before each sample, then runs the whole sample (purge, sample, shield, unlock) with one
// command. Both print META lines, which the session log keeps in the .log file; the sample name becomes the id of the
// sample record, so its JSON file carries the name. The session is kept in RAM: a reset needs `session` again.
// Named run_sample, not sample: a line starting with "sample" goes to the legacy Pi protocol (main.cpp) after `stop`.

// Maximum wait for a typed answer (PLACEHOLDER, 2026-10-23): no answer within 10 min cancels the question.
static const uint32_t FIELD_ANSWER_WAIT_MS = 600000UL;

struct FieldSession
{
    bool set = false;
    String site;
    String date;
    String experiment;
    String operator_name;
};
static FieldSession field_session;

// The PC session log (tools/session_log.py) says it is running with `log_on`, sent when it starts. Without it a run_sample
// still runs, but its record is not saved, so run_sample warns first. Reset at every boot.
static bool pc_log_attached = false;

// A typed answer for the META line: no ';', '|' or '=' (they separate the fields), at most 48 characters.
static String meta_clean(String s)
{
    s.trim();
    s.replace(';', '_');
    s.replace('|', '_');
    s.replace('=', '_');
    return s.substring(0, 48);
}

// One typed line. 0 = answer (may be empty), 1 = x typed, 2 = no answer within FIELD_ANSWER_WAIT_MS.
static int read_answer(String &answer)
{
    const uint32_t start = millis();
    while (millis() - start < FIELD_ANSWER_WAIT_MS)
    {
        if (Serial.available() <= 0)
        {
            delay(20);
            continue;
        }
        answer = Serial.readStringUntil('\n');
        answer.trim();
        return answer.equalsIgnoreCase("x") ? 1 : 0;
    }
    return 2;
}

// Asks one question and waits for the answer. A required answer is asked again while it is empty. False = cancelled.
static bool ask(const char *label, String &answer, bool required)
{
    for (;;)
    {
        Serial.println(String("ASK | ") + label + "  (type the answer + ENTER, x = cancel)");
        const int got = read_answer(answer);
        if (got != 0)
        {
            Serial.println(got == 1 ? "CANCELLED | x typed" : "CANCELLED | no answer in time");
            return false;
        }
        if (!required || answer.length() > 0)
        {
            return true;
        }
        Serial.println(String("REQUIRED | ") + label + " cannot be empty");
    }
}

// YYYY-MM-DD with digits and dashes only (the typed date went into every record of a run: F199 check, 2026-10-23)
static bool valid_date(const String &d)
{
    if (d.length() != 10)
        return false;
    for (int i = 0; i < 10; i++)
    {
        const char ch = d.charAt(i);
        if (i == 4 || i == 7 ? ch != '-' : (ch < '0' || ch > '9'))
            return false;
    }
    return true;
}

static void cmd_session(const Cmd &c)
{
    (void)c;
    FieldSession s;
    String a;
    if (!ask("site", a, true))
        return;
    s.site = meta_clean(a);
    for (;;)
    {
        if (!ask("date (YYYY-MM-DD)", a, true))
            return;
        a.trim();
        if (valid_date(a))
            break;
        Serial.println("REQUIRED | the date must look like 2026-10-23 (digits and dashes)");
    }
    s.date = meta_clean(a);
    if (!ask("experiment name", a, true))
        return;
    s.experiment = meta_clean(a);
    if (!ask("operator (optional)", a, false))
        return;
    s.operator_name = meta_clean(a);
    for (;;)
    {
        if (!ask("spool module on the machine? (y/n)", a, true))
            return;
        a.trim();
        a.toLowerCase();
        if (a == "y" || a == "n")
            break;
        Serial.println("REQUIRED | answer y or n");
    }
    spool_installed_session = (a == "y"); // sets the purge volume (purge_volume_ml) for this session
    s.set = true;
    field_session = s;
    Serial.println("META|session;site=" + s.site + ";date=" + s.date + ";experiment=" + s.experiment + ";operator=" + s.operator_name +
                   ";spool=" + (spool_installed_session ? "installed" : "absent"));
}

static void cmd_log_on()
{
    pc_log_attached = true;
    Serial.println("META|log;attached=yes");
}

static void cmd_run_sample(const Cmd &c)
{
    static const char *const keys[] = {"volume"};
    if (!check_args(c, "run_sample [slot] [volume=mL]   (send `session` first; no slot = the slot the manifold is on)", keys, 1, nullptr, 0, 1))
        return;
    long slot = -1;
    float volume = STX_SAMPLE_MILLILITERS;
    if (!positional_long(c, 0, &slot, 1, NB_SLOT - 1, "slot") || !arg_float(c, "volume", &volume, 1, 5000))
        return;
    if (!field_session.set)
    {
        Serial.println("REFUSED | run_sample: no session yet - send `session` first (site, date, experiment)");
        return;
    }
    if (!pc_log_attached)
        Serial.println("WARNING | run_sample: no PC session log is running (tools/session_log.py). The sample WILL run, but its "
                       "record is NOT saved: write down the slot, the name and the condition");
    if (slot < 0)
    {
        // F197 (user, 2026-10-23): no slot typed = the sample at the slot the manifold is on now (encoder), if it is on one
        float error = 0;
        bool encoder_ok = false;
        const int nearest = manifold_nearest_slot(&error, &encoder_ok);
        if (!encoder_ok || nearest < 1 || fabsf(error) > ACTUATOR_HOME_SLOT_TOLERANCE_DEG)
        {
            Serial.println(String("REFUSED | run_sample: no slot typed and the manifold is not on a sample slot (nearest ") + nearest +
                           ", error " + String(error, 2) + " deg). Type run_sample <slot>");
            return;
        }
        slot = nearest;
        Serial.println("run_sample: no slot typed, using the slot the manifold is on: " + String(slot));
    }
    String name, condition, notes;
    if (!ask("sample name", name, true) || !ask("condition (e.g. control, blank, filter A)", condition, true) ||
        !ask("notes (optional)", notes, false))
    {
        Serial.println("Sample not started.");
        return;
    }
    const String clean_name = meta_clean(name);
    Serial.println("META|sample;slot=" + String(slot) + ";name=" + clean_name + ";condition=" + meta_clean(condition) +
                   ";notes=" + meta_clean(notes) + ";site=" + field_session.site +
                   ";date=" + field_session.date + ";experiment=" + field_session.experiment);
    test_begin("run_sample");
    if (PRESSURE_SENSOR_AVAILABLE && !require_known("run_sample"))
    {
        test_end();
        return;
    }
    set_sample_record_name(clean_name.c_str());
    const String clean_condition = meta_clean(condition);
    set_sample_record_meta(field_session.site.c_str(), field_session.date.c_str(), clean_condition.c_str()); // F199: in the records
    String kv = "name=" + clean_name + ";condition=" + meta_clean(condition) +
                ";site=" + field_session.site + ";experiment=" + field_session.experiment;
    core::TestResult r = run_full_cycle((int)slot, true, true, true, volume, &kv);
    set_sample_record_name("");
    set_sample_record_meta("", "", "");
    test_result(String(slot).c_str(), r, kv);
    test_end();
}

// ================================================================ dispatcher

bool mech_tests_dispatch(const String &line)
{
    Cmd c;
    core::ArgsError e = core::parse_args(line.c_str(), c.name, sizeof c.name, &c.args);
    const char *known[] = {"help", "oring_state", "actuator_home", "oring_lock", "oring_unlock", "lock_cycle",
                           "lock_endurance", "rotate_all", "encoder_repeat", "valve_check", "leak_hold",
                           "multi_sample", "full_cycle", "full_all", "shield_only", "prime_shield", "pressure_ramp", "fill_test",
                           "session", "run_sample", "log_on"};
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
    else if (!strcmp(c.name, "pressure_ramp"))
        cmd_pressure_ramp(c);
    else if (!strcmp(c.name, "fill_test"))
        cmd_fill_test(c);
    else if (!strcmp(c.name, "multi_sample"))
        cmd_multi_sample(c);
    else if (!strcmp(c.name, "full_cycle"))
        cmd_full_cycle(c);
    else if (!strcmp(c.name, "full_all"))
        cmd_full_all(c);
    else if (!strcmp(c.name, "shield_only"))
        cmd_shield_only(c);
    else if (!strcmp(c.name, "prime_shield"))
        cmd_prime_shield(c);
    else if (!strcmp(c.name, "session"))
        cmd_session(c);
    else if (!strcmp(c.name, "run_sample"))
        cmd_run_sample(c);
    else if (!strcmp(c.name, "log_on"))
        cmd_log_on();
    return true;
}
