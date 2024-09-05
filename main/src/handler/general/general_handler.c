#include "c_mqtt.h"
#include "general_handler.h"
#include "mqtt_handler.h"
#include "c_M24SR.h"
#include "m24sr_handler.h"
#include "M24SR_def.h"
#include "esp_log.h"
#include "c_nvs.h"
#include "string.h"

static const char *TAG = "GENERAL_HANDLER";

void on_wifi_connected_handler() {
    mqtt_setup();
}

void send_data_to_topic(char* topic, char* data, int dataLen) {
    mqtt_publish_to_topic(topic, (uint8_t*)data, dataLen);
}

void on_receive_attr_fmode(int fmode) {
    if (m24sr_set_fmode(fmode) != STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set fmode");
    }
}

void on_receive_attr_nfc_content(const char* buf) {
    if (m24sr_set_content(buf) != STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Failed to set content");
    }
} 

void on_receive_attr_writepwd(uint8_t* buf) {
    if(buf != NULL) {
        uint8_t* prev_pwd = malloc(sizeof(char) * 20);
        if(nvs_read_string(WRITE_PWD_KEY, &prev_pwd) != NVS_OK){
            if(strlen((char *)prev_pwd) != 0x10){
                printf("prev_pwd is not 16 bytes\n");
                memcpy(prev_pwd, DEFAULT_PWD_16B, 16);
            }
        }
        printf("prev_pwd: %s\n", prev_pwd);
        printf("new pwd: %s\n", buf);

        if(m24sr_enableWritePwd(prev_pwd, buf) == STATUS_SUCCESS){
            ESP_LOGI(TAG, "Write password set");
            nvs_write_string(WRITE_PWD_KEY, (char *)buf);

        } else {
            ESP_LOGE(TAG, "Failed to set write password");
        }
        free(prev_pwd);

    } else {
        ESP_LOGE(TAG, "on_receive_attr_writepwd buf is NULL");
    }
}

 