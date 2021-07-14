#include <stdlib.h>

#include "pipeline.h"


///////////////////////////////////
//  PIPELINE
///////////////////////////////////


pipeline_entry_t *pipeline_create(int id, packet_handler_t on_read, packet_handler_t on_write)
{
    pipeline_entry_t *entry = malloc(sizeof(pipeline_entry_t));

    // Set entry values.
    entry->id       = id;
    entry->on_read  = on_read;
    entry->on_write = on_write;
    entry->next     = NULL;

    return (entry);
}

void pipeline_free(pipeline_entry_t **start)
{
    pipeline_entry_t *actual = *start;
    pipeline_entry_t *tmp;

    // Free all entries.
    while (actual) {
        tmp    = actual->next;
        free(actual);
        actual = tmp;
    }

    // Reset reference.
    *start = NULL;
}


///////////////////////////////////
//  ENTRY
///////////////////////////////////


void pipeline_set(pipeline_entry_t **start, int id, packet_handler_t on_read, packet_handler_t on_write)
{
    pipeline_entry_t *actual = *start;

    while (actual) {

        // Set entry handlers and return.
        if (actual->id == id) {
            actual->on_read  = on_read;
            actual->on_write = on_write;
            return;
        }

        // Skip entry.
        actual = actual->next;
    }
}

void pipeline_add_before(pipeline_entry_t **start, int before, pipeline_entry_t *entry)
{
    pipeline_entry_t *actual = *start;

    // Get before entry.
    while (actual && actual->id != before) {
        start  = &actual->next;
        actual = actual->next;
    }

    // Set next.
    if (actual)
        entry->next = actual;

    // Set entry.
    *start = entry;
}

void pipeline_add_after(pipeline_entry_t **start, int after, pipeline_entry_t *entry)
{
    pipeline_entry_t *actual = *start;

    // Get after entry.
    while (actual && actual->id != after) {
        actual = actual->next;
        start  = &actual->next;
    }

    // Set previous next.
    if (actual)
        entry->next = actual->next;

    // Set entry.
    *start = entry;
}
