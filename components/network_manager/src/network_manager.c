#include "network_manager.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

static const char *TAG = "NET_MGR";
static bool is_connected = false;

// Internal prototypes
esp_err_t wifi_handler_init(void);
esp_err_t eth_handler_init(void);

static void on_got_ip(void *arg, esp_event_base_t base, int32_t id,
                      void *data) {
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
  ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
  is_connected = true;
  // Trigger MQTT Start here in next stage
}

esp_err_t network_manager_init(net_interface_config_t config) {
  // 1. Initialize NVS (Required for Wi-Fi storage)
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // 2. Initialize TCP/IP stack and Event Loop
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // 3. Register IP Events
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
