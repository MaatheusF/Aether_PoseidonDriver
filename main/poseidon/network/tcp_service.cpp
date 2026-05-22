#include "tcp_service.hpp"
#include <string>
#include <vector>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <lwip/sockets.h>
#include <ctime>
#include "esp_sntp.h"
#include "../components/dht/dht.h"
#include "../components/ds18b20/ds18b20.h"
#include "driver/temperature_sensor.h"
#include "sdkconfig.h"
#include "../../../external/json.hpp"
#include "freertos/semphr.h"
#include "../service/relay_change.hpp"

// Refactor init

static auto TAG = "[TCP]";                              // Tag para logs relacionados ao TCP
static int tcp_socket = -1;                             // Socket TCP global para comunicação com o servidor Aether
static bool connected = false;                          // Estado de conexão TCP
static tcp_event_handler_t event_handler = nullptr;     // Ponteiro para a função de callback de eventos TCP (conexão, desconexão, dados recebidos, erros)
static char server_host[64];                            // Endereço IP ou hostname do servidor Aether
static uint16_t server_port;                            // Porta do servidor Aether

std::string deviceId = "IDESP32";                       // ID do dispositivo, utilizado no Handshake inicial com o servidor Aether

static volatile bool send_packet_waiting_ack = false;   // Indica se o sistema está aguardando um ACK do servidor após enviar um pacote.
static volatile bool send_packet_ack_received = false;  // Indica se o ACK do servidor foi recebido para o pacote enviado.
static SemaphoreHandle_t tcp_mutex = nullptr;           // Mutex para controle de acesso à conexão TCP,

static constexpr uint16_t CMD_ACK = 0x0007;             // Código de comando para ACK, utilizado para confirmar o recebimento de pacotes enviados ao servidor Aether
static constexpr uint16_t HELLO_ACK = 0x0005;           // Codigo de comando para ACK do Handshake inicial.
static constexpr uint16_t AETHER_MAGIC = 0xAA55;        // Codigo do Magic do protocolo Aether.
static constexpr uint8_t AETHER_VERSION = 0x01;         // Versão do protocolo Aether, utilizada para validar a compatibilidade entre cliente e servidor durante o Handshake inicial.
static constexpr uint16_t CMD_DATA_PUSH = 0x0102;       // Codigo do comando para enviar um dado para o Servidor sem solicitação
static constexpr uint16_t CMD_HELLO = 0x0004;           // Código do comando para iniciar o handshake iniciar da conexão
static constexpr uint16_t MODULE_ESP32  = 0x03;         // Codigo do modulo Poseidon

/**
 * @brief Inicializa a conexão TCP com o servidor Aether e inicia a task de gerenciamento da conexão.
 * @param host Endereço IP ou hostname do servidor Aether
 * @param port Porta do servidor Aether
 *
 * Esta função configura os parâmetros de conexão, cria um mutex para controle de acesso à conexão TCP e inicia a task responsável por gerenciar a conexão, incluindo reconexão automática, envio e recebimento de dados, e emissão de eventos para a função de callback registrada.
 */
void tcp_service_start(const char* host, uint16_t port)
{
    ESP_LOGI(TAG, "Inicializando conexão com o Servidor Aether...");
    strncpy(server_host, host, sizeof(server_host));
    server_port = port;
    tcp_mutex = xSemaphoreCreateMutex(); // Instancia o Mutex
    xTaskCreate(tcp_task, "tcp_task", 6144, nullptr, 5, nullptr);
}

/**
 * @brief Emite um evento TCP para a função de callback registrada, se houver.
 * @param event Tipo do evento TCP (conexão, desconexão, dados recebidos, erro)
 * @param data Ponteiro para os dados recebidos (válido apenas para eventos de dados recebidos)
 * @param len Tamanho dos dados recebidos (válido apenas para eventos de dados recebidos)
 */
static void emit_event(tcp_event_t event, const uint8_t* data = nullptr, size_t len = 0)
{
    if (event_handler)
    {
        event_handler(event, data, len);
    }
}

/**
 * @brief Registra uma função de callback para eventos TCP.
 * @param handler Ponteiro para a função que será chamada quando ocorrerem eventos TCP (conexão, desconexão, dados recebidos, erro)
 */
void tcp_register_handler(tcp_event_handler_t handler)
{
    event_handler = handler;
}

/**
 * @brief Verifica se a conexão TCP com o servidor Aether está ativa.
 * @return true se conectado, false caso contrário
 */
bool tcp_is_connected()
{
    return connected;
}

/**
 * @brief Envia dados para o servidor Aether via TCP e valida o ACK de retorno.
 * @param data Dados a serem enviados
 * @param len Tamanho dos dados a serem enviados
 * @return true se o envio foi bem-sucedido, false caso contrário
 */
bool tcp_send(const uint8_t* data, size_t len)
{
    xSemaphoreTake(tcp_mutex, portMAX_DELAY); // Controle do Mutex para não concorrer a função

    // Valida se o TCP esta conectado
    if (!connected)
    {
        xSemaphoreGive(tcp_mutex);
        return false;
    }

    // Prepara as variaveis para receber o ACK, precisa ser antes do envio para evitar competição
    send_packet_ack_received = false;
    send_packet_waiting_ack = true;

    // Total enviado
    size_t total_sent = 0;

    // Realiza o envio dos dados
    while (total_sent < len)
    {
        int sent = send(
            tcp_socket,
            data + total_sent,
            len - total_sent,
            0
        );

        if (sent <= 0)
        {
            ESP_LOGE(TAG, "Erro envio errno=%d (%s)", errno, strerror(errno));
            connected = false;
            emit_event(TCP_EVENT_DISCONNECTED);
            close(tcp_socket);
            tcp_socket = -1;
            xSemaphoreGive(tcp_mutex);
            return false;
        }

        total_sent += sent;
    }

    // =========================
    // Aguarda ACK do servidor
    // =========================

    const int timeout_ms = 10000;    // Tempo de time out do ACK
    int time_elapsed = 0;

    // Aguarda o retorno do ACK durante o tempo de time out
    while (!send_packet_ack_received && time_elapsed < timeout_ms)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        time_elapsed += 100;
    }

    send_packet_waiting_ack = false;

    // Retorna false caso não receber o ACK no tempo previsto
    if (!send_packet_ack_received)
    {
        xSemaphoreGive(tcp_mutex);
        return false;
    }

    xSemaphoreGive(tcp_mutex); // Controle do Mutex para não concorrer a função

    return true;
}

/**
 * @brief Task principal para gerenciar a conexão TCP com o servidor Aether, incluindo reconexão automática, recebimento de dados e emissão de eventos.
 * @param arg Argumento de entrada (não utilizado)
 */
static void tcp_task(void* arg)
{
    static std::vector<uint8_t> tcp_rx_buffer;

    while (true)
    {
        if (!connected)
        {
            ESP_LOGI(TAG, "Conectando...");

            tcp_socket = socket(
                AF_INET,
                SOCK_STREAM,
                IPPROTO_IP
            );

            if (tcp_socket < 0)
            {
                ESP_LOGE(TAG, "Erro socket");
                vTaskDelay(pdMS_TO_TICKS(5000));
                continue;
            }

            sockaddr_in dest_addr{};

            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons(server_port);

            inet_pton(
                AF_INET,
                server_host,
                &dest_addr.sin_addr
            );

            int err = connect(
                tcp_socket,
                reinterpret_cast<sockaddr*>(&dest_addr),
                sizeof(dest_addr)
            );

            if (err != 0)
            {
                ESP_LOGW(TAG, "Falha conexao");
                close(tcp_socket);
                tcp_socket = -1;
                vTaskDelay(pdMS_TO_TICKS(5000));
                continue;
            }

            connected = true;

            // Realiza o Handshake inicial no Aether Server
            if (!sendHandshake())
            {
                ESP_LOGE(TAG, "Falha no handshake");
                close(tcp_socket);
                tcp_socket = -1;
                connected = false;

                vTaskDelay(pdMS_TO_TICKS(5000));
                continue;
            }

            emit_event(TCP_EVENT_CONNECTED);
        }

        uint8_t temp_buffer[512];

        int len = recv(
            tcp_socket,
            temp_buffer,
            sizeof(temp_buffer),
            MSG_DONTWAIT
        );

        if (len > 0)
        {
            tcp_rx_buffer.insert(
                tcp_rx_buffer.end(),
                temp_buffer,
                temp_buffer + len
            );

            while (true)
            {
                // Precisa ter header completo
                if (tcp_rx_buffer.size() < 11)
                {
                    break;
                }

                uint16_t magic = read16(&tcp_rx_buffer[0]);

                // Sincroniza no header
                if (magic != AETHER_MAGIC)
                {
                    tcp_rx_buffer.erase(tcp_rx_buffer.begin());
                    continue;
                }

                uint16_t type = read16(&tcp_rx_buffer[3]);
                uint32_t payload_size = read32(&tcp_rx_buffer[7]);

                size_t packet_size = 11 + payload_size;

                // Pacote ainda incompleto
                if (tcp_rx_buffer.size() < packet_size)
                {
                    break;
                }

                ESP_LOGI(TAG, "Pacote completo recebido (%u bytes)", packet_size);

                switch (type)
                {
                    case CMD_ACK:
                    case HELLO_ACK:
                    {
                        if (send_packet_waiting_ack)
                        {
                            send_packet_ack_received = true;
                        }
                        break;
                    }

                    default:
                        {
                            emit_event(
                                TCP_EVENT_DATA_RECEIVED,
                                tcp_rx_buffer.data(),
                                packet_size
                            );
                            break;
                        }
                }

                // Remove pacote processado
                tcp_rx_buffer.erase(
                    tcp_rx_buffer.begin(),
                    tcp_rx_buffer.begin() + packet_size
                );
            }
        }
        else if (len == 0)
        {
            ESP_LOGW(TAG, "Servidor desconectou");
            connected = false;
            emit_event(TCP_EVENT_DISCONNECTED);
            close(tcp_socket);
            tcp_socket = -1;
        }
        else
        {
            if (errno != EWOULDBLOCK && errno != EAGAIN)
            {
                ESP_LOGE(TAG, "Erro recv errno=%d (%s)", errno, strerror(errno));
                connected = false;
                emit_event(TCP_EVENT_DISCONNECTED);
                close(tcp_socket);
                tcp_socket = -1;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * Função de callback para eventos TCP, que pode ser registrada usando tcp_register_handler.
 * Esta função é chamada quando ocorrem eventos como conexão, desconexão ou recebimento de dados, e pode ser personalizada para processar os dados recebidos conforme necessário.
 */
void my_tcp_handler(tcp_event_t event,const uint8_t* data,size_t len)
{
    switch(event)
    {
        case TCP_EVENT_CONNECTED:
        {
            ESP_LOGI(TAG, "TCP conectado");
            break;
        }

        case TCP_EVENT_DISCONNECTED:
        {
            ESP_LOGW(TAG, "TCP desconectado");
            break;
        }

        case TCP_EVENT_DATA_RECEIVED:
        {
            if (len < 11)
            {
                ESP_LOGW(TAG, "Pacote invalido");
                break;
            }

            uint32_t payload_size = read32(&data[7]);

            if ((11 + payload_size) > len)
            {
                ESP_LOGW(TAG, "Payload incompleto");
                break;
            }

            std::string json_payload(
                reinterpret_cast<const char*>(&data[11]),
                payload_size
            );

            ESP_LOGI(TAG, "Payload JSON: %s", json_payload.c_str());

            // Desserializa o JSON
            using json = nlohmann::json;
            json j = json::parse(json_payload);

            //Identifica o tipo do DATA PUSH
            if (j.contains("relay"))
            {
                relay_change_state(j);
                break;
            }

            ESP_LOGI(TAG, "Recebido DATA_PUSH sem roteamento!");

            break;
        }

        default:
        {
            break;
        }

    }
}

/*
 * Funções auxiliares para construção e leitura de pacotes no formato do protocolo Aether
*/
static void push16(std::vector<uint8_t>& buffer, uint16_t value)
{
    buffer.push_back((value >> 8) & 0xFF);
    buffer.push_back(value & 0xFF);
}

/**
 * @brief Adiciona um valor de 32 bits a um buffer de bytes em formato big-endian, utilizado para construir pacotes no formato do protocolo Aether.
 * @param buffer Vetor de bytes onde o valor será adicionado
 * @param value Valor de 32 bits a ser adicionado ao buffer
 */
static void push32(std::vector<uint8_t>& buffer, uint32_t value)
{
    buffer.push_back((value >> 24) & 0xFF);
    buffer.push_back((value >> 16) & 0xFF);
    buffer.push_back((value >> 8) & 0xFF);
    buffer.push_back(value & 0xFF);
}

/**
 * Funções auxiliares para leitura de valores de 16 e 32 bits a partir de um buffer de bytes, interpretando os dados em formato big-endian, utilizado para processar pacotes recebidos no formato do protocolo Aether.
*/
uint16_t read16(const uint8_t* data)
{
    return
        (static_cast<uint16_t>(data[0]) << 8) |
        static_cast<uint16_t>(data[1]);
}

/**
 * @brief Lê um valor de 32 bits a partir de um buffer de bytes, interpretando os dados em formato big-endian, utilizado para processar pacotes recebidos no formato do protocolo Aether.
 * @param data Ponteiro para o buffer de bytes onde o valor de 32 bits está armazenado
 * @return Valor de 32 bits lido a partir do buffer
 */
uint32_t read32(const uint8_t* data)
{
    return
        (static_cast<uint32_t>(data[0]) << 24) |
        (static_cast<uint32_t>(data[1]) << 16) |
        (static_cast<uint32_t>(data[2]) << 8)  |
        static_cast<uint32_t>(data[3]);
}

/**
 * @brief Constrói e envia um pacote no formato do protocolo Aether para o servidor.
 * @param type Código de comando que identifica o tipo de mensagem (ex: CMD_HELLO, CMD_DATA_PUSH)
 * @param module Código do módulo remetente (ex: MODULE_ESP32)
 * @param payload Dados a serem enviados no corpo do pacote
 * @return true se o pacote foi enviado com sucesso, false caso contrário
 */
bool sendAetherPacket(uint16_t type, uint16_t module, const std::vector<uint8_t>& payload)
{
    std::vector<uint8_t> buffer;
    buffer.reserve(11 + payload.size());

    // Header
    push16(buffer, AETHER_MAGIC);
    buffer.push_back(AETHER_VERSION);
    push16(buffer, type);
    push16(buffer, module);
    push32(buffer, static_cast<uint32_t>(payload.size()));

    // Payload
    buffer.insert(buffer.end(), payload.begin(), payload.end());

    return tcp_send(buffer.data(), buffer.size());
}

/**
 * @brief Envia o pacote de Handshake inicial (CMD_HELLO) para o servidor Aether, contendo o ID do dispositivo.
 * @return true se o pacote de Handshake foi enviado com sucesso, false caso contrário
 */
bool sendHandshake()
{
    std::vector<uint8_t> payload(
        deviceId.begin(),
        deviceId.end()
    );

    std::vector<uint8_t> buffer;

    push16(buffer, AETHER_MAGIC);
    buffer.push_back(AETHER_VERSION);
    push16(buffer, CMD_HELLO);
    push16(buffer, MODULE_ESP32);
    push32(buffer, payload.size());

    buffer.insert(
        buffer.end(),
        payload.begin(),
        payload.end()
    );

    size_t total_sent = 0;

    while (total_sent < buffer.size())
    {
        int sent = send(
            tcp_socket,
            buffer.data() + total_sent,
            buffer.size() - total_sent,
            0
        );

        if (sent <= 0)
        {
            return false;
        }

        total_sent += sent;
    }

    return true;
}