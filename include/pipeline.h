#pragma once

#include <stdint.h>

#include "buffer.h"


///////////////////////////////////
//  STRUCTURE
///////////////////////////////////


typedef struct pipeline_entry_t pipeline_entry_t;

typedef void (*packet_handler_t)(void *data, buffer_t *packet, pipeline_entry_t *next);

struct pipeline_entry_t
{
    uint8_t             id;
    packet_handler_t    on_read;
    packet_handler_t    on_write;
    pipeline_entry_t    *next;
};


///////////////////////////////////
//  PIPELINE
///////////////////////////////////


pipeline_entry_t *pipeline_create(int id, packet_handler_t *on_read, packet_handler_t *on_write);
void pipeline_free(pipeline_entry_t **start);


///////////////////////////////////
//  ENTRY
///////////////////////////////////


void pipeline_set(pipeline_entry_t **start, int id, packet_handler_t on_read, packet_handler_t on_write);
void pipeline_add_before(pipeline_entry_t **start, int before, pipeline_entry_t *entry);
void pipeline_add_after(pipeline_entry_t **start, int after, pipeline_entry_t *entry);
