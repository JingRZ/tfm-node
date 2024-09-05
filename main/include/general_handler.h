#ifndef GENERAL_HANDLER_H
#define GENERAL_HANDLER_H

void on_wifi_connected_handler();
void send_data_to_topic(char* topic, char* data, int dataLen);
void on_receive_attr_fmode(int fmode);
void on_receive_attr_nfc_content(const char* buf);
void on_receive_attr_writepwd(uint8_t* buf);
#endif