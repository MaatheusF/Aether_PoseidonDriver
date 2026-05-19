#include "sensor_read.hpp"
#include "ds18b20.h"
#include "esp_log.h"
#include "soc/gpio_num.h"
#include "../external/json.hpp"
#include "poseidon/network/tcp_service.hpp"

// ========================
//   Definições de pinos
// ========================

#define SENSOR_TEMP_AQUARIO_DS18B20_01 GPIO_NUM_19      // Definição do pino G19 para uso do sensor DS18B20 de Temperatura da agua

// ========================
//   Variáveis globais
// ========================

DeviceAddress sensor_addr;                             // Endereço do sensor DS18B20, utilizado para identificar o sensor durante as operações de leitura e configuração.
static auto TAG = "[POSEIDON]";                        // Tag para logs relacionados ao Poseidon
static constexpr uint16_t CMD_DATA_PUSH = 0x0102;      // Código de comando para envio de dados do sensor para o servidor Aether, utilizado na função read_send_sensor_temp_ds18b20 para identificar o tipo de mensagem enviada.
static constexpr uint16_t MODULE_ESP32  = 0x03;        // Código do módulo remetente (ESP32), utilizado na função read_send_sensor_temp_ds18b20 para identificar a origem dos dados enviados ao servidor Aether.

/**
 * @brief Inicia o processo de leitura dos sensores, incluindo a inicialização do sensor DS18B20 e a criação da task responsável por realizar as leituras periódicas.
 * Esta função é responsável por iniciar o processo de leitura dos sensores, realizando a inicialização do sensor DS18B20 e criando uma task separada para realizar as leituras periódicas dos sensores. A função utiliza as funções da biblioteca do driver do DS18B20 para realizar a inicialização do sensor, e a função xTaskCreate para criar a task de leitura dos sensores, garantindo que as leituras sejam realizadas em intervalos regulares sem bloquear a execução principal do programa.
 */
void sensor_read_start()
{
    ESP_LOGI(TAG, "Inicializando processo de leitura dos sensores");

    /// Inicialização do Sensor DS18B20
    sensor_ds18b20_init();

    // Inicializa a Thread de leitura dos sensores
    xTaskCreate(sensor_read_task, "sensor_read_task", 4096, nullptr, 5, nullptr);
}

/**
 * @brief Inicializa o sensor DS18B20, realizando a busca pelo sensor e configurando a resolução de leitura.
 * @return true se o sensor foi encontrado e configurado com sucesso, false caso contrário
 * Esta função é responsável por inicializar o sensor DS18B20, realizando a busca pelo sensor conectado ao pino definido e configurando a resolução de leitura para 11 bits. A função utiliza as funções da biblioteca do driver do DS18B20 para realizar a inicialização e configuração do sensor, e retorna um valor booleano indicando se o processo foi bem-sucedido ou não.
 */
bool sensor_ds18b20_init()
{
    ds18b20_init(SENSOR_TEMP_AQUARIO_DS18B20_01);
    vTaskDelay(pdMS_TO_TICKS(1000));
    reset_search();
    if (search(sensor_addr, true))
    {
        ESP_LOGI("[DS18B20]", "Sensor encontrado!");
        ds18b20_setResolution((const DeviceAddress*)&sensor_addr, 1, 11);
    }
    else
    {
        ESP_LOGE("[DS18B20]", "Nenhum sensor encontrado!");
        return false;
    }

    return true;
}

/**
 * @brief Task responsável por realizar a leitura periódica dos sensores e enviar os dados para o servidor Aether.
 * @param arg Argumento de entrada (não utilizado)
 * Esta função é executada em uma task separada e realiza a leitura periódica dos sensores, utilizando a função read_send_sensor_temp_ds18b20 para ler a temperatura do sensor DS18B20, construir um JSON com os dados e enviar para o servidor Aether. A task é configurada para aguardar 60 segundos entre cada leitura, evitando consumo elevado de CPU e garantindo que as leituras sejam realizadas em intervalos regulares.
 */
static void sensor_read_task(void* arg)
{
    ESP_LOGI(TAG,"Inicializando Thread para leitura dos sensores...");

    while (true)
    {
        /// Leitura do Sensor de Temperatura da agua
        read_send_sensor_temp_ds18b20();

        vTaskDelay(pdMS_TO_TICKS(60000)); //Evita consumo elevado de CPU
    }
}

/**
 * @brief Lê a temperatura do sensor DS18B20, constrói um JSON com os dados e envia para o servidor Aether utilizando o protocolo definido.
 * @return true se os dados foram enviados com sucesso, false caso contrário
 * Esta função realiza a leitura da temperatura do sensor DS18B20, valida a leitura, constrói um objeto JSON contendo as informações do sensor e o timestamp da leitura, e envia os dados para o servidor Aether utilizando a função sendAetherPacket. A função também verifica se o relógio do sistema está sincronizado antes de enviar os dados, garantindo que o timestamp seja válido.
 */
bool read_send_sensor_temp_ds18b20()
{
    using json = nlohmann::json;

    ds18b20_requestTemperatures();
    vTaskDelay(pdMS_TO_TICKS(5000));
    float water_temperature_ds18b2 = ds18b20_getTempC(&sensor_addr);
    vTaskDelay(pdMS_TO_TICKS(5000));

    // Verifica se a leitura é válida (driver retorna valores negativos especiais em erro)
    if (water_temperature_ds18b2 <= DEVICE_DISCONNECTED_C) {
        ESP_LOGE(TAG,"Erro ao buscar dados de temperatura do DS18B20 (sensor desconectado)");
        return false;
    }

    json j;
    time_t now;
    time(&now);
    long timestamp = static_cast<long>(now);

    // Valida a sincronização do NTP
    if (now < 100000)
    {
        ESP_LOGW(TAG, "Relógio ainda não sincronizado");
        return false;
    }

    j["event"]["type"] = "sensor";
    j["event"]["sensor_type"] = "temperature";
    j["event"]["sensor_external_id"] = "water_temp_01";
    j["event"]["value"] = water_temperature_ds18b2;
    j["event"]["read_timestamp"] = timestamp;

    std::string jsonStr = j.dump();
    std::vector<uint8_t> payload(jsonStr.begin(), jsonStr.end());

    if (sendAetherPacket(CMD_DATA_PUSH, MODULE_ESP32, payload))
    {
        ESP_LOGI(TAG,"Dados do Sensor (ds18b20) enviado com sucesso!");
        return true;
    }

    ESP_LOGE(TAG,"Ocorreu um erro ao enviar dados do Sensor (ds18b20)");

    return false;
}