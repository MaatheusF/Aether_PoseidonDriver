#include "relay_change.hpp"
#include <cstdint>
#include <string>

#include "esp_log.h"
#include "../../../external/json.hpp"
#include "driver/gpio.h"

static auto TAG = "[Relay]";                        // Tag para logs relacionados aos Relays

// ========================
//   Definições de pinos
// ========================

#define RELAY_LIGHT_AQUARIO GPIO_NUM_20                  // Definição do pino G20 para uso do Relay que controle o estado da Luz do Aquario
#define RELAY_LIGHT_PLANTAS GPIO_NUM_21                  // Definição do pino G21 para uso do Relay que controle o estado da Luz das plantas

/**
 * @brief Inicializa os pinos GPIO utilizados para controlar os relays.
 * Configura os pinos definidos para os relays como saídas e define o estado inicial como "Desligado" (nível lógico alto).
 */
void relay_init()
{
    gpio_set_direction(RELAY_LIGHT_AQUARIO, GPIO_MODE_OUTPUT); // Define o pino do rele como Output
    gpio_set_direction(RELAY_LIGHT_PLANTAS, GPIO_MODE_OUTPUT); // Define o pino do rele como Output

    gpio_set_level(RELAY_LIGHT_AQUARIO, 1); // Seta o rele para o estado padrão "Desligado"
    gpio_set_level(RELAY_LIGHT_PLANTAS, 1); // Seta o rele para o estado padrão "Desligado"
}

/**
 *
 * @param data Payload da requisição recebida via TCP
 * @param len Tamanho do Payload
 *
 *  {
 *    "relay":
 *     {
 *       "relay_external_id":"LIGHT1",
 *        "relay_value":"on"
 *     }
 *  }
 */
void relay_change_state(const nlohmann::json& j){

    // Valida o preenchimento das Tags
    if (!j.contains("relay")) {
        return;
    }

    // Verifica se a TAG "relay_external_id" existe no Json
    if (!j["relay"].contains("relay_external_id") || !j["relay"]["relay_external_id"].is_string()) {
        return;
    }

    // Verifica se a TAG "relay_value" existe no Json
    if (!j["relay"].contains("relay_value") || !j["relay"]["relay_value"].is_string()) {
        return;
    }

    if (!relay_gpio_alter_state(j["relay"]["relay_external_id"], j["relay"]["relay_value"]))
    {
        ESP_LOGW(TAG, "Ocorreu um erro ao mudar o status do rele %s", j["relay"]["relay_external_id"]);
    }
}

/**
 * Altera o estado do rele de acordo com o device_id e o rele_state recebidos
 * @param device_id String contendo o nome do dispositivo a ser encontrado
 * @param rele_state String contendo o estado desejado para o rele ("on" ou "off")
 * @return true se o estado do rele foi alterado com sucesso, false caso contrário
 */
bool relay_gpio_alter_state(std::string device_id, std::string rele_state)
{
    if (rele_state == "on" || rele_state == "ON")
    {
        if (device_id == "LIGHT1")
        {
            gpio_set_level(RELAY_LIGHT_AQUARIO, 0); // Liga
            ESP_LOGI(TAG, "Luz do Aquario ligada!");
            return true;
        }

        if (device_id == "LIGHT2")
        {
            gpio_set_level(RELAY_LIGHT_PLANTAS, 0); // Liga
            ESP_LOGI(TAG, "Luz das plantas ligada!");
            return true;
        }
    }

    if (rele_state == "off" || rele_state == "OFF")
    {
        if (device_id == "LIGHT1")
        {
            gpio_set_level(RELAY_LIGHT_AQUARIO, 1); // Desliga
            ESP_LOGI(TAG, "Luz do Aquario desligada!");
            return true;
        }

        if (device_id == "LIGHT2")
        {
            gpio_set_level(RELAY_LIGHT_PLANTAS, 1); // Desliga
            ESP_LOGI(TAG, "Luz das plantas desligada!");
            return true;
        }
    }

    return false;
}
