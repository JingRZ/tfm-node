#include "RecordHeader.h"
#include "EmptyRecord.h"


void EmptyRecord_ctor(EmptyRecord_t *record){
    set_TNF(&record->header, Empty);
    set_MB(&record->header, true);
    set_ME(&record->header, true);
    set_type_length(&record->header, 0);
    set_payload_length(&record->header, 0);
}

uint16_t empty_record_write(uint8_t *buffer){
    EmptyRecord_t empty_record;
    EmptyRecord_ctor(&empty_record);
    return write_header(empty_record.header, buffer);
}

uint16_t empty_record_get_byte_length(){
    EmptyRecord_t empty_record;
    EmptyRecord_ctor(&empty_record);
    return get_record_length(empty_record.header);
}