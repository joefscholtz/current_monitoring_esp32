#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "esp_err.h"
#include <stdbool.h>

typedef enum {
  NET_INTERFACE_WIFI,
  NET_INTERFACE_ETH,
  NET_INTERFACE_BOTH
} net_interface_config_t;

esp_err_t network_manager_init(net_interface_config_t config);

bool network_is_connected(void);

#endif
