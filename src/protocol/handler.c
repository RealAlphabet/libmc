#include <stdio.h>

#include "minecraft.h"
#include "miniz.h"


///////////////////////////////////
//  CONFIG
///////////////////////////////////


#define __MC_TOKEN      "YOUR_TOKEN"
#define __MC_UUID       "YOUR_UUID"


///////////////////////////////////
//  HANDLER
///////////////////////////////////


void on_packet_test(connection_t *connection, buffer_t *packet, packet_handler_t *next)
{
    int id = buffer_read_varint(packet);

    printf("\tID: %d\n", id);
}

void on_packet_decrypt(connection_t *connection, buffer_t *packet, packet_handler_t *next)
{
    // Decrypt packet.
    crypto_aes_decrypt(connection->crypto, packet->data, packet->data, packet->capacity);

    // Pass to next handler.
    next->on_read(connection, packet, next->next);
}

void on_packet_otoak(connection_t *connection, buffer_t *packet, packet_handler_t *next)
{
    int id = buffer_read_varint(packet);

    printf("OTOAK: %d\n", id);

    // Decode and handle packet.
    decoders_login[id](connection, packet);
}

void on_packet_decompress(connection_t *connection, buffer_t *packet, packet_handler_t *next)
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

    printf("\n\nEZ\n\n");
}

void on_login_packet_raw(connection_t *connection, buffer_t *packet, packet_handler_t *next)
{
    size_t length   = buffer_read_varint(packet);
    int id          = buffer_read_varint(packet);

    // Decode and handle packet.
    if (id >= 0 && id <= 3)
        decoders_login[id](connection, packet);

    printf("RAW: %ld %ld\n", packet->capacity, packet->pos);
}


