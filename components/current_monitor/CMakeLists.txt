set(srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/current_monitor.c")

idf_component_register(
    SRCS 
      "${srcs}"
    INCLUDE_DIRS 
      "include"
    REQUIRES 
      esp_timer
      esp_hw_support
      esp_adc
      esp_hal_ana_conv
)

target_compile_features(${COMPONENT_LIB} PRIVATE)
