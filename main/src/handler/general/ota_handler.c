#include <stdio.h>
#include <stdint.h>
#include "cJSON.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_event.h"

#include "esp_log.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"

const char *TAG = "OTA_HANDLER";

extern const uint8_t server_cert_pem_start[] asm("_binary_github_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_github_cert_pem_end");

static bool _diagnostic(void) {
    ESP_LOGI(TAG, "Diagnostics (5 sec)...");
    //vTaskDelay(5000 / portTICK_PERIOD_MS);
    return 1;
}

void ota_check() {
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            bool diagnostic_is_ok = _diagnostic();
            if (diagnostic_is_ok) {
                ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
                esp_ota_mark_app_valid_cancel_rollback();
            } else {
                ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        } 
        else{
            ESP_LOGI(TAG, "ota_state != ESP_OTA_IMG_PENDING_VERIFY");
        }
    }

    printf("\n-------------------\nOTA VERIFIED!\n-------------------\n");
    fflush(stdout);
}


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

void _download_update(void *pvParameter){
    ESP_LOGI(TAG, "Starting OTA _download_update");
    char *url = *(char *)pvParameter;

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
        .cert_pem = (char *)server_cert_pem_start,
        .keep_alive_enable = true,
        #ifdef CONFIG_OTA_FIRMWARE_UPGRADE_BIND_IF
        .if_name = &ifr,
        #endif
    };

    #ifdef CONFIG_OTA_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
    #endif

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    ESP_LOGI(TAG, "Attempting to download update from %s", config.url);

    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA Succeed, Rebooting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Firmware upgrade failed");
    }
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void _ota_get_update_url(char *payload, char *url){
    cJSON* root = cJSON_Parse(payload);
    if (root != NULL){

        cJSON* sw_urlItem = cJSON_GetObjectItem(root, "fw_url");
        char *res = sw_urlItem->valuestring;
        //char * https = "https://";
        int len = strlen(res);
        url = (char *)malloc((len + 1) * sizeof(char) );
        strcpy(url, res);
        url[len] = '\0';
    }
    cJSON_Delete(root);
}


void ota_handler(char *data){
    char *url = malloc(100);
    _ota_get_update_url(data, url);
    xTaskCreate(&_download_update, "_download_update_task", 8192, url, 5, NULL);
    free(url);
}




