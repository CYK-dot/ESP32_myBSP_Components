idf_component_register(
    SRCS "esp32c3flighter.cpp" 
         
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash esp_wifi esp_driver_ledc espressif__servo
    PRIV_REQUIRES my_espnow my_wifi)
