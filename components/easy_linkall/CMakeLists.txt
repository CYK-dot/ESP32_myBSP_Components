idf_component_register(
    SRCS "elink_now.c"
    INCLUDE_DIRS "include"
    REQUIRES easy_wifi easy_nvs easy_mavlink
    PRIV_REQUIRES esp_wifi nvs_flash
)
