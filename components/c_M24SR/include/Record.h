#ifndef __RECORD_H
#define __RECORD_H

#include "RecordHeader.h"

typedef enum {
    TYPE_UNKNOWN,        //!< UNKNOWN record
    TYPE_TEXT,           //!< TEXT
    TYPE_AAR,            //!< Android Archive record
    TYPE_MIME,           //!< generic MIME type
    TYPE_URI,            //!< generic URI
    TYPE_URI_MAIL,       //!< Email URI record
    TYPE_URI_SMS,        //!< SMS URI record
    TYPE_URI_GEOLOCATION,//!< position URI record
    TYPE_MIME_VCARD,     //!< VCard record
    TYPE_WIFI_CONF 		 //!< Wifi configuration
} RecordType_t;

typedef struct Record {
    RecordHeader_t header;
    uint8_t *payload;
} Record_t;

void Record_ctor(Record_t *record);
void set_as_first_record(RecordHeader_t *header);
void set_as_last_record(RecordHeader_t *header);
int is_first_record(RecordHeader_t header);
int is_last_record(RecordHeader_t header);
void set_as_middle_record(RecordHeader_t *header);
bool is_middle_record(RecordHeader_t header);
uint16_t get_byte_length(RecordHeader_t header);

#endif // RECORD_H