idf_component_register(
    SRCS "easy_wifi_wifi.c" "easy_wifi_command.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi nvs_flash
)
