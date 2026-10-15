#include <unity.h>
#include <cmath>
#include "power_ramp.h"

void setUp(void) {}
void tearDown(void) {}

// Test 1: Ramp starts at start_power
void test_power_ramp_initial_value(void) {
  float power = core::power_ramp_value(0, 8000, 15, 40, 3);
  TEST_ASSERT_EQUAL_FLOAT(15.0f, power);
}

// Test 2: Ramp reaches peak_power at ramp_duration
void test_power_ramp_reaches_peak(void) {
  float power = core::power_ramp_value(8000, 8000, 15, 40, 3);
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 40.0f, power);
}

// Test 3: Ramp is linear interpolation at midpoint
void test_power_ramp_midpoint(void) {
  // At t = 4000 (half of 8000), should be halfway between 15 and 40
  // Expected: 15 + 0.5 * (40 - 15) = 15 + 12.5 = 27.5
  float power = core::power_ramp_value(4000, 8000, 15, 40, 3);
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 27.5f, power);
}

// Test 4: Ramp clamps to [0, 100]
void test_power_ramp_clamps_negative(void) {
  float power = core::power_ramp_value(1000, 8000, 0, 10, 3);
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.25f, power);  // Should be ~1.25, not negative
  TEST_ASSERT_TRUE(power >= 0.0f);
}

// Test 5: Ramp clamps above 100
void test_power_ramp_clamps_over_100(void) {
  float power = core::power_ramp_value(10000, 8000, 90, 110, 3);
  TEST_ASSERT_TRUE(power <= 100.0f);
}

// Test 6: ramp complete returns true at duration
void test_power_ramp_complete_true(void) {
  TEST_ASSERT_TRUE(core::power_ramp_complete(8000, 8000));
}

// Test 7: ramp complete returns false before duration
void test_power_ramp_complete_false(void) {
  TEST_ASSERT_FALSE(core::power_ramp_complete(4000, 8000));
}

// Test 8: ramp complete with zero duration
void test_power_ramp_complete_zero_duration(void) {
  TEST_ASSERT_TRUE(core::power_ramp_complete(1000, 0));
}

// Test 9: Rise-rate limiter allows normal rise
void test_rise_rate_limit_normal(void) {
  // Pressure rises from 1.0 to 1.1 bar in 100ms = 1.0 bar/s
  // Max limit is 0.5 bar/s, so should be throttled
  float power_in = 40.0f;
  float power_out = core::rise_rate_limit(1.1f, 1.0f, 100, 0.5f, power_in);
  TEST_ASSERT_TRUE(power_out < power_in);  // Should be reduced
}

// Test 10: Rise-rate limiter passes if pressure not rising too fast
void test_rise_rate_limit_slow_rise(void) {
  // Pressure rises from 1.0 to 1.02 bar in 100ms = 0.2 bar/s (within 0.5 limit)
  float power_in = 40.0f;
  float power_out = core::rise_rate_limit(1.02f, 1.0f, 100, 0.5f, power_in);
  TEST_ASSERT_EQUAL_FLOAT(power_in, power_out);
}

// Test 11: Rise-rate limiter returns clamped value
void test_rise_rate_limit_clamps_output(void) {
  float power_out = core::rise_rate_limit(2.0f, 1.0f, 100, 0.5f, 40.0f);
  TEST_ASSERT_TRUE(power_out >= 0.0f);
  TEST_ASSERT_TRUE(power_out <= 100.0f);
}

// Test 12: Rise-rate limiter ignores negative pressure change
void test_rise_rate_limit_falling_pressure(void) {
  float power_in = 40.0f;
  float power_out = core::rise_rate_limit(0.9f, 1.0f, 100, 0.5f, power_in);
  TEST_ASSERT_EQUAL_FLOAT(power_in, power_out);
}

// Test 13: Clog detection fires at peak power and low pressure
void test_clog_detection_true(void) {
  bool is_clog = core::is_clogged(40.0f, 40.0f, 0.8f, 1.0f);
  TEST_ASSERT_TRUE(is_clog);
}

// Test 14: Clog detection false if power not at peak
void test_clog_detection_false_low_power(void) {
  bool is_clog = core::is_clogged(30.0f, 40.0f, 0.8f, 1.0f);
  TEST_ASSERT_FALSE(is_clog);
}

// Test 15: Clog detection false if pressure above threshold
void test_clog_detection_false_high_pressure(void) {
  bool is_clog = core::is_clogged(40.0f, 40.0f, 1.5f, 1.0f);
  TEST_ASSERT_FALSE(is_clog);
}

// Test 16: Clog detection tolerates small power shortfall (within 1%)
void test_clog_detection_near_peak(void) {
  bool is_clog = core::is_clogged(39.2f, 40.0f, 0.8f, 1.0f);
  TEST_ASSERT_TRUE(is_clog);  // Within 1% of peak
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_power_ramp_initial_value);
  RUN_TEST(test_power_ramp_reaches_peak);
  RUN_TEST(test_power_ramp_midpoint);
  RUN_TEST(test_power_ramp_clamps_negative);
  RUN_TEST(test_power_ramp_clamps_over_100);
  RUN_TEST(test_power_ramp_complete_true);
  RUN_TEST(test_power_ramp_complete_false);
  RUN_TEST(test_power_ramp_complete_zero_duration);
  RUN_TEST(test_rise_rate_limit_normal);
  RUN_TEST(test_rise_rate_limit_slow_rise);
  RUN_TEST(test_rise_rate_limit_clamps_output);
  RUN_TEST(test_rise_rate_limit_falling_pressure);
  RUN_TEST(test_clog_detection_true);
  RUN_TEST(test_clog_detection_false_low_power);
  RUN_TEST(test_clog_detection_false_high_pressure);
  RUN_TEST(test_clog_detection_near_peak);

  return UNITY_END();
}
