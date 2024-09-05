#ifndef __M24SR_DEF_H
#define __M24SR_DEF_H


/***********************************
    Default I2C address
************************************/
#define M24SR_I2C_ADDR                  0x56


#define CLASS_DEFAULT_VALUE             0x00
#define LE_DEFAULT_VALUE                0x00

/**
 * @defgroup Command Instruction code (C-APDU)
*/
#define SELECT_CMD                              0xA4
#define READ_BINARY_CMD                         0xB0
#define UPDATE_BINARY_CMD                       0xD6
#define VERIFY_CMD                              0x20
#define CHANGE_REF_DATA_CMD                     0x24    //Change pwd
#define DISABLE_VERIFICATION_REQUIREMENT_CMD    0x26
#define ENABLE_VERIFICATION_REQUIREMENT_CMD     0x28

#define GET_I2C_SESSION_CMD                     {0x26} 
#define KILL_RF_SESSION_CMD                     {0x52}

#define M24SR_DESELECT_CMD	{0xC2,0xE0,0xB4}

/**
 * @defgroup File identifier
*/
#define CC_FILE_ID              {0xE1, 0x03}
#define SYSTEM_FILE_ID          {0xE1, 0x01}
#define NDEF_FILE_ID            0x0001

#define CC_FILE_P1P2            0x000C
#define NDEF_FILE_P1P2          0x000C
#define SYSTEM_FILE_P1P2        0x000C

/**
 * @brief NDEF Tag Application identifier
*/
#define NDEF_TAG_APP_ID         {0xD2,0x76,0x00,0x00,0x85,0x01,0x01}
#define NDEF_TAG_APP_P1P2       0x0400


/**
 * @defgroup Verify cmd Password identification
*/
#define READ_PWD        0x0001
#define WRITE_PWD       0x0002
#define I2C_PWD         0x0003
#define PWD_LENGTH      0x0010
static const uint8_t DEFAULT_PWD_16B[PWD_LENGTH] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

/**
 * S-Block PCB byte
*/
#define WTX_WO_DID      0xF2
#define WTX_W_DID       0xFA
#define DES_WO_DID      0xC2
#define DES_W_DID       0xFA

/**
 * @defgroup Security Attributes:
 *      - Enable Verification Requirement cmd
 *      - Disable Verification Requirement cmd
*/
#define READ_PROTECTION      0x0001
#define WRITE_PROTECTION     0x0002

/**
 * @defgroup Status code
*/
#define UB_STATUS_OFFSET 4
#define LB_STATUS_OFFSET 3


/**
 * @defgroup Length
*/
#define M24SR_STATUSRESPONSE_NBYTE              5
#define M24SR_WATINGTIMEEXTRESPONSE_NBYTE		4
#define M24SR_PASSWORD_NBYTE					0x10


/*  Command structure Mask -------------------------------------------------------------------*/
#define M24SR_PCB_NEEDED				0x0001		/* PCB byte present or not */
#define M24SR_CLA_NEEDED				0x0002 		/* CLA byte present or not */
#define M24SR_INS_NEEDED				0x0004 		/* Operation code present or not*/
#define M24SR_P1_NEEDED					0x0008		/* Selection Mode  present or not*/
#define M24SR_P2_NEEDED					0x0010		/* Selection Option present or not*/
#define M24SR_LC_NEEDED					0x0020		/* Data field length byte present or not */
#define M24SR_DATA_NEEDED				0x0040		/* Data present or not */
#define M24SR_LE_NEEDED					0x0080		/* Expected length present or not */
#define M24SR_CRC_NEEDED				0x0100		/* 2 CRC bytes present	or not */

#define M24SR_DID_NEEDED				0x08			/* DID byte present or not */

/*  Command structure	------------------------------------------------------------------------*/
#define M24SR_CMDSTRUCT_SELECTAPPLICATION					0x01FF
#define M24SR_CMDSTRUCT_SELECTCCFILE						0x017F
#define M24SR_CMDSTRUCT_SELECTNDEFFILE						0x017F
#define M24SR_CMDSTRUCT_READBINARY							0x019F
#define M24SR_CMDSTRUCT_UPDATEBINARY						0x017F
#define M24SR_CMDSTRUCT_VERIFYBINARYWOPWD					0x013F
#define M24SR_CMDSTRUCT_VERIFYBINARYWITHPWD				    0x017F
#define M24SR_CMDSTRUCT_CHANGEREFDATA						0x017F
#define M24SR_CMDSTRUCT_ENABLEVERIFREQ						0x011F
#define M24SR_CMDSTRUCT_DISABLEVERIFREQ						0x011F
#define M24SR_CMDSTRUCT_SENDINTERRUPT						0x013F
#define M24SR_CMDSTRUCT_GPOSTATE							0x017F


/*  Offset	----------------------------------------------------------------------------------*/
#define M24SR_OFFSET_PCB													0
#define M24SR_OFFSET_CLASS													1
#define M24SR_OFFSET_INS													2
#define M24SR_OFFSET_P1														3

/*  mask	------------------------------------------------------------------------------------*/
#define M24SR_MASK_BLOCK													0xC0
#define M24SR_MASK_IBLOCK													0x00
#define M24SR_MASK_RBLOCK													0x80
#define M24SR_MASK_SBLOCK													0xC0


/**
 * @brief  APDU-Header command structure
 */
typedef struct {
	uint8_t CLA; /* Command class */
	uint8_t INS; /* Operation code */
	uint8_t P1; /* Selection Mode */
	uint8_t P2; /* Selection Option */
} C_APDU_Header;

/**
 * @brief  APDU-Body command structure
 */
typedef struct {
	uint8_t LC; /* Data field length */
	const uint8_t *data; /* Command parameters */
	uint8_t LE; /* Expected length of data to be returned */
} C_APDU_Body;

/**
 * @brief  APDU Command structure
 */
typedef struct {
	C_APDU_Header header;
	C_APDU_Body body;
} C_APDU;

/**
 * @brief  SC response structure
 */
typedef struct {
	uint8_t *data; /* Data returned from the card */ // pointer on the transceiver buffer = ReaderRecBuf[CR95HF_DATA_OFFSET ];
	uint8_t SW1; /* Command Processing status */
	uint8_t SW2; /* Command Processing qualification */
} R_APDU;



/* MACROS ------------------------------------- */

/** @brief Get Most Significant Byte
 * @param  val: number where MSB must be extracted
 * @retval MSB
 */
#define GET_MSB(val) 		( (uint8_t) ((val & 0xFF00 )>>8) )

/** @brief Get Least Significant Byte
 * @param  val: number where LSB must be extracted
 * @retval LSB
 */
#define GET_LSB(val) 		( (uint8_t) (val & 0x00FF ))

/** @brief Used to toggle the block number by adding 0 or 1 to default block number value
 * @param  val: number to know if incrementation is needed
 * @retval  0 or 1 if incrementation needed
 */
#define TOGGLE(val) 		((val != 0x00)? 0x00 : 0x01)

















#endif
