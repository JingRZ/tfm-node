#ifndef C_WIFI_H
#define C_WIFI_H

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(C_WIFI_EVENT_BASE);

enum{
    C_WIFI_EVENT_CUSTOM_DATA,
    C_WIFI_EVENT_CONNECTED,
    C_WIFI_EVENT_DISCONNECTED,
};

/**
 * @brief Initializes the WiFi module.
 *
 * This function sets up the necessary parameters, including SSID and password which are set in menuconfig, and starts the WiFi module.
 * It should be called at the beginning of the program before any WiFi operations.
 */
void wifi_init(void);

#endif