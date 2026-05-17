idf_component_register(
        SRCS
        "main.cpp"
        "core/network/wifi_service.cpp"
        "poseidon/poseidon_main.cpp"
        "poseidon/network/tcp_service.cpp"

        INCLUDE_DIRS
        "."

        REQUIRES
        esp_netif
        esp_event
        esp_wifi
        nvs_flash
        lwip
        driver
        freertos
        esp_ssd1306
        esp_type_utils
        ds18b20
        dht
        esp_driver_tsens
)