#pragma once

#include <esp_event_base.h>
#include <string>

/**
 * @brief Inicia a configuração do Wifi, conectando o dispositivo a uma rede wifi utilizando o SSID e senha fornecidos
 * @param wifi_ssid SSID da rede wifi a ser conectada
 * @param wifi_pass Senha da rede wifi a ser conectada
 */
void wifi_start(const char* wifi_ssid, const char* wifi_pass);

/**
 * @brief Handler de eventos do Wifi, responsável por monitorar o status da conexão e reagir a eventos de desconexão ou conexão
 * @param arg Argumento genérico (não utilizado)
 * @param event_base Base do evento (WIFI_EVENT ou IP_EVENT)
 * @param event_id ID do evento específico (WIFI_EVENT_STA_DISCONNECTED ou IP_EVENT_STA_GOT_IP)
 * @param event_data Dados adicionais do evento (informações sobre a desconexão ou o IP obtido)
 */
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);