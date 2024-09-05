#ifndef __C_M24SR_H
#define __C_M24SR_H

#include "driver/i2c.h"

/**
 * @defgroup i2c related values
*/
#define I2C_TIMEOUT_MS              1000

#define ACK_CHECK_EN                0x1
#define ACK_CHECK_DIS               0x0

#define ACK_VAL                     0x0
#define NACK_VAL                    0x1

//NVS related values
#define WRITE_PWD_KEY               "writepwd"


typedef enum {
    M24SR_SUCCESS = 0,
    M24SR_EOF = 0x6282,
    M24SR_ERROR = 0x6F00, //??
    M24SR_FILE_OVERFLOW_LE = 0x6280,
    M24SR_PASSWORD_REQUIRED = 0x6300,
    M24SR_PASSWORD_INCORRECT0RETRY_LEFT = 0x63C0,
    M24SR_PASSWORD_INCORRECT1RETRY_LEFT = 0x63C1,
    M24SR_PASSWORD_INCORRECT2RETRY_LEFT = 0x63C2,
    M24SR_RF_SESSION_KILLED = 0x6500, //??
    M24SR_UNSUCCESSFUL_UPDATING = 0x6581,
    M24SR_WRONG_LENGTH = 0x6700,
    M24SR_CONDITIONS_NOT_SATISFIED = 0x6985,
    M24SR_INCOMPATIBLE_WITH_FILE = 0x6981,
    M24SR_SECURITY_STATUS_NOT_SATISFIED = 0x6982,
    M24SR_INCORRECT_PARAM = 0x6A80,
    M24SR_FILE_OR_APP_NOT_FOUND = 0x6A82,
    M24SR_INCORRECT_P1_OR_P2 = 0x6A86,
    M24SR_CLASS_NOT_SUPPORTED = 0x6E00,

    M24SR_REFERENCE_DATA_NOT_USABLE = 0x6984,
    M24SR_FILE_OVERFLOW_LC = 0x6A84,
    M24SR_INS_NOT_SUPPORTED = 0x6D00,

    // IOError ????
    M24SR_IO_ERROR_I2CTIMEOUT = 0x0011,
    M24SR_IO_ERROR_CRC = 0x0012,
    M24SR_IO_ERROR_NACK = 0x0013,
    M24SR_IO_ERROR_PARAMETER = 0x0014,
    M24SR_IO_ERROR_NBATEMPT = 0x0015,
    M24SR_IO_NOACKNOWLEDGE = 0x0016,
    M24SR_IO_PIN_NOT_CONNECTED = 0x0017

} M24SR_Status_t;


/**
 * Command that the component can accept
 */
typedef enum{
    NONE,                            //!< NONE
    DESELECT,                        //!< DESELECT
    SELECT_APPLICATION,              //!< SELECT_APPLICATION
    SELECT_CC_FILE,                  //!< SELECT_CC_FILE
    SELECT_NDEF_FILE,                //!< SELECT_NDEF_FILE
    SELECT_SYSTEM_FILE,              //!< SELECT_SYSTEM_FILE
    READ,                            //!< READ
    UPDATE,                          //!< UPDATE
    VERIFY,                          //!< VERIFY
    MANAGE_I2C_GPO,                  //!< MANAGE_I2C_GPO
    MANAGE_RF_GPO,                   //!< MANAGE_RF_GPO
    CHANGE_REFERENCE_DATA,           //!< CHANGE_REFERENCE_DATA
    ENABLE_VERIFICATION_REQUIREMENT, //!< ENABLE_VERIFICATION_REQUIREMENT
    DISABLE_VERIFICATION_REQUIREMENT,//!< DISABLE_VERIFICATION_REQUIREMENT
    ENABLE_PERMANET_STATE,           //!< ENABLE_PERMANET_STATE
    DISABLE_PERMANET_STATE,          //!< DISABLE_PERMANET_STATE
    
} M24SR_Command_t;

/**
 * User parameter used to invoke a command,
 * it is used to provide the data back with the response
 */
typedef struct{
    uint8_t *data; //!< data
    uint16_t length; //!< number of bytes in the data array
    uint16_t offset; //!< offset parameter used in the read/write command
} M24SR_command_data_t;

typedef enum{
    ReadPwd,   //!< Password to use before reading the tag
    WritePwd,  //!< Password to use before writing the tag
    I2CPwd,    //!< Root password, used only through nfc
} PasswordType_t;


/**
 * Buffer used to build the command to send to the chip.
 */
extern uint8_t mM24SRbuffer[0xFF];//max command length is 255

extern uint8_t mDIDByte;


/**
 * Last pending command
 */
extern M24SR_Command_t mLastCommandSend;

/**
 * Parameter used to invoke the last command
 */
extern M24SR_command_data_t mLastCommandData;


/**
 * @brief Init i2c
*/
esp_err_t c_M24SR_i2c_init();

/**
 * @brief Send the Select NDEF Tag Application command
*/
M24SR_Status_t c_M24SR_sendSelectApplication(void);

/**
 * @brief Build and send the Select CC file command
*/
M24SR_Status_t c_M24SR_sendSelectCCfile(void);

/**
 * @brief Build and send the Select System file command
*/
M24SR_Status_t c_M24SR_sendSelectSystemfile(void);

/**
 * @brief Build and send the Select NDEF file command
 * 
 * 
 * @param fileID NDEF file ID, which is usually 0x0001
*/
M24SR_Status_t c_M24SR_sendSelectNDEFfile(uint16_t fileID);

/**
 * @brief Build and send the Waiting Frame eXtension command
 * 
 * @param fwtByte The requested time extension multiplier
*/
M24SR_Status_t c_M24SR_sendFWTExtension(uint8_t fwtByte);

/**
 * @brief Build and send the Read Binary command
 * 
 * 
 * @param offset The point from which to start reading between [0x0000, 0x2000]
 * 
 * @param nBytes2Read How many bytes to read
 * 
 * @param bufRead The buffer to store the read data
*/
M24SR_Status_t c_M24SR_sendReadBinary(uint16_t offset, uint8_t nBytes2Read, uint8_t **bufRead);

/**
 * @brief Build and send the Update Binary command
 * 
 * 
 * @param offset The point from which to start writing between [0x0000, 0x2000]
 * 
 * @param nBytes2Write How many bytes to write
 * 
 * @param buf The buffer that stores the data you want to write
*/
M24SR_Status_t c_M24SR_sendUpdateBinary(uint16_t offset, uint8_t nBytes2Write, uint8_t *buf);

/**
 * @brief Build and send the Send Verify command
 * 
 * 
 * @param pwdID See [Verify cmd Password identification] in M24SR_def.h
 * 
 * @param nPwdBytes How many bytes the password has (max 0x10)
 * 
 * @param pwd The buffer that stores the password
*/
M24SR_Status_t c_M24SR_sendVerify(uint16_t pwdID, uint8_t nPwdBytes, const uint8_t *pwd);

/**
 * @brief Build and send the Change Reference Data command
 * 
 * 
 * @param pwdID See [Verify cmd Password identification] in M24SR_def.h
 * 
 * @param pwd The buffer that stores the password
*/
M24SR_Status_t c_M24SR_sendChangeReferenceData(uint16_t pwdID, uint8_t *pwd);

/**
 * @brief Build and send the Enable Verification Requirement command
 * 
 * 
 * @param rw See [Security Attributes] in M24SR_def.h
*/
M24SR_Status_t c_M24SR_sendEnableVerificationRequirement(uint16_t rw);

/**
 * @brief Build and send the Disable Verification Requirement command
 * 
 * 
 * @param rw See [Security Attributes] in M24SR_def.h
*/
M24SR_Status_t c_M24SR_sendDisableVerificationRequirement(uint16_t rw);

/**
 * @brief Force open an I2C session
*/
M24SR_Status_t c_M24SR_forceI2CSession();

/**
 * @brief Try to get I2C session. Will fail if RF session is open.
*/
M24SR_Status_t c_M24SR_getI2CSession();


/**
 * @brief Build and send the Deselect command to release the I2C session(or maybe any session?)
*/
M24SR_Status_t c_M24SR_sendDeselect();

#endif