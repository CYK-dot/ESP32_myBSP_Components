idf_component_register(
    SRCS "esp32center_joystick.cpp"    
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash esp_wifi esp_driver_ledc espressif__servo esp_driver_uart
    PRIV_REQUIRES rdlc my_espnow my_wifi)
