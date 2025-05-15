idf_component_register(
    SRCS "recorder.cpp"
    INCLUDE_DIRS "include"
    REQUIRES rdlc my_wifi lwip esp_wifi esp_driver_uart
)