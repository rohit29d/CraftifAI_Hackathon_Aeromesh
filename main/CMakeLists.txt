# Auto-generated ESP-IDF build glue.
# This `main` component is a thin shim that compiles the vendor-agnostic
# `firmware/` tree. Edit firmware/, not this file.
set(FW_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../firmware")
if(NOT IS_DIRECTORY "${FW_ROOT}")
    message(FATAL_ERROR "Firmgen: firmware root '${FW_ROOT}' not found — the firmware/ tree is missing or main/CMakeLists.txt was edited. Recreate the project or restore firmware/.")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/firmgen_sources.cmake")

idf_component_register(
    SRCS "entry.c" ${APP_SOURCES}
    INCLUDE_DIRS "." ${FW_INCLUDE_DIRS}
    REQUIRES esp_driver_gpio esp_driver_i2c esp_timer esp_wifi esp_event esp_netif esp_http_server esp_http_client json led_strip edge_impulse nvs_flash freertos log)
