#include "poseidon_main.hpp"
#include "service/sensor_read.hpp"
#include "service/relay_change.hpp"

#include "network/tcp_service.hpp"

/**
 * Função para iniciar a tarefa do Poseidon
 */
void poseidon_main(){
    // Registra o handler de TCP para processar os pacotes recebidos
    tcp_register_handler(my_tcp_handler);

    // Inicia o serviço TCP para escutar e enviar para o Servidor Aether
    tcp_service_start("192.168.0.133", 9000);

    // Inicia o processo de leitura dos sensores
    sensor_read_start();

    // Inicializa os Relays
    relay_init();
}