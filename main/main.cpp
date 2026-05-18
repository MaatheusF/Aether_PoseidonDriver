#include "core/network/wifi_service.hpp"
#include "poseidon/poseidon_main.hpp"
#include "core/network/sntp.hpp"
#include "core/network/debug_server.hpp"

extern "C" void app_main()
{

    // Conexão com o Wifi (Core)
    wifi_start("Matheus","favero10");

    // Inicializa o Sistema de Logs Remoto
    debug_server_start();

    // Inicialização do Simple Network Time Protocol
    initialize_sntp();

    // Inicialização do Poseidon
    poseidon_main();
}