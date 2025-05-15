idf_component_register(
    SRCS "wifi_basic.c" "wifi_http.c" "wifi_nvs.c" "wifi_advance.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi nvs_flash esp_http_server json
    EMBED_TXTFILES "./data/wifi_config.html" "./data/wifi_config.txt"
)
