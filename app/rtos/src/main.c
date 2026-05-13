#include "current_monitor.h"
#include "esp_log.h"
#include "esp_rom_serial_output.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "network_manager.h"
#include <stdio.h>
#include <sys/time.h>

static const char *TAG = "RTOS_APP";

static QueueHandle_t data_queue;

void vSensorProcessingTask(void *pvParameters) {
  current_monitor_handle_t *monitor = (current_monitor_handle_t *)pvParameters;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(TOTAL_LOOP_MS);

  int task_should_run = 1;
  while (task_should_run) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    current_readings_t readings = current_monitor_get_readings(monitor);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    readings.timestamp_ms =
        (uint64_t)tv.tv_sec * 1000LL + (uint64_t)tv.tv_usec / 1000LL;

    xQueueSend(data_queue, &readings, 0);
  }
  current_monitor_deinit(monitor);
  vTaskDelete(NULL);
}

void vNetworkTask(void *pvParameters) {
  network_manager_t *net_mgr = (network_manager_t *)pvParameters;
  current_readings_t data;
  char payload_buffer[200];

  int task_should_run = 1;
  while (task_should_run) {
    if (xQueueReceive(data_queue, &data, portMAX_DELAY)) {
      if (network_is_connected(net_mgr)) {
        current_monitor_readings_to_str(&data, payload_buffer,
                                        sizeof(payload_buffer));
        network_mqtt_publish(net_mgr, CONFIG_MQTT_TOPIC, payload_buffer);
        ESP_LOGD(TAG, "Published: %s", payload_buffer);
      }
    }
  }
  network_manager_destroy(net_mgr);
  vTaskDelete(NULL);
}

void app_main(void) {
  data_queue = xQueueCreate(10, sizeof(current_readings_t));

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

  xTaskCreatePinnedToCore(
      vSensorProcessingTask, "SensorMonitor", 4096, (void *)monitor,
      configMAX_PRIORITIES - 1, // high priority to minimize jitter
      NULL, 1);

  esp_rom_printf("\n--- ROM PRINT: RTOS Task Started ---\n");
  printf("RTOS Task Started\n");

  ESP_LOGI(TAG, "Sensor task spawned on Core 1.");

  network_manager_t *net_mgr =
      network_manager_create(NET_INTERFACE_WIFI, CONFIG_MQTT_BROKER_URL);
  ESP_ERROR_CHECK(network_manager_init(net_mgr));

  xTaskCreatePinnedToCore(vNetworkTask, "Network", 4096, (void *)net_mgr,
                          configMAX_PRIORITIES - 2, NULL, 0);
}
