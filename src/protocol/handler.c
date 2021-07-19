#include <stdio.h>

#include "minecraft.h"
#include "miniz.h"


///////////////////////////////////
//  HANDLER
///////////////////////////////////


void handler_decrypt(connection_t *connection, buffer_t *packet, pipeline_entry_t *next)
{
    // Continue decryption of the stream.
    // Previous bytes are already decrypted.
    crypto_aes_decrypt(
        connection->crypto,
        &packet->data[packet->posw],
        &packet->data[packet->posw],
        (packet->size - packet->posw)
    );

    // Pass to the next handler.
    next->on_read(connection, packet, next->next);
}

void handler_decompress(connection_t *connection, buffer_t *packet, pipeline_entry_t *next)
{
    size_t clen;
    size_t ulen;

    // Decode packets.
    while (packet->posr < packet->size) {

        // Save packet position.
        packet->markr = packet->posr;

        // Read compressed and uncompressed data length.
        //
        // Packet Length    : Length of Data Length + Compressed length of (Packet ID + Data).
        // Data Length      : Length of Uncompressed Data (Packet ID + Data) or 0.
        // Data             : Compressed Data (Packet ID + Data).
        //
        clen = buffer_read_varint(packet);
        ulen = buffer_read_varint(packet);
        clen -= buffer_size_varint(ulen);

        if ((packet->posr + clen) > packet->size) {
            // CAS OU LE PACKET MANQUE DES DONNEES.
            exit(84);
            return;
        }

        // Skip decompression if the packet is uncompressed.
        if (ulen == 0) {

            // Pass to next handler.
            handler_ak(connection, packet);

        } else {
            buffer_t pkt = { 0 };

            // Allocate a buffer for the uncompressed packet.
            pkt.data = malloc(sizeof(char) * ulen);
            pkt.size = ulen;

            // Uncompress packet.
            uncompress(pkt.data, &pkt.size, &packet->data[packet->posr], clen);

            // Pass to next handler.
            handler_ak(connection, &pkt);

            // Free allocated data.
            free(pkt.data);
        }
        
        // Set packet position.
        packet->posr = (packet->markr + clen);
    }
}

void handler_raw(connection_t *connection, buffer_t *packet, pipeline_entry_t *next)
{
    size_t len;
    size_t remaining;

    // Decode packets.
    while (packet->posr < packet->size) {

        // Save packet position.
        packet->markr = packet->posr;

        // Read "Packet Length".
        len = buffer_read_varint(packet);

        // Check if the packet is incomplete.
        remaining = (packet->size - packet->posr);

        if (remaining < len) {

            // Include "Packet Length" length.
            remaining += buffer_size_varint(len);

            // Copy remaining bytes to the beginning of the packet.
            memcpy(packet->data, &packet->data[packet->markr], remaining);

            // Reset read position.
            // Set write position to the end of the remaining bytes.
            // Set size to fit remaining bytes.
            packet->posr = 0;
            packet->posw = remaining;
            packet->size = remaining;

            // Return and wait for the next chunk of bytes.
            return;

        } else {
            // Save packet position.
            packet->markr = packet->posr;

            // Decode and handle packet.
            next->on_read(connection, packet, next->next);

            // Restore packet position.
            packet->posr = (packet->markr + len);
        }
    }
}
