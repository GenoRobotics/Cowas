// Native unit tests for src/Core/trusted_volume (run: pio test -e native). F150: only fast flow counts as confident.
#include <unity.h>

#include "trusted_volume.h"

using namespace core;

void setUp(void) {}
void tearDown(void) {}

static const float MIN_RATE = 100.0f; // mL/min (placeholder in Settings.h)
static const uint32_t WINDOW = 5000;  // ms

// Feeds `seconds` of steady flow at rate_ml_per_min, one step of 100 ms, starting at *now and *total.
static void feed(TrustedVolume &tv, float rate_ml_per_min, int seconds, uint32_t *now, float *total)
{
    for (int i = 0; i < seconds * 10; i++)
    {
        *now += 100;
        *total += rate_ml_per_min / 600.0f;
        tv.update(*total, *now, MIN_RATE, WINDOW);
    }
}

void test_fast_flow_is_confident(void)
{
    TrustedVolume tv;
    uint32_t now = 0;
    float total = 0.0f;
    tv.reset(total, now);
    feed(tv, 150.0f, 30, &now, &total);
    tv.close(total, now, MIN_RATE);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, total, tv.trusted_ml()); // 150 mL/min for 30 s = 75 mL, all confident
}

void test_slow_flow_is_not_confident(void)
{
    TrustedVolume tv;
    uint32_t now = 0;
    float total = 0.0f;
    tv.reset(total, now);
    feed(tv, 50.0f, 30, &now, &total); // 25 mL, below the 100 mL/min floor
    tv.close(total, now, MIN_RATE);
    TEST_ASSERT_TRUE(total > 20.0f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, tv.trusted_ml());
}

void test_mixed_counts_only_the_fast_part(void)
{
    TrustedVolume tv;
    uint32_t now = 0;
    float total = 0.0f;
    tv.reset(total, now);
    feed(tv, 50.0f, 20, &now, &total);  // slow: not counted
    feed(tv, 150.0f, 20, &now, &total); // fast: counted (50 mL)
    tv.close(total, now, MIN_RATE);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 50.0f, tv.trusted_ml());
    TEST_ASSERT_TRUE(total > tv.trusted_ml());
}

void test_reset_clears_the_count(void)
{
    TrustedVolume tv;
    uint32_t now = 0;
    float total = 0.0f;
    tv.reset(total, now);
    feed(tv, 150.0f, 10, &now, &total);
    tv.reset(total, now);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, tv.trusted_ml());
}

int main(int, char **)
{
    UNITY_BEGIN();
    RUN_TEST(test_fast_flow_is_confident);
    RUN_TEST(test_slow_flow_is_not_confident);
    RUN_TEST(test_mixed_counts_only_the_fast_part);
    RUN_TEST(test_reset_clears_the_count);
    return UNITY_END();
}
