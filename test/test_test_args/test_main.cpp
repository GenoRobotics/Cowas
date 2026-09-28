// Native unit tests for src/Core/test_args and src/Core/test_line (run: pio test -e native).
// The FW2 console test commands: argument parsing, and the TEST|... output line.
#include <unity.h>

#include <string.h>

#include "test_args.h"
#include "test_line.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

static Args parse_ok(const char *line, char *cmd)
{
    Args a;
    TEST_ASSERT_EQUAL_INT(ARGS_OK, parse_args(line, cmd, 24, &a));
    return a;
}

// ---------------------------------------------------------------- parse_args

void test_command_only(void)
{
    char cmd[24];
    Args a = parse_ok("lock_cycle", cmd);
    TEST_ASSERT_EQUAL_STRING("lock_cycle", cmd);
    TEST_ASSERT_EQUAL_INT(0, a.count);
}

void test_empty_line_and_whitespace(void)
{
    char cmd[24];
    Args a = parse_ok("", cmd);
    TEST_ASSERT_EQUAL_STRING("", cmd);
    TEST_ASSERT_EQUAL_INT(0, a.count);
    a = parse_ok("   \t \r\n", cmd);
    TEST_ASSERT_EQUAL_STRING("", cmd);
    TEST_ASSERT_EQUAL_INT(0, a.count);
}

void test_full_line_with_crlf(void)
{
    char cmd[24];
    Args a = parse_ok("  rotate_all  dry from=3\tcount=4 order=index 7 auto\r\n", cmd);
    TEST_ASSERT_EQUAL_STRING("rotate_all", cmd);
    TEST_ASSERT_EQUAL_INT(6, a.count);
    TEST_ASSERT_TRUE(args_has_flag(a, "dry"));
    TEST_ASSERT_TRUE(args_has_flag(a, "auto"));
    TEST_ASSERT_FALSE(args_has_flag(a, "from"));
    TEST_ASSERT_EQUAL_STRING("3", args_value(a, "from"));
    TEST_ASSERT_EQUAL_STRING("4", args_value(a, "count"));
    TEST_ASSERT_EQUAL_STRING("index", args_value(a, "order"));
    TEST_ASSERT_NULL(args_value(a, "volume"));
    TEST_ASSERT_EQUAL_STRING("dry", args_positional(a, 0));
    TEST_ASSERT_EQUAL_STRING("7", args_positional(a, 1));
    TEST_ASSERT_EQUAL_STRING("auto", args_positional(a, 2));
    TEST_ASSERT_NULL(args_positional(a, 3));
    TEST_ASSERT_EQUAL_INT(3, args_positional_count(a));
}

void test_parse_errors(void)
{
    char cmd[24];
    Args a;
    TEST_ASSERT_EQUAL_INT(ARGS_EMPTY_KEY, parse_args("x =5", cmd, 24, &a));
    TEST_ASSERT_EQUAL_INT(ARGS_EMPTY_VALUE, parse_args("x count=", cmd, 24, &a));
    TEST_ASSERT_EQUAL_INT(ARGS_DUPLICATE_KEY, parse_args("x count=1 count=2", cmd, 24, &a));
    TEST_ASSERT_EQUAL_INT(ARGS_TOKEN_TOO_LONG, parse_args("x abcdefghijklmnopqrstuvwxyz", cmd, 24, &a));
    TEST_ASSERT_EQUAL_INT(ARGS_TOKEN_TOO_LONG, parse_args("x k=abcdefghijklmnopqrstuvwxyz", cmd, 24, &a));
    TEST_ASSERT_EQUAL_INT(ARGS_TOO_MANY, parse_args("x 1 2 3 4 5 6 7 8 9 10 11 12 13", cmd, 24, &a));
    TEST_ASSERT_EQUAL_INT(ARGS_OK, parse_args("x 1 2 3 4 5 6 7 8 9 10 11 12", cmd, 24, &a));
    TEST_ASSERT_EQUAL_INT(ARGS_OK, parse_args("x 1 1", cmd, 24, &a)); // repeated positionals are fine
    // 23 characters fit (24 with the terminating 0)
    TEST_ASSERT_EQUAL_INT(ARGS_OK, parse_args("x abcdefghijklmnopqrstuvw", cmd, 24, &a));
}

void test_long_command_word_is_truncated(void)
{
    char cmd[8];
    Args a;
    TEST_ASSERT_EQUAL_INT(ARGS_OK, parse_args("encoder_repeat 3", cmd, 8, &a));
    TEST_ASSERT_EQUAL_STRING("encoder", cmd);
    TEST_ASSERT_EQUAL_INT(1, a.count);
}

void test_first_unknown(void)
{
    char cmd[24];
    static const char *const keys[] = {"from", "count", "order"};
    static const char *const flags[] = {"auto", "dry"};
    Args a = parse_ok("rotate_all dry auto from=1", cmd);
    TEST_ASSERT_EQUAL_INT(-1, args_first_unknown(a, keys, 3, flags, 2, 0));
    a = parse_ok("rotate_all dry bogus=1", cmd);
    TEST_ASSERT_EQUAL_INT(1, args_first_unknown(a, keys, 3, flags, 2, 0));
    a = parse_ok("rotate_all dry 5", cmd);
    TEST_ASSERT_EQUAL_INT(1, args_first_unknown(a, keys, 3, flags, 2, 0)); // no positional allowed
    TEST_ASSERT_EQUAL_INT(-1, args_first_unknown(a, keys, 3, flags, 2, 1));
    a = parse_ok("encoder_repeat 3 10 auto 4", cmd);
    TEST_ASSERT_EQUAL_INT(3, args_first_unknown(a, keys, 3, flags, 2, 2));
}

// ---------------------------------------------------------------- numbers

void test_parse_long_strict(void)
{
    long v = 0;
    TEST_ASSERT_TRUE(parse_long_strict("0", &v));
    TEST_ASSERT_EQUAL_INT(0, v);
    TEST_ASSERT_TRUE(parse_long_strict("200", &v));
    TEST_ASSERT_EQUAL_INT(200, v);
    TEST_ASSERT_TRUE(parse_long_strict("-7", &v));
    TEST_ASSERT_EQUAL_INT(-7, v);
    TEST_ASSERT_TRUE(parse_long_strict("+7", &v));
    TEST_ASSERT_TRUE(parse_long_strict("999999999", &v));
    TEST_ASSERT_FALSE(parse_long_strict("1000000000", &v)); // 10 digits
    TEST_ASSERT_FALSE(parse_long_strict("", &v));
    TEST_ASSERT_FALSE(parse_long_strict("-", &v));
    TEST_ASSERT_FALSE(parse_long_strict("12x", &v));
    TEST_ASSERT_FALSE(parse_long_strict("x12", &v));
    TEST_ASSERT_FALSE(parse_long_strict("1.5", &v));
    TEST_ASSERT_FALSE(parse_long_strict(0, &v));
}

void test_parse_float_strict(void)
{
    float f = 0;
    TEST_ASSERT_TRUE(parse_float_strict("50", &f));
    TEST_ASSERT_EQUAL_FLOAT(50.0f, f);
    TEST_ASSERT_TRUE(parse_float_strict("0.5", &f));
    TEST_ASSERT_EQUAL_FLOAT(0.5f, f);
    TEST_ASSERT_TRUE(parse_float_strict(".5", &f));
    TEST_ASSERT_TRUE(parse_float_strict("2.", &f));
    TEST_ASSERT_TRUE(parse_float_strict("-1.25", &f));
    TEST_ASSERT_EQUAL_FLOAT(-1.25f, f);
    TEST_ASSERT_FALSE(parse_float_strict(".", &f));
    TEST_ASSERT_FALSE(parse_float_strict("1e3", &f));
    TEST_ASSERT_FALSE(parse_float_strict("nan", &f));
    TEST_ASSERT_FALSE(parse_float_strict("1.2.3", &f));
    TEST_ASSERT_FALSE(parse_float_strict("", &f));
    TEST_ASSERT_FALSE(parse_float_strict("1234567890123", &f)); // too many digits
}

void test_parse_yes_no(void)
{
    bool b = false;
    TEST_ASSERT_TRUE(parse_yes_no("y", &b));
    TEST_ASSERT_TRUE(b);
    TEST_ASSERT_TRUE(parse_yes_no("YES", &b));
    TEST_ASSERT_TRUE(b);
    TEST_ASSERT_TRUE(parse_yes_no("n", &b));
    TEST_ASSERT_FALSE(b);
    TEST_ASSERT_TRUE(parse_yes_no("No", &b));
    TEST_ASSERT_FALSE(b);
    TEST_ASSERT_FALSE(parse_yes_no("yess", &b));
    TEST_ASSERT_FALSE(parse_yes_no("1", &b));
    TEST_ASSERT_FALSE(parse_yes_no("", &b));
}

void test_parse_ml_or_seconds(void)
{
    float ml = -1;
    long s = -1;
    TEST_ASSERT_TRUE(parse_ml_or_seconds("50", &ml, &s));
    TEST_ASSERT_EQUAL_FLOAT(50.0f, ml);
    TEST_ASSERT_EQUAL_INT(0, s);
    TEST_ASSERT_TRUE(parse_ml_or_seconds("12.5", &ml, &s));
    TEST_ASSERT_EQUAL_FLOAT(12.5f, ml);
    TEST_ASSERT_TRUE(parse_ml_or_seconds("20s", &ml, &s));
    TEST_ASSERT_EQUAL_INT(20, s);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, ml);
    TEST_ASSERT_TRUE(parse_ml_or_seconds("20S", &ml, &s));
    TEST_ASSERT_FALSE(parse_ml_or_seconds("s", &ml, &s));
    TEST_ASSERT_FALSE(parse_ml_or_seconds("0", &ml, &s));
    TEST_ASSERT_FALSE(parse_ml_or_seconds("0s", &ml, &s));
    TEST_ASSERT_FALSE(parse_ml_or_seconds("-5", &ml, &s));
    TEST_ASSERT_FALSE(parse_ml_or_seconds("2.5s", &ml, &s)); // whole seconds only
    TEST_ASSERT_FALSE(parse_ml_or_seconds("ml", &ml, &s));
    TEST_ASSERT_FALSE(parse_ml_or_seconds("", &ml, &s));
}

// ---------------------------------------------------------------- selection

void test_selection_from_args(void)
{
    char cmd[24];
    const char *err = 0;
    SlotSelection sel;
    SlotSelection base = default_selection();

    Args a = parse_ok("x", cmd);
    TEST_ASSERT_TRUE(selection_from_args(a, base, &sel, &err));
    TEST_ASSERT_FALSE(sel.index_order);
    TEST_ASSERT_EQUAL_INT(-1, sel.from_slot);
    TEST_ASSERT_EQUAL_INT(-1, sel.count);

    a = parse_ok("x from=12 count=3 order=index", cmd);
    TEST_ASSERT_TRUE(selection_from_args(a, base, &sel, &err));
    TEST_ASSERT_TRUE(sel.index_order);
    TEST_ASSERT_EQUAL_INT(12, sel.from_slot);
    TEST_ASSERT_EQUAL_INT(3, sel.count);

    base.include_purge = true;
    a = parse_ok("x order=fill from=0", cmd);
    TEST_ASSERT_TRUE(selection_from_args(a, base, &sel, &err));
    TEST_ASSERT_FALSE(sel.index_order);
    TEST_ASSERT_TRUE(sel.include_purge); // kept from the base
    TEST_ASSERT_EQUAL_INT(0, sel.from_slot);

    a = parse_ok("x count=0", cmd);
    TEST_ASSERT_FALSE(selection_from_args(a, base, &sel, &err));
    TEST_ASSERT_NOT_NULL(err);
    a = parse_ok("x from=-1", cmd);
    TEST_ASSERT_FALSE(selection_from_args(a, base, &sel, &err));
    a = parse_ok("x from=abc", cmd);
    TEST_ASSERT_FALSE(selection_from_args(a, base, &sel, &err));
    a = parse_ok("x order=random", cmd);
    TEST_ASSERT_FALSE(selection_from_args(a, base, &sel, &err));
}

// ---------------------------------------------------------------- TEST| line

void test_format_test_line(void)
{
    char buf[128];
    int n = format_test_line(buf, sizeof buf, "rotate_all", "3", TEST_PASS, "slot=3;err_deg=0.42");
    TEST_ASSERT_EQUAL_STRING("TEST|rotate_all|3|PASS|slot=3;err_deg=0.42", buf);
    TEST_ASSERT_EQUAL_INT((int)strlen(buf), n);

    format_test_line(buf, sizeof buf, "lock_cycle", "summary", TEST_FAIL, "");
    TEST_ASSERT_EQUAL_STRING("TEST|lock_cycle|summary|FAIL|", buf);
    format_test_line(buf, sizeof buf, "valve_check", "V1", TEST_SKIP, 0);
    TEST_ASSERT_EQUAL_STRING("TEST|valve_check|V1|SKIP|", buf);
}

void test_format_test_line_sanitizes(void)
{
    char buf[128];
    format_test_line(buf, sizeof buf, "a|b;c", "i=d\r\n", TEST_PASS, "k=v|x;m=1\n");
    TEST_ASSERT_EQUAL_STRING("TEST|a_b_c|i_d__|PASS|k=v_x;m=1_", buf);
}

void test_format_test_line_truncates(void)
{
    char buf[16];
    int n = format_test_line(buf, sizeof buf, "rotate_all", "3", TEST_PASS, "slot=3");
    TEST_ASSERT_EQUAL_INT(-1, n);
    TEST_ASSERT_EQUAL_INT(15, (int)strlen(buf));
    TEST_ASSERT_EQUAL_STRING("TEST|rotate_all", buf);
    TEST_ASSERT_EQUAL_INT(-1, format_test_line(buf, 0, "a", "b", TEST_PASS, "c"));
}

void test_sanitize_kv_value(void)
{
    char s[] = "joint 3; left=wet|drip\r\n";
    sanitize_kv_value(s);
    TEST_ASSERT_EQUAL_STRING("joint 3_ left_wet_drip__", s);
}

void test_result_names(void)
{
    TEST_ASSERT_EQUAL_STRING("PASS", test_result_name(TEST_PASS));
    TEST_ASSERT_EQUAL_STRING("FAIL", test_result_name(TEST_FAIL));
    TEST_ASSERT_EQUAL_STRING("SKIP", test_result_name(TEST_SKIP));
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_command_only);
    RUN_TEST(test_empty_line_and_whitespace);
    RUN_TEST(test_full_line_with_crlf);
    RUN_TEST(test_parse_errors);
    RUN_TEST(test_long_command_word_is_truncated);
    RUN_TEST(test_first_unknown);
    RUN_TEST(test_parse_long_strict);
    RUN_TEST(test_parse_float_strict);
    RUN_TEST(test_parse_yes_no);
    RUN_TEST(test_parse_ml_or_seconds);
    RUN_TEST(test_selection_from_args);
    RUN_TEST(test_format_test_line);
    RUN_TEST(test_format_test_line_sanitizes);
    RUN_TEST(test_format_test_line_truncates);
    RUN_TEST(test_sanitize_kv_value);
    RUN_TEST(test_result_names);
    return UNITY_END();
}
