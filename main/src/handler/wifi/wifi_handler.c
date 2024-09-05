
#include "esp_log.h"
#include "mqtt_handler.h"
#include "general_handler.h"
#include "c_wifi.h"

static const char *TAG = "wifi_handler";

void wifi_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    
    switch(id){
        case C_WIFI_EVENT_CONNECTED:
           ESP_LOGI(TAG, "C_WIFI_EVENT_CONNECTED");
           on_wifi_connected_handler();
        break;
        case C_WIFI_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Disconnected from AP");
            break;
        default:
            ESP_LOGE(TAG, "Unknown event");
            break;
    }
}

static void _register_handlers(){
    ESP_ERROR_CHECK(esp_event_handler_register(C_WIFI_EVENT_BASE, C_WIFI_EVENT_CONNECTED, wifi_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(C_WIFI_EVENT_BASE, C_WIFI_EVENT_DISCONNECTED, wifi_handler, NULL));
}

void wifi_setup(){
    _register_handlers();
    wifi_init();
}