#include "current_monitor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "network_manager.h"

static const char *TAG = "BARE_METAL";

void app_main(void) {
  current_reading_config_t vref_cfg = {.adc_unit_id = ADC_UNIT_1,
                                       .adc_channel_id = ADC_CHANNEL_4};
  current_reading_config_t leakage_cfg = {.adc_unit_id = ADC_UNIT_1,
                                          .adc_channel_id = ADC_CHANNEL_6};
  current_reading_config_t load_cfg = {.adc_unit_id = ADC_UNIT_1,
                                       .adc_channel_id = ADC_CHANNEL_7};

  current_monitor_handle_t *monitor =
      current_monitor_init(vref_cfg, leakage_cfg, load_cfg);
  monitor->vref_enabled = true;
  monitor->cali_enabled = true;

  if (current_monitor_begin(monitor) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize sensors");
    return;
  }
  network_manager_t *net_mgr =
      network_manager_create(NET_INTERFACE_WIFI, CONFIG_MQTT_BROKER_URL);
  ESP_ERROR_CHECK(network_manager_init(net_mgr));

  ESP_LOGI(TAG, "Starting Bare-Metal Super-Loop...");

  int task_should_run = 1;
  while (task_should_run) {
    TickType_t start_tick = xTaskGetTickCount();
    current_readings_t data = current_monitor_get_readings(monitor);
    char payload_buffer[200];

    ESP_LOGI(TAG, "Leakage RMS: %.2f | Load RMS: %.2f", data.leakage_rms,
             data.load_rms);

    current_monitor_readings_to_str(&data, payload_buffer,
                                    sizeof(payload_buffer));

    if (network_is_connected(net_mgr)) {
      network_mqtt_publish(net_mgr, CONFIG_MQTT_TOPIC, payload_buffer);
    }

    vTaskDelayUntil(&start_tick, pdMS_TO_TICKS(TOTAL_LOOP_MS));
  }
  current_monitor_deinit(monitor);
  network_manager_destroy(net_mgr);
}
