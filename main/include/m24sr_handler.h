#include  <stdint.h>

typedef enum {
    STATUS_SUCCESS,
    STATUS_ERROR
} Status_t;

Status_t m24sr_init_i2c();

Status_t m24sr_release_i2c();

Status_t m24sr_check_rw_rights();

Status_t m24sr_unlockRWaccess();

Status_t m24sr_checkNDEFSecurityStatus();

int m24sr_getNDEFfileContentSize();

Status_t m24sr_writeNDEFfile(uint16_t offset, uint8_t* writeBuffer, uint8_t nBytes, int checkSuccess);

Status_t m24sr_readNDEFfile(uint16_t offset, uint8_t nBytes, uint8_t *readBuf);

Status_t m24sr_set_fmode(uint8_t mode);

Status_t m24sr_set_content(const char* buf);

Status_t m24sr_enableWritePwd(uint8_t *current_pwd, uint8_t *new_pwd);

Status_t m24sr_disableWritePwd(uint8_t *current_pwd);

bool m24sr_parse_context(char* data);