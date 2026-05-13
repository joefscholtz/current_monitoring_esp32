#include "esp_log.h"
#include "esp_mac.h"
#include "mqtt_client.h"
#include <stdio.h>

static const char *TAG = "MQTT_H";
static esp_mqtt_client_handle_t client = NULL;
static char mqtt_client_id[32];

// Use Base MAC so identity is persistent across WiFi/ETH
static void generate_client_id(void) {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(mqtt_client_id, sizeof(mqtt_client_id),
           "current_monitor_%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2],
           mac[3], mac[4], mac[5]);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    // subscribe to commands here
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED");
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    break;
  }
}

esp_err_t network_mqtt_init(const char *broker_url) {
  if (client != NULL)
    return ESP_OK; // already initialized

  generate_client_id();

  const esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = broker_url,
      .credentials.client_id = mqtt_client_id,
      .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
  };

  client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 NULL);
  return esp_mqtt_client_start(client);
}

esp_err_t network_mqtt_publish(const char *topic, const char *data) {
  if (client == NULL) {
    ESP_LOGW(TAG, "Cannot publish, MQTT client not initialized");
    return ESP_FAIL;
  }
  int msg_id = esp_mqtt_client_publish(client, topic, data, 0, 1, 0);
  return (msg_id != -1) ? ESP_OK : ESP_FAIL;
}
