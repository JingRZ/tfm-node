#ifndef __I2C_H
#define __I2C_H

#include "esp_log.h"
#include "driver/i2c.h"


#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define I2C_MASTER_NUM I2C_NUMBER(0)
#define I2C_MASTER_FREQ_HZ 400000

#define I2C_MASTER_TX_BUF_DISABLE 0         /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0         /*!< I2C master doesn't need buffer */



#define I2C_TIMEOUT_MS              1000

#define ACK_CHECK_EN                0x1
#define ACK_CHECK_DIS               0x0

#define ACK_VAL                     0x0
#define NACK_VAL                    0x1

#define I2C_MASTER_SCL_IO 23               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 22               /*!< gpio number for I2C master data  */



/*** I2C Driver Function Pointers ***/
typedef esp_err_t (*m24sr_read_fptr_t)(const i2c_port_t i2c_num, uint8_t *output, const size_t nbytes, void *intf_ptr);

typedef esp_err_t (*m24sr_write_fptr_t)(const i2c_port_t i2c_num, const uint8_t *i2c_command, const size_t nbytes, void *intf_ptr);

/** m24sr Main Data Struct */
typedef struct m24sr_dev {
    /**< The 48-bit serial number, this value is set when you call m24sr_init */
    uint16_t serial_number[3];

    /**< Interface pointer, used to store I2C address of the device */
    void *intf_ptr;

    /**< I2C read driver function pointer */
    m24sr_read_fptr_t i2c_read;

    /**< I2C write driver function pointer */
    m24sr_write_fptr_t i2c_write;

    i2c_port_t i2c_num;
} m24sr_dev_t;


esp_err_t i2c_master_driver_initialize(int i2c_master_port, int i2c_master_sda_io, int i2c_master_scl_io, int i2c_master_freq_hz, int i2c_master_rx_buf_disable, int i2c_master_tx_buf_disable);

esp_err_t readResponseBytes(const i2c_port_t i2c_num, uint8_t *output, const size_t nbytes, void *intf_ptr);

esp_err_t writeCommandBytes(const i2c_port_t i2c_num, const uint8_t *i2c_command, const size_t nbytes, void *intf_ptr);

//esp_err_t m24sr_execute_command(m24sr_dev_t *device, uint8_t command[], uint8_t command_len, uint16_t delay, uint16_t *read_data, uint8_t read_len);
esp_err_t m24sr_execute_command(m24sr_dev_t *device, uint8_t command[], uint8_t command_len, uint16_t delay, uint8_t *read_data, uint8_t read_len);

void m24sr_init(m24sr_dev_t *sensor, m24sr_read_fptr_t user_i2c_read, m24sr_write_fptr_t user_i2c_write, i2c_port_t i2c_num);
esp_err_t read_response(m24sr_dev_t *device, uint16_t delay, uint8_t *read_data, uint8_t read_len);
#endif