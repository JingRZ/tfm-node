
#include "esp_log.h"

#include "mqtt_handler.h"
#include "c_mqtt.h"
#include "cJSON.h"
#include "method_parser.h"
#include "context.h"
#include "c_nvs.h"
#include "string.h"
#include "ota_handler.h"

static const char* TAG = "MQTT_HANDLER";
int mqtt_status = BLANK;
static const char* TB_SA_RPC_REQUEST_METHOD = "getSharedAttributes";

static void _set_shared_attributes(char* data){
    cJSON* root = cJSON_Parse(data);
    cJSON* params = cJSON_GetObjectItem(root, "params");
    
    cJSON* nfc_content = cJSON_GetObjectItem(params, "nfc_content");
    cJSON* ble_content = cJSON_GetObjectItem(params, "ble_content");

    if(nfc_content != NULL){
        char *nfc_content_str = cJSON_Print(nfc_content);
        nvs_write_string("nfc_content", nfc_content_str);
        cJSON_free(nfc_content_str);
    }
    if(ble_content != NULL){
        char *ble_content_str = cJSON_Print(ble_content);
        nvs_write_string("ble_content", ble_content_str);
        cJSON_free(ble_content_str);
    }

    cJSON_Delete(root);
}

static void method_handler(char *topic, char* data){
    switch(parse_method(data)){
        case METHOD_SET_BLE_DATA:
            ESP_LOGI(TAG, "METHOD_SET_BLE_DATA");
            //change_ble_data_handler(topic, data);
            break;
        case METHOD_GET_BLE_DATA:
            ESP_LOGI(TAG, "METHOD_GET_BLE_DATA");
            //get_ble_data_handler(topic);
            break;
        case METHOD_GET_SHARED_ATTRIBUTES:
            ESP_LOGI(TAG, "METHOD_GET_SHARED_ATTRIBUTES");
            _set_shared_attributes(data);
            esp_event_post(CONTEXT_EVENT_BASE, NODE_STATE_REGULAR, NULL, 0, 0);
            break;
        default:
            ESP_LOGI(TAG, "UNKNOWN METHOD");
            break;
    }
}

static void attribute_handler(char* data){

    switch(parse_attribute(data)){
        case ATTRIBUTE_PATHLOSS:
            ESP_LOGI(TAG, "ATTRIBUTE_PATHLOSS");
            //change_pathloss_handler(data);
            break;
        case ATTRIBUTE_FMODE:
            ESP_LOGI(TAG, "ATTRIBUTE_FMODE");
            change_fmode_handler(data);
            break;
        case ATTRIBUTE_NFC_CONTENT:
            ESP_LOGI(TAG, "ATTRIBUTE_NFC_CONTENT");
            //change_nfc_content_handler(data);
            change_nfc_content_handler(data, FROM_THINGBOARD);
            break;
        case ATTRIBUTE_BLE_CONTENT:
            ESP_LOGI(TAG, "ATTRIBUTE_BLE_CONTENT");
            break;
        case ATTRIBUTE_WRITEPWD:
            ESP_LOGI(TAG, "ATTRIBUTE_WRITEPWD");
            change_writepwd_handler(data);
            break;
        case ATTRIBUTE_OTA:
            ESP_LOGI(TAG, "ATTRIBUTE_OTA");
            ota_handler(data);
            break;
        case ATTRIBUTE_NOT_FOUND:
            ESP_LOGI(TAG, "ATTRIBUTE_NOT_FOUND");
            break;
    }
}

static void _start_client(){
    mqtt_set_qos(1);
    char *data = malloc(100);

    nvs_read_string("tb_broker_url", &data);
    if(data != NULL){
        mqtt_set_broker(data, strlen(data));
        if(nvs_read_string("tb_token", &data)== NVS_OK){
            ESP_LOGI(TAG, "TB TOKEN FOUND");
            mqtt_set_username(data, strlen(data));
            mqtt_start_client();
        }
        else{
            ESP_LOGE(TAG, "TB TOKEN NOT FOUND");
        }
    }
    else{
        ESP_LOGE(TAG, "No broker url found");
    }

    free(data);
}

static void _start_with_tb_token(char *token){
    mqtt_set_qos(1);
    mqtt_set_username(token, strlen(token));
    mqtt_start_client();
}

static void handler(char* topic, char* data){
    switch(parse_msg_type(topic, data)){
        case IS_METHOD:
            method_handler(topic, data);
            break;
        case IS_ATTRIBUTE:
            attribute_handler(data);
            break;
        case IS_NOTHING:
            ESP_LOGI(TAG, "UNKNOWN MSG TYPE");
            break;
    }
    
}

void _request_shared_attributes(){
    cJSON* request = cJSON_CreateObject();
    cJSON_AddStringToObject(request, "method", TB_SA_RPC_REQUEST_METHOD);
    cJSON *params = cJSON_CreateObject();
    cJSON_AddItemToObject(request, "params", params);

    char *json_string = cJSON_PrintUnformatted(request);
    ESP_LOGI(TAG, "Requesting shared attributes %s", json_string);

    mqtt_publish_to_topic("v1/devices/me/rpc/request/1", (uint8_t*)json_string, strlen(json_string));
    cJSON_Delete(request);
}

void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
    switch(id){
        case C_MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT CONNECTED");
            mqtt_subscribe_to_topic(CONFIG_TB_SS_RPC_REQUEST_TOPIC);
            mqtt_subscribe_to_topic(CONFIG_TB_CS_RPC_RESPONSE_TOPIC);
            mqtt_subscribe_to_topic(CONFIG_TB_ATTR_RESPONSE_TOPIC);
            mqtt_subscribe_to_topic(CONFIG_TB_ATTR_TOPIC);
            mqtt_subscribe_to_topic("v2/fw/response/+");
            
            esp_event_post(CONTEXT_EVENT_BASE, NODE_STATE_VALID_TB_TOKEN, NULL, 0, 0);
        break;
        case C_MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT DISCONNECTED");
        break;
        case C_MQTT_EVENT_RECEIVED_DATA:
            struct c_mqtt_data* mqtt_data = (struct mqtt_com_data*)event_data;
            ESP_LOGI(TAG, "MQTT Received data %s from topic %s", mqtt_data->data, mqtt_data->topic);
            handler(mqtt_data->topic, mqtt_data->data);
        break;
        case C_MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT SUBSCRIBED");
            char *topic = (char*)event_data;
            if(strncmp(CONFIG_TB_CS_RPC_RESPONSE_TOPIC, topic, strlen(CONFIG_TB_CS_RPC_RESPONSE_TOPIC)) == 0){
                _request_shared_attributes();
            }
        break;
        default:
            ESP_LOGE(TAG, "Unknown event");
    }
}

void mqtt_context_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
    switch(id){
        case CTX_VALID_WIFI_CRED:
            if(mqtt_status == GOT_TB_CRED){
                ESP_LOGI(TAG, "mqtt_start_client");
                _start_client();
                mqtt_status = COMPLETED;
            } else {
                mqtt_status = GOT_WIFI_CRED;
            }
        case CTX_VALID_TB_TOKEN:
            if(mqtt_status == GOT_WIFI_CRED){
                ESP_LOGI(TAG, "mqtt_start_client");
                _start_client();
                mqtt_status = COMPLETED;
            } else {
                mqtt_status = GOT_TB_CRED;
            }
        break;
    }
}

void mqtt_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {

    if(base == C_MQTT_EVENT_BASE){
        mqtt_event_handler(handler_args, base, id, event_data);
    }
    else if (base == CONTEXT_EVENT_BASE) {
        mqtt_context_event_handler(handler_args, base, id, event_data);
    }
    else{
        ESP_LOGE(TAG, "Unknown event");
    }
}




bool tb_parse_context(char* data){
    cJSON* root = cJSON_Parse(data);
    if (root != NULL){
        cJSON *tb = cJSON_GetObjectItem(root, "tb");
        cJSON *token = cJSON_GetObjectItem(tb, "token");
        cJSON *broker_url = cJSON_GetObjectItem(tb, "broker_url");

        if (token != NULL){
            if(nvs_write_string("tb_token", token->valuestring) == NVS_OK){
                if(broker_url != NULL){
                    if(nvs_write_string("tb_broker_url", broker_url->valuestring) == NVS_OK){
                        ESP_LOGI(TAG, "TB TOKEN AND BROKER URL SAVED");
                    }
                    else{
                        return false;
                    }
                }
                else{
                    return false;
                }
            }
        }
        else{
            return false;
        }
    }
    cJSON_Delete(root);
    return true;
}

void mqtt_setup(){
    //_start_with_tb_token(TB_TOKEN);

    printf("MQTT SETUP IS ALL COMMENTED. DOES NOTHING\n");
}