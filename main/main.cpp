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

    /// Conexão com o Wifi
    while (true)
    {
        ESP_LOGI("[INFO]","Iniciando o processo de conexão com o Wifi...");
        if (wifi_start("Matheus","favero10"))
        {
            vTaskDelay(pdMS_TO_TICKS(20000)); // Aguarda 5 segundos antes de tentar novamente
            ESP_LOGI("[INFO]", "Conexão com o Wifi realizada com sucesso");
            break;
        }

        ESP_LOGI("[ERROR]", "Falha ao conectar com o Wifi, tentando novamente...");
        vTaskDelay(pdMS_TO_TICKS(10000)); // Aguarda 5 segundos antes de tentar novamente
    }

    // ==================================
    //     Inicialização do Poseidon
    // ==================================

    poseidon_main();
}