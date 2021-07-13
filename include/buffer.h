#pragma once

#include <stdint.h>
#include <stddef.h>


///////////////////////////////////
//  STRUCTURE
///////////////////////////////////


typedef struct
{
    char *data;
    size_t capacity;
    size_t pos;
} buffer_t;


///////////////////////////////////
//  READER
///////////////////////////////////


uint8_t buffer_read_u8(buffer_t *buf);
uint16_t buffer_read_u16(buffer_t *buf);
uint32_t buffer_read_u32(buffer_t *buf);
uint64_t buffer_read_u64(buffer_t *buf);
float buffer_read_float(buffer_t *buf);
double buffer_read_double(buffer_t *buf);
char *buffer_read_string(buffer_t *buf, char *dst, size_t max);
char *buffer_read_array(buffer_t *buf, char *dst, size_t max);
int32_t buffer_read_varint(buffer_t *buf);
int64_t buffer_read_varlong(buffer_t *buf);


///////////////////////////////////
//  WRITER
///////////////////////////////////


void buffer_write_u8(buffer_t *buf, uint8_t i);
void buffer_write_u16(buffer_t *buf, uint16_t i);
void buffer_write_u32(buffer_t *buf, uint32_t i);
void buffer_write_u64(buffer_t *buf, uint64_t i);
void buffer_write_float(buffer_t *buf, float i);
void buffer_write_double(buffer_t *buf, double i);
void buffer_write_string(buffer_t *buf, const char *src, size_t len);
void buffer_write_array(buffer_t *buf, const char *src, size_t len);
void buffer_write_varint(buffer_t *buf, uint32_t i);
void buffer_write_varlong(buffer_t *buf, uint64_t l);


///////////////////////////////////
//  UTIL
///////////////////////////////////


int buffer_size_varint(int i);
