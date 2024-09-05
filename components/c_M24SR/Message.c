#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "Record.h"
#include "RecordText.h"
#include "Message.h"
#include "EmptyRecord.h"


int Message_ctor(Message_t *msg, uint8_t capacity){
    msg->size = 0;
    msg->capacity = capacity;
    msg->records = (RecordText_t *)malloc(capacity * sizeof(RecordText_t));
    if(msg->records == NULL){
        printf("Error allocating memory for records\n");
        return MSG_ERR;
    }

    for(int i = 0; i < capacity; i++){
        RecordText_ctor(&msg->records[i]);
    }
    
    return MSG_OK;
}

void Message_dtor(Message_t *msg) {
    free(msg->records);
    msg->records = NULL;
    msg->size = 0;
    msg->capacity = 0;
}

void message_add_text_record(Message_t *msg, RecordText_t record){
    if(msg->size == msg->capacity){
        printf("Message is full\n");
        return;
    }

    msg->records[msg->size++] = record;
}

uint16_t message_get_byte_length(Message_t msg){
    uint16_t length = 2;

    if(msg.size == 0){
        return length + empty_record_get_byte_length();
    }

    for(int i = 0; i < msg.size; i++){
        length += get_byte_length(msg.records[i].super.header);
    }

    return length;
}

uint16_t message_write(Message_t msg, uint8_t *buffer) {
    const uint16_t length = message_get_byte_length(msg) - 2;
    uint16_t offset = 0;

    buffer[offset++] = (uint8_t) ((length & 0xFF00) >> 8);
	buffer[offset++] = (uint8_t) ((length & 0x00FF));

    if(msg.size == 0){
        offset += empty_record_write(buffer);
        return offset;
    }

    for(uint32_t i = 0; i < msg.size; i++){
        RecordText_t *r = &msg.records[i];

        set_as_middle_record(&r->super.header);
		if (i == 0)
			set_as_first_record(&r->super.header);
		if (i == msg.size - 1)
			set_as_last_record(&r->super.header);

        offset += record_text_write(*r, buffer + offset);
    }

    return offset;
}

