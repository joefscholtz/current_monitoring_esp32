if(CMAKE_C_FLAGS MATCHES "CHOSEN_APP=bare_metal")
    set(APP_SUBDIR "bare_metal")
else()
    set(APP_SUBDIR "rtos")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/${APP_SUBDIR}/app_definition.cmake")

idf_component_register(
    SRCS "${APP_SRCS}"
    INCLUDE_DIRS "${APP_INCLUDES}"
    REQUIRES current_monitor network_manager
)
