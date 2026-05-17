#include "wifi_service.hpp"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <cstring>
#include <ostream>
#include <iostream>
#include "driver/gpio.h"

/**
 * @brief Inicia a configuração do Wifi, conectando o dispositivo a uma rede wifi utilizando o SSID e senha fornecidos
 * @param wifi_ssid SSID da rede wifi a ser conectada
 * @param wifi_pass Senha da rede wifi a ser conectada
 * @return true se a conexão for bem sucedida, false caso contrário
 */
bool wifi_start(const char* wifi_ssid, const char* wifi_pass)
{
    // Controle da Luz de Debug
    const gpio_num_t LED_GPIO = GPIO_NUM_2;
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);

    // Inicializa o NVS
    nvs_flash_init();

    // Inicia o ESP Network Interface
    if (esp_netif_init() == ESP_FAIL){
        std::cout << "[Error] Falha ao iniciar o ESP Network Interface!" << std::endl;
        return false;
    }

    // Cria o loop de eventos padrão para gerar log dos estados do Wifi, necessário implementar posteriormente!
    esp_event_loop_create_default();

    // Cria ‘interface’ de rede padrão para conexão STA
    esp_netif_create_default_wifi_sta();

    // Inicialização padrão da estrutura de configuração do Wi-Fi
    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg); // Inicia o Driver

    // Estrutura de dados para configuração da rede
    wifi_config_t wifi_config = {};
    strcpy(reinterpret_cast<char*>(wifi_config.sta.ssid), wifi_ssid);         // Define o SSID de conexão
    strcpy(reinterpret_cast<char*>(wifi_config.sta.password), wifi_pass);     // Define a senha da conexão
    esp_wifi_set_mode(WIFI_MODE_STA);                                         // Define o driver para atuar como cliente
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);                           // Aplica as configurações no driver de rede

    // Inicia o Driver de Rede
    switch(esp_wifi_start()){
        case ESP_OK:
            std::cout << "[Info] Driver Wifi iniciado com sucesso!" << std::endl;
            // Habilita modo de economia de energia do WiFi para reduzir consumo e aquecimento
            // (minimiza atividade do modem quando possível)
            esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
            break;
        case ESP_ERR_WIFI_NOT_INIT:
            std::cout << "[Erro] O driver de rede não foi inicializado pelo esp_wifi_init!" << std::endl;
            gpio_set_level(LED_GPIO, 1);    // Liga a luz de debug indicando erro no processo
            break;
        case ESP_ERR_INVALID_ARG:
            std::cout << "[Erro] Ocorreu um erro inesperado, verifique as configurações do Wifi!" << std::endl;
            gpio_set_level(LED_GPIO, 1);    // Liga a luz de debug indicando erro no processo
            break;
        case ESP_ERR_NO_MEM:
            std::cout << "[Erro] Não á memoria suficiente para realizar a conexão com o Wifi!" << std::endl;
            gpio_set_level(LED_GPIO, 1);    // Liga a luz de debug indicando erro no processo
            break;
        case ESP_ERR_WIFI_CONN:
            std::cout << "[Erro] Erro interno do WiFi, driver ou bloco de controle!" << std::endl;
            gpio_set_level(LED_GPIO, 1);    // Liga a luz de debug indicando erro no processo
            break;
        default:
            std::cout << "[Erro] Erro desconhecido!" << std::endl;
            gpio_set_level(LED_GPIO, 1);    // Liga a luz de debug indicando erro no processo
            break;
    }

    // Inicia a conexão com o Wifi
    if (esp_wifi_connect() == ESP_OK) {
        gpio_set_level(LED_GPIO, 0);
        return true;
    }

    return false;
}
