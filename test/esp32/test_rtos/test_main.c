#include "current_monitor.h"
#include <unity.h>

void test_sensor_init_success(void) {
  // Example test: verify monitor handle is created
  current_reading_config_t cfg = {0};
  current_monitor_handle_t *handle = current_monitor_init(cfg, cfg);
  TEST_ASSERT_NOT_NULL(handle);
}

void app_main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_sensor_init_success);
  UNITY_END();

  // Optional: Renode can be told to stop here via a hook
  // or just let the timeout handle it.
}
