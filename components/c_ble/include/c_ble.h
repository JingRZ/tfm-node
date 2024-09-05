#ifndef C_BLE_H
#define C_BLE_H

enum {
    C_BLE_OK,
    C_BLE_ERR
};


/**
 * @brief Initialize the Bluetooth Low Energy (BLE) module.
 *
 * It sets up the necessary parameters and starts any required services.
 * It should be called at the beginning of the program before any BLE operations.
 */
int ble_init(void (*set_ble_name)(char*), void (*set_ble_content)(char*));


#endif // C_BLE_H