#include "esp_log.h"
#include "esp_mac.h"
#include "network_manager.h"
#include <stdio.h>

static const char *TAG = "MQTT_H";

static void generate_client_id(network_manager_t *mgr) {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(mgr->mqtt_client_id, sizeof(mgr->mqtt_client_id),
           "current_monitor_%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2],
           mac[3], mac[4], mac[5]);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  // Note: handler_args is 'mgr' because we pass it in client_init
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED");
    break;
  default:
    break;
  }
}

esp_err_t network_mqtt_init(network_manager_t *mgr) {
  if (!mgr || mgr->mqtt_client != NULL)
    return ESP_OK;

  generate_client_id(mgr);

  const esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = mgr->broker_url,
      .credentials.client_id = mgr->mqtt_client_id,
      .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
  };

  mgr->mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(mgr->mqtt_client, ESP_EVENT_ANY_ID,
                                 mqtt_event_handler, mgr);
  return esp_mqtt_client_start(mgr->mqtt_client);
}

esp_err_t network_mqtt_publish(network_manager_t *mgr, const char *topic,
                               const char *data) {
  if (!mgr || mgr->mqtt_client == NULL)
    return ESP_FAIL;
  int msg_id = esp_mqtt_client_publish(mgr->mqtt_client, topic, data, 0, 1, 0);
  return (msg_id != -1) ? ESP_OK : ESP_FAIL;
}
