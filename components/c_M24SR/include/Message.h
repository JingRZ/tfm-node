#ifndef __MESSAGE_H
#define __MESSAGE_H

#include <stdint.h>
#include "RecordText.h"

enum {
    MSG_OK,
    MSG_ERR,
};

typedef struct {
    RecordText_t *records;
    size_t size;
    size_t capacity;
} Message_t;


int Message_ctor(Message_t *msg, uint8_t capacity);

void Message_dtor(Message_t *msg);

void message_add_text_record(Message_t *msg, RecordText_t record);

uint16_t message_get_byte_length(Message_t msg);

uint16_t message_write(Message_t msg, uint8_t *buffer);

#endif // MESSAGE_H