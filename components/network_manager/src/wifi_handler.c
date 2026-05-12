#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "ETH_H";

esp_err_t eth_handler_init(void) {
  // This is where you'd init SPI for ENC28J60/W5500
  // and create the esp_netif_t for Ethernet.
  ESP_LOGW(TAG, "Ethernet requested but currently disabled in code.");
  return ESP_OK;
}
