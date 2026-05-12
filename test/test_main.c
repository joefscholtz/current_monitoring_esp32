#include "current_monitor_utils.h"
#include <unity.h>

void test_rms_with_dc_signal(void) {
  int samples[] = {2148, 2148, 2148, 2148}; // Offset of 100 above 2048
  float result = compute_rms(samples, 4, 2048);
  TEST_ASSERT_EQUAL_FLOAT(100.0f, result);
}

void test_rms_with_alternating_signal(void) {
  // square wave: +/- 10 centered at 2048
  int samples[] = {2058, 2038, 2058, 2038};
  float result = compute_rms(samples, 4, 2048);
  TEST_ASSERT_EQUAL_FLOAT(10.0f, result);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_rms_with_dc_signal);
  RUN_TEST(test_rms_with_alternating_signal);
  return UNITY_END();
}
