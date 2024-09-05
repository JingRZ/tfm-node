idf_component_register(
    SRCS 
        "c_provisioning.c"
    INCLUDE_DIRS "include"
    REQUIRES
        esp_wifi wifi_provisioning
)
