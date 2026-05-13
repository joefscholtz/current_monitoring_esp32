#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <stdbool.h>

typedef enum {
  NET_INTERFACE_WIFI,
  NET_INTERFACE_ETH,
  NET_INTERFACE_BOTH
} net_interface_config_t;

esp_err_t wifi_handler_init(void);
esp_err_t eth_handler_init(void);

void on_got_ip(void *arg, esp_event_base_t base, int32_t id, void *data);

esp_err_t network_manager_init(net_interface_config_t config);

bool network_is_connected(void);

esp_err_t network_mqtt_init(const char *broker_url);

esp_err_t network_mqtt_publish(const char *topic, const char *data);

#endif
