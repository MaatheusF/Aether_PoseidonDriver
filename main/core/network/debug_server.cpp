#include "debug_server.hpp"
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static int debug_client_socket = -1;        // Socket do cliente TCP conectado para debug
static auto TAG = "[DEBUG_SERVER]";         // Tag para logs do servidor de debug

/**
 * Função de log que redireciona logs para um cliente TCP se conectado.
 */
int custom_log_vprintf(const char* fmt, va_list args)
{
    char buffer[512];

    const int len = vsnprintf(buffer, sizeof(buffer), fmt, args);

    // Mantém saída normal UART
    printf("%s", buffer);

    // Envia para cliente TCP conectado
    if (debug_client_socket >= 0)
    {
        int err = send(debug_client_socket, buffer, len, 0);

        if (err < 0)
        {
            ESP_LOGW(TAG, "Cliente debug desconectou!");
            close(debug_client_socket);
            debug_client_socket = -1;
        }
    }

    return len;
}

/**
 * Thread do servidor de debug, que aceita conexões TCP e redireciona logs.
 */
void debug_server_task(void* args)
{
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (server_socket < 0)
    {
        ESP_LOGE(TAG, "Falta ao cirar socket");
        vTaskDelete(nullptr);
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(7777);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
    {
        ESP_LOGE(TAG, "Falha no Bind");
        close(server_socket);
        vTaskDelete(nullptr);
        return;
    }

    if (listen(server_socket, 1) < 0)
    {
        ESP_LOGE(TAG, "Falha no Listen");
        close(server_socket);
        vTaskDelete(nullptr);
        return;
    }

    ESP_LOGI(TAG, "Servidor Debug iniciado na porta 7777");

    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_socket, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        if (client_socket >= 0)
        {
            ESP_LOGI(TAG, "Cliente debug conectado!");

            // Fecha cliente anterior
            if (debug_client_socket >= 0)
            {
                close(debug_client_socket);
            }

            debug_client_socket = client_socket;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * Inicia o servidor de debug, que redireciona logs para clientes TCP conectados.
 */
void debug_server_start()
{
    // Hook global dos logs
    esp_log_set_vprintf(custom_log_vprintf);

    // Cria thread do servidor
    xTaskCreate(debug_server_task, "debug_server_task", 4096, nullptr, 5, nullptr);
}