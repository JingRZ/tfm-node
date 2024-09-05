#include <stdio.h>
#include <stdint.h>
#include <string.h> // memcpy
#include "esp_log.h"
#include "driver/i2c.h"

#include "c_M24SR.h"
#include "M24SR_def.h"
#include "i2c_config.h"
#include "cmdFactory.h"


#define M24SR_MAX_I2C_ACCESS_TRY 1000
#define M24SR_MAX_BYTE_OPERATION_LENGHT (246)

static M24SR_Status_t _receiveUpdateBinary(void);
static uint8_t DEVICE_ADDR = M24SR_I2C_ADDR;

uint8_t mM24SRbuffer[0xFF];//max command length is 255
M24SR_Command_t mLastCommandSend;
M24SR_command_data_t mLastCommandData;

static const char *TAG = "c_M24SR";


m24sr_dev_t device;

esp_err_t c_M24SR_i2c_init(){

    i2c_master_driver_initialize(I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE);

    m24sr_init(&device, (m24sr_read_fptr_t)readResponseBytes, (m24sr_write_fptr_t)writeCommandBytes, I2C_MASTER_NUM);

    return ESP_OK;
}



esp_err_t _writeCommandBytes(const uint8_t *i2c_command, const size_t nbytes) {

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    // write the 7-bit address of the sensor to the bus, using the last bit to
    // indicate we are performing a write.
    i2c_master_write_byte(cmd, DEVICE_ADDR << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);

    for (size_t i = 0; i < nbytes; i++)
        i2c_master_write_byte(cmd, i2c_command[i], ACK_CHECK_EN);

    // send all queued commands, blocking until all commands have been sent.
    // note that this is *not* thread-safe.
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}


esp_err_t _readResponseBytes(uint8_t *output, const size_t nbytes) {

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    // write the 7-bit address of the sensor to the queue, using the last bit
    // to indicate we are performing a read.
    i2c_master_write_byte(cmd, DEVICE_ADDR << 1 | I2C_MASTER_READ, ACK_CHECK_EN);
    // read nbytes number of bytes from the response into the buffer. make
    // sure we send a NACK with the final byte rather than an ACK.

    //i2c_master_read(cmd, output, nbytes, I2C_MASTER_LAST_NACK);

    
    for (size_t i = 0; i < nbytes; i++)
    {
        i2c_master_read_byte(cmd, &output[i], i == nbytes - 1
                                              ? NACK_VAL
                                              : ACK_VAL);
    }

    // send all queued commands, blocking until all commands have been sent.
    // note that this is *not* thread-safe.
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}


/**
 * @brief  Updates the CRC
 */
static uint16_t c_M24SR_updateCrc(uint8_t ch, uint16_t *lpwCrc) {

    ch = (ch ^ (uint8_t) ((*lpwCrc) & 0x00FF));
    ch = (ch ^ (ch << 4));

    *lpwCrc = (*lpwCrc >> 8) ^ ((uint16_t) ch << 8) ^ ((uint16_t) ch << 3) ^ ((uint16_t) ch >> 4);

    return (*lpwCrc);
}


/**
 * @brief  This function returns the CRC 16
 * @param  Data : pointer on the data used to compute the CRC16
 * @param  Length : number of bytes of the data
 * @retval CRC16
 */
static uint16_t c_M24SR_ComputeCrc(uint8_t *data, uint8_t len) {
    uint8_t chBlock;
    uint16_t wCrc = 0x6363; // ITU-V.41
    
    /** It uses CRC-A Algorithm 
     * Check	Poly	Init	RefIn	RefOut	XorOut
     * 0xBF05	0x1021	0xC6C6	true	true	0x0000
     * 
     * Shouldn't wCrC be 0xC6C6?
    */

    do {
        chBlock = *data++;
        c_M24SR_updateCrc(chBlock, &wCrc);
    } while (--len);

    return wCrc;
}



M24SR_Status_t _isCorrectCRC16Residue(uint8_t *data, uint8_t len){
    uint16_t resCRC = 0x0000;
    M24SR_Status_t status;

    if(len > 0){
        resCRC = c_M24SR_ComputeCrc(data, len);
    }
    
    if(resCRC == 0x0000){
        status = (M24SR_Status_t) (((data[len - UB_STATUS_OFFSET] << 8)
        & 0xFF00) | (data[len - LB_STATUS_OFFSET] & 0x00FF));
    } else {
        resCRC = 0x0000;
        resCRC = c_M24SR_ComputeCrc(data, 5);
        if (resCRC != 0x0000) {
            /* Bad CRC */
            return M24SR_IO_ERROR_CRC;
        } else {
            /* Good CRC, but error status from M24SR */
            status= (M24SR_Status_t) (((data[1] << 8) & 0xFF00)
                | (data[2] & 0x00FF));
        }
    }

    return status;
}


M24SR_Status_t _sendCommand(uint8_t *buffer, uint8_t nBytes){

    int nTry = 0;
    esp_err_t err = ESP_FAIL;

    while( err!=ESP_OK && (nTry++) < M24SR_MAX_I2C_ACCESS_TRY ) {
        err = _writeCommandBytes(buffer, sizeof(buffer));
        //err = writeCommandBytes(I2C_MASTER_NUM, buffer, sizeof(buffer), &DEVICE_ADDR);
        //err = i2c_master_write_to_device(I2C_MASTER_NUM, DEVICE_ADDR, buffer, sizeof(buffer), I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
    }

    if(err==ESP_OK)
        return M24SR_SUCCESS;
    
    return M24SR_IO_ERROR_I2CTIMEOUT;
}


M24SR_Status_t _receiveResponse(uint8_t *buffer, uint8_t nBytes){

    int nTry = 0;
    esp_err_t err = ESP_FAIL;
    
    while( err!=ESP_OK && (nTry++) < M24SR_MAX_I2C_ACCESS_TRY ) {
        err = _readResponseBytes(buffer, nBytes);
        //err = i2c_master_read_from_device(0, DEVICE_ADDR, buffer, nBytes, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
        if(err!=ESP_OK)
            vTaskDelay(1 / portTICK_PERIOD_MS);  //delay required to avoid an error
    }
    
    if (err != ESP_OK) {
        printf("I2C read error. Error code: 0x%x\n", err);
    } else{
        printf("buffer %s\n", buffer);
        return M24SR_SUCCESS;
    }
    
    return M24SR_IO_ERROR_I2CTIMEOUT;
}


/**
 * @brief    This function returns M24SR_STATUS_SUCCESS if the buf is an s-block
 * @param    buf    :  pointer to the data
 * @retval   M24SR_SUCCESS  :  the data is a S-Block
 * @retval   NFC_ERROR    :  the data is not a S-Block
 */
static M24SR_Status_t _isSBlock(uint8_t *buf) {

    if ((buf[M24SR_OFFSET_PCB] & M24SR_MASK_BLOCK) == M24SR_MASK_SBLOCK) {
        return M24SR_SUCCESS;
    } else {
        return M24SR_ERROR;
    }

}


/**
 * @brief     This functions creates an I block command according to the structures cmdStructure and cmd.
 * @param     cmd : structure which contains the field of the different parameters
 * @param     cmdStructure : structure of the command
 * @param     nBytes : number of bytes of the command
 * @param     res : pointer to the command created
 */
static void _buildIBlockCommand(uint16_t cmdStructure, C_APDU *cmd, uint8_t didByte, uint16_t *nBytes, uint8_t *res){
    uint16_t crc16;
    static uint8_t blockNumber = 0x01;

    (*nBytes) = 0;

    /* add PCD byte */
    if ((cmdStructure & M24SR_PCB_NEEDED) != 0) {
        /* toggle the block number */
        blockNumber = TOGGLE(blockNumber);
        res[(*nBytes)++] = 0x02 | blockNumber;
    }

    /* add the DID byte */
    if ((blockNumber & M24SR_DID_NEEDED) != 0) {
        res[(*nBytes)++] = didByte;
    }

    /* add the Class byte */
    if ((cmdStructure & M24SR_CLA_NEEDED) != 0) {
        res[(*nBytes)++] = cmd->header.CLA;
    }

    /* add the instruction byte byte */
    if ((cmdStructure & M24SR_INS_NEEDED) != 0) {
        res[(*nBytes)++] = cmd->header.INS;
    }
    
    /* add the Selection Mode byte */
    if ((cmdStructure & M24SR_P1_NEEDED) != 0) {
        res[(*nBytes)++] = cmd->header.P1;
    }

    /* add the Selection Mode byte */
    if ((cmdStructure & M24SR_P2_NEEDED) != 0) {
        res[(*nBytes)++] = cmd->header.P2;
    }

    /* add Data field lengthbyte */
    if ((cmdStructure & M24SR_LC_NEEDED) != 0) {
        res[(*nBytes)++] = cmd->body.LC;
    }

    /* add Data field  */
    if ((cmdStructure & M24SR_DATA_NEEDED) != 0) {
        memcpy(&(res[(*nBytes)]), cmd->body.data, cmd->body.LC);
        (*nBytes) += cmd->body.LC;
    }
    /* add Le field  */
    if ((cmdStructure & M24SR_LE_NEEDED) != 0) {
        res[(*nBytes)++] = cmd->body.LE;
    }
    /* add CRC field  */
    if ((cmdStructure & M24SR_CRC_NEEDED) != 0) {
        crc16 = c_M24SR_ComputeCrc(res, (uint8_t) (*nBytes));
        /* append the CRC16 */
        res[(*nBytes)++] = GET_LSB(crc16);
        res[(*nBytes)++] = GET_MSB(crc16);
    }

}


static M24SR_Status_t _receiveSelect(void){

    uint8_t buffer[M24SR_STATUSRESPONSE_NBYTE];
    mLastCommandSend = NONE;

    M24SR_Status_t status = _receiveResponse(buffer, sizeof(buffer));

    if(status!= M24SR_SUCCESS)
        return status;
    

    status = _isCorrectCRC16Residue(buffer, sizeof(buffer));

    return status;
}


M24SR_Status_t _checkCRC(uint8_t *data, uint8_t len){

    for(int i=0; i<len; i++){
        printf("%2X ", data[i]);
    }
    M24SR_Status_t status = _isCorrectCRC16Residue(data, len);

    if(status == M24SR_IO_ERROR_CRC){
        printf("M24SR_IO_ERROR_CRC \n");
        for(int i=0; i<len; i++){
            printf("0x%02X ", data[i]);
        }
        printf("\n");
    }

    return status;
}


int _checkCorrect(const uint8_t *buf, uint8_t len, uint8_t offset){

    if(len < 2){
        return 0;
    }

    int posA, posB;

    if(offset){
        posA = offset;
        posB = offset - 1;
    } else {
        posA = 2;
        posB = 1;
    }

    printf("0x%02X 0x%02X\n", buf[len - 2], buf[len - 1]);

    return (buf[len - posA] == 0x90) && (buf[len - posB] == 0x00);
}


M24SR_Status_t _check_SW1SW2(uint8_t *buf, uint8_t len, uint8_t offset){
    return _checkCorrect(buf, len, offset) ? M24SR_SUCCESS : M24SR_ERROR;
}


M24SR_Status_t c_M24SR_sendSelectApplication(void){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;
    build_SelectApplicationCMD(buf, &nByte);

    uint8_t buffer[3];
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 10, &buffer, sizeof(buffer));

    if(err!=ESP_OK)
        return M24SR_ERROR;

    uint8_t size = sizeof(buffer);

    return _check_SW1SW2(buffer, size, 0);
}



M24SR_Status_t c_M24SR_sendSelectCCfile(void){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;
    build_SelectCCfileCMD(buf, &nByte);

    uint8_t buffer[3];
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 10, &buffer, 3);
    mLastCommandSend = SELECT_CC_FILE;

    if(err!=ESP_OK)
        return M24SR_ERROR;

    uint8_t size = sizeof(buffer);
    return _check_SW1SW2(buffer, size, 0);
}


M24SR_Status_t c_M24SR_sendSelectSystemfile(void){
    
    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;
    
    build_SelectSystemfileCMD(buf, &nByte);

    uint8_t buffer[3];
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 10, &buffer, 3);
    mLastCommandSend = SELECT_SYSTEM_FILE;

    if(err!=ESP_OK)
        return M24SR_ERROR;

    uint8_t size = sizeof(buffer);
    return _check_SW1SW2(buffer, size, 0);
}


M24SR_Status_t c_M24SR_sendSelectNDEFfile(uint16_t fileID){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;

    build_SelectNDEFfileCMD(buf, &nByte, fileID);

    uint8_t buffer[3];
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 10, &buffer, 3);
    mLastCommandSend = SELECT_NDEF_FILE;

    if(err!=ESP_OK)
        return M24SR_ERROR;
    
    uint8_t size = sizeof(buffer);
    return _check_SW1SW2(buffer, size, 0);
    //return M24SR_SUCCESS;
}


M24SR_Status_t c_M24SR_sendDeselect(){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;

    build_DeselectCMD(buf, &nByte);

    uint8_t buffer[3];
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 10, &buffer, 3);

    if(err!=ESP_OK)
        return M24SR_ERROR;
    
    uint8_t size = sizeof(buffer);
    return _check_SW1SW2(buffer, size, 0);
}


M24SR_Status_t c_M24SR_sendReadBinary(uint16_t offset, uint8_t nBytes2Read, uint8_t **bufRead){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;

    build_ReadBinaryCMD(buf, &nByte, offset, nBytes2Read);

    uint8_t totalSize = nBytes2Read + 3;
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 10, bufRead, totalSize);

    if(err!=ESP_OK)
        return M24SR_ERROR;

    return _check_SW1SW2(bufRead, totalSize, 0);
}


M24SR_Status_t c_M24SR_sendFWTExtension(uint8_t fwtByte){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;
    build_FWTExtensionCMD(buf, &nByte, fwtByte);

    uint8_t resBuf[3];
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 100, resBuf, 3);
    mLastCommandSend = UPDATE;

    if(err!=ESP_OK)
        return M24SR_ERROR;

    return M24SR_SUCCESS;
}


static M24SR_Status_t _receiveUpdateBinary(void){

    uint8_t buf[M24SR_STATUSRESPONSE_NBYTE];

    M24SR_Status_t status;
    const uint16_t length = mLastCommandData.length;
    const uint16_t offset = mLastCommandData.offset;
    uint8_t *data = mLastCommandData.data;

    mLastCommandSend = NONE;

    status = _receiveResponse(buf, sizeof(buf));

    if(status!= M24SR_SUCCESS)
        return status;

    if(_isSBlock(buf) == M24SR_SUCCESS) {
        status = _isCorrectCRC16Residue(buf, M24SR_WATINGTIMEEXTRESPONSE_NBYTE);
        /*
        if(status == M24SR_SUCCESS){
            // Send FrameExtension Response
            status = c_M24SR_sendFWTExtension( buf[M24SR_OFFSET_PCB] + 1 );
        }*/
    } else {
        status = _isCorrectCRC16Residue(buf, M24SR_STATUSRESPONSE_NBYTE);
    }

    return status;
}


M24SR_Status_t c_M24SR_sendUpdateBinary(uint16_t offset, uint8_t nBytes2Write, uint8_t *bufWrite){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;

    build_UpdateBinaryCMD(buf, &nByte, offset, nBytes2Write, bufWrite);

    uint8_t buffer[3];
    uint8_t size = sizeof(buffer);
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 30, buffer, size);
    mLastCommandSend = UPDATE;

    if(_isSBlock(buffer) == M24SR_SUCCESS) {
        uint8_t n = buffer[M24SR_OFFSET_PCB + 1];
        c_M24SR_sendFWTExtension(n);

        int ms_requested = n * 10;
        esp_err_t err = read_response(&device, ms_requested, buffer, size);
    } 

    if(err!=ESP_OK)
        return M24SR_ERROR;

    return _check_SW1SW2(buffer, size, 0);
}

/**
 * convert  a uint16 into an enum value
 * @param type
 * @return
 */
PasswordType_t constToPasswordType(const uint16_t type) {
    switch(type){
        case READ_PWD:
            return ReadPwd;
        case WRITE_PWD:
            return WritePwd;
        case I2C_PWD:
        default:
            return I2CPwd;
    }
}


M24SR_Status_t c_M24SR_sendVerify(uint16_t pwdID, uint8_t nPwdBytes, const uint8_t *pwd){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;

    build_VerifyCMD(buf, &nByte, pwdID, nPwdBytes, pwd);

    // Send the request
    uint8_t buffer[3];
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 10, buffer, 3);
    mLastCommandSend = VERIFY;

    if(err!=ESP_OK)
        return M24SR_ERROR;

    uint8_t size = sizeof(buffer);
    return _check_SW1SW2(buffer, size, 0);

}


M24SR_Status_t c_M24SR_sendChangeReferenceData(uint16_t pwdID, uint8_t *pwd){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;

    build_ChangeReferenceDataCMD(buf, &nByte, pwdID, pwd);

    // Send the request
    uint8_t buffer[3];
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 10, buffer, 3);
    mLastCommandSend = CHANGE_REFERENCE_DATA;

    if(err!=ESP_OK)
        return M24SR_ERROR;

    uint8_t size = sizeof(buffer);
    return _check_SW1SW2(buffer, size, size - 2);
}


static M24SR_Status_t _receiveEnableVerificationRequirement(void){

    uint8_t buf[M24SR_STATUSRESPONSE_NBYTE];
    M24SR_Status_t status;
    const PasswordType_t type = constToPasswordType(mLastCommandData.offset);

    status = _receiveResponse(buf, M24SR_STATUSRESPONSE_NBYTE);

    if(status!= M24SR_SUCCESS)
        return status;

    status = _isCorrectCRC16Residue(buf, M24SR_STATUSRESPONSE_NBYTE);

    return status;
}


M24SR_Status_t c_M24SR_sendEnableVerificationRequirement(uint16_t rw){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;

    build_EnableVerificationRequirementCMD(buf, &nByte, rw);

    // Send the request
    uint8_t buffer[3];
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 10, buffer, 3);
    mLastCommandSend = ENABLE_VERIFICATION_REQUIREMENT;

    if(err!=ESP_OK){
        ESP_LOGE(TAG, "Error sending command");
        return M24SR_ERROR;
    }

    return M24SR_SUCCESS;
}


static M24SR_Status_t _receiveDisableVerificationRequirement(void){

    uint8_t buf[M24SR_STATUSRESPONSE_NBYTE];
    M24SR_Status_t status;
    const PasswordType_t type = constToPasswordType(mLastCommandData.offset);

    status = _receiveResponse(buf, M24SR_STATUSRESPONSE_NBYTE);

    if(status!= M24SR_SUCCESS)
        return status;

    status = _isCorrectCRC16Residue(buf, M24SR_STATUSRESPONSE_NBYTE);

    return status;
}


M24SR_Status_t c_M24SR_sendDisableVerificationRequirement(uint16_t rw){

    uint8_t *buf = mM24SRbuffer;
    uint16_t nByte;

    build_DisableVerificationRequirementCMD(buf, &nByte, rw);

    uint8_t buffer[3];
    esp_err_t err = m24sr_execute_command(&device, buf, nByte, 10, buffer, 3);
    mLastCommandSend = DISABLE_VERIFICATION_REQUIREMENT;

    if(err!=ESP_OK)
        return M24SR_ERROR;

    return M24SR_SUCCESS;
}



M24SR_Status_t c_M24SR_forceI2CSession(){
    uint8_t commandBuffer[] = KILL_RF_SESSION_CMD;
    M24SR_Status_t status = _sendCommand(commandBuffer, sizeof(commandBuffer));
    return status;
}


M24SR_Status_t c_M24SR_getI2CSession(){
    uint8_t commandBuffer[] = GET_I2C_SESSION_CMD;

    M24SR_Status_t status = _sendCommand(commandBuffer, sizeof(commandBuffer));

    if(status!=M24SR_SUCCESS){
        status = c_M24SR_forceI2CSession();
    }

    return status;
}

