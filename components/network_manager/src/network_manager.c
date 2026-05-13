#include "network_manager.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "NET_MGR";

void on_got_ip(void *arg, esp_event_base_t base, int32_t id, void *data) {
  network_manager_t *mgr = (network_manager_t *)arg;
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
  ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

  mgr->is_connected = true;
  network_mqtt_init(mgr);
}

network_manager_t *network_manager_create(net_interface_config_t config,
                                          const char *broker_url) {
  network_manager_t *mgr =
      (network_manager_t *)malloc(sizeof(network_manager_t));
  if (mgr != NULL) {
    memset(mgr, 0, sizeof(network_manager_t));
    mgr->config = config;
    mgr->is_connected = false;
    strncpy(mgr->broker_url, broker_url, sizeof(mgr->broker_url) - 1);
  }
  return mgr;
}

esp_err_t network_manager_init(network_manager_t *mgr) {
  if (!mgr)
    return ESP_ERR_INVALID_ARG;

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, mgr, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_ETH_GOT_IP, &on_got_ip, mgr, NULL));

  if (mgr->config == NET_INTERFACE_WIFI || mgr->config == NET_INTERFACE_BOTH) {
    wifi_handler_init(mgr);
  }
  if (mgr->config == NET_INTERFACE_ETH || mgr->config == NET_INTERFACE_BOTH) {
    eth_handler_init(mgr);
  }

  return ESP_OK;
}

bool network_is_connected(network_manager_t *mgr) {
  return (mgr != NULL) && mgr->is_connected;
}

esp_err_t network_manager_destroy(network_manager_t *mgr) {
  if (mgr == NULL)
    return ESP_ERR_INVALID_ARG;

  if (mgr->mqtt_client) {
    esp_mqtt_client_stop(mgr->mqtt_client);
    esp_mqtt_client_destroy(mgr->mqtt_client);
  }

  free(mgr);
  mgr = NULL;
  ESP_LOGI(TAG, "Network Manager destroyed and memory freed.");

  return ESP_OK;
}
