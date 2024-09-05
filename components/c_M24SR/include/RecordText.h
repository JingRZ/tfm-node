#ifndef __RECORD_TEXT_H
#define __RECORD_TEXT_H

#include "Record.h"

typedef enum {
    UTF8, //!< UTF8
    UTF16,//!< UTF16
} TextEncoding;

typedef enum {
    ENGLISH,
    SPANISH,
} Language;

extern const char* LanguageCode[];

typedef struct {
    Record_t super;
    TextEncoding encode;
    uint8_t status;
    Language language;
    //char *buffer;
} RecordText_t;

void RecordText_ctor(RecordText_t *text_record);
//void record_text_update_payload_length(RecordText_t *text_record);
void record_text_set_text(RecordText_t *text_record, const char * const text);
uint16_t record_text_write(RecordText_t text_record, uint8_t *buffer);

#endif // __RECORD_TEXT_H