#pragma once

#include <cstdint>
#include <string>
#include "../../../external/json.hpp"

void relay_change_state(const nlohmann::json& j);

bool relay_gpio_alter_state(std::string device_id, std::string rele_state);

void relay_init();