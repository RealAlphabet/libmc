#include <stdio.h>

#include "minecraft.h"
#include "miniz.h"


///////////////////////////////////
//  DECODERS
///////////////////////////////////


const decoder_t DECODERS[] = {
    on_packet_disconnect,
    on_packet_encryption_request,
    on_packet_login_success,
    on_packet_set_compression,
    NULL
};


///////////////////////////////////
//  HANDLER
///////////////////////////////////


void handler_decrypt(connection_t *connection, buffer_t *packet, pipeline_entry_t *next)
{
    // Decrypt packet.
    crypto_aes_decrypt(connection->crypto, packet->data, packet->data, packet->capacity);

    // Pass to next handler.
    next->on_read(connection, packet, next->next);
}

void handler_decompress(connection_t *connection, buffer_t *packet, pipeline_entry_t *next)
{
    static char buf[131072] = { 0 };
    buffer_t decompressed   = { buf, 131072, 0 };
    size_t clen;
    size_t ulen;
    size_t len;

    printf("\n\nSTART\n\n");

    // Decode all packets.
    while (packet->pos < packet->capacity) {

        // Read packet data length.
        clen = buffer_read_varint(packet);   // Length of Data Length + Compressed data length
        ulen = buffer_read_varint(packet);   // Length of Uncompressed Data

        // Skip decompression if the packet is uncompressed.
        printf("%ld %ld %ld\n", packet->capacity, clen, ulen);

        if (ulen == 0) {
            next->on_read(connection, packet, next->next);
            return;
        }

        // Set decompress packet.
        decompressed.pos        = 0;
        decompressed.capacity   = 131072;

        // Decompress packet.
        uncompress(buf, &decompressed.capacity, &packet->data[packet->pos], clen);

        // Set packet position.
        packet->pos         += (clen - buffer_size_varint(ulen));

        // Pass to next handler.
        // next->on_read(connection, &decompressed, next->next);
    }
}

void handler_raw(connection_t *connection, buffer_t *packet, pipeline_entry_t *next)
{
    size_t length   = buffer_read_varint(packet);
    int id          = buffer_read_varint(packet);

    printf("%ld %d\n", length, id);

    // Decode and handle packet.
    DECODERS[id](connection, packet);
}
