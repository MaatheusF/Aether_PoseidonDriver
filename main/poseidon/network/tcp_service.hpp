#pragma once
#include <cstdint>
#include <string>
#include <vector>

/*
 * Socket TCP para comunicação com o servidor Aether
*/
bool tcp_service_init(const char* server_ip, uint16_t server_port);

void tcp_service_thread_lister(void* arg);
void sensor_read_thread(void* arg);
bool sendAetherPacket(int sock, uint16_t type, uint16_t module, const std::vector<uint8_t>& payload);

void processServerMessageBinary(const std::string& rawMessage);

bool sendHandshake();