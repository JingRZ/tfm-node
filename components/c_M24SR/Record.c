#include "Record.h"
#include <stdbool.h>
#include <stdio.h>

void Record_ctor(Record_t *record){
    RecordHeader_ctor(&record->header);
    record->payload = NULL;
}

void set_as_first_record(RecordHeader_t *header){
    set_MB(header, true);
}

void set_as_last_record(RecordHeader_t *header){
    set_ME(header, true);
}

int is_first_record(RecordHeader_t header){
    return get_MB(header);
}

int is_last_record(RecordHeader_t header){
    return get_ME(header);
}

void set_as_middle_record(RecordHeader_t *header){
    set_MB(header, false);
    set_ME(header, false);
}

bool is_middle_record(RecordHeader_t header){
    return !get_MB(header) && !get_ME(header);
}

uint16_t get_byte_length(RecordHeader_t header){
    return get_record_length(header);
}