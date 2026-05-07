file(GLOB_RECURSE app_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

idf_component_register(
    SRCS 
        "${app_sources}"
    INCLUDE_DIRS 
        "include"
    REQUIRES 
        esp_adc
        driver
)

# enforce C++20/23 specifically for this component if needed
# target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_20)
target_compile_features(${COMPONENT_LIB} PRIVATE)
