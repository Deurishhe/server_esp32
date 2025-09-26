FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.*)

idf_component_register(SRCS ${app_sources}
    INCLUDE_DIRS "."
    REQUIRES
        bt
        esp_event
        esp_wifi
        esp_netif
        esp_system
        freertos
        lwip
        nvs_flash
)
