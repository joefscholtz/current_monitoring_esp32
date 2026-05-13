#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "esp_netif_types.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  NET_INTERFACE_WIFI,
  NET_INTERFACE_ETH,
  NET_INTERFACE_BOTH
} net_interface_config_t;

typedef struct {
  net_interface_config_t config;
  esp_mqtt_client_handle_t mqtt_client;
  bool is_connected;
  char broker_url[128];
  char mqtt_client_id[32];
} network_manager_t;

void on_got_ip(void *arg, esp_event_base_t base, int32_t id, void *data);

network_manager_t *network_manager_create(net_interface_config_t config,
                                          const char *broker_url);
esp_err_t network_manager_destroy(network_manager_t *mgr);

esp_err_t network_manager_init(network_manager_t *mgr);

esp_err_t wifi_handler_init(network_manager_t *mgr);
esp_err_t eth_handler_init(network_manager_t *mgr);

bool network_is_connected(network_manager_t *mgr);

esp_err_t network_mqtt_init(network_manager_t *mgr);
esp_err_t network_mqtt_publish(network_manager_t *mgr, const char *topic,
                               const char *data);

#endif
