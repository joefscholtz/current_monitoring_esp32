file(GLOB_RECURSE app_sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

idf_component_register(
    SRCS "${app_sources}"
    INCLUDE_DIRS "include"
