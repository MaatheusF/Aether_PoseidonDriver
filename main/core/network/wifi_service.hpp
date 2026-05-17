#pragma once

#include <string>

/**
 * @brief Inicia a configuração do Wifi, conectando o dispositivo a uma rede wifi utilizando o SSID e senha fornecidos
 * @param wifi_ssid SSID da rede wifi a ser conectada
 * @param wifi_pass Senha da rede wifi a ser conectada
 * @return true se a conexão for bem sucedida, false caso contrário
 */
bool wifi_start(const char* wifi_ssid, const char* wifi_pass);
