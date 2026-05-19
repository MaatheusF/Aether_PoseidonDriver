#pragma once

/**
 * @brief Inicia o processo de leitura dos sensores, incluindo a inicialização do sensor DS18B20 e a criação da task responsável por realizar as leituras periódicas.
 * Esta função é responsável por iniciar o processo de leitura dos sensores, realizando a inicialização do sensor DS18B20 e criando uma task separada para realizar as leituras periódicas dos sensores. A função utiliza as funções da biblioteca do driver do DS18B20 para realizar a inicialização do sensor, e a função xTaskCreate para criar a task de leitura dos sensores, garantindo que as leituras sejam realizadas em intervalos regulares sem bloquear a execução principal do programa.
 */
void sensor_read_start();

/**
 * @brief Inicializa o sensor DS18B20, realizando a busca pelo sensor e configurando a resolução de leitura.
 * @return true se o sensor foi encontrado e configurado com sucesso, false caso contrário
 * Esta função é responsável por inicializar o sensor DS18B20, realizando a busca pelo sensor conectado ao pino definido e configurando a resolução de leitura para 11 bits. A função utiliza as funções da biblioteca do driver do DS18B20 para realizar a inicialização e configuração do sensor, e retorna um valor booleano indicando se o processo foi bem-sucedido ou não.
 */
bool sensor_ds18b20_init();

/**
 * @brief Task responsável por realizar a leitura periódica dos sensores e enviar os dados para o servidor Aether.
 * @param arg Argumento de entrada (não utilizado)
 * Esta função é executada em uma task separada e realiza a leitura periódica dos sensores, utilizando a função read_send_sensor_temp_ds18b20 para ler a temperatura do sensor DS18B20, construir um JSON com os dados e enviar para o servidor Aether. A task é configurada para aguardar 60 segundos entre cada leitura, evitando consumo elevado de CPU e garantindo que as leituras sejam realizadas em intervalos regulares.
 */
static void sensor_read_task(void* arg);

/**
 * @brief Lê a temperatura do sensor DS18B20, constrói um JSON com os dados e envia para o servidor Aether utilizando o protocolo definido.
 * @return true se os dados foram enviados com sucesso, false caso contrário
 * Esta função realiza a leitura da temperatura do sensor DS18B20, valida a leitura, constrói um objeto JSON contendo as informações do sensor e o timestamp da leitura, e envia os dados para o servidor Aether utilizando a função sendAetherPacket. A função também verifica se o relógio do sistema está sincronizado antes de enviar os dados, garantindo que o timestamp seja válido.
 */
bool read_send_sensor_temp_ds18b20();