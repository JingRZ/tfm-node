#include <stdio.h>
#include "esp_log.h"
#include "iot_button.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "factory_reset.h"
#include "c_nvs.h"

#define BOOT_BUTTON_NUM         9
#define BUTTON_ACTIVE_LEVEL     0

static uint32_t start_time = 0;
static int hold_time = 0;
static const char *TAG = "FACTORY_RESET_BUTTON";

enum {
    BTN_PRESS_DOWN,
    BTN_PRESS_UP,
    BTN_PRESS_REPEAT,
    BTN_PRESS_REPEAT_DONE,
    BTN_SINGLE_CLICK,
    BTN_DOUBLE_CLICK,
    BTN_MULTIPLE_CLICK,
    BTN_LONG_PRESS_START,
    BTN_LONG_PRESS_HOLD,
    BTN_LONG_PRESS_UP,
    BTN_EVENT_COUNT  // Total number of button events
};

const char *button_event_table[BTN_EVENT_COUNT] = {
    "BTN_PRESS_DOWN",
    "BTN_PRESS_UP",
    "BTN_PRESS_REPEAT",
    "BTN_PRESS_REPEAT_DONE",
    "BTN_SINGLE_CLICK",
    "BTN_DOUBLE_CLICK",
    "BTN_MULTIPLE_CLICK",
    "BTN_LONG_PRESS_START",
    "BTN_LONG_PRESS_HOLD",
    "BTN_LONG_PRESS_UP",
};


static void button_event_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Button event %s", button_event_table[(button_event_t)data]);

    switch ((button_event_t)data)
    {
        case BTN_LONG_PRESS_START:
            start_time = esp_timer_get_time();
        break;

        case BTN_LONG_PRESS_UP:
            uint32_t end_time = esp_timer_get_time();
            uint32_t elapsed_time_ms = (end_time - start_time) / 1000;
            ESP_LOGI("Button Timer", "Button pressed for %ld milliseconds", elapsed_time_ms);
            if(elapsed_time_ms >=  hold_time * 1000 ) {
                nvs_delete_all();
                esp_restart();
            }
        break;
        
        default:
        break;
    }
    
}


void button_init(uint32_t button_num)
{
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = button_num,
            .active_level = BUTTON_ACTIVE_LEVEL,
        },
    };

    button_handle_t btn = iot_button_create(&btn_cfg);
    assert(btn);
    esp_err_t err = iot_button_register_cb(btn, BUTTON_PRESS_DOWN, button_event_cb, (void *)BUTTON_PRESS_DOWN);
    err |= iot_button_register_cb(btn, BUTTON_PRESS_UP, button_event_cb, (void *)BUTTON_PRESS_UP);
    err |= iot_button_register_cb(btn, BUTTON_PRESS_REPEAT, button_event_cb, (void *)BUTTON_PRESS_REPEAT);
    err |= iot_button_register_cb(btn, BUTTON_PRESS_REPEAT_DONE, button_event_cb, (void *)BUTTON_PRESS_REPEAT_DONE);
    err |= iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, button_event_cb, (void *)BUTTON_SINGLE_CLICK);
    err |= iot_button_register_cb(btn, BUTTON_DOUBLE_CLICK, button_event_cb, (void *)BUTTON_DOUBLE_CLICK);
    err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_START, button_event_cb, (void *)BUTTON_LONG_PRESS_START);
    err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_HOLD, button_event_cb, (void *)BUTTON_LONG_PRESS_HOLD);
    err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_UP, button_event_cb, (void *)BUTTON_LONG_PRESS_UP);
    ESP_ERROR_CHECK(err);
}

void init_factory_reset_button(int hold_time_to_reset) {
    hold_time = hold_time_to_reset;
    button_init(BOOT_BUTTON_NUM);
}