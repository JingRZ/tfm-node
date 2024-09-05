#ifndef __CMDFACTORY_H
#define __CMDFACTORY_H

#include <stdint.h>

/**
 * @brief Build Select NDEF Tag Application command
 *
 * This function constructs the Select NDEF Tag Application command and stores it in the provided buffer.
 *
 * @param buf       Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                      sufficient memory. The function will modify the contents of this buffer.
 * @param nByte     Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                      function call. After the function call, it will be updated to store the actual size of the built command.
 */
void build_SelectApplicationCMD(uint8_t *buf, uint16_t *nByte);

/**
 * @brief Build Select CC File command
 *
 * This function constructs the Select CC File command and stores it in the provided buffer.
 *
 * @param buf       Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                      sufficient memory. The function will modify the contents of this buffer.
 * 
 * @param nByte     Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                      function call. After the function call, it will be updated to store the actual size of the built command.
 */
void build_SelectCCfileCMD(uint8_t *buf, uint16_t *nByte);

/**
 * @brief Build Select System File command
 *
 * This function constructs the Select System File command and stores it in the provided buffer.
 *
 * @param buf       Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                      sufficient memory. The function will modify the contents of this buffer.
 * 
 * @param nByte     Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                      function call. After the function call, it will be updated to store the actual size of the built command.
 */
void build_SelectSystemfileCMD(uint8_t *buf, uint16_t *nByte);

/**
 * @brief Build Select NDEF File command
 *
 * This function constructs the Select NDEF File command and stores it in the provided buffer.
 *
 * @param buf       Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                      sufficient memory. The function will modify the contents of this buffer.
 * 
 * @param nByte     Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                      function call. After the function call, it will be updated to store the actual size of the built command.
 */
void build_SelectNDEFfileCMD(uint8_t *buf, uint16_t *nByte, uint16_t fileID);

/**
 * @brief Build Read Binary command
 *
 * This function constructs the Build Read Binary command and stores it in the provided buffer.
 *
 * @param buf           Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                          sufficient memory. The function will modify the contents of this buffer.
 * 
 * @param nByte         Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                          function call. After the function call, it will be updated to store the actual size of the built command.
 * 
 * @param offset        Offset from where start reading [0x0000, 0x2000]
 * 
 * @param nBytes2Read   Number of bytes to read
 */
void build_ReadBinaryCMD(uint8_t *buf, uint16_t *nByte, uint16_t offset, uint8_t nBytes2Read);

/**
 * @brief Build Update Binary command
 *
 * This function constructs the Update Binary command and stores it in the provided buffer.
 *
 * @param buf           Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                          sufficient memory. The function will modify the contents of this buffer.
 * 
 * @param nByte         Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                          function call. After the function call, it will be updated to store the actual size of the built command.
 * 
 * @param offset        Offset from where start writing [0x0000, 0x2000]
 * 
 * @param nBytes2Write  Number of bytes to write
 * 
 * @param bufWrite      Buffer that stores the data to be written
 */
void build_UpdateBinaryCMD(uint8_t *buf, uint16_t *nByte, uint16_t offset, uint8_t nBytes2Write, uint8_t *bufWrite);

/**
 * @brief Build Verify command
 *
 * This function constructs the Verify command and stores it in the provided buffer.
 *
 * @param buf       Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                      sufficient memory. The function will modify the contents of this buffer.
 * 
 * @param nByte     Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                      function call. After the function call, it will be updated to store the actual size of the built command.
 * 
 * @param pwdID     See [Verify cmd Password identification] in M24SR_def.h
 * 
 * @param nPwdBytes How many bytes the password has (max 0x10)
 * 
 * @param pwd       The buffer that stores the password
 */
M24SR_Status_t build_VerifyCMD(uint8_t *buf, uint16_t *nByte, uint16_t pwdID, uint8_t nPwdBytes, const uint8_t *pwd);

/**
 * @brief Build Change Reference Data command
 *
 * This function constructs the Change Reference Data command and stores it in the provided buffer.
 *
 * @param buf       Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                      sufficient memory. The function will modify the contents of this buffer.
 * 
 * @param nByte     Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                      function call. After the function call, it will be updated to store the actual size of the built command.
 * 
 * @param pwdID     See [Verify cmd Password identification] in M24SR_def.h
 * 
 * @param pwd       The buffer that stores the password
 */
M24SR_Status_t build_ChangeReferenceDataCMD(uint8_t *buf, uint16_t *nByte, uint16_t pwdID, uint8_t *pwd);

/**
 * @brief Build Waiting Frame eXtension response
 *
 * This function constructs the Waiting Frame eXtension response and stores it in the provided buffer.
 *
 * @param buf       Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                      sufficient memory. The function will modify the contents of this buffer.
 * 
 * @param nByte     Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                      function call. After the function call, it will be updated to store the actual size of the built command.
 * 
 * @param fwtByte   The requested waiting time multiplier
 */
void build_FWTExtensionCMD(uint8_t *buf, uint16_t *nByte, uint8_t fwtByte);

/**
 * @brief Build Enable Verification Requirement command
 *
 * This function constructs the Enable Verification Requirement command and stores it in the provided buffer.
 *
 * @param buf       Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                      sufficient memory. The function will modify the contents of this buffer.
 * 
 * @param nByte     Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                      function call. After the function call, it will be updated to store the actual size of the built command.
 * 
 * @param rw        See [Security Attributes] in M24SR_def.h
 */
M24SR_Status_t build_EnableVerificationRequirementCMD(uint8_t *buf, uint16_t *nByte, uint16_t rw);

/**
 * @brief Build Disable Verification Requirement command
 *
 * This function constructs the Disable Verification Requirement command and stores it in the provided buffer.
 *
 * @param buf       Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 *                      sufficient memory. The function will modify the contents of this buffer.
 * 
 * @param nByte     Pointer to a variable that represents the available size (in bytes) of the provided buffer before the
 *                      function call. After the function call, it will be updated to store the actual size of the built command.
 * 
 * @param rw        See [Security Attributes] in M24SR_def.h
 */
M24SR_Status_t build_DisableVerificationRequirementCMD(uint8_t *buf, uint16_t *nByte, uint16_t rw);


/**
 * @brief Build Deselect command to release i2c session (allow RF session without disabling power supply)
 * 
 * @param buf       Pointer to the buffer where the built command will be stored. The caller is responsible for allocating
 * 
 * @param nByte     Returns the size of buf after storing the cmd
*/
void build_DeselectCMD(uint8_t *buf, uint16_t *nByte);

#endif