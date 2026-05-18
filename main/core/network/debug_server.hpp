#pragma once

#include "debug_server.hpp"
#include <esp_log.h>

/**
 * Inicia o servidor de debug, que redireciona logs para clientes TCP conectados.
 */
void debug_server_start();

/**
 * Thread do servidor de debug, que aceita conexões TCP e redireciona logs.
 */
void debug_server_task(void* args);

/**
 * Função de log que redireciona logs para um cliente TCP se conectado.
 */
int custom_log_vprintf(const char* fmt, va_list args);