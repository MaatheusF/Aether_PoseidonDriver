#include "tcp_service.hpp"
#include "../external/json.hpp"
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

// Refactor init

static auto TAG = "[TCP]";
static int tcp_socket = -1;
static bool connected = false;
static tcp_event_handler_t event_handler = nullptr;
static char server_host[64];
static uint16_t server_port;

static void emit_event(tcp_event_t event, const uint8_t* data = nullptr, size_t len = 0)
{
    if (event_handler)
    {
        event_handler(event, data, len);
    }
}

void tcp_register_handler(tcp_event_handler_t handler)
{
    event_handler = handler;
}

bool tcp_is_connected()
{
    return connected;
}

bool tcp_send(const std::vector<uint8_t>& data)
{
    if (!connected)
    {
        return false;
    }

    int err = send(tcp_socket, data.data(), data.size(), 0);
    if (err < 0)
    {
        ESP_LOGE(TAG, "Erro no envio");
        connected = false;
        emit_event(TCP_EVENT_DISCONNECTED);
        close(tcp_socket);
        tcp_socket = -1;
        return false;
    }

    return true;
}

static void tcp_task(void* arg)
{
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
            sendHandshake();

            emit_event(TCP_EVENT_CONNECTED);
        }

        uint8_t rx_buffer[512];

        int len = recv(
            tcp_socket,
            rx_buffer,
            sizeof(rx_buffer),
            MSG_DONTWAIT
        );

        if (len > 0)
        {
            emit_event(
                TCP_EVENT_DATA_RECEIVED,
                rx_buffer,
                len
            );
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

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void tcp_service_start(const char* host, uint16_t port)
{
    ESP_LOGI(TAG, "Inicializando conexão com o Servidor Aether...");
    strncpy(server_host, host, sizeof(server_host));
    server_port = port;
    xTaskCreate(tcp_task, "tcp_task", 4096, nullptr, 5, nullptr);
}

void my_tcp_handler(tcp_event_t event,const uint8_t* data,size_t len)
{
    switch(event)
    {
    case TCP_EVENT_CONNECTED:
        ESP_LOGI(TAG, "TCP conectado");
        break;

    case TCP_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "TCP desconectado");
        break;

    case TCP_EVENT_DATA_RECEIVED:
        ESP_LOGI(TAG, "Pacote recebido");
        break;

    default:
        break;
    }
}


// Refactor end


TaskHandle_t tcpListenTaskHandle = nullptr;     // Ponteiro da Task de comunicação TCP
TaskHandle_t sensorReadTaskHandle = nullptr;    // Ponteiro da Task de leitura dos sensores
int sock = -1; //Variavel permanente de conexão socket com o servidor;

static constexpr uint16_t AETHER_MAGIC = 0xAA55;
static constexpr uint8_t AETHER_VERSION = 0x01;
static constexpr uint16_t CMD_HELLO     = 0x0004;
static constexpr uint16_t CMD_DATA_PUSH = 0x0102;
static constexpr uint16_t MODULE_ESP32  = 0x03;

// ========================
// Definições de pinos
// ========================

#define SENSOR_TEMP_AQUARIO_DS18B20_01 GPIO_NUM_19

// ========================
// Variáveis globais
// ========================
float TEMPERATURA_AQUARIO_DS18B2;
DeviceAddress sensor_addr;

// ========================
// Configuração inicial
// ========================
void initialConfig() {

    /// Sensor DS18B20
    ds18b20_init(SENSOR_TEMP_AQUARIO_DS18B20_01);
    vTaskDelay(pdMS_TO_TICKS(1000));
    reset_search();
    //memset(sensor_addr, 0, sizeof(sensor_addr));
    if (search(sensor_addr, true))
    {
        ESP_LOGI("[DS18B20]", "Sensor encontrado!");
        ds18b20_setResolution(
            (const DeviceAddress*)&sensor_addr,
            1,
            11
        );
    }
    else
    {
        ESP_LOGE("[DS18B20]", "Nenhum sensor encontrado!");
    }
}

/*
 * Socket TCP para comunicação com o servidor Aether
*/
bool tcp_service_init(const char* server_ip, uint16_t server_port){
    /*ESP_LOGI("[POSEIDON]","Inicializando conexão TCP com o servidor Aether...");

    // Cria o Socket de Conexão
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        ESP_LOGE("[POSEIDON]","Falha ao criar o socket TCP");
        sock = -1;
        return false;
    }

    // Configuração do Servidor
    struct sockaddr_in server_addr = {};
    server_addr.sin_len = sizeof(server_addr);                     // Inicializa sin_len (Não serve pra nada mas precisa declarar)
    server_addr.sin_family = AF_INET;                              // Define o protocolo IPV4
    server_addr.sin_port = htons(server_port);                     // Converte o numero da porta para o formato de rede (big-endian)
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr.s_addr);

    // Conecta ao Servidor
    if(connect(sock, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0){
        ESP_LOGE("[POSEIDON]","Falha ao conectar com o servidor Aether");
        close(sock);    // Fecha o Socket
        sock = -1;
        return false;
    }

    ESP_LOGI("[POSEIDON]","Conexão realizada com sucesso com o servidor Aether");

    // Realiza o Handshake inicial se identificando
    while (true)
    {
        if (sendHandshake())
        {
            vTaskDelay(pdMS_TO_TICKS(10000)); //Evita consumo elevado de CPU
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(30000)); //Evita consumo elevado de CPU
    }

    //xTaskCreate(tcp_service_thread_lister, "tcp_service_thread_lister", 4096, nullptr, 5, &tcpListenTaskHandle);*/
    xTaskCreate(sensor_read_thread, "sensor_read_thread", 4096, nullptr, 5, &sensorReadTaskHandle);

    return true;
}

static void push16(std::vector<uint8_t>& buffer, uint16_t value)
{
    buffer.push_back((value >> 8) & 0xFF);
    buffer.push_back(value & 0xFF);
}

static void push32(std::vector<uint8_t>& buffer, uint32_t value)
{
    buffer.push_back((value >> 24) & 0xFF);
    buffer.push_back((value >> 16) & 0xFF);
    buffer.push_back((value >> 8) & 0xFF);
    buffer.push_back(value & 0xFF);
}

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

    size_t totalSend = 0;

    while (totalSend < buffer.size())
    {
        int sent = send(
            tcp_socket,
            buffer.data() + totalSend,
            buffer.size() - totalSend,
            0
        );

        if (sent <= 0)
        {
            ESP_LOGE("[POSEIDON]","Erro ao enviar dados para o Servidor Aether: errno=%d (%s)", errno, strerror(errno));
            return false;
        }

        totalSend += sent;
    }
    return true;
}

bool sendHandshake()
{
    std::string deviceId = "IDESP32";
    sendAetherPacket(
        CMD_HELLO,
        MODULE_ESP32,
        std::vector<uint8_t>(deviceId.begin(), deviceId.end())
    );
    return true;
}

void sensor_read_thread(void* arg)
{
    ESP_LOGI("[POSEIDON]","Inicializando Thread para leitura dos sensores...");
    initialConfig();

    using json = nlohmann::json;

    while (true)
    {
        json j;
        time_t now;
        time(&now);
        long timestamp = static_cast<long>(now);

        // ========================
        //    Leitura do Sensor
        // ========================

        // Ler temperatura do DS18B20 (externa)
        ds18b20_requestTemperatures();
        vTaskDelay(pdMS_TO_TICKS(5000));
        TEMPERATURA_AQUARIO_DS18B2 = ds18b20_getTempC(&sensor_addr);
        vTaskDelay(pdMS_TO_TICKS(5000));

        // Verifica se a leitura é válida (driver retorna valores negativos especiais em erro)
        if (TEMPERATURA_AQUARIO_DS18B2 <= DEVICE_DISCONNECTED_C) {
            ESP_LOGE("[POSEIDON]","Erro ao buscar dados de temperatura do DS18B20 (sensor desconectado)");
            continue;
        }

        j["event"]["type"] = "sensor";
        j["event"]["sensor_type"] = "temperature";
        j["event"]["sensor_external_id"] = "water_temp_01";
        j["event"]["value"] = TEMPERATURA_AQUARIO_DS18B2;
        j["event"]["read_timestamp"] = timestamp;

        std::string jsonStr = j.dump();
        std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());

        sendAetherPacket(
            CMD_DATA_PUSH, // type / command
            MODULE_ESP32, // module Poseidon
            payload
        );

        ESP_LOGI("[POSEIDON]","Dados do Sensor enviado com sucesso!");

        vTaskDelay(pdMS_TO_TICKS(60000)); //Evita consumo elevado de CPU
    }
}