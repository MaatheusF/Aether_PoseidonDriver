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
    ESP_LOGI("[POSEIDON]","Inicializando conexão TCP com o servidor Aether...");

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

    //xTaskCreate(tcp_service_thread_lister, "tcp_service_thread_lister", 4096, nullptr, 5, &tcpListenTaskHandle);
    xTaskCreate(sensor_read_thread, "sensor_read_thread", 4096, nullptr, 5, &sensorReadTaskHandle);

    return true;
}

void tcp_service_thread_lister(void* arg){
    ESP_LOGE("[POSEIDON]","Inicializando Thread...");
    /*
    char rece_buffer[128]; // Buffer de recepção dos dados (128 bits)

    while (true)
    {
        if (sock <0)
        {
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        //Bloqueia a Thread e aguarda o recebimento de dados do Servidor
        int len = recv(sock, rece_buffer, sizeof(rece_buffer) - 1, 0);

        if (len > 0) {
            rece_buffer[len] = 0;
            processServerMessageBinary(std::string(rece_buffer));
        }
        else if (len == 0) {
            ESP_LOGE("[POSEIDON]","[ERROR] Conexão com o servidor perdida.");
            close(sock);
            sock = -1;
            vTaskDelete(nullptr);
        }
        else {
            ESP_LOGE("[POSEIDON]","[ERROR] Ocorreu um erro ao receber os dados do Servidor");
            close(sock);
            sock = -1;
            vTaskDelete(nullptr);
        }

        // Apenas para tratar o return do while
        if (sock == -1) {
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(3000)); //Evita consumo elevado de CPU
    }*/
    vTaskDelay(pdMS_TO_TICKS(3000)); //Evita consumo elevado de CPU
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

bool sendAetherPacket(int sock, uint16_t type, uint16_t module, const std::vector<uint8_t>& payload)
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
            sock,
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
        sock,
        CMD_HELLO,
        MODULE_ESP32,
        std::vector<uint8_t>(deviceId.begin(), deviceId.end())
    );

    return true;
}

void initialize_sntp()
{
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = {};

    while (timeinfo.tm_year < (2020 - 1900))
    {
        ESP_LOGI("NTP", "Aguardando sincronização...");
        vTaskDelay(pdMS_TO_TICKS(2000));

        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

void sensor_read_thread(void* arg)
{
    ESP_LOGI("[POSEIDON]","Inicializando Thread para leitura dos sensores...");
    initialize_sntp();
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
        //TEMPERATURA_AQUARIO_DS18B2 = ds18b20_get_temp();

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

        /*
        std::string json = R"({
        "event":
            {
              "type": "sensor",
              "sensor_type": "temperature",
              "sensor_external_id": "water_temp_01",
              "value": 26.5,
              "read_timestamp": 1700000000
            }
        })";

        std::vector<uint8_t> payload(json.begin(), json.end());*/
        std::string jsonStr = j.dump();
        std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());

        if (!sendAetherPacket(
            sock,
            CMD_DATA_PUSH, // type / command
            MODULE_ESP32, // module Poseidon
            payload
        ))
        {
            close(sock);
            sock = -1;
            tcp_service_init("192.168.0.133", 9000);
            vTaskDelay(pdMS_TO_TICKS(1000)); //Evita consumo elevado de CPU
            vTaskDelete(nullptr);
        }

        ESP_LOGI("[POSEIDON]","Dados do Sensor enviado com sucesso!");

        vTaskDelay(pdMS_TO_TICKS(60000)); //Evita consumo elevado de CPU
    }
}