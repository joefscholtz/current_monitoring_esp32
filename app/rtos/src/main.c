#include "current_monitor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "RTOS_APP";

void vSensorProcessingTask(void *pvParameters) {
  current_monitor_handle_t *monitor = (current_monitor_handle_t *)pvParameters;

  while (1) {
    current_readings_t data = current_monitor_get_readings(monitor);

    ESP_LOGD(TAG, "Processing complete, notifying MQTT task...");

    // relinquish CPU for the remainder of the 100ms cycle
    vTaskDelay(pdMS_TO_TICKS(TOTAL_LOOP_MS -
                             (SAMPLE_COUNT * SAMPLE_INTERVAL_US / 1000)));
  }
}

void app_main(void) {
  current_reading_config_t leakage_cfg = {.adc_unit_id = ADC_UNIT_1,
                                          .adc_channel_id = ADC_CHANNEL_6};
  current_reading_config_t load_cfg = {.adc_unit_id = ADC_UNIT_1,
                                       .adc_channel_id = ADC_CHANNEL_7};

  current_monitor_handle_t *monitor =
      current_monitor_init(leakage_cfg, load_cfg);
  current_monitor_begin(monitor);

  xTaskCreatePinnedToCore(
      vSensorProcessingTask, "SensorMonitor", 4096, (void *)monitor,
      configMAX_PRIORITIES - 1, // high priority to minimize jitter
      NULL, 1);

  ESP_LOGI(TAG, "Sensor task spawned on Core 1.");
}
