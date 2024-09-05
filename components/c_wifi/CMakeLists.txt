idf_component_register(SRCS "c_wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi nvs_flash wifi_provisioning)
