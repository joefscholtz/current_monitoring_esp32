set(srcs "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "include"
    REQUIRES current_monitor
)
