#include "wifi_service.hpp"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <cstring>
#include <esp_log.h>
#include <ostream>
#include <iostream>
#include "driver/gpio.h"

constexpr gpio_num_t LED_GPIO = GPIO_NUM_2; // Pino do LED de Debug

/**
 * @brief Inicia a configuração do Wifi, conectando o dispositivo a uma rede wifi utilizando o SSID e senha fornecidos
 * @param wifi_ssid SSID da rede wifi a ser conectada
 * @param wifi_pass Senha da rede wifi a ser conectada
 */
void wifi_start(const char* wifi_ssid, const char* wifi_pass)
{
    ESP_LOGI("WIFI", "Inicializando WiFi...");
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    // Inicializa o NVS
   ESP_ERROR_CHECK(nvs_flash_init());

    // Inicia o ESP Network Interface
    ESP_ERROR_CHECK(esp_netif_init());

    // Cria o loop de eventos padrão para gerar log dos estados do Wifi, necessário implementar posteriormente!
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Registra o Handler do Wifi para monitorar o status posteriormente
    esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        nullptr,
        nullptr
    );

    // Registra o Handler do Wifi para monitorar o status posteriormente
    esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &wifi_event_handler,
        nullptr,
        nullptr
    );

    // Cria ‘interface’ de rede padrão para conexão STA
    esp_netif_create_default_wifi_sta();

    // Inicialização padrão da estrutura de configuração do Wi-Fi
    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // Inicia o Driver

    // Estrutura de dados para configuração da rede
    wifi_config_t wifi_config = {};
    strcpy(reinterpret_cast<char*>(wifi_config.sta.ssid), wifi_ssid);         // Define o SSID de conexão
    strcpy(reinterpret_cast<char*>(wifi_config.sta.password), wifi_pass);     // Define a senha da conexão
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));                                         // Define o driver para atuar como cliente
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));                           // Aplica as configurações no driver de rede

    // Inicia o Driver de Rede
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    // Inicia a conexão com o Wifi
    ESP_LOGI("WIFI", "Tentando conectar...");
    esp_err_t err = esp_wifi_connect();
    if (err != ESP_OK)
    {
        ESP_LOGE("WIFI","Falha ao iniciar conexao: %s",esp_err_to_name(err));
    }
}

/**
 * @brief Handler de eventos do Wifi, responsável por monitorar o status da conexão e reagir a eventos de desconexão ou conexão
 * @param arg Argumento genérico (não utilizado)
 * @param event_base Base do evento (WIFI_EVENT ou IP_EVENT)
 * @param event_id ID do evento específico (WIFI_EVENT_STA_DISCONNECTED ou IP_EVENT_STA_GOT_IP)
 * @param event_data Dados adicionais do evento (informações sobre a desconexão ou o IP obtido)
 */
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        auto* event = static_cast<wifi_event_sta_disconnected_t*>(event_data);

        switch (event->reason)
        {
            case WIFI_REASON_BEACON_TIMEOUT:
                ESP_LOGW("WIFI","Beacon Timeout");
                break;
            case WIFI_REASON_NO_AP_FOUND:
                ESP_LOGW("WIFI","AP não encontrado");
                break;
            case WIFI_REASON_AUTH_EXPIRE:
                ESP_LOGW("WIFI","Autenticação Expirou");
                break;
            case WIFI_REASON_AUTH_FAIL:
                ESP_LOGW("WIFI","Autenticação invalida para se conectar ao Wifi");
                gpio_set_level(LED_GPIO, 1);
                break;
            case WIFI_REASON_ASSOC_TOOMANY:
                ESP_LOGW("WIFI","Muitos dispositivos conectados");
                break;
            default:
                ESP_LOGW("WIFI","Erro desconhecido: motivo=%d", event->reason);
                gpio_set_level(LED_GPIO, 1);
        }
        esp_wifi_connect(); // Tenta reconectar
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        gpio_set_level(LED_GPIO, 0); // Desliga a luz de debug
        ESP_LOGI("WIFI","Conectado com sucesso!");
    }
}
