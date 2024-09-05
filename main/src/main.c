#include <stdio.h>
#include <esp_event.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "init.h"
#include "factory_reset.h"
#include "ota_handler.h"

void neverending_task(){
    while (1) vTaskDelay(1000 / portTICK_PERIOD_MS);
}


void app_main(void) {
    ota_check();
    init_factory_reset_button(5);
    setup();

    fflush(stdout);
    neverending_task();
}
