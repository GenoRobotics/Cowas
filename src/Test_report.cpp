/**
 * @file Test_report.cpp
 * @brief FW2: the TEST|... result lines and the end-of-test summary table of the
 * console test suite (Tests_mech.cpp, Test_openloop.cpp). The line format itself is the
 * pure function core::format_test_line() (unit-tested natively).
 */
#include <Arduino.h>
#include "Test_report.h"

static const int REPORT_MAX_ROWS = 40; // rows kept for the table; the counts include every row
static const int REPORT_ID_LEN = 12;
static const int REPORT_KV_LEN = 80;

struct ReportRow
{
    char id[REPORT_ID_LEN];
    core::TestResult result;
    char kv[REPORT_KV_LEN];
};

static char report_name[24] = "";
static ReportRow rows[REPORT_MAX_ROWS];
static int row_count = 0;
static int counts[3] = {0, 0, 0}; // PASS, FAIL, SKIP

static void copy_trunc(char *dst, int cap, const char *src)
{
    int i = 0;
    for (; src && src[i] && i < cap - 1; i++)
    {
        dst[i] = src[i];
    }
    dst[i] = 0;
}

static void print_line(const char *id, core::TestResult r, const char *kv)
{
    char line[200];
    core::format_test_line(line, sizeof line, report_name, id, r, kv); // truncated if too long, still one line
    Serial.println(line);
}

void test_begin(const char *name)
{
    copy_trunc(report_name, sizeof report_name, name);
    row_count = 0;
    counts[0] = counts[1] = counts[2] = 0;
    Serial.println(String("=== ") + name + " ===");
}

void test_result(const char *id, core::TestResult r, const String &kv)
{
    print_line(id, r, kv.c_str());
    counts[r]++;
    if (row_count < REPORT_MAX_ROWS)
    {
        copy_trunc(rows[row_count].id, REPORT_ID_LEN, id);
        rows[row_count].result = r;
        copy_trunc(rows[row_count].kv, REPORT_KV_LEN, kv.c_str());
    }
    row_count++;
}

int test_end(const String &extra_kv)
{
    Serial.println(String("--- ") + report_name + " summary ---");
    Serial.println("id          | result | data");
    int shown = row_count < REPORT_MAX_ROWS ? row_count : REPORT_MAX_ROWS;
    for (int i = 0; i < shown; i++)
    {
        String id = rows[i].id;
        while (id.length() < 11)
        {
            id += ' ';
        }
        Serial.println(id + " | " + core::test_result_name(rows[i].result) + "   | " + rows[i].kv);
    }
    if (row_count > shown)
    {
        Serial.println("(" + String(row_count - shown) + " more rows: see the TEST lines above)");
    }

    core::TestResult overall = counts[core::TEST_FAIL] > 0 ? core::TEST_FAIL
                               : counts[core::TEST_PASS] > 0 ? core::TEST_PASS
                                                             : core::TEST_SKIP;
    String kv = "pass=" + String(counts[core::TEST_PASS]) + ";fail=" + String(counts[core::TEST_FAIL]) +
                ";skip=" + String(counts[core::TEST_SKIP]);
    if (extra_kv.length())
    {
        kv += ";" + extra_kv;
    }
    Serial.println(String("TOTAL: ") + counts[core::TEST_PASS] + " PASS, " + counts[core::TEST_FAIL] + " FAIL, " +
                   counts[core::TEST_SKIP] + " SKIP -> " + core::test_result_name(overall));
    print_line("summary", overall, kv.c_str());
    return counts[core::TEST_FAIL];
}

String read_console_line()
{
    while (!Serial.available())
    {
        delay(10);
    }
    String line = Serial.readStringUntil('\n');
    line.trim();
    return line;
}
