#include <cJSON.h>
#include <stdio.h>

#include "string.h"
#include "c_nvs.h"
#include "esp_log.h"

static const char* TAG = "BLE_HANDLER";

void set_ble_name(char* name){
    char *data = (char *)malloc(100);
    nvs_read_string("ble_content", &data);
    cJSON *root = cJSON_Parse(data);
    cJSON *item = cJSON_GetObjectItem(root, "name");
    ESP_LOGI(TAG, "Read data: %s", item->valuestring);
    int len = strlen(item->valuestring);
    ESP_LOGW(TAG, "len: %d", len);
    strncpy(name, item->valuestring, len);
    name[len] = '\0';
    free(data);
}

void set_ble_content(char* content){
    char *data = (char *)malloc(100);
    nvs_read_string("ble_content", &data);
    cJSON *root = cJSON_Parse(data);
    cJSON *item = cJSON_GetObjectItem(root, "content");
    ESP_LOGI(TAG, "Read data: %s", item->valuestring);
    int len = strlen(item->valuestring);
    ESP_LOGW(TAG, "len: %d", len);
    strncpy(content, item->valuestring, len);
    content[len] = '\0';
    free(data);
}