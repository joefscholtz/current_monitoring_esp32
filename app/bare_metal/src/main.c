#include "current_monitor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "network_manager.h"

static const char *TAG = "BARE_METAL";

void app_main(void) {
  current_reading_config_t leakage_cfg = {.adc_unit_id = ADC_UNIT_1,
                                          .adc_channel_id = ADC_CHANNEL_6};
  current_reading_config_t load_cfg = {.adc_unit_id = ADC_UNIT_1,
                                       .adc_channel_id = ADC_CHANNEL_7};

  current_monitor_handle_t *monitor =
      current_monitor_init(leakage_cfg, load_cfg);

  if (current_monitor_begin(monitor) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize sensors");
    return;
  }

  ESP_ERROR_CHECK(network_manager_init(NET_INTERFACE_WIFI))

  ESP_LOGI(TAG, "Starting Bare-Metal Super-Loop...");

  while (1) {
    TickType_t start_tick = xTaskGetTickCount();

    current_readings_t data = current_monitor_get_readings(monitor);

    ESP_LOGI(TAG, "Leakage RMS: %.2f | Load RMS: %.2f", data.leakage_rms,
             data.load_rms);

    if (network_is_connected()) {
      char payload[128];
      snprintf(payload, sizeof(payload),
               "{\"leakage_rms\": %.2f, \"load_rms\": %.2f}", data.leakage_rms,
               data.load_rms);

      network_mqtt_publish("v1/devices/me/telemetry", payload);
    }

    vTaskDelayUntil(&start_tick, pdMS_TO_TICKS(TOTAL_LOOP_MS));
  }
}
