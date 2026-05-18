#pragma once

#include <esp_log.h>
#include "esp_sntp.h"

/**
 * Função para inicializar o SNTP e sincronizar o tempo
 */
inline void initialize_sntp()
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