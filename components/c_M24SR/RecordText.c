#include "RecordHeader.h"
#include "RecordText.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const uint8_t NDEF_TEXT_ID_CODE = 'T';

const char* LanguageCode[] = {
    "en",
    "es",
};

static void _set_text(RecordText_t *text_record, const char * const text){
    if(text_record->super.payload != NULL){
        free(text_record->super.payload);
    }
    int len = strlen(text);
    text_record->super.payload = (uint8_t *)malloc(len + 1);
    strlcpy((char*)text_record->super.payload, text, len + 1);
}

static uint8_t _get_text_status(TextEncoding enc, Language lang){
    uint8_t status = strlen(LanguageCode[lang]);
    if(enc == UTF16){
        status &= 0x80; // set to 1 the bit 7
    }
    return status;
}

static void _update_payload_length(RecordText_t *text_record) {
    size_t lang_len = strlen(LanguageCode[text_record->language]);
    size_t text_len = 0;
    if(text_record->super.payload != NULL){
        text_len = strlen((char *)text_record->super.payload);
    }
    set_payload_length(&text_record->super.header, lang_len + text_len + 1);
}

static void _set_record_header(RecordText_t *text_record){
    set_TNF(&text_record->super.header, NFC_well_known);
    set_type_length(&text_record->super.header, sizeof(NDEF_TEXT_ID_CODE));
    _update_payload_length(text_record);
}


void RecordText_ctor(RecordText_t *text_record){
    Record_ctor(&text_record->super);
    text_record->encode = UTF8;
    text_record->language = ENGLISH;
    text_record->status = _get_text_status(text_record->encode, text_record->language);
    _set_record_header(text_record);
}


void record_text_set_text(RecordText_t *text_record, const char * const text){
    text_record->encode = UTF8;
    text_record->language = ENGLISH;
    _set_text(text_record, text);
    text_record->status = _get_text_status(text_record->encode, text_record->language);

    set_payload_length(&text_record->super.header, strlen(text) + 3);

    printf("record_text_set_text headerFlags: %x\n", text_record->super.header.headerFlags);
}

uint16_t record_text_write(RecordText_t text_record, uint8_t *buffer){
    int16_t offset = 0;
    offset += write_header(text_record.super.header, buffer);

    buffer[offset++] = NDEF_TEXT_ID_CODE;
	buffer[offset++] = text_record.status;
    const char * lang = LanguageCode[text_record.language];
    memcpy(buffer + offset, lang, strlen(lang)); 
    offset += strlen(lang);

    char *payload = (char *)text_record.super.payload;
    int16_t len = strlen(payload);
    memcpy(buffer + offset, payload, len);
    offset += len;

    printf("record_text_write: ");

    for(int i = 0; i < offset; i++){
        printf("%02X ", buffer[i]);
    }
    printf("\n");

    return offset;
}