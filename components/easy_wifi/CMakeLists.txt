idf_component_register(
    SRCS "ewifi_basic.c" "ewifi_advance.c" "ewifi_if_cli.c" "ewifi_if_web.c"
    INCLUDE_DIRS "include"
    REQUIRES easy_nvs
    PRIV_REQUIRES esp_wifi nvs_flash
)
