#include "c_M24SR.h"
#include "i2c_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "M24SR_def.h"
#include "m24sr_handler.h"
#include "Message.h"
#include "RecordText.h"
#include "c_nvs.h"
#include "cJSON.h"
#include "esp_log.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define CONTROL_NBYTES 3

enum {
    FMODE_NONE=-1,
    FMODE_I2C,
    FMODE_RF
};

enum {
    BLANK,
    I2C_STARTED,
    GOT_I2C_TOKEN,
    SELECTED_APP,
    SELECTED_CC,
    SELECTED_NDEF
};

static uint8_t status = BLANK;
static uint8_t fmode = FMODE_NONE;
static const char *TAG = "M24SR_HANDLER";

Status_t m24sr_init_i2c(){
    status = BLANK;
    esp_err_t err =  c_M24SR_i2c_init();
    if(err != ESP_OK)
        return STATUS_ERROR;

    status = I2C_STARTED;

    return STATUS_SUCCESS;
}

Status_t m24sr_release_i2c(){
    esp_err_t err =  c_M24SR_sendDeselect();
    if(err != ESP_OK)
        return STATUS_ERROR;

    status = BLANK;

    return STATUS_SUCCESS;
}

Status_t _selectNDEFTagApplication(){
    if(status != BLANK){
        if(c_M24SR_sendSelectApplication() == M24SR_SUCCESS){
            status = SELECTED_APP;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_ERROR;
}


Status_t _getI2CToken(){
    if(status == I2C_STARTED){
        if(c_M24SR_forceI2CSession() == M24SR_SUCCESS){
            status = GOT_I2C_TOKEN;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_ERROR;
}

void _status_check(){
    if(status == BLANK){
        m24sr_init_i2c();
    }
    if(status == I2C_STARTED){
        _getI2CToken();
    }
    if(status == GOT_I2C_TOKEN){
        _selectNDEFTagApplication();
    }
}


void _selectCCfile(){
    _status_check();

    if(status > I2C_STARTED){
        if(c_M24SR_sendSelectCCfile() == M24SR_SUCCESS){
            status = SELECTED_CC;
        }
    }
}

void _selectNDEFfile(){
    _status_check();

    if(status > I2C_STARTED){
        if(c_M24SR_sendSelectNDEFfile(NDEF_FILE_ID) == M24SR_SUCCESS){
            status = SELECTED_NDEF;
        }
    }
}




void _printHex(uint8_t *buf, uint8_t len){

    for(int i=0; i<=len; i++){
        //printf("0x%02X ", buf[i]);
        printf("%c", buf[i]);
    }
    printf("\n");
}

Status_t m24sr_check_rw_rights(){
    if(status != SELECTED_CC) {
        _selectCCfile();
    }

    if(status > I2C_STARTED){
        int readnBytes = 2;
        uint8_t buf[readnBytes + CONTROL_NBYTES];

        if(c_M24SR_sendReadBinary(0x0000, readnBytes, &buf) == M24SR_SUCCESS){
            printf("Read access 0x%02X\n", buf[1]);
            printf("Write access 0x%02X\n", buf[2]);
            return STATUS_SUCCESS;
        }        
    }
    return STATUS_ERROR;
}


Status_t m24sr_checkNDEFSecurityStatus(){

    if(status != SELECTED_NDEF){
        _selectNDEFfile();
    }


    const uint8_t aux[] = {};
    if(c_M24SR_sendVerify(WRITE_PWD, 0x00, aux) == M24SR_SUCCESS){
        printf("Write access security status checked\n");
    }  

    if(c_M24SR_sendVerify(READ_PWD, 0x00, aux) == M24SR_SUCCESS){
        printf("Read access security status checked\n");
    }          
    
    return STATUS_SUCCESS;
}

Status_t m24sr_unlockRWaccess(){
    int completed = 0;

    if(status != SELECTED_NDEF){
        _selectNDEFfile();
    }

    if(c_M24SR_sendDisableVerificationRequirement(WRITE_PROTECTION) == M24SR_SUCCESS) {
        printf("Write access unlocked\n");
        completed++;
    }

    if(c_M24SR_sendDisableVerificationRequirement(READ_PROTECTION) == M24SR_SUCCESS) {
        printf("Read access unlocked\n");
        completed++;
    }
    
    return completed==2?STATUS_SUCCESS:STATUS_ERROR;
    
}


int m24sr_getNDEFfileContentSize() {

    if(status != SELECTED_NDEF){
        _selectNDEFfile();
    }

    int filesize = -1;

    if(status > I2C_STARTED){
        
        int readnBytes = 2;
        uint8_t buf[readnBytes + CONTROL_NBYTES];
        if(c_M24SR_sendReadBinary(0x0000, readnBytes, &buf) == M24SR_SUCCESS){
            filesize = (buf[1] << 8) | buf[2];
            printf("filesize = %d\n", filesize);
        }     
    }
    return filesize;
}


void _getData(uint8_t *src, uint8_t *dst, uint8_t nBytes){

    for(int i=1; i<=nBytes; i++){
        dst[i-1] = src[i];
    }
}


Status_t m24sr_readNDEFfile(uint16_t offset, uint8_t nBytes, uint8_t *readBuf) {
    if(offset < 0 || nBytes == NULL)
        return STATUS_ERROR;

    if(status != SELECTED_NDEF)
        _selectNDEFfile();
    
    if(status > I2C_STARTED){
        
        uint8_t buf[nBytes + CONTROL_NBYTES];

        if(c_M24SR_sendReadBinary(offset, nBytes, &buf) == M24SR_SUCCESS){
            _printHex(buf, nBytes);
            if(readBuf != NULL)
                _getData(buf, readBuf, nBytes);
            return STATUS_SUCCESS;
        } 
    }

    return STATUS_ERROR;
}


Status_t _checkWriteSuccessful(uint16_t offset, uint8_t* wBuffer, uint8_t wSize){
    int len = m24sr_getNDEFfileContentSize();

    if(len == wSize){
        uint8_t readBuf[len];
        if(m24sr_readNDEFfile(offset, len, readBuf) == STATUS_SUCCESS){

            int sizeA = wSize;
            int sizeB = len;

            int result = memcmp(wBuffer, readBuf, (sizeA < sizeB) ? sizeA : sizeB);
            if(result == 0){
                if(sizeA == sizeB){
                    return STATUS_SUCCESS;
                }
            }
        }
    }
    return STATUS_ERROR;
}

Status_t m24sr_enableWritePwd(uint8_t *current_pwd, uint8_t *new_pwd){
    if(status != SELECTED_NDEF){
        _selectNDEFfile();
        printf("Selected NDEF file\n");
    }

    if(status > I2C_STARTED){
       
        if(c_M24SR_sendVerify(WRITE_PWD, 0x10, current_pwd) == M24SR_SUCCESS){
            printf("Verify OK\n");
            if(c_M24SR_sendEnableVerificationRequirement(WRITE_PWD) == M24SR_SUCCESS){
                printf("Enable Verification Requirements OK\n");
            }

            if(c_M24SR_sendChangeReferenceData(WRITE_PWD, new_pwd) == M24SR_SUCCESS){
                printf("Change Reference Data OK\n");
            }

            return STATUS_SUCCESS;
        }  
    }
    return STATUS_ERROR;
}



Status_t m24sr_disableWritePwd(uint8_t *current_pwd){
    if(status != SELECTED_NDEF){
        _selectNDEFfile();
        printf("Selected NDEF file\n");
    }

    if(status > I2C_STARTED){
       
        if(c_M24SR_sendVerify(WRITE_PWD, 0x10, current_pwd) == M24SR_SUCCESS){
            printf("Verify OK\n");
            if(c_M24SR_sendDisableVerificationRequirement(WRITE_PROTECTION) == M24SR_SUCCESS) {
                printf("Write access unlocked\n");
                return STATUS_SUCCESS;
            }
        }  
    }
    return STATUS_ERROR;
}


Status_t m24sr_writeNDEFfile(uint16_t offset, uint8_t* writeBuffer, uint8_t nBytes, int checkSuccess){
    if(offset < 0 || writeBuffer == NULL)
        return STATUS_ERROR;

    if(status != SELECTED_NDEF)
        _selectNDEFfile();

    if(status > I2C_STARTED){
        uint8_t *pwd = malloc(sizeof(char) * 20);
        if (nvs_read_string(WRITE_PWD_KEY, &pwd) != NVS_OK) {
            //Si no hay pwd guardado, se usa el default
            strncpy((char *)pwd, (char *)DEFAULT_PWD_16B, 16);
        }

        if(c_M24SR_sendVerify(WRITE_PWD, 0x10, pwd) == M24SR_SUCCESS) {
            if(c_M24SR_sendUpdateBinary(offset, nBytes, writeBuffer) == M24SR_SUCCESS) {
                if(checkSuccess){
                    printf("Checking if write successful\n");
                    return _checkWriteSuccessful(offset, writeBuffer, nBytes);
                }
                free(pwd);
                return STATUS_SUCCESS;
            } 
        }
        free(pwd);
    }
    return STATUS_ERROR;
}

Status_t m24sr_set_fmode(uint8_t mode){

    if (mode > 0) {
        if(status != BLANK)
            m24sr_release_i2c();
    } else if (mode < 0) {
        return STATUS_ERROR;
    }
    
    fmode = mode;
    return STATUS_SUCCESS;
}


Status_t m24sr_set_content(const char* buf){

    if(m24sr_init_i2c() == STATUS_SUCCESS){
        Message_t msg;
        Message_ctor(&msg, 3);

        RecordText_t record;
        RecordText_ctor(&record);

        record_text_set_text(&record, (const char *)buf);
        message_add_text_record(&msg, record);

        uint8_t buffer[500];
        uint16_t length = message_write(msg, buffer);

        if(m24sr_writeNDEFfile(0x0000, buffer, length, 0) == STATUS_SUCCESS){
            m24sr_getNDEFfileContentSize();
            printf("------Correctly written-------\n");
        }
        else{
            printf("------Error writing-------\n");
            return STATUS_ERROR;
        }
    }
    else{
        return STATUS_ERROR;
    }
    fflush(stdout);
    m24sr_release_i2c();
    return STATUS_SUCCESS;
}


void m24sr_set_writepwd(uint8_t *pwd){
    /*
    if(pwd != NULL) {
        char* prev_pwd = malloc(sizeof(char) * 20);
        nvs_read_string(WRITE_PWD_KEY, &prev_pwd);
        if(!prev_pwd){
            strncpy((char *)prev_pwd, (char *)DEFAULT_PWD_16B, 16);
        }

        if(m24sr_enableWritePwd(prev_pwd, pwd) == STATUS_SUCCESS){
            ESP_LOGI(TAG, "Write password set");
            nvs_write_string(WRITE_PWD_KEY, pwd);

        } else {
            ESP_LOGE(TAG, "Failed to set write password");
        }

    } else {
        ESP_LOGE(TAG, "writepwd buf is NULL");
    }*/
}


bool m24sr_parse_context(char* data){
    cJSON* root = cJSON_Parse(data);
    if (root != NULL){
        cJSON *tag = cJSON_GetObjectItem(root, "tag");
        cJSON *writepwd = cJSON_GetObjectItem(tag, WRITE_PWD_KEY);

        if (writepwd != NULL){
            if(nvs_write_string(WRITE_PWD_KEY, writepwd->valuestring) != NVS_OK){
                return false;
            }

            if(m24sr_enableWritePwd(DEFAULT_PWD_16B, (uint8_t *)writepwd->valuestring) == STATUS_SUCCESS){
                ESP_LOGI(TAG, "Write password set");
            } else {
                ESP_LOGE(TAG, "Failed to set write password");
            }
        }
        else{
            return false;
        }
    }
    cJSON_Delete(root);
    return true;
}