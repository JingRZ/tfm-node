#ifndef METHOD_PARSER_H
#define METHOD_PARSER_H

enum {
    METHOD_NOT_FOUND,
    METHOD_SET_BLE_DATA,
    METHOD_GET_BLE_DATA,
    METHOD_GET_SHARED_ATTRIBUTES
};

enum {
    ATTRIBUTE_PATHLOSS,
    ATTRIBUTE_FMODE,
    ATTRIBUTE_NFC_CONTENT,
    ATTRIBUTE_BLE_CONTENT,
    ATTRIBUTE_WRITEPWD,
    ATTRIBUTE_OTA,
    ATTRIBUTE_NOT_FOUND
};

enum {
    IS_METHOD,
    IS_ATTRIBUTE,
    IS_NOTHING
};

enum {
    FROM_THINGBOARD,
    FROM_NVS
};

int parse_method(char* data);
int parse_attribute(char* data);
void change_ble_data_handler(char* topic, char* data);
void change_pathloss_handler(char* data);
int parse_msg_type(char *topic, char* data);
void get_ble_data_handler(char *topic);

void change_fmode_handler(char* data);
void change_nfc_content_handler(char* data, int from);
void change_writepwd_handler(char* data);

#endif