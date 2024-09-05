#include <stdio.h>
#include <string.h> // memcpy
#include "driver/i2c.h"

#include "c_M24SR.h"
#include "M24SR_def.h"
#include "i2c_config.h"

#define M24SR_MAX_BYTE_OPERATION_LENGHT (246)

uint8_t mDIDByte;

/**
 * @brief  Updates the CRC
 */
static uint16_t _updateCrc(uint8_t ch, uint16_t *lpwCrc) {

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
static uint16_t _ComputeCrc(uint8_t *data, uint8_t len) {
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
        _updateCrc(chBlock, &wCrc);
    } while (--len);

    return wCrc;
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
        crc16 = _ComputeCrc(res, (uint8_t) (*nBytes));
        /* append the CRC16 */
        res[(*nBytes)++] = GET_LSB(crc16);
        res[(*nBytes)++] = GET_MSB(crc16);
    }

}


void build_SelectApplicationCMD(uint8_t *buf, uint16_t *nByte){

    C_APDU command;
    uint8_t dataOut[] = NDEF_TAG_APP_ID;
    uint8_t uLe = LE_DEFAULT_VALUE;
    uint16_t uP1P2 = NDEF_TAG_APP_P1P2;

    // Build the command
    command.header.CLA = CLASS_DEFAULT_VALUE;
    command.header.INS = SELECT_CMD;

    // Offsets
    command.header.P1 = GET_MSB(uP1P2);
    command.header.P2 = GET_LSB(uP1P2);

    // Size of data field
    command.body.LC = sizeof(dataOut);

    // Data field
    command.body.data = dataOut;

    // Number of bytes to read for this cmd
    command.body.LE = uLe;

    // Build Select cmd
    _buildIBlockCommand( M24SR_CMDSTRUCT_SELECTAPPLICATION, &command, mDIDByte, nByte, buf);
}


void build_SelectCCfileCMD(uint8_t *buf, uint16_t *nByte){
    C_APDU command;
    uint8_t dataOut[] = CC_FILE_ID;
    uint16_t uP1P2 = CC_FILE_P1P2;

    // Build the command
    command.header.CLA = CLASS_DEFAULT_VALUE;
    command.header.INS = SELECT_CMD;

    // Offsets
    command.header.P1 = GET_MSB(uP1P2);
    command.header.P2 = GET_LSB(uP1P2);

    // Size of data field
    command.body.LC = sizeof(dataOut);

    // Data field
    command.body.data = dataOut;

    // Build Select cmd
    _buildIBlockCommand( M24SR_CMDSTRUCT_SELECTCCFILE, &command, mDIDByte, nByte, buf);
}


void build_SelectNDEFfileCMD(uint8_t *buf, uint16_t *nByte, uint16_t fileID){
    C_APDU command;
    uint8_t dataOut[] = {GET_MSB(fileID), GET_LSB(fileID)};
    uint16_t uP1P2 = NDEF_FILE_P1P2;

    // Build the command
    command.header.CLA = CLASS_DEFAULT_VALUE;
    command.header.INS = SELECT_CMD;

    // Offsets
    command.header.P1 = GET_MSB(uP1P2);
    command.header.P2 = GET_LSB(uP1P2);

    // Size of data field
    command.body.LC = sizeof(dataOut);

    // Data field
    command.body.data = dataOut;

    // Build Select cmd
    _buildIBlockCommand( M24SR_CMDSTRUCT_SELECTNDEFFILE, &command, mDIDByte, nByte, buf);
}



void build_SelectSystemfileCMD(uint8_t *buf, uint16_t *nByte){
    C_APDU command;
    uint8_t dataOut[] = SYSTEM_FILE_ID;
    uint16_t uP1P2 = SYSTEM_FILE_P1P2;

    // Build the command
    command.header.CLA = CLASS_DEFAULT_VALUE;
    command.header.INS = SELECT_CMD;

    // Offsets
    command.header.P1 = GET_MSB(uP1P2);
    command.header.P2 = GET_LSB(uP1P2);

    // Size of data field
    command.body.LC = sizeof(dataOut);

    // Data field
    command.body.data = dataOut;

    // Build Select cmd
    _buildIBlockCommand( M24SR_CMDSTRUCT_SELECTCCFILE, &command, mDIDByte, nByte, buf);
}


void build_DeselectCMD(uint8_t *buf, uint16_t *nByte){

    uint8_t cmd[] = M24SR_DESELECT_CMD;

    (*nByte) = 0;

    for (int i = 0; i<sizeof(cmd); i++){
        buf[(*nByte)++] = cmd[i];
    }
}


void build_FWTExtensionCMD(uint8_t *buf, uint16_t *nByte, uint8_t fwtByte){

    uint16_t crc16;

    // Create response
    buf[(*nByte)++] = WTX_WO_DID;
    buf[(*nByte)++] = fwtByte;

    // Compute CRC
    crc16 = _ComputeCrc(buf, *nByte);

    // Append CRC16
    buf[(*nByte)++] = GET_LSB(crc16);
    buf[(*nByte)++] = GET_MSB(crc16);
}



void build_ReadBinaryCMD(uint8_t *buf, uint16_t *nByte, uint16_t offset, uint8_t nBytes2Read){

    if(nBytes2Read > M24SR_MAX_BYTE_OPERATION_LENGHT)
        nBytes2Read = M24SR_MAX_BYTE_OPERATION_LENGHT;

    C_APDU command;

    // Build the command
    command.header.CLA = CLASS_DEFAULT_VALUE;
    command.header.INS = READ_BINARY_CMD;

    // Offsets
    command.header.P1 = GET_MSB(offset);
    command.header.P2 = GET_LSB(offset);

    // Number of bytes to read
    command.body.LE = nBytes2Read;

    // Build Select cmd
    _buildIBlockCommand( M24SR_CMDSTRUCT_READBINARY, &command, mDIDByte, nByte, buf);
}



void build_UpdateBinaryCMD(uint8_t *buf, uint16_t *nByte, uint16_t offset, uint8_t nBytes2Write, uint8_t *bufWrite){

    if(nBytes2Write > M24SR_MAX_BYTE_OPERATION_LENGHT)
        nBytes2Write = M24SR_MAX_BYTE_OPERATION_LENGHT;

    C_APDU command;

    // Build the command
    command.header.CLA = CLASS_DEFAULT_VALUE;
    command.header.INS = UPDATE_BINARY_CMD;

    // Offsets
    command.header.P1 = GET_MSB(offset);
    command.header.P2 = GET_LSB(offset);

    // Number of bytes to read
    command.body.LC = nBytes2Write;
    command.body.data = bufWrite;

    // Build Select cmd
    _buildIBlockCommand( M24SR_CMDSTRUCT_UPDATEBINARY, &command, mDIDByte, nByte, buf);

}


M24SR_Status_t build_VerifyCMD(uint8_t *buf, uint16_t *nByte, uint16_t pwdID, uint8_t nPwdBytes, const uint8_t *pwd){

    C_APDU command;

    /*check the parameters */
    if ((pwdID > 0x0003) || ((nPwdBytes != 0x00) && (nPwdBytes != 0x10))) {
        return M24SR_IO_ERROR_PARAMETER;
    }

    // Build the command
    command.header.CLA = CLASS_DEFAULT_VALUE;
    command.header.INS = VERIFY_CMD;

    // Offsets
    command.header.P1 = GET_MSB(pwdID);
    command.header.P2 = GET_LSB(pwdID);

    // Number of bytes in data field
    command.body.LC = nPwdBytes;

    if(nPwdBytes == 0x10) {
        command.body.data = pwd;
        _buildIBlockCommand( M24SR_CMDSTRUCT_VERIFYBINARYWITHPWD, &command, mDIDByte, nByte, buf);

    } else {
        command.body.data = NULL;
        _buildIBlockCommand( M24SR_CMDSTRUCT_VERIFYBINARYWOPWD, &command, mDIDByte, nByte, buf);
    }

    return M24SR_SUCCESS;
}



M24SR_Status_t build_ChangeReferenceDataCMD(uint8_t *buf, uint16_t *nByte, uint16_t pwdID, uint8_t *pwd){

    C_APDU command;

    /*check the parameters */
    if ( pwdID > 0x0003 ) {
        return M24SR_IO_ERROR_PARAMETER;
    }

    // Build the command
    command.header.CLA = CLASS_DEFAULT_VALUE;
    command.header.INS = CHANGE_REF_DATA_CMD;

    // Offsets
    command.header.P1 = GET_MSB(pwdID);
    command.header.P2 = GET_LSB(pwdID);

    // Number of bytes in data field
    command.body.LC = M24SR_PASSWORD_NBYTE;

    command.body.data = pwd;

    _buildIBlockCommand( M24SR_CMDSTRUCT_CHANGEREFDATA, &command, mDIDByte, nByte, buf);

    return M24SR_SUCCESS;
}


M24SR_Status_t build_EnableVerificationRequirementCMD(uint8_t *buf, uint16_t *nByte, uint16_t rw){
    C_APDU command;

    /*check the parameters */
    if ( (rw != READ_PROTECTION) && (rw != WRITE_PROTECTION) ) {
        return M24SR_IO_ERROR_PARAMETER;
    }

    // Build the command
    command.header.CLA = CLASS_DEFAULT_VALUE;
    command.header.INS = ENABLE_VERIFICATION_REQUIREMENT_CMD;

    // Offsets
    command.header.P1 = GET_MSB(rw);
    command.header.P2 = GET_LSB(rw);

    _buildIBlockCommand( M24SR_CMDSTRUCT_ENABLEVERIFREQ, &command, mDIDByte, nByte, buf);

    return M24SR_SUCCESS;
}



M24SR_Status_t build_DisableVerificationRequirementCMD(uint8_t *buf, uint16_t *nByte, uint16_t rw){

    C_APDU command;

    /*check the parameters */
    if ( (rw != READ_PROTECTION) && (rw != WRITE_PROTECTION) ) {
        return M24SR_IO_ERROR_PARAMETER;
    }

    // Build the command
    command.header.CLA = CLASS_DEFAULT_VALUE;
    command.header.INS = DISABLE_VERIFICATION_REQUIREMENT_CMD;

    // Offsets
    command.header.P1 = GET_MSB(rw);
    command.header.P2 = GET_LSB(rw);

    _buildIBlockCommand( M24SR_CMDSTRUCT_DISABLEVERIFREQ, &command, mDIDByte, nByte, buf);

    return M24SR_SUCCESS;
}
