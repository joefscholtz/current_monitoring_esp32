#include "current_monitor.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_MV_VALUE 3300
#define MAX_DIGI_VALUE 4095

static const char *TAG = "CURRENT_MONITOR";

void current_monitor_readings_to_str(const current_readings_t *readings,
                                     char *out_str, size_t len) {
  if (readings == NULL || out_str == NULL)
    return;

  snprintf(out_str, len, "{\"leakage_rms\":%.4f,\"load_rms\":%.2f,\"ts\":%llu}",
           readings->leakage_rms, readings->load_rms,
           (unsigned long long)(readings->timestamp_ms));
}

esp_err_t calibrate_adc(adc_unit_t unit, adc_channel_t channel,
                        adc_atten_t atten, adc_cali_handle_t *out_handle) {
  adc_cali_handle_t handle = NULL;
  esp_err_t ret = ESP_FAIL;

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  adc_cali_line_fitting_config_t cali_config = {
      .unit_id = unit,
      .atten = atten,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
#endif

  if (ret == ESP_OK) {
    *out_handle = handle;
  }
  return ret;
}

current_monitor_handle_t *
current_monitor_init(current_reading_config_t vref_cfg,
                     current_reading_config_t leakage_cfg,
                     current_reading_config_t load_cfg) {

  current_monitor_handle_t *handle = malloc(sizeof(current_monitor_handle_t));
  if (handle) {
    handle->vref_config = vref_cfg;
    handle->leakage_config = leakage_cfg;
    handle->load_config = load_cfg;

    handle->vref_adc_handle = NULL;
    handle->leakage_adc_handle = NULL;
    handle->load_adc_handle = NULL;
    handle->cali_enabled = false;
    handle->vref_enabled = false;
  }
  return handle;
}

esp_err_t current_monitor_begin(current_monitor_handle_t *handle) {
  if (!handle)
    return ESP_ERR_INVALID_ARG;

  adc_oneshot_chan_cfg_t chan_cfg = {
      .bitwidth = ADC_BITWIDTH_12,
      .atten = ADC_ATTEN_DB_12,
  };

  // 1. Init Leakage Unit
  adc_oneshot_unit_init_cfg_t leakage_init_cfg = {
      .unit_id = handle->leakage_config.adc_unit_id};
  ESP_ERROR_CHECK(
      adc_oneshot_new_unit(&leakage_init_cfg, &handle->leakage_adc_handle));
  ESP_ERROR_CHECK(adc_oneshot_config_channel(
      handle->leakage_adc_handle, handle->leakage_config.adc_channel_id,
      &chan_cfg));

  // 2. Init Load Unit (Reuse or New)
  if (handle->load_config.adc_unit_id == handle->leakage_config.adc_unit_id) {
    handle->load_adc_handle = handle->leakage_adc_handle;
  } else {
    adc_oneshot_unit_init_cfg_t load_init_cfg = {
        .unit_id = handle->load_config.adc_unit_id};
    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&load_init_cfg, &handle->load_adc_handle));
  }
  ESP_ERROR_CHECK(adc_oneshot_config_channel(
      handle->load_adc_handle, handle->load_config.adc_channel_id, &chan_cfg));

  // 3. Init Vref Unit (Reuse or New)
  if (handle->vref_enabled) {
    if (handle->vref_config.adc_unit_id == handle->leakage_config.adc_unit_id) {
      handle->vref_adc_handle = handle->leakage_adc_handle;
    } else if (handle->vref_config.adc_unit_id ==
               handle->load_config.adc_unit_id) {
      handle->vref_adc_handle = handle->load_adc_handle;
    } else {
      adc_oneshot_unit_init_cfg_t vref_init_cfg = {
          .unit_id = handle->vref_config.adc_unit_id};
      ESP_ERROR_CHECK(
          adc_oneshot_new_unit(&vref_init_cfg, &handle->vref_adc_handle));
    }
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        handle->vref_adc_handle, handle->vref_config.adc_channel_id,
        &chan_cfg));
  }

  // 4. Cali (Using Leakage as reference for Cali Scheme)
  handle->cali_enabled =
      (calibrate_adc(handle->leakage_config.adc_unit_id,
                     handle->leakage_config.adc_channel_id, ADC_ATTEN_DB_12,
                     &handle->cali_handle) == ESP_OK);
  return ESP_OK;
}

current_readings_t
current_monitor_get_readings(current_monitor_handle_t *handle) {
  current_readings_t readings = {0};
  int le_samples[SAMPLE_COUNT];
  int lo_samples[SAMPLE_COUNT];
  int le_raw, lo_raw, vr_raw;

  // Measure the hardware Vref
  int vref_mv = 1650; // Standard fallback
  if (handle->vref_enabled) {
    adc_oneshot_read(handle->vref_adc_handle,
                     handle->vref_config.adc_channel_id, &vr_raw);
    if (handle->cali_enabled) {
      adc_cali_raw_to_voltage(handle->cali_handle, vr_raw, &vref_mv);
    } else {
      vref_mv = (vr_raw * MAX_MV_VALUE) / MAX_DIGI_VALUE;
    }
  }

  int64_t next_sample_time = esp_timer_get_time();

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    while (esp_timer_get_time() < next_sample_time)
      ;

    // Check for missed interval
    if (esp_timer_get_time() > next_sample_time + (SAMPLE_INTERVAL_US / 2)) {
      ESP_LOGW(TAG, "Sampling interval missed at index %d", i);
    }

    adc_oneshot_read(handle->leakage_adc_handle,
                     handle->leakage_config.adc_channel_id, &le_raw);
    adc_oneshot_read(handle->load_adc_handle,
                     handle->load_config.adc_channel_id, &lo_raw);

    // Convert raw to mV for the RMS function
    if (handle->cali_enabled) {
      adc_cali_raw_to_voltage(handle->cali_handle, le_raw, &le_samples[i]);
      adc_cali_raw_to_voltage(handle->cali_handle, lo_raw, &lo_samples[i]);
    } else {
      le_samples[i] = (le_raw * MAX_MV_VALUE) / MAX_DIGI_VALUE;
      lo_samples[i] = (lo_raw * MAX_MV_VALUE) / MAX_DIGI_VALUE;
    }

    next_sample_time += SAMPLE_INTERVAL_US;
  }

  // Compute RMS using your provided function
  // We pass the measured vref_mv as the offset
  readings.leakage_rms = compute_rms(le_samples, SAMPLE_COUNT, vref_mv);
  readings.load_rms = compute_rms(lo_samples, SAMPLE_COUNT, vref_mv);

  return readings;
}

esp_err_t current_monitor_deinit(current_monitor_handle_t *handle) {
  if (!handle)
    return ESP_ERR_INVALID_ARG;

  if (handle->cali_enabled) {
    adc_cali_delete_scheme_line_fitting(handle->cali_handle);
  }

  if (handle->leakage_adc_handle) {
    adc_oneshot_del_unit(handle->leakage_adc_handle);
  }
  if (handle->load_adc_handle &&
      handle->load_adc_handle != handle->leakage_adc_handle) {
    adc_oneshot_del_unit(handle->load_adc_handle);
  }
  if (handle->vref_adc_handle &&
      handle->vref_adc_handle != handle->leakage_adc_handle &&
      handle->vref_adc_handle != handle->load_adc_handle) {
    adc_oneshot_del_unit(handle->vref_adc_handle);
  }

  free(handle);
  return ESP_OK;
}
