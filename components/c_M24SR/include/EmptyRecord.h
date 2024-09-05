#ifndef __EMPTY_RECORD_H
#define __EMPTY_RECORD_H

#include <stdint.h>

typedef struct {
    RecordHeader_t header;
    uint8_t status;
    char *language;
    char *buffer;
} EmptyRecord_t;

void EmptyRecord_ctor(EmptyRecord_t *record);
uint16_t empty_record_write(uint8_t *buffer);
uint16_t empty_record_get_byte_length();
#endif