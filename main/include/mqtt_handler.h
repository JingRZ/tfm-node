#ifndef __MQTT_HANDLER
#define __MQTT_HANDLER

#include <stdio.h>
#include "esp_event.h"

extern int mqtt_status;

enum {
    BLANK,
    GOT_WIFI_CRED,
    GOT_TB_CRED,
    COMPLETED
};

void mqtt_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
void mqtt_setup();
bool tb_parse_context(char* data);

#endif