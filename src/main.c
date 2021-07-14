#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minecraft.h"
#include "socket.h"
#include "http.h"


///////////////////////////////////
//  MINECRAFT
///////////////////////////////////


int minecraft_connect(const char *host, uint16_t port)
{
    static char     buf[32768];
    buffer_t        packet      = { buf, 32768, 0 };
    connection_t    connection  = { 0 };
    size_t          len;

    // Connect to minecraft server.
    if ((connection.fd = socket_connect(host, port)) == -1)
        return (-1);

    // Set network pipe handler.
    pipeline_add_after(&connection.pipeline, 0, pipeline_create(0, packet_handler_raw, NULL));

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
    while ((len = read(connection.fd, buf, 32768)) > 0) {

        // Reset position and set read capacity.
        packet.pos      = 0;
        packet.capacity = len;

        // Decode and handle packet.
        connection.pipeline->on_read(&connection, &packet, connection.pipeline->next);
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
