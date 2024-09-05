#include "provisioning_handler.h"
#include "c_provisioning.h"

#include "esp_log.h"
#include "cJSON.h"
#include "context.h"

#include "c_nvs.h"
#include "mqtt_handler.h"
#include "m24sr_handler.h"

static const char *TAG = "provisioning handler";

void provisioning_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    
    switch(id){
        case C_PROVISIONG_EVENT_CUSTOM_DATA:
            ESP_LOGI(TAG, "C_PROVISIONG_EVENT_CUSTOM_DATA");
            char *data = (char *)event_data;
            if(!tb_parse_context(data)){
                ESP_LOGE(TAG, "Error parsing tb token");
            }
            if(!m24sr_parse_context(data)){
                ESP_LOGE(TAG, "Error parsing m24sr token");
            }
        break;
        case C_PROVISIONG_EVENT_CONNECTED:
            ESP_LOGI(TAG, "C_PROVISIONG_EVENT_CONNECTED");
            esp_event_post(CONTEXT_EVENT_BASE, NODE_STATE_VALID_WIFI_CRED, NULL, 0, 0);

        break;
        case C_PROVISIONG_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "C_PROVISIONG_EVENT_DISCONNECTED");
            esp_event_post(CONTEXT_EVENT_BASE, NODE_STATE_WIFI_DISCONNECTED, NULL, 0, 0);
        break;
    }
}