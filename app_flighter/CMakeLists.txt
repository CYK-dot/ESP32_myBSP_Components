idf_component_register(
    SRCS "slave_motor.cpp" "host_keyboard.cpp"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash esp_wifi esp_driver_ledc espressif__servo my_espnow my_wifi
    PRIV_REQUIRES my_adc rdlc)
