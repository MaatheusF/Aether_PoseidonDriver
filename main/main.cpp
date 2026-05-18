#include "core/network/wifi_service.hpp"
#include "poseidon/poseidon_main.hpp"
#include "core/network/sntp.hpp"

extern "C" void app_main()
{

    // Conexão com o Wifi (Core)
    wifi_start("Matheus","favero10");

    // Inicialização do Simple Network Time Protocol
    initialize_sntp();

    // Inicialização do Poseidon
    poseidon_main();
}