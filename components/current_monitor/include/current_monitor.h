#ifndef CURRENT_MONITOR_H
#define CURRENT_MONITOR_H

#include "current_monitor_utils.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include <assert.h>

// SAMPLE_COUNT*SAMPLE_INTERVAL_US == 50ms satisfies Nyquist for 50 adn 60Hz
#define SAMPLE_COUNT 100
#define SAMPLE_INTERVAL_US 500 // 0.5ms
#define TOTAL_LOOP_MS 100

static_assert((SAMPLE_COUNT * SAMPLE_INTERVAL_US) / 1000.0 <=
                  TOTAL_LOOP_MS / 1.5,
              "(SAMPLE_COUNT * SAMPLE_INTERVAL_US) / 1000.0 > TOTAL_LOOP_MS / "
              "1.5. Sampling interval must be at most 3/2 of the total loop "
              "interval"); // use at max 3/2 of the loop to get acquisitions

typedef struct {
  adc_unit_t adc_unit_id;
  adc_channel_t adc_channel_id;
} current_reading_config_t;

typedef struct {
  current_reading_config_t leakage_config;
  current_reading_config_t load_config;
  adc_oneshot_unit_handle_t leakage_adc_handle;
  adc_oneshot_unit_handle_t load_adc_handle;
} current_monitor_handle_t;

typedef struct {
  float leakage_rms;
  float load_rms;
} current_readings_t;

current_monitor_handle_t *
current_monitor_init(current_reading_config_t leakage_config,
                     current_reading_config_t load_config);

esp_err_t current_monitor_begin(current_monitor_handle_t *handle);

esp_err_t current_monitor_calibrate_adc();

esp_err_t current_monitor_deinit(current_monitor_handle_t *handle);

current_readings_t
current_monitor_get_readings(current_monitor_handle_t *handle);

#endif
