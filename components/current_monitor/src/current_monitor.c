#include "current_monitor.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include <math.h>
#include <stdlib.h>

current_monitor_handle_t *
current_monitor_init(current_reading_config_t leakage_config,
                     current_reading_config_t load_config) {
  current_monitor_handle_t *handle = malloc(sizeof(current_monitor_handle_t));
  if (handle) {
    handle->leakage_config = leakage_config;
    handle->leakage_config = load_config;
    handle->leakage_adc_handle = NULL;
    handle->load_adc_handle = NULL;
  }
  return handle;
}

esp_err_t current_monitor_begin(current_monitor_handle_t *handle) {
  if (!handle)
    return ESP_ERR_INVALID_ARG;

  adc_oneshot_unit_init_cfg_t leakage_init_cfg = {
      .unit_id = handle->leakage_config.adc_unit_id,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  ESP_ERROR_CHECK(
      adc_oneshot_new_unit(&leakage_init_cfg, &handle->leakage_adc_handle));

  adc_oneshot_chan_cfg_t chan_cfg = {
      .bitwidth = ADC_BITWIDTH_12,
      .atten = ADC_ATTEN_DB_12, // For WROOM-32E (up to ~3.1V)
  };

  ESP_ERROR_CHECK(adc_oneshot_config_channel(
      handle->leakage_adc_handle, handle->leakage_config.adc_channel_id,
      &chan_cfg));

  if (handle->leakage_config.adc_unit_id != handle->load_config.adc_unit_id) {
    adc_oneshot_unit_init_cfg_t load_init_cfg = {
        .unit_id = handle->load_config.adc_unit_id,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&load_init_cfg, &handle->load_adc_handle));
  } else {
    handle->load_adc_handle = handle->leakage_adc_handle;
  }

  ESP_ERROR_CHECK(adc_oneshot_config_channel(
      handle->load_adc_handle, handle->load_config.adc_channel_id, &chan_cfg));

  return ESP_OK;
}

current_readings_t
current_monitor_get_readings(current_monitor_handle_t *handle) {
  current_readings_t readings = {0};
  double sum_sq_leakage = 0;
  double sum_sq_load = 0;
  int le_raw, lo_raw;
  int64_t next_sample_time = esp_timer_get_time(); // in ms

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int64_t current_time;
    while ((current_time = esp_timer_get_time()) < next_sample_time)
      ;
    if (current_time >= next_sample_time + SAMPLE_INTERVAL_US) {
      // acquisition interval missed
    }

    adc_oneshot_read(handle->leakage_adc_handle,
                     handle->leakage_config.adc_channel_id, &le_raw);
    adc_oneshot_read(handle->load_adc_handle,
                     handle->load_config.adc_channel_id, &lo_raw);

    // center the 12-bit value (0-4095) around the 1.65V virtual ground
    float le_centered = (float)le_raw - 2048.0f;
    float lo_centered = (float)lo_raw - 2048.0f;

    sum_sq_leakage += (le_centered * le_centered);
    sum_sq_load += (lo_centered * lo_centered);

    next_sample_time += SAMPLE_INTERVAL_US;
  }

  // RMS
  readings.leakage_rms = sqrt(sum_sq_leakage / SAMPLE_COUNT);
  readings.load_rms = sqrt(sum_sq_load / SAMPLE_COUNT);
  // convert using calibration

  return readings;
}

esp_err_t current_monitor_deinit(current_monitor_handle_t *handle) {
  if (!handle)
    return ESP_ERR_INVALID_ARG;

  if (handle->leakage_config.adc_unit_id != handle->load_config.adc_unit_id) {
    if (!handle->load_adc_handle)
      return ESP_ERR_INVALID_ARG;
    ESP_ERROR_CHECK(adc_oneshot_del_unit(handle->load_adc_handle));
  }
  if (!handle->leakage_adc_handle)
    return ESP_ERR_INVALID_ARG;

  ESP_ERROR_CHECK(adc_oneshot_del_unit(handle->leakage_adc_handle));
  if (handle) {
    free(handle);
  }
  handle = NULL;
  return ESP_OK;
}
