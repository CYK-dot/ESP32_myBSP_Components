idf_component_register(SRCS "my_espnow_remote.c" "my_espnow.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash esp_wifi
                    PRIV_REQUIRES my_wifi)
