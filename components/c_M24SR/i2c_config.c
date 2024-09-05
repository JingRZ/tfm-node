#include <stdio.h>
#include "i2c_config.h"


#define WORD_LEN 2           /**< 2 bytes per word */

static uint8_t DEVICE_ADDR = 0x56;
static const char *TAG = "i2c_config";



esp_err_t read_response(m24sr_dev_t *device, uint16_t delay, uint8_t *read_data, uint8_t read_len) {

    uint8_t reply_len = read_len * (WORD_LEN + 1);
    uint8_t reply_buffer[reply_len];

    vTaskDelay(delay / portTICK_PERIOD_MS);

    // Tries to read device reply
    esp_err_t err = device->i2c_read(device->i2c_num, reply_buffer, reply_len, device->intf_ptr);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read m24sr I2C command reply! err: 0x%02x", err);
        return err; 
    }

    for (uint8_t i = 0; i < read_len; i++) {
        read_data[i] = reply_buffer[i];
        ESP_LOGI(TAG, "%s - Read data: %02x", __FUNCTION__, read_data[i]);
    }
    ESP_LOGI(TAG, "*******************");

    return ESP_OK;
}


esp_err_t m24sr_execute_command(m24sr_dev_t *device, uint8_t command[], uint8_t command_len, uint16_t delay, uint8_t *read_data, uint8_t read_len) {

    esp_err_t err;

    // Writes m24sr Command
    err = device->i2c_write(device->i2c_num, command, command_len, device->intf_ptr);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write m24sr I2C command! err: 0x%02x", err);
        return err;  
    }

    // Waits for device to process command and measure desired value
    vTaskDelay(delay / portTICK_PERIOD_MS);

    // Checks if there is data to be read from the user, (or if it's just a simple command write)
    if (read_len == 0) {
        return ESP_OK;
    }

    uint8_t reply_len = read_len * (WORD_LEN + 1);
    uint8_t reply_buffer[reply_len];

    // Tries to read device reply
    err = device->i2c_read(device->i2c_num, reply_buffer, reply_len, device->intf_ptr);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read m24sr I2C command reply! err: 0x%02x", err);
        return err;  // failed to read reply buffer from chip
    }

    // Calculates expected CRC and compares it with the response
    for (uint8_t i = 0; i < read_len; i++) {

        // If CRCs are equal, save data
        read_data[i] = reply_buffer[i];

        ESP_LOGI(TAG, "%s - Read data: %02x", __FUNCTION__, read_data[i]);
    }
    ESP_LOGI(TAG, "----------------------");

    return ESP_OK;
}

void m24sr_init(m24sr_dev_t *sensor, m24sr_read_fptr_t user_i2c_read, m24sr_write_fptr_t user_i2c_write, i2c_port_t i2c_num) {
    sensor->intf_ptr = &DEVICE_ADDR; 
    sensor->i2c_read = user_i2c_read;
    sensor->i2c_write = user_i2c_write;
    sensor->i2c_num = i2c_num;
}






esp_err_t i2c_master_driver_initialize(int i2c_master_port_p, int i2c_master_sda_io, int i2c_master_scl_io, int i2c_master_freq_hz, int i2c_master_rx_buf_disable, int i2c_master_tx_buf_disable)
{
    int i2c_master_port = i2c_master_port_p;
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = i2c_master_sda_io,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = i2c_master_scl_io,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = i2c_master_freq_hz
    };
    
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        return err;
    }

    return i2c_driver_install(i2c_master_port, conf.mode, i2c_master_rx_buf_disable, i2c_master_tx_buf_disable, 0);
}


esp_err_t readResponseBytes(const i2c_port_t i2c_num, uint8_t *output, const size_t nbytes, void *intf_ptr){

    uint8_t chip_addr = *(uint8_t*)intf_ptr;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    // write the 7-bit address of the sensor to the queue, using the last bit
    // to indicate we are performing a read.
    i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_READ, ACK_CHECK_EN);

    // read nbytes number of bytes from the response into the buffer. make
    // sure we send a NACK with the final byte rather than an ACK.
    for (size_t i = 0; i < nbytes; i++)
    {
        i2c_master_read_byte(cmd, &output[i], i == nbytes - 1
                                              ? NACK_VAL
                                              : ACK_VAL);
    }

    // send all queued commands, blocking until all commands have been sent.
    // note that this is *not* thread-safe.
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

esp_err_t writeCommandBytes(const i2c_port_t i2c_num, const uint8_t *i2c_command, const size_t nbytes, void *intf_ptr){

    uint8_t chip_addr = *(uint8_t*)intf_ptr;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    // write the 7-bit address of the sensor to the bus, using the last bit to
    // indicate we are performing a write.
    i2c_master_write_byte(cmd, chip_addr << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);

    for (size_t i = 0; i < nbytes; i++)
        i2c_master_write_byte(cmd, i2c_command[i], ACK_CHECK_EN);

    // send all queued commands, blocking until all commands have been sent.
    // note that this is *not* thread-safe.
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}