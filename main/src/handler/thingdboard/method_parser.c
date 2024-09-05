#include <cJSON.h>
#include <string.h>
#include <stdio.h>
#include "method_parser.h"
#include "general_handler.h"
#include "c_nvs.h"
#include "esp_log.h"

const static char* TB_TELEMETRY_TOPIC = "v1/devices/me/telemetry";
const static char* TB_ATTRB_TOPIC = "v1/devices/me/attributes";
const static char* TB_RPC_REQUEST_TOPIC = "v1/devices/me/rpc/request";
const static char* TB_RPC_RESPONSE_TOPIC = "v1/devices/me/rpc/response";

const static char* SA_PATHLOSS = "pathloss";
const static char* SA_FMODE = "fmode";
const static char* SA_NFC_CONTENT = "nfc_content";
const static char* SA_BLE_CONTENT = "ble_content";
const static char* SA_WRITEPWD = "writepwd";
const static char* OTA_FW = "fw_title";

const static char* TAG = "METHOD_PARSER";

int parse_attribute(char* data){
    cJSON* json = cJSON_Parse(data);
    int res = ATTRIBUTE_NOT_FOUND;

    if (json != NULL) {
        cJSON* item = json->child;
        if (item != NULL) {
            const char* key = item->string;
            printf("Key: %s\n", key);
            if(strncmp(key, SA_PATHLOSS, strlen(SA_PATHLOSS)) == 0){
                res = ATTRIBUTE_PATHLOSS;
            } else if (strncmp(key, SA_FMODE, strlen(SA_FMODE)) == 0){
                res = ATTRIBUTE_FMODE;
            } else if (strncmp(key, SA_NFC_CONTENT, strlen(SA_NFC_CONTENT)) == 0){
                res = ATTRIBUTE_NFC_CONTENT;
            } else if (strncmp(key, SA_BLE_CONTENT, strlen(SA_BLE_CONTENT)) == 0){
                res = ATTRIBUTE_BLE_CONTENT;
            } else if (strncmp(key, SA_WRITEPWD, strlen(SA_WRITEPWD)) == 0){
                res = ATTRIBUTE_WRITEPWD;
            } else if (strncmp(key, OTA_FW, strlen(OTA_FW)) == 0){
                res = ATTRIBUTE_OTA;
            }
        }
    }

    cJSON_Delete(json);
    return res;
}

int parse_msg_type(char *topic, char* data){
    cJSON *root = cJSON_Parse(data);
    cJSON *type = cJSON_GetObjectItem(root, "method");
    if(type != NULL){
        cJSON_Delete(root);
        return IS_METHOD;
    }

    if(strncmp(topic, TB_ATTRB_TOPIC, strlen(TB_ATTRB_TOPIC)) == 0){
        ESP_LOGI(TAG, "%s IS_ATTRIBUTE", topic);
        return IS_ATTRIBUTE;
    }

    return IS_NOTHING;
}


int parse_method(char* data){
    cJSON *root = cJSON_Parse(data);
    cJSON *method = cJSON_GetObjectItem(root, "method");
    if(method == NULL){
        cJSON_Delete(root);
        return METHOD_NOT_FOUND;
    }

    if(strcmp(method->valuestring, "setBLEDataType") == 0){
        return METHOD_SET_BLE_DATA;
    } else if (strcmp(method->valuestring, "getBLEDataType") == 0) {
        return METHOD_GET_BLE_DATA;
    } else if (strcmp(method->valuestring, "getSharedAttributes") == 0) {
        return METHOD_GET_SHARED_ATTRIBUTES;
    } else {
        return METHOD_NOT_FOUND;
    }
}


void replace_request_with_response(char* input, char* output) {
    char* request_part = strstr(input, "request");
    if (request_part != NULL) {
        int position = request_part - input;
        strncpy(output, input, position);
        sprintf(output + position, "response%s", request_part + strlen("request"));
    } else {
        strcpy(output, input);
    }
}

void change_fmode_handler(char* data){
    cJSON *root = cJSON_Parse(data);
    cJSON *params = cJSON_GetObjectItem(root, SA_FMODE);
    on_receive_attr_fmode(params->valueint);
    cJSON_Delete(root);
}

void _change_nfc_content_from_tb(char *data){
    cJSON *root = cJSON_Parse(data);
    cJSON *params = cJSON_GetObjectItem(root, SA_NFC_CONTENT);
    cJSON *sub_content_array = cJSON_GetObjectItem(params, "content");
    if (!cJSON_IsArray(sub_content_array)) {
        cJSON_Delete(root);
        return;
    }
    char *sub_content_string = cJSON_Print(sub_content_array);
    if (sub_content_string == NULL) {
        cJSON_Delete(root);
        return;
    }
    printf("Sub-content array string: %s\n", sub_content_string);
    on_receive_attr_nfc_content(sub_content_string);
    cJSON_Delete(root);
}

void _change_nfc_content_from_nvs(char *data){
    
    printf("Sub-content array string: %s\n", data);
    on_receive_attr_nfc_content(data);
}


void change_nfc_content_handler(char* data, int from){
   
    switch (from) {
    case FROM_THINGBOARD:
        _change_nfc_content_from_tb(data);
        break;
    case FROM_NVS:
        //If its from NVS, data is ignored and the content is read from NVS
        char *content = malloc(255);
        nvs_read_string("nfc_content", &content);
        _change_nfc_content_from_nvs(content);
        free(content);
        break;
    default:
        ESP_LOGE(TAG, "Unknown source %d", from);
        break;
    }
   
}

void change_writepwd_handler(char* data){
    cJSON *root = cJSON_Parse(data);
    cJSON *item = cJSON_GetObjectItem(root, SA_WRITEPWD);
    if (cJSON_IsString(item) && (item->valuestring != NULL)) {
        printf("The value for key '%s' is: %s\n", SA_WRITEPWD, item->valuestring);
        
        on_receive_attr_writepwd((uint8_t *)item->valuestring);
    } else {
        printf("Key '%s' not found or not a string\n", SA_WRITEPWD);
    }
    cJSON_Delete(root);
}


/*
void change_pathloss_handler(char* data){
    cJSON *root = cJSON_Parse(data);
    cJSON *params = cJSON_GetObjectItem(root, SA_PATHLOSS);
    set_pathloss(params->valuedouble);
    cJSON_Delete(root);
}

void get_ble_data_handler(char *topic){
    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "status", get_ble_data_mode());
    char *str = cJSON_Print(root);

    char *response_topic = malloc(strlen(topic) + 1);
    replace_request_with_response(topic, response_topic);
    send_data_to_topic(response_topic, str, strlen(str));
    cJSON_Delete(root);
}

void change_ble_data_handler(char* topic, char* data){
    cJSON *root = cJSON_Parse(data);
    cJSON *params = cJSON_GetObjectItem(root, "params");
    set_ble_data_mode(params->valuestring);
    cJSON_Delete(root);

    //Send response
    cJSON *answer = cJSON_CreateObject();
    cJSON_AddBoolToObject(answer, "status", cJSON_True);
    char *str = cJSON_Print(answer);

    char *response_topic = malloc(strlen(topic) + 1);
    replace_request_with_response(topic, response_topic);
    send_data_to_topic(response_topic, str, strlen(str));
    cJSON_Delete(answer);
    free(response_topic);
}
*/