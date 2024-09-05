
#include "context.h"
#include "esp_log.h"

int NODE_STATE = NODE_STATE_BLANK;
pthread_mutex_t NODE_STATE_MUTEX = PTHREAD_MUTEX_INITIALIZER;

ESP_EVENT_DEFINE_BASE(CONTEXT_EVENT_BASE);


void context_set_state(int state){
    pthread_mutex_lock(&NODE_STATE_MUTEX);
    NODE_STATE = state;
    pthread_mutex_unlock(&NODE_STATE_MUTEX);
}

int context_get_state(){
    pthread_mutex_lock(&NODE_STATE_MUTEX);
    int state = NODE_STATE;
    pthread_mutex_unlock(&NODE_STATE_MUTEX);
    return state;
}


void ctx_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {
    
    switch(id){
        case CTX_BLANK:
            ESP_LOGI("CTX_EVENT_HANDLER", "CTX_BLANK");
            context_set_state(NODE_STATE_BLANK);
            break;
        case CTX_PROV:
            ESP_LOGI("CTX_EVENT_HANDLER", "CTX_PROV");
            context_set_state(NODE_STATE_PROV);
            break;
        case CTX_VALID_WIFI_CRED:
            ESP_LOGI("CTX_EVENT_HANDLER", "CTX_VALID_WIFI_CRED");
            context_set_state(NODE_STATE_VALID_WIFI_CRED);
            break;
        case CTX_VALID_TB_TOKEN:
            ESP_LOGI("CTX_EVENT_HANDLER", "CTX_VALID_TB_TOKEN");
            context_set_state(NODE_STATE_VALID_TB_TOKEN);
            break;
        case CTX_WAIT_CTX:
            ESP_LOGI("CTX_EVENT_HANDLER", "CTX_WAIT_CTX");
            context_set_state(NODE_STATE_WAIT_CTX);
            break;
        case CTX_REGULAR:
            ESP_LOGI("CTX_EVENT_HANDLER", "CTX_REGULAR");
            context_set_state(NODE_STATE_REGULAR);
            break;
    }
}