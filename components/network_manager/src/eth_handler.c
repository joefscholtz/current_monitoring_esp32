#ifdef MQTT_ETH_ENABLED
#include "esp_eth_enc28j60.h"
#endif
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"

static const char *TAG = "ETH_H";

esp_err_t eth_handler_init(void) {
#ifndef MQTT_ETH_ENABLED
  ESP_LOGW(TAG, "Ethernet is disabled in build flags.");
  return ESP_OK;
#else
  ESP_LOGI(TAG, "Initializing Ethernet (ENC28J60)...");

  // SPI Bus Configuration (Standard VSPI pins)
  spi_bus_config_t buscfg = {
      .miso_io_num = 19,
      .mosi_io_num = 23,
      .sclk_io_num = 18,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
  };
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

  // ENC28J60 Config (CS on GPIO 5, INT on GPIO 4)
  eth_enc28j60_config_t enc28j60_config =
      ETH_ENC28J60_DEFAULT_CONFIG(SPI2_HOST, 5);
  eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
  esp_eth_mac_t *mac = esp_eth_mac_new_enc28j60(&enc28j60_config, &mac_config);

  eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
  esp_eth_phy_t *phy = esp_eth_phy_new_enc28j60(&phy_config);

  esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
  esp_eth_handle_t eth_handle = NULL;
  ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));

  // Set Base MAC for Ethernet as well
  uint8_t mac_addr[6];
  esp_read_mac(mac_addr, ESP_MAC_WIFI_STA);
  ESP_ERROR_CHECK(esp_eth_update_input_path(eth_handle, NULL, NULL));
  mac->set_addr(mac, mac_addr);

  esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
  esp_netif_t *eth_netif = esp_netif_new(&cfg);
  ESP_ERROR_CHECK(esp_eth_netif_glue_port_attach(
      esp_eth_new_netif_glue(eth_handle), eth_netif));

  return esp_eth_start(eth_handle);
#endif
}
