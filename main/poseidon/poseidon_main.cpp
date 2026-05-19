#include "poseidon_main.hpp"
#include <iostream>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "network/tcp_service.hpp"

/**
 * Função para iniciar a tarefa do Poseidon
 */
void poseidon_main(){
    /// Conexão com o Servidor
    /*while (true)
    {
        if (tcp_service_init("192.168.0.133", 9000))
        {
            break;
        }

        ESP_LOGI("[POSEIDON]","Falha ao conectar com o Servidor Aether, tentando novamente...");
        vTaskDelay(pdMS_TO_TICKS(30000));
    }*/

    tcp_register_handler(my_tcp_handler);

    tcp_service_start("192.168.0.133", 9000);

    tcp_service_init("192.168.0.133", 9000); // TODO: Refatorar
}