#include <stdlib.h>
#include <string.h>

#include "buffer.h"


///////////////////////////////////
//  READER
///////////////////////////////////


inline uint8_t buffer_read_u8(buffer_t *buf)
{
    return (buf->data[buf->pos++]);
}

inline uint16_t buffer_read_u16(buffer_t *buf)
{
    return (((uint16_t)buffer_read_u8(buf) << 8) | buffer_read_u8(buf));
}

inline uint32_t buffer_read_u32(buffer_t *buf)
{
    return (((uint32_t)buffer_read_u16(buf) << 16) | buffer_read_u16(buf));
}

inline uint64_t buffer_read_u64(buffer_t *buf)
{
    return (((uint64_t)buffer_read_u32(buf) << 32) | buffer_read_u32(buf));
}

float buffer_read_float(buffer_t *buf)
{
    uint8_t data[4];

    // Decode Big Endian.
    data[3] = buf->data[buf->pos + 0];
    data[2] = buf->data[buf->pos + 1];
    data[1] = buf->data[buf->pos + 2];
    data[0] = buf->data[buf->pos + 3];
    buf->pos += sizeof(float);

    return *((float*)data);
}

double buffer_read_double(buffer_t *buf)
{
    uint8_t data[8];

    // Decode Little Endian.
    data[7] = buf->data[buf->pos + 0];
    data[6] = buf->data[buf->pos + 1];
    data[5] = buf->data[buf->pos + 2];
    data[4] = buf->data[buf->pos + 3];
    data[3] = buf->data[buf->pos + 4];
    data[2] = buf->data[buf->pos + 5];
    data[1] = buf->data[buf->pos + 6];
    data[0] = buf->data[buf->pos + 7];
    buf->pos += sizeof(double);

    return *((double*)data);
}

char *buffer_read_string(buffer_t *buf, char *dst, size_t max)
{
    size_t i        = 0;
    size_t length   = buffer_read_varint(buf);
    char *storage   = &buf->data[buf->pos];

    // Check for string length.
    if (length > max)
        return (NULL);

    // Allocate memory if destination is NULL.
    if (dst == NULL)
        dst = malloc(sizeof(char) * (length + 1));

    // Read string.
    while (i < length) {
        dst[i] = storage[i];
        i++;
    }
    dst[i] = 0;

    // Set new position.
    buf->pos += length;
    return (dst);
}

char *buffer_read_array(buffer_t *buf, char *dst, size_t max)
{
    size_t length = buffer_read_varint(buf);
    char *storage = &buf->data[buf->pos];

    // Allocate memory if destination is NULL.
    if (dst == NULL)
        dst = malloc(sizeof(char) * (length + 1));

    // Read array.
    for (size_t i = 0; i < length; i++)
        dst[i] = storage[i];

    // Set new position.
    buf->pos += length;
    return (dst);
}

int32_t buffer_read_varint(buffer_t *buf)
{
    int num_read    = 0;
    uint32_t result = 0;
    uint8_t byte;

    do {
        byte    = buffer_read_u8(buf);
        result  |= (byte & 0b01111111) << (7 * num_read++);

        // @TODO : Exceptions
        if (num_read > 5)
            return (-1);

    } while (byte & 0b10000000);

    return (result);
}

int64_t buffer_read_varlong(buffer_t *buf)
{
    int num_read    = 0;
    uint64_t result = 0;
    uint8_t byte;

    do {
        byte = buffer_read_u8(buf);
        result |= (byte & 0b01111111) << (7 * num_read++);

        // @TODO : Exceptions
        if (num_read > 10)
            return (-1);

    } while (byte & 0b10000000);

    return (result);
}


///////////////////////////////////
//  WRITER
///////////////////////////////////


inline void buffer_write_u8(buffer_t *buf, uint8_t i)
{
    buf->data[buf->pos++] = i;
}

inline void buffer_write_u16(buffer_t *buf, uint16_t i)
{
    buffer_write_u8(buf, i >> 8);
    buffer_write_u8(buf, i);
}

inline void buffer_write_u32(buffer_t *buf, uint32_t i)
{
    buffer_write_u16(buf, i >> 16);
    buffer_write_u16(buf, i);
}

inline void buffer_write_u64(buffer_t *buf, uint64_t i)
{
    buffer_write_u32(buf, i >> 32);
    buffer_write_u32(buf, i);
}

void buffer_write_float(buffer_t *buf, float i)
{
    uint32_t bits;

    memcpy(&bits, &i, sizeof(bits));
    buffer_write_u32(buf, bits);
}

void buffer_write_double(buffer_t *buf, double i)
{
    uint64_t bits;

    memcpy(&bits, &i, sizeof(bits));
    buffer_write_u64(buf, bits);
}

inline void buffer_write_string(buffer_t *buf, const char *src, size_t len)
{
    char *storage;

    // Write string length.
    buffer_write_varint(buf, len);

    // Get current buf position.
    storage = &buf->data[buf->pos];

    // Write string.
    for (size_t i = 0; i < len; i++)
        storage[i] = src[i];

    // Set new position.
    buf->pos += len;
}

void buffer_write_array(buffer_t *buf, const char *src, size_t len)
{
    char *storage;

    // Write array length.
    buffer_write_varint(buf, len);

    // Get current buf position.
    storage = &buf->data[buf->pos];

    // Write array.
    for (size_t i = 0; i < len; i++)
        storage[i] = src[i];

    // Set new position.
    buf->pos += len;
}

void buffer_write_varint(buffer_t *buf, uint32_t i)
{
    uint8_t byte;

    do {
        byte = (i & 0b01111111);
        i >>= 7;

        if (i)
            byte |= 0b10000000;

        buffer_write_u8(buf, byte);
    } while (i);
}

void buffer_write_varlong(buffer_t *buf, uint64_t l)
{
    uint8_t byte;

    do {
        byte = (l & 0b01111111);
        l >>= 7;

        if (l)
            byte |= 0b10000000;

        buffer_write_u8(buf, byte);
    } while (l);
}


///////////////////////////////////
//  UTIL
///////////////////////////////////


int buffer_size_varint(int i)
{
    int size = 0;

    do {
        i >>= 7;
        size++;
    } while (i);

    return (size);
}
