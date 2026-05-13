#include "network_manager.h"

static const char *TAG = "NET_MGR";
static bool is_connected = false;

void on_got_ip(void *arg, esp_event_base_t base, int32_t id, void *data) {
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
  ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

  is_connected = true;

  // if MQTT was already started by another interface, the function returns
  // ESP_OK
  network_mqtt_init(CONFIG_MQTT_BROKER_URL);
}

esp_err_t network_manager_init(net_interface_config_t config) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // listen for IP from both Wi-Fi and Ethernet
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_ETH_GOT_IP, &on_got_ip, NULL, NULL));

  if (config == NET_INTERFACE_WIFI || config == NET_INTERFACE_BOTH) {
    wifi_handler_init();
  }

  if (config == NET_INTERFACE_ETH || config == NET_INTERFACE_BOTH) {
    eth_handler_init();
  }

  return ESP_OK;
}

bool network_is_connected(void) { return is_connected; }
