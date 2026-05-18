#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include <iostream>
#include "core/network/wifi_service.hpp"
#include "poseidon/poseidon_main.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/temperature_sensor.h"

extern "C" void app_main()
{

    // ==================================
    //     Conexão com o Wifi (Core)
    // ==================================

    wifi_start("Matheus","favero10");

    // ==================================
    //     Inicialização do Poseidon
    // ==================================

    poseidon_main();
}