set(srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/network_manager.c ${CMAKE_CURRENT_SOURCE_DIR}/src/wifi_handler.c ${CMAKE_CURRENT_SOURCE_DIR}/src/eth_handler.c")

idf_component_register(
    SRCS
      "${srcs}"
    INCLUDE_DIRS
      "include"
    REQUIRES
      esp_wifi
      esp_eth
      esp_netif
      esp_event
      nvs_flash
)

target_compile_features(${COMPONENT_LIB} PRIVATE)
