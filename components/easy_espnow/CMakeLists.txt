idf_component_register(SRCS "easy_espnow_remote.c"
                    INCLUDE_DIRS "include"
                    REQUIRES easy_wifi easy_nvs
                    PRIV_REQUIRES esp_wifi nvs_flash)
