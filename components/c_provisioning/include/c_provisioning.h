#ifndef __C_PROVISIONG
#define __C_PROVISIONG

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(C_PROVISIONING_EVENT_BASE);

enum{
    C_PROVISIONG_EVENT_CUSTOM_DATA,
    C_PROVISIONG_EVENT_CONNECTED,
    C_PROVISIONG_EVENT_DISCONNECTED,
};

void prov_init();
bool prov_is_provisioned();

#endif