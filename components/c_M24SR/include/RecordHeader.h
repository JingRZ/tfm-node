#ifndef __RECORDHEADER_H
#define __RECORDHEADER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum TypeNameFormat {
    Empty = 0x00,          //!< Empty
    NFC_well_known = 0x01, //!< NFC_well_known
    Mime_media_type = 0x02,//!< Mime_media_type
    Absolute_URI = 0x03,   //!< Absolute_URI
    NFC_external = 0x04,   //!< NFC_external
    Unknown = 0x05,        //!< Unknown
    Unchanged = 0x06,      //!< Unchanged
    Reserved = 0x07        //!< Reserved
} TypeNameFormat_t;

typedef struct RecordHeader {
    uint8_t idLength;
    uint8_t headerFlags;
    uint8_t typeLength;
    uint32_t payloadLength;
} RecordHeader_t;

/*
extern uint8_t idLength;
extern uint8_t headerFlags;
extern uint8_t typeLength;
extern uint32_t payloadLength;
*/

void RecordHeader_ctor(RecordHeader_t *header);
void set_MB(RecordHeader_t *header, bool value);
int get_MB(const RecordHeader_t header);
void set_ME(RecordHeader_t *header, bool value);
int get_ME(const RecordHeader_t header);
void set_CF(RecordHeader_t *header, bool value);
int get_CF(const RecordHeader_t header);
void set_SR(RecordHeader_t *header, bool value);
int get_SR(const RecordHeader_t header);
void set_IL(RecordHeader_t *header, bool value);
int get_IL(const RecordHeader_t header);
void set_TNF(RecordHeader_t *header, TypeNameFormat_t value);
TypeNameFormat_t get_TNF(const RecordHeader_t header);
void set_type_length(RecordHeader_t *header, uint8_t value);
uint8_t get_type_length(const RecordHeader_t header);
void set_payload_length(RecordHeader_t *header, uint32_t value);
uint32_t get_payload_length(const RecordHeader_t header);
void set_id_length(RecordHeader_t *header, uint8_t value);
uint8_t get_id_length(const RecordHeader_t header);
uint16_t get_record_length(const RecordHeader_t header);
uint8_t write_header(RecordHeader_t header, uint8_t *buffer);
uint16_t load_header(RecordHeader_t *header, const uint8_t *const buffer);

#endif // __RECORDHEADER_H