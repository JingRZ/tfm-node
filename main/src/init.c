#include <stdio.h>
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_log.h"

#include "c_provisioning.h"
#include "c_mqtt.h"
#include "c_nvs.h"
#include "c_ble.h"
#include "context.h"

#include "provisioning_handler.h"
#include "mqtt_handler.h"
#include "ble_handler.h"
#include "method_parser.h"

static const char* TAG = "INIT";

static void init_when_context_ready(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
    if (base == CONTEXT_EVENT_BASE) {
        switch(id){
            case NODE_STATE_REGULAR:
                if(ble_init(set_ble_name, set_ble_content) != C_BLE_OK){
                    ESP_LOGE(TAG, "BLE init failed");
                }
                change_nfc_content_handler(NULL, FROM_NVS);
            break;
            default:
            break;
        }
    }
}

static void init_event_handlers() {
    ESP_ERROR_CHECK(esp_event_handler_register(C_PROVISIONING_EVENT_BASE, C_PROVISIONG_EVENT_CONNECTED, provisioning_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_PROVISIONING_EVENT_BASE, C_PROVISIONG_EVENT_DISCONNECTED, provisioning_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_PROVISIONING_EVENT_BASE, C_PROVISIONG_EVENT_CUSTOM_DATA, provisioning_handler, NULL));
    
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_CONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_DISCONNECTED, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_RECEIVED_DATA, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_MQTT_EVENT_BASE, C_MQTT_EVENT_SUBSCRIBED, mqtt_handler, NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(CONTEXT_EVENT_BASE, ESP_EVENT_ANY_ID, mqtt_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(CONTEXT_EVENT_BASE, ESP_EVENT_ANY_ID, ctx_event_handler, NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(CONTEXT_EVENT_BASE, ESP_EVENT_ANY_ID, init_when_context_ready, NULL));
}



void setup(){
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    nvs_init();

    init_event_handlers();
    prov_init();
}