#include "RecordHeader.h"
#include <stdio.h>
/*
uint8_t idLength;
uint8_t headerFlags;
uint8_t typeLength;
uint32_t payloadLength;
*/
#define MB_BIT 0x80
#define ME_BIT 0x40
#define CF_BIT 0x20
#define SR_BIT 0x10
#define IL_BIT 0x08
#define TNF_BITS 0x07

void RecordHeader_ctor(RecordHeader_t *header) {
    header->idLength = 0;
    header->headerFlags = 0;
    header->typeLength = 0;
    header->payloadLength = 0;
}


void set_MB(RecordHeader_t *header, bool value) {
    if (value)
        header->headerFlags |= MB_BIT;
    else
        header->headerFlags &= ~MB_BIT;
}

int get_MB(const RecordHeader_t header) {
    return (header.headerFlags & MB_BIT) != 0;
}

void set_ME(RecordHeader_t *header, bool value) {
    if (value)
        header->headerFlags |= ME_BIT;
    else
        header->headerFlags &= ~ME_BIT;
}

int get_ME(const RecordHeader_t header) {
    return (header.headerFlags & ME_BIT) != 0;
}

void set_CF(RecordHeader_t *header, bool value) {
    if (value)
        header->headerFlags |= CF_BIT;
    else
        header->headerFlags &= ~CF_BIT;
}

int get_CF(const RecordHeader_t header) {
    return (header.headerFlags & CF_BIT) != 0;
}

void set_SR(RecordHeader_t *header, bool value) {
    if (value)
        header->headerFlags |= SR_BIT;
    else
        header->headerFlags &= ~SR_BIT;
}

int get_SR(const RecordHeader_t header) {
    return (header.headerFlags & SR_BIT) != 0;
}

void set_IL(RecordHeader_t *header, bool value) {
    if (value)
        header->headerFlags |= IL_BIT;
    else
        header->headerFlags &= ~IL_BIT;
}

int get_IL(const RecordHeader_t header) {
    return (header.headerFlags & IL_BIT) != 0;
}

void set_TNF(RecordHeader_t *header, TypeNameFormat_t value) {
    uint8_t temp = (uint8_t)value;
    temp &= 0x07; // Keep the first 3 bits
    header->headerFlags &= 0xF8; // Clear the first 3 bits
    header->headerFlags |= temp; // Set the first 3 bits
}

TypeNameFormat_t get_TNF(const RecordHeader_t header) {
    return (TypeNameFormat_t)(header.headerFlags & TNF_BITS);
}

void set_type_length(RecordHeader_t *header, uint8_t value) {
    header->typeLength = value;
}

uint8_t get_type_length(const RecordHeader_t header) {
    return header.typeLength;
}

void set_payload_length(RecordHeader_t *header, uint32_t value) {
    header->payloadLength = value;
    set_SR(header, value < 256);
}

uint32_t get_payload_length(const RecordHeader_t header) {
    return header.payloadLength;
}

void set_id_length(RecordHeader_t *header, uint8_t value) {
    header->idLength = value;
    set_IL(header, value > 0);
}

uint8_t get_id_length(const RecordHeader_t header) {
    return header.idLength;
}

uint16_t get_record_length(const RecordHeader_t header) {
    return (get_SR(header) ? 3 : 6) + (get_IL(header) ? header.idLength : 0) + header.typeLength + header.payloadLength;
}

uint8_t write_header(RecordHeader_t header, uint8_t *buffer) {
    uint32_t index = 0;
    buffer[index++] = header.headerFlags;
    buffer[index++] = header.typeLength;
    if (get_SR(header)){
        buffer[index++] = (uint8_t)header.payloadLength;
    }
    else {
        buffer[index++] = (uint8_t)((header.payloadLength & 0xFF000000) >> 24);
        buffer[index++] = (uint8_t)((header.payloadLength & 0x00FF0000) >> 16);
        buffer[index++] = (uint8_t)((header.payloadLength & 0x0000FF00) >> 8);
        buffer[index++] = (uint8_t)(header.payloadLength & 0x000000FF);
    }

    if (get_IL(header))
        buffer[index++] = header.idLength;

    printf("write_header: ");

    for(int i = 0; i < index; i++){
        printf("%02X ", buffer[i]);
    }
    printf("\n");
    
    return index;
}

uint16_t load_header(RecordHeader_t *header, const uint8_t *const buffer) {
    uint32_t index = 0;
    header->headerFlags = buffer[index++];
    header->typeLength = buffer[index++];
    if (get_SR(*header))
        header->payloadLength = buffer[index++];
    else {
        header->payloadLength = (((uint32_t)buffer[index + 0]) << 24)
                                | (((uint32_t)buffer[index + 1]) << 16)
                                | (((uint32_t)buffer[index + 2]) << 8)
                                | ((uint32_t)buffer[index + 3]);
        index += 4;
    }

    if (get_IL(*header))
        header->idLength = buffer[index++];
    else
        header->idLength = 0;

    return index;
}