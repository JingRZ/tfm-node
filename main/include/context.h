#ifndef __CONTEXT_H
#define __CONTEXT_H

#include <pthread.h>
#include "esp_event.h"

enum {
    CONTEXT_OFF = 0,
    CONTEXT_ON = 1
};

enum NODE_STATE_ENUM{
    NODE_STATE_BLANK = 0,                   //Factory reset (no provisioning) 

    /**
     * NODE_STATE_PROV: Waiting for provisioning:
     *      - Wifi's SSID and pwd 
     *      - Access token to thingsboard
    */
    NODE_STATE_PROV = 1,               

    /**
     * Has valid wifi ssid & pwd
    */
    NODE_STATE_VALID_WIFI_CRED,

    /**
     * Has valid wifi credentials and TB Access token
    */
    NODE_STATE_VALID_TB_TOKEN,

    /**
     * Waiting to be provided of the node's context data
    */
    NODE_STATE_WAIT_CTX,

    /**
     * NODE_STATE_REGULAR: Has WiFi connection, access token and context
    */
    NODE_STATE_REGULAR,

    /**
     * NODE_STATE_REGULAR: We've got Wifi credentials & access token, but Wifi has suddenly disconnected
    */
    NODE_STATE_WIFI_DISCONNECTED,

    /**
     * NODE_STATE_INVALID_DATA: If any of the provisioned data is invalid
    */
    NODE_STATE_INVALID_DATA,

};

ESP_EVENT_DECLARE_BASE(CONTEXT_EVENT_BASE);

enum {
    CTX_BLANK,
    CTX_PROV,
    CTX_VALID_WIFI_CRED,
    CTX_VALID_TB_TOKEN,
    CTX_WAIT_CTX,
    CTX_REGULAR,
};

extern int NODE_STATE;
extern pthread_mutex_t NODE_STATE_MUTEX;

//void context_set_state(int state);
//int context_get_state();
void ctx_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);

#endif