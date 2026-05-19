#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>


// Refactor

enum tcp_event_t
{
    TCP_EVENT_CONNECTED,
    TCP_EVENT_DISCONNECTED,
    TCP_EVENT_DATA_RECEIVED,
    TCP_EVENT_ERROR
};

typedef void (*tcp_event_handler_t)(
    tcp_event_t event,
    const uint8_t* data,
    size_t len
);

void tcp_service_start(const char* host, uint16_t port);

bool tcp_send(const std::vector<uint8_t>& data);

bool tcp_is_connected();

void tcp_register_handler(tcp_event_handler_t handler);

void my_tcp_handler(tcp_event_t event,const uint8_t* data,size_t len);


// ======================


/*
 * Socket TCP para comunicação com o servidor Aether
*/
bool tcp_service_init(const char* server_ip, uint16_t server_port);

void tcp_service_thread_lister(void* arg);
void sensor_read_thread(void* arg);
bool sendAetherPacket(uint16_t type, uint16_t module, const std::vector<uint8_t>& payload);

void processServerMessageBinary(const std::string& rawMessage);

bool sendHandshake();