#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minecraft.h"
#include "socket.h"
#include "http.h"
#include "miniz.h"


///////////////////////////////////
//  CONFIG
///////////////////////////////////


#define __MC_TOKEN      "YOUR_TOKEN"
#define __MC_UUID       "YOUR_SESSION_ID"


///////////////////////////////////
//  HANDLER
///////////////////////////////////


typedef void (*decoder_t)(connection_t *connection, buffer_t *packet);

void on_packet_decrypt(connection_t *connection, buffer_t *packet, packet_handler_t *next);
void on_packet_decompress(connection_t *connection, buffer_t *packet, packet_handler_t *next);
void on_login_packet_raw(connection_t *connection, buffer_t *packet, packet_handler_t *next);
void on_packet_otoak(connection_t *connection, buffer_t *packet, packet_handler_t *next);

void packet_on_disconnect(connection_t *connection, buffer_t *packet)
{

}

void packet_on_encryption_request(connection_t *connection, buffer_t *packet)
{
    char server_id[20];
    char pub_key[512];
    char verify_token[16];

    // Read packet.
    buffer_read_string(packet , server_id    , 20);
    buffer_read_array(packet  , pub_key      , 512);
    buffer_read_array(packet  , verify_token , 16);

    // Init crypto.
    connection->crypto = malloc(sizeof(crypto_context_t));
    crypto_init(connection->crypto, pub_key, 162);

    // Login to Mojang.
    mojang_auth_login(&(mojang_session_t){
        __MC_TOKEN,
        __MC_UUID,
        connection->crypto->iv,
        server_id,
        strlen(server_id),
        pub_key,
        162
    });

    // Send encryption response.
    packet_send_encryption_response(connection->fd, connection->crypto, verify_token, 4);

    // Set network handler.
    connection->handler.on_read         = on_packet_decrypt;
    connection->handler.next            = calloc(1, sizeof(packet_handler_t));
    connection->handler.next->on_read   = on_login_packet_raw;
}

void on_packet_test(connection_t *connection, buffer_t *packet, packet_handler_t *next)
{
    int id = buffer_read_varint(packet);

    printf("%d\n", id);
}

void packet_on_login_success(connection_t *connection, buffer_t *packet)
{
    char uuid[64];
    char username[32];

    // Read packet.
    buffer_read_string(packet, uuid     , 64);
    buffer_read_string(packet, username , 32);

    // Set packet handler.
    connection->handler.next->next->on_read = on_packet_test;

    printf("ONELA NOUS ONELA !\n");
    printf("%s\n", uuid);
    printf("%s\n", username);
}

void packet_on_set_compression(connection_t *connection, buffer_t *packet)
{
    packet_handler_t *save = connection->handler.next;

    // Read packet.
    connection->threshold = buffer_read_varint(packet);

    // Save current handler and create a new.
    if (connection->threshold >= 0) {
        connection->handler.next            = calloc(1, sizeof(packet_handler_t));
        connection->handler.next->on_read   = on_packet_decompress;
        connection->handler.next->next      = save;

        // test
        connection->handler.next->next->on_read = on_packet_otoak;
    }
}

const decoder_t decoders_login[] = {
    packet_on_disconnect,
    packet_on_encryption_request,
    packet_on_login_success,
    packet_on_set_compression
};

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

    // Decode and handle packet.
    if (id >= 0 && id <= 3)
        decoders_login[id](connection, packet);
}

void on_packet_decompress(connection_t *connection, buffer_t *packet, packet_handler_t *next)
{
    char        buf[8192 * 4] = { 0 };
    buffer_t    decompressed = { buf, 8192 * 4, 0 };
    size_t      clen;
    size_t      ulen;
    size_t      len;

    // Decode all packets.
    while (packet->pos < packet->capacity) {

        // Read packet data length.
        clen = buffer_read_varint(packet);   // Length of Data Length + Compressed data length
        ulen = buffer_read_varint(packet);   // Length of Uncompressed Data

        // Skip decompression if the packet is uncompressed.
        if (ulen == 0) {
            next->on_read(connection, packet, next->next);
            return;
        }

        // Decompress packet.
        uncompress(buf, &decompressed.capacity, &packet->data[packet->pos], clen);

        // Set packet position.
        packet->pos         += (clen - buffer_size_varint(ulen));
        decompressed.pos    = 0;

        // Pass to next handler.
        // next->on_read(connection, &decompressed, next->next);
    }
}

void on_login_packet_raw(connection_t *connection, buffer_t *packet, packet_handler_t *next)
{
    size_t length   = buffer_read_varint(packet);
    int id          = buffer_read_varint(packet);

    // Decode and handle packet.
    if (id >= 0 && id <= 3)
        decoders_login[id](connection, packet);
}


///////////////////////////////////
//  MINECRAFT
///////////////////////////////////


int minecraft_connect(const char *host, uint16_t port)
{
    char            buf[8192];
    buffer_t        packet      = { buf, 8192, 0 };
    connection_t    connection  = { 0 };
    size_t          len;

    // Connect to minecraft server.
    if ((connection.fd = socket_connect(host, port)) == -1)
        return (-1);

    // Set network pipe handler.
    connection.handler.on_read = on_login_packet_raw;

    // Send Handshake packet.
    packet.pos = 4;
    buffer_write_varint(&packet, 0x00);
    buffer_write_varint(&packet, 47);                       // Protocol ID
    buffer_write_string(&packet, host, strlen(host));       // Server
    buffer_write_u16(&packet, port);                        // Port
    buffer_write_u8(&packet, 2);                            // State
    send_packet(connection.fd, &packet);

    // Send Login Start packet.
    packet.pos = 4;
    buffer_write_varint(&packet, 0x00);
    buffer_write_string(&packet, "iRandomXx", 9);
    send_packet(connection.fd, &packet);

    // Read and handle packets.
    while ((len = read(connection.fd, buf, 8192)) > 0) {

        // Reset position and set read capacity.
        packet.pos      = 0;
        packet.capacity = len;

        // Decode and handle packet.
        connection.handler.on_read(&connection, &packet, connection.handler.next);
    }

    // Free crypto context.
    if (connection.crypto) {
        crypto_free(connection.crypto);
        free(connection.crypto);
    }

    // Close connection.
    close(connection.fd);

    return (0);
}


///////////////////////////////////
//  MAIN
///////////////////////////////////


int main(int argc, char **argv)
{
    // Init http and crypto.
    http_setup();
    crypto_setup();

    // Connect to Minecraft server.
    if (minecraft_connect(argv[1], atoi(argv[2])))
        return (1);

    // Destroy http and crypto.
    crypto_destroy();
    http_destroy();

    return (0);
}
